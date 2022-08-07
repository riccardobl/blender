/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2021 Blender Foundation.
 */

/** \file
 * \ingroup eevee
 *
 * Cryptomatte.
 *
 * Cryptomatte stores the output during rendering in a single texture.
 * Inside the film the output is extracted per enabled cryptomatte layer.
 * Each cryptomatte layer can hold N samples. These are stored in multiple
 * sequentially bound textures. The samples are sorted and merged.
 */

#pragma once

#include "eevee_shader_shared.hh"

namespace blender::eevee {

class Instance;

/* -------------------------------------------------------------------- */
/** \name Cryptomatte
 * \{ */

class Cryptomatte {
 private:
  class Instance &inst_;

 public:
  DepthOfField(Instance &inst) : inst_(inst){};
  ~DepthOfField(){};

  void init();
};

/** \} */

}  // namespace blender::eevee
