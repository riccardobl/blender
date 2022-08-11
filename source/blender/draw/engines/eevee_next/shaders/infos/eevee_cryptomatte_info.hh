#include "gpu_shader_create_info.hh"

GPU_SHADER_CREATE_INFO(eevee_cryptomatte)
    .fragment_source("eevee_cryptomatte_frag.glsl")
    .push_constant(Type::VEC4, "cryptomatte_hash")
    .fragment_out(0, Type::VEC4, "fragColor")
    .additional_info("eevee_surf_forward");

GPU_SHADER_CREATE_INFO(eevee_cryptomatte_curves)
    .do_static_compilation(true)
    .additional_info("eevee_geom_curves", "eevee_cryptomatte");
GPU_SHADER_CREATE_INFO(eevee_cryptomatte_mesh)
    .do_static_compilation(true)
    .additional_info("eevee_geom_mesh", "eevee_cryptomatte");
