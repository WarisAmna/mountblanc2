__kernel void naive_kernel(
        __global const float* restrict in_image,
        __global float* restrict out_image,
        const int size
)
{
    // global 2D NDRange sizes (size of the image)
    int num_cols = get_global_size(0);
    int num_rows = get_global_size(1);
    // point in currently being executed (each pixel)
    int senterX = get_global_id(0);
    int senterY = get_global_id(1);

    // For each pixel we compute a box blur
    float3 sum = (float3) (0.0f, 0.0f, 0.0f);
    int countIncluded = 0;

    for(int y = -size; y <= size; y++) {
        int currentX = senterX;
        int currentY = senterY + y;

        // Check if we are outside the bounds
        if(currentX < 0)
            continue;
        if(currentX >= num_cols)
            continue;
        if(currentY < 0)
            continue;
        if(currentY >= num_rows)
            continue;

        // Now we can begin
        int offsetOfThePixel = (num_cols * currentY + currentX);
        float3 tmp = vload3(offsetOfThePixel, in_image);
        sum += tmp;
        // Keep track of how many values we have included
        countIncluded++;
    }

    // Now we compute the final value
    float3 value = sum / countIncluded;

    // Update the output image
    int offsetOfThePixel = (num_cols * senterY + senterX);
    vstore3(value, offsetOfThePixel, out_image);
/*
    sum = (float3) (0.0f, 0.0f, 0.0f);
    countIncluded = 0;


        if (senterX-(size+1)<0){
            sum = sum * 0;
            countIncluded = 0;

            for(int x = -size; x <= size; x++) {
                int currentX = senterX + x;
                int currentY = senterY;

                if(currentX < 0)
                    continue;

                int offsetOfThePixel = (num_cols * currentY + currentX);
                float3 tmp = vload3(offsetOfThePixel, out_image);
                sum += tmp;
                countIncluded++;
            }
        }

        else if (senterX+size>=num_cols){

                if(senterX < num_cols){
                    int offsetOfThePixel = (num_cols * senterY + senterX-(size+1));
                    float3 tmp = vload3(offsetOfThePixel, out_image);
                    sum -= tmp;
                    countIncluded--;
                }
                    //continue;
                
            
        }
        else{
        
            int offsetOfThePixel_remove = (num_cols * senterY + senterX-(size+1));
            int offsetOfThePixel_add = (num_cols * senterY + senterX+(size));
            float3 tmp1 = vload3(offsetOfThePixel_remove, out_image);
            float3 tmp2 = vload3(offsetOfThePixel_add, out_image);
            sum = sum - tmp1 + tmp2;
        }

        // Now we compute the final value
        value = sum / countIncluded;
        //printf("hello hello");
        // Update the output image
        offsetOfThePixel = (num_cols * senterY + senterX);
        vstore3(value, offsetOfThePixel, out_image);
    */



    //in the x direction
    sum = sum * 0;
    countIncluded = 0;
    for(int x = -size; x <= size; x++) {
        int currentX = senterX + x;
        int currentY = senterY;

        // Check if we are outside the bounds
        if(currentX < 0)
            continue;
        if(currentX >= num_cols)
            continue;
        if(currentY < 0)
            continue;
        if(currentY >= num_rows)
            continue;

        // Now we can begin
        int offsetOfThePixel = (num_cols * currentY + currentX);
        float3 tmp = vload3(offsetOfThePixel, out_image);
        sum += tmp;
        // Keep track of how many values we have included
        countIncluded++;
    }

    // Now we compute the final value
    value = sum / countIncluded;

    // Update the output image
    offsetOfThePixel = (num_cols * senterY + senterX);
    vstore3(value, offsetOfThePixel, out_image);
}
