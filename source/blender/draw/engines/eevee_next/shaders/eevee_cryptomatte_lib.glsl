/** Storing/merging and sorting cryptomatte samples. */

void film_store_cryptomatte_sample(FilmSample dst, int cryptomatte_layer_id, float hash){
    float weight = dst.weight;
    /*
        first need to detect the operation.
        - when hash exists it should be updated and can optionally be reinserted into a new position.
        - when hash doesn't exist we should find an insertion point. only samples to a null sample (hash 0, weight 0) should be moved. When no null sample exist it will remove the lowest weight.

        Second option would be to find the place to fit the sample. Doing the sorting in a separate shader
        pro is that the performance when adding samples. Sorting only happens once during
        final rendering. When using the viewport compositor this shader could be called
        as a post process for active layers. 

        perhaps in the viewport the first option would fit better. The second option
        is better for final rendering, but at that time performance is secondary.
        Technically the order of the samples don't matter that much, But it depends on how many
        cryptomatte nodes are used to make sorting more efficient.
        Would need some feedback from Beau/Andy on this subject.
          
    */
    int operation = 0;

    
}
