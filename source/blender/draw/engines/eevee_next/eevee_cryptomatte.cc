#include "BKE_cryptomatte.h"

#include "GPU_material.h"

#include "eevee_cryptomatte.hh"
#include "eevee_instance.hh"
#include "eevee_renderbuffers.hh"

namespace blender::eevee {

void Cryptomatte::begin_sync()
{
  const eViewLayerEEVEEPassType enabled_passes = inst_.film.enabled_passes_get();
  if (!(enabled_passes &
        (EEVEE_RENDER_PASS_CRYPTOMATTE_OBJECT | EEVEE_RENDER_PASS_CRYPTOMATTE_ASSET))) {
    cryptomatte_object_buf.resize(16);
  }
}

void Cryptomatte::sync_object(Object *ob)
{
  const eViewLayerEEVEEPassType enabled_passes = inst_.film.enabled_passes_get();
  if (!(enabled_passes &
        (EEVEE_RENDER_PASS_CRYPTOMATTE_OBJECT | EEVEE_RENDER_PASS_CRYPTOMATTE_ASSET))) {
    return;
  }

  uint32_t resource_id = DRW_object_resource_id_get(ob);
  float2 object_hashes(0.0f, 0.0f);

  if (enabled_passes & EEVEE_RENDER_PASS_CRYPTOMATTE_OBJECT) {
    object_hashes[0] = hash(ob->id);
  }

  if (enabled_passes & EEVEE_RENDER_PASS_CRYPTOMATTE_ASSET) {
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

}  // namespace blender::eevee