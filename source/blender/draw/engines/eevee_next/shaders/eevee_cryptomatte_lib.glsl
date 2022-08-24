/** Storing/merging and sorting cryptomatte samples. */

bool can_merge_cryptomatte_sample(vec2 cryptomatte_sample, float hash)
{
  if (cryptomatte_sample == vec2(0.0, 0.0)) {
    return true;
  }
  if (cryptomatte_sample.x == hash) {
    return true;
  }
  return false;
}

vec2 merge_cryptomatte_sample(vec2 cryptomatte_sample, float hash, float weight)
{
  return vec2(hash, cryptomatte_sample.y + weight);
}

vec4 cryptomatte_false_color(float hash)
{
#define UINT32_MAX (4294967295U)

  uint m3hash = floatBitsToUint(hash);
  return vec4(
      hash, float(m3hash << 8) / float(UINT32_MAX), float(m3hash << 16) / float(UINT32_MAX), 1.0);
}

void film_store_cryptomatte_sample(FilmSample dst,
                                   int cryptomatte_layer_id,
                                   float hash,
                                   out vec4 out_color)
{
  float weight = dst.weight;

  for (int i = 0; i < film_buf.cryptomatte_samples_len / 2; i++) {
    ivec3 img_co = ivec3(dst.texel, i);
    vec4 sample_pair = imageLoad(cryptomatte_img, img_co);
    if (can_merge_cryptomatte_sample(sample_pair.xy, hash)) {
      sample_pair.xy = merge_cryptomatte_sample(sample_pair.xy, hash, weight);
      if (i == 0) {
        out_color = cryptomatte_false_color(sample_pair.x);
      }
    }
    else if (can_merge_cryptomatte_sample(sample_pair.zw, hash)) {
      sample_pair.zw = merge_cryptomatte_sample(sample_pair.zw, hash, weight);
    }
    else if (i == film_buf.cryptomatte_samples_len / 2 - 1) {
      // TODO(jbakker): New hash detected, but there is no space left to store it. Currently we
      // will ignore this sample, but ideally we could replace a sample with a lowest weight.
      continue;
    }
    else {
      continue;
    }
    imageStore(cryptomatte_img, img_co, sample_pair);
    if (i == 0) {
    }
    break;
  }
}
