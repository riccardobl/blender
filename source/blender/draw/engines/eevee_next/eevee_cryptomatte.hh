/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2021 Blender Foundation.
 */

/** \file
 * \ingroup eevee
 *
 * Cryptomatte.
 *
 * During rasterization, cryptomatte hashes are stored into a single array texture.
 * The film pass then resamples this texture using pixel filter weighting.
 * Each cryptomatte layer can hold N samples. These are stored in sequential layers
 * of the array texture. The samples are sorted and merged only for final rendering.
 */

#pragma once

#include "eevee_shader_shared.hh"

extern "C" {
struct Material;
}

namespace blender::eevee {

class Instance;

/* -------------------------------------------------------------------- */
/** \name Cryptomatte
 * \{ */

class Cryptomatte {
 private:
  class Instance &inst_;

  /** Contains per object hashes (object and asset hash). Indexed by resource ID. */
  CryptomatteObjectBuf cryptomatte_object_buf;

 public:
  Cryptomatte(Instance &inst) : inst_(inst){};
  ~Cryptomatte(){};

  void begin_sync();
  void sync_object(Object *ob);
  void end_sync();

  void bind_resources(DRWShadingGroup *grp);

  float hash(const ID &id) const;
};

/** \} */

}  // namespace blender::eevee
