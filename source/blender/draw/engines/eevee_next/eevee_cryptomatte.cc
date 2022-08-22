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

float Cryptomatte::hash(const ID &id) const
{
  const char *name = &id.name[2];
  const int name_len = BLI_strnlen(name, MAX_NAME - 2);
  uint32_t cryptomatte_hash = BKE_cryptomatte_hash(name, name_len);
  return BKE_cryptomatte_hash_to_float(cryptomatte_hash);
}

}  // namespace blender::eevee