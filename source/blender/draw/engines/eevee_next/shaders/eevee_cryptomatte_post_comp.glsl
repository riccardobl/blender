#define CRYPTOMATTE_LEVELS_MAX 16

void cryptomatte_sort_layer(ivec2 texel, int layer)
{
  int pass_len = (cryptomatte_levels + 1) / 2;
  int layer_id = layer * pass_len;

  vec2 samples[CRYPTOMATTE_LEVELS_MAX];
  /* Read all samples from the cryptomatte layer. */
  for (int p = 0; p < pass_len; p++) {
    vec4 pass_sample = imageLoad(cryptomatte_img, ivec3(texel, p + layer_id));
    samples[p * 2] = pass_sample.xy;
    samples[p * 2 + 1] = pass_sample.zw;
  }
  for (int i = pass_len * 2; i < CRYPTOMATTE_LEVELS_MAX; i++) {
    samples[i] = vec2(0.0);
  }

  /* Sort samples. Lame implementation, can be replaced with a more efficient algorithm. */
  bool samples_changed = false;
  for (int i = 0; i < cryptomatte_levels - 1 && samples[i].y != 0.0; i++) {
    int highest_index = i;
    float highest_weight = samples[i].y;
    for (int j = i + 1; j < cryptomatte_levels && samples[j].y != 0.0; j++) {
      if (samples[j].y > highest_weight) {
        highest_index = j;
        highest_weight = samples[j].y;
      }
    };

    if (highest_index != i) {
      vec2 tmp = samples[i];
      samples[i] = samples[highest_index];
      samples[highest_index] = tmp;
      samples_changed = true;
    }
  }

  /* Store samples back to the cryptomatte layer. */
  if (samples_changed) {
    for (int p = 0; p < pass_len; p++) {
      vec4 pass_sample;
      pass_sample.xy = samples[p * 2];
      pass_sample.zw = samples[p * 2 + 1];
      imageStore(cryptomatte_img, ivec3(texel, p + layer_id), pass_sample);
    }
  }
}

void main()
{
  ivec2 texel = ivec2(gl_GlobalInvocationID.xy);
  for (int layer = 0; layer < cryptomatte_layer_len; layer++) {
    cryptomatte_sort_layer(texel, layer);
  }
}
