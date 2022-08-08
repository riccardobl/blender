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
  GPUShader *sh_curves = inst_.shaders.static_shader_get(CRYPTOMATTE_CURVES);
  hair_grp_ = DRW_shgroup_create(sh_curves, cryptomatte_ps_);
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