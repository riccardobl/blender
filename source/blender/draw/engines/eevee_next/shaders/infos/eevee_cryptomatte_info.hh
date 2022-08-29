/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "eevee_defines.hh"
#include "gpu_shader_create_info.hh"

GPU_SHADER_CREATE_INFO(eevee_cryptomatte_post)
    .do_static_compilation(true)
    .image(0, GPU_RGBA32F, Qualifier::READ_WRITE, ImageType::FLOAT_2D_ARRAY, "cryptomatte_img")
    .push_constant(Type::INT, "cryptomatte_layer_len")
    .push_constant(Type::INT, "cryptomatte_samples_per_layer")
    .local_group_size(FILM_GROUP_SIZE, FILM_GROUP_SIZE)
    .compute_source("eevee_cryptomatte_post_comp.glsl");