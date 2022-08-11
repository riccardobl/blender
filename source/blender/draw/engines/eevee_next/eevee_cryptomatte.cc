#include "BKE_cryptomatte.h"

#include "GPU_material.h"

#include "eevee_cryptomatte.hh"
#include "eevee_instance.hh"
#include "eevee_renderbuffers.hh"

namespace blender::eevee {

void Cryptomatte::init()
{
  eViewLayerEEVEEPassType enabled_passes = inst_.film.enabled_passes_get();
  layer_len_ = 0;
  object_offset_ = (enabled_passes & EEVEE_RENDER_PASS_CRYPTOMATTE_OBJECT) ? layer_len_++ : -1;
  asset_offset_ = (enabled_passes & EEVEE_RENDER_PASS_CRYPTOMATTE_ASSET) ? layer_len_++ : -1;
  material_offset_ = (enabled_passes & EEVEE_RENDER_PASS_CRYPTOMATTE_MATERIAL) ? layer_len_++ : -1;

  BLI_assert_msg(layer_len_ == inst_.film.cryptomatte_layer_len_get(),
                 "Cryptomatte and film mismatch");
}

void Cryptomatte::sync()
{
  if (layer_len_ == 0) {
    return;
  }

  cryptomatte_ps_ = DRW_pass_create("Cryptomatte", DRW_STATE_WRITE_COLOR | DRW_STATE_DEPTH_EQUAL);
  GPUShader *sh_mesh = inst_.shaders.static_shader_get(CRYPTOMATTE_MESH);
  mesh_grp_ = DRW_shgroup_create(sh_mesh, cryptomatte_ps_);
  //GPUShader *sh_curves = inst_.shaders.static_shader_get(CRYPTOMATTE_CURVES);
  //hair_grp_ = DRW_shgroup_create(sh_curves, cryptomatte_ps_);
}

static float hash_id(const ID *id)
{
  const char *name = &id->name[2];
  const int name_len = BLI_strnlen(name, MAX_NAME - 2);
  uint32_t cryptomatte_hash = BKE_cryptomatte_hash(name, name_len);
  return BKE_cryptomatte_hash_to_float(cryptomatte_hash);
}

void Cryptomatte::add_hash(const Object *object, float4 &r_hash) const
{
  if (object_offset_ != -1) {
    r_hash[object_offset_] = hash_id(&object->id);
  }

  if (asset_offset_ != -1) {
    const Object *asset_object = object;
    while (asset_object->parent != nullptr) {
      asset_object = asset_object->parent;
    }
    r_hash[asset_offset_] = hash_id(&asset_object->id);
  }
}

void Cryptomatte::add_hash(const ::Material *mat, float4 &r_hash) const
{
  if (material_offset_ != -1) {
    r_hash[material_offset_] = mat ? hash_id(&mat->id) : 0.0f;
  }
}

void Cryptomatte::sync_mesh(Object *ob)
{
  if (layer_len_ == 0) {
    return;
  }

  float4 hash(0.0f, 0.0f, 0.0f, 0.0f);
  add_hash(ob, hash);

  // TODO(jbakker): decide based on availability of surface geom if we should have an optimzied
  // path, or that it has more overhead (memory)
  if (material_offset_ == -1) {
    GPUBatch *geom = DRW_cache_object_surface_get(ob);
    if (geom) {
      DRWShadingGroup *grp = DRW_shgroup_create_sub(mesh_grp_);
      DRW_shgroup_uniform_vec4_copy(grp, "cryptomatte_hash", hash);
      DRW_shgroup_call(grp, geom, ob);
    }
  }
  else {
    bool has_motion = false;
    // TODO:  inst_.velocity.step_object_sync(ob, ob_handle.object_key,
    // ob_handle.recalc);

    MaterialArray &material_array = inst_.materials.material_array_get(ob, has_motion);

    GPUBatch **mat_geom = DRW_cache_object_surface_material_get(
        ob, material_array.gpu_materials.data(), material_array.gpu_materials.size());

    if (mat_geom == nullptr) {
      return;
    }

    for (auto i : material_array.gpu_materials.index_range()) {
      GPUBatch *geom = mat_geom[i];
      if (geom == nullptr) {
        continue;
      }
      const ::Material *material = GPU_material_get_material(material_array.gpu_materials[i]);
      add_hash(material, hash);
      DRWShadingGroup *grp = DRW_shgroup_create_sub(mesh_grp_);
      DRW_shgroup_uniform_vec4_copy(grp, "cryptomatte_hash", hash);
      DRW_shgroup_call(grp, geom, ob);
    }
  }
}

void Cryptomatte::sync_curves(Object *ob, ModifierData *modifier_data)
{
  if (layer_len_ == 0) {
    return;
  }
  BLI_assert_msg(false, "Not implemented yet");
}

void Cryptomatte::render()
{
  if (layer_len_ == 0) {
    return;
  }
  const RenderBuffers &rbufs = inst_.render_buffers;
  cryptomatte_fb_.ensure(GPU_ATTACHMENT_TEXTURE(rbufs.depth_tx),
                         GPU_ATTACHMENT_TEXTURE(rbufs.combined_tx));
  GPU_framebuffer_bind(cryptomatte_fb_);
  DRW_draw_pass(cryptomatte_ps_);
}

}  // namespace blender::eevee