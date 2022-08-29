/** Storing/merging and sorting cryptomatte samples. */

bool cryptomatte_can_merge_sample(vec2 cryptomatte_sample, float hash)
{
  if (cryptomatte_sample == vec2(0.0, 0.0)) {
    return true;
  }
  if (cryptomatte_sample.x == hash) {
    return true;
  }
  return false;
}

vec2 cryptomatte_merge_sample(vec2 cryptomatte_sample, float hash, float weight)
{
  return vec2(hash, cryptomatte_sample.y + weight);
}

vec4 cryptomatte_false_color(float hash)
{
  uint m3hash = floatBitsToUint(hash);
  return vec4(hash,
              float(m3hash << 8) / float(0xFFFFFFFFu),
              float(m3hash << 16) / float(0xFFFFFFFFu),
              1.0);
}

void cryptomatte_store_film_sample(FilmSample dst,
                                   int cryptomatte_layer_id,
                                   float hash,
                                   out vec4 out_color)
{
  float weight = dst.weight;

  if (!film_buf.use_history || film_buf.use_reprojection) {
    for (int i = 0; i < film_buf.cryptomatte_samples_len / 2; i++) {
      ivec3 img_co = ivec3(dst.texel, cryptomatte_layer_id + i);
      imageStore(cryptomatte_img, img_co, vec4(0.0));
    }
  }

  for (int i = 0; i < film_buf.cryptomatte_samples_len / 2; i++) {
    ivec3 img_co = ivec3(dst.texel, cryptomatte_layer_id + i);
    vec4 sample_pair = imageLoad(cryptomatte_img, img_co);
    if (cryptomatte_can_merge_sample(sample_pair.xy, hash)) {
      sample_pair.xy = cryptomatte_merge_sample(sample_pair.xy, hash, weight);
      if (i == 0) {
        out_color = cryptomatte_false_color(sample_pair.x);
      }
    }
    else if (cryptomatte_can_merge_sample(sample_pair.zw, hash)) {
      sample_pair.zw = cryptomatte_merge_sample(sample_pair.zw, hash, weight);
    }
    else if (i == film_buf.cryptomatte_samples_len / 2 - 1) {
      /* TODO(jbakker): New hash detected, but there is no space left to store it. Currently we
       * will ignore this sample, but ideally we could replace a sample with a lowest weight. */
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
