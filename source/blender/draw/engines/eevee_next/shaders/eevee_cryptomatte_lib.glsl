/** Storing/merging and sorting cryptomatte samples. */

void film_store_cryptomatte_sample(FilmSample dst, int cryptomatte_layer_id, float hash){
    float weight = dst.weight;
    /*
        first need to detect the operation.
        - when hash exists it should be updated and can optionally be moved to a new position.
        - when hash doesn't exist we should find an insertion point. only samples to a null sample (hash 0, weight 0) should be moved. When no null sample exist it will remove the lowest weight.
          
    */
    int operation = 0;

    
}
