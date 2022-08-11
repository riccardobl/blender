/** Storing/merging and sorting cryptomatte samples. */

bool can_merge_cryptomatte_sample(vec2 cryptomatte_sample, float hash) {
    if (cryptomatte_sample == vec2(0.0, 0.0)) {
        return true;
    }
    if (cryptomatte_sample.x == hash) {
        return true;
    }
}

vec2 merge_cryptomatte_sample(vec2 cryptomatte_sample, float hash, float weight) {
    return vec2(hash, cryptomatte_sample.y + weight);
}

void film_store_cryptomatte_sample(FilmSample dst, int cryptomatte_layer_id, float hash){
    float weight = dst.weight;

    for (int i = 0; i < film_buf.cryptomatte_samples_len/2; i ++) {
        ivec3 img_co = ivec3(dst.texel, i);
        vec4 sample_pair = imageLoad(cryptomatte_img, img_co);
        if (can_merge_cryptomatte_sample(sample_pair.xy, hash)) {
            sample_pair.xy = merge_cryptomatte_sample(sample_pair.xy, hash, weight);
        }
        else if (can_merge_cryptomatte_sample(sample_pair.zw, hash)) {
            sample_pair.zw = merge_cryptomatte_sample(sample_pair.zw, hash, weight);
        }
        else if (i == film_buf.cryptomatte_samples_len / 2 -1) {
            // TODO: new hash, no space, we should compare/overwrite lowest sample.
            continue;
        }
        else {
            continue;
        }
        imageStore(cryptomatte_img, img_co, sample_pair);
        break;
    }   
}
