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

void Cryptomatte::begin_sync()
{
  post_ps_ = nullptr;

  if (object_offset_ == -1 && asset_offset_ == -1) {
    cryptomatte_object_buf.resize(16);
  }

  if (layer_len_ == 0) {
    return;
  }

  const bool do_sorting = inst_.is_viewport() == false;
  if (!do_sorting) {
    return;
  }

  post_ps_ = DRW_pass_create("Cryptomatte.Sort", DRW_STATE_NO_DRAW);
  GPUShader *sh = inst_.shaders.static_shader_get(CRYPTOMATTE_POST);
  DRWShadingGroup *grp = DRW_shgroup_create(sh, post_ps_);
  Texture &cryptomatte_tx = inst_.film.cryptomatte_tx_get();
  DRW_shgroup_uniform_image_ref(grp, "cryptomatte_img", &cryptomatte_tx);
  DRW_shgroup_uniform_int_copy(grp, "cryptomatte_layer_len", layer_len_);
  DRW_shgroup_uniform_int_copy(grp, "cryptomatte_levels", inst_.view_layer->cryptomatte_levels);
  int3 dispatch_size = math::divide_ceil(cryptomatte_tx.size(), int3(FILM_GROUP_SIZE));
  DRW_shgroup_call_compute(grp, UNPACK2(dispatch_size), 1);
}

void Cryptomatte::sync_object(Object *ob)
{
  if (object_offset_ == -1 && asset_offset_ == -1) {
    return;
  }

  uint32_t resource_id = DRW_object_resource_id_get(ob);
  float2 object_hashes(0.0f, 0.0f);

  if (object_offset_ != -1) {
    object_hashes[0] = hash(ob->id);
  }

  if (asset_offset_ != -1) {
    Object *asset = ob;
    while (asset->parent) {
      asset = asset->parent;
    }
    object_hashes[1] = hash(asset->id);
  }

  cryptomatte_object_buf.get_or_resize(resource_id) = object_hashes;
}

void Cryptomatte::end_sync()
{
  cryptomatte_object_buf.push_update();
}

float Cryptomatte::hash(const ID &id) const
{
  const char *name = &id.name[2];
  const int name_len = BLI_strnlen(name, MAX_NAME - 2);
  uint32_t cryptomatte_hash = BKE_cryptomatte_hash(name, name_len);
  return BKE_cryptomatte_hash_to_float(cryptomatte_hash);
}

void Cryptomatte::bind_resources(DRWShadingGroup *grp)
{
  DRW_shgroup_storage_block_ref(grp, "cryptomatte_object_buf", &cryptomatte_object_buf);
}

void Cryptomatte::sort()
{
  if (post_ps_) {
    DRW_draw_pass(post_ps_);
  }
}

}  // namespace blender::eevee