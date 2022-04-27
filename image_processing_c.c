#include <math.h>
#include <string.h>
#include <stdlib.h>

#include <omp.h>

#include "ppm.h"

// Image from:
// http://7-themes.com/6971875-funny-flowers-pictures.html

typedef struct {
     double red,green,blue;
} AccuratePixel;

typedef struct {
     int x, y;
     AccuratePixel *data;
} AccurateImage;

void omp_set_num_threads(int num_threads);


// Convert ppm to high precision format.
AccurateImage *convertToAccurateImage(PPMImage *image) {
	// Make a copy
	AccurateImage *imageAccurate;
	imageAccurate = (AccurateImage *)malloc(sizeof(AccurateImage));
	imageAccurate->data = (AccuratePixel*)malloc(image->x * image->y * sizeof(AccuratePixel));

	
	
	for(int i = 0; i < image->x * image->y; i++) {
		imageAccurate->data[i].red   = (double) image->data[i].red;
		imageAccurate->data[i].green = (double) image->data[i].green;
		imageAccurate->data[i].blue  = (double) image->data[i].blue;
	}
	imageAccurate->x = image->x;
	imageAccurate->y = image->y;
	
	return imageAccurate;
}

// CHANGED TO ACCUMULATION ALGORITHM IN THE X DIRECTION
void blurIteration(AccurateImage *imageOut, AccurateImage *imageIn, AccurateImage *temp, int size) {
	
	// Iterate over each pixel
	
	omp_set_num_threads(4);
	#pragma omp parallel for schedule (dynamic, 4)

	for(int senterY = 0; senterY < imageIn->y; senterY++) {
	
		for(int senterX = 0; senterX < imageIn->x; senterX++) {


			// For each pixel we compute the magic number
			double sum1= 0;
			double sum2= 0;
			double sum3= 0;
			int countIncluded = 0;

			for(int y = -size; y <= size; y++) {
				int currentX = senterX ;
				int currentY = senterY + y;

				if(currentY < 0)
					continue;
				if(currentY >= imageIn->y)
					continue;

		
				// Now we can begin
				int numberOfValuesInEachRow = imageIn->x;
				int offsetOfThePixel = (numberOfValuesInEachRow * currentY + currentX);
				sum1 += imageIn->data[offsetOfThePixel].red;
				sum2 += imageIn->data[offsetOfThePixel].green;
				sum3 += imageIn->data[offsetOfThePixel].blue;

				// Keep track of how many values we have included
				countIncluded++;
			}

			// Now we compute the final value
			double value1 = sum1 / countIncluded;
			double value2 = sum2 / countIncluded;
			double value3 = sum3 / countIncluded;


			// Update the output image
			int numberOfValuesInEachRow = imageOut->x; // R, G and B
			int offsetOfThePixel = (numberOfValuesInEachRow * senterY + senterX);
			temp->data[offsetOfThePixel].red = value1;
			temp->data[offsetOfThePixel].green = value2;
			temp->data[offsetOfThePixel].blue = value3;
		}

	}

		double sum1= 0;
		double sum2= 0;
		double sum3= 0;
		int countIncluded = 0;
		int numberOfValuesInEachRow = imageIn->x;

	omp_set_num_threads(4);
	#pragma omp parallel for schedule (dynamic, 4)

	for(int senterY = 0; senterY < temp->y; senterY++) {
		for(int senterX = 0; senterX < temp->x; senterX++) {

			if (senterX-(size+1)<0){
				sum1= 0;
				sum2= 0;
				sum3= 0;
				countIncluded = 0;
	
				for(int x = -size; x <= size; x++) {
					int currentX = senterX + x;
					int currentY = senterY;

					if(currentX < 0)
						continue;

					int offsetOfThePixel = (numberOfValuesInEachRow * currentY + currentX);
					sum1 += temp->data[offsetOfThePixel].red;
					sum2 += temp->data[offsetOfThePixel].green;
					sum3 += temp->data[offsetOfThePixel].blue;

					countIncluded++;

				}
			}

			else if (senterX+size>=temp->x){

					if(senterX >= temp->x)
						continue;
					
				int offsetOfThePixel = (numberOfValuesInEachRow * senterY + senterX-(size+1));
				sum1 = sum1 - temp->data[offsetOfThePixel].red;
				sum2 = sum2 - temp->data[offsetOfThePixel].green;
				sum3 = sum3 - temp->data[offsetOfThePixel].blue;
				countIncluded--;
			}
			else{
			
				int offsetOfThePixel_remove = (numberOfValuesInEachRow * senterY + senterX-(size+1));
				int offsetOfThePixel_add = (numberOfValuesInEachRow * senterY + senterX+(size));
				sum1 = sum1 - temp->data[offsetOfThePixel_remove].red + temp->data[offsetOfThePixel_add].red ;
				sum2 = sum2 - temp->data[offsetOfThePixel_remove].green + temp->data[offsetOfThePixel_add].green ;
				sum3 = sum3 - temp->data[offsetOfThePixel_remove].blue + temp->data[offsetOfThePixel_add].blue ;
			}

			double value1 = sum1 / countIncluded;
			double value2 = sum2 / countIncluded;
			double value3 = sum3 / countIncluded;


			int offsetOfThePixel = (numberOfValuesInEachRow * senterY + senterX);
			imageOut->data[offsetOfThePixel].red = value1;
			imageOut->data[offsetOfThePixel].green = value2;
			imageOut->data[offsetOfThePixel].blue = value3;
		}

	}
}


// Perform the final step, and return it as ppm.
PPMImage * imageDifference(AccurateImage *imageInSmall, AccurateImage *imageInLarge) {
	PPMImage *imageOut;
	imageOut = (PPMImage *)malloc(sizeof(PPMImage));
	imageOut->data = (PPMPixel*)malloc(imageInSmall->x * imageInSmall->y * sizeof(PPMPixel));
	
	imageOut->x = imageInSmall->x;
	imageOut->y = imageInSmall->y;

	for(int i = 0; i < imageInSmall->x * imageInSmall->y; i++) {
		double value = (imageInLarge->data[i].red - imageInSmall->data[i].red);
		if(value > 255)
			imageOut->data[i].red = 255;
		else if (value < -1.0) {
			value = 257.0+value;
			if(value > 255)
				imageOut->data[i].red = 255;
			else
				imageOut->data[i].red = floor(value);
		} else if (value > -1.0 && value < 0.0) {
			imageOut->data[i].red = 0;
		} else {
			imageOut->data[i].red = floor(value);
		}

		value = (imageInLarge->data[i].green - imageInSmall->data[i].green);
		if(value > 255)
			imageOut->data[i].green = 255;
		else if (value < -1.0) {
			value = 257.0+value;
			if(value > 255)
				imageOut->data[i].green = 255;
			else
				imageOut->data[i].green = floor(value);
		} else if (value > -1.0 && value < 0.0) {
			imageOut->data[i].green = 0;
		} else {
			imageOut->data[i].green = floor(value);
		}

		value = (imageInLarge->data[i].blue - imageInSmall->data[i].blue);
		if(value > 255)
			imageOut->data[i].blue = 255;
		else if (value < -1.0) {
			value = 257.0+value;
			if(value > 255)
				imageOut->data[i].blue = 255;
			else
				imageOut->data[i].blue = floor(value);
		} else if (value > -1.0 && value < 0.0) {
			imageOut->data[i].blue = 0;
		} else {
			imageOut->data[i].blue = floor(value);
		}
	}
	return imageOut;
}


int main(int argc, char** argv) {
    // read image
    PPMImage *image;
    // select where to read the image from
    if(argc > 1) {
        // from file for debugging (with argument)
        image = readPPM("flower.ppm");
    } else {
        // from stdin for cmb
        image = readStreamPPM(stdin);
    }
	
	
	AccurateImage *imageAccurate1_tiny = convertToAccurateImage(image);
	AccurateImage *imageAccurate2_tiny = convertToAccurateImage(image);
	AccurateImage *temp = convertToAccurateImage(image);

	// Process the tiny case:
	int size = 2;
	
	blurIteration(imageAccurate2_tiny, imageAccurate1_tiny, temp, size);
	blurIteration(imageAccurate1_tiny, imageAccurate2_tiny, temp, size);
	blurIteration(imageAccurate2_tiny, imageAccurate1_tiny, temp, size);
	blurIteration(imageAccurate1_tiny, imageAccurate2_tiny, temp, size);
	blurIteration(imageAccurate2_tiny, imageAccurate1_tiny, temp, size);
		
	AccurateImage *imageAccurate1_small = convertToAccurateImage(image);
	AccurateImage *imageAccurate2_small = convertToAccurateImage(image);
	
	// Process the small case:
		size = 3;
	blurIteration(imageAccurate2_small, imageAccurate1_small, temp, size);
	blurIteration(imageAccurate1_small, imageAccurate2_small, temp, size);
	blurIteration(imageAccurate2_small, imageAccurate1_small, temp, size);
	blurIteration(imageAccurate1_small, imageAccurate2_small, temp, size);
	blurIteration(imageAccurate2_small, imageAccurate1_small, temp, size);

    // an intermediate step can be saved for debugging like this
	// writePPM("imageAccurate2_tiny.ppm", convertToPPPMImage(imageAccurate2_tiny));
	
	AccurateImage *imageAccurate1_medium = convertToAccurateImage(image);
	AccurateImage *imageAccurate2_medium = convertToAccurateImage(image);
	
	// Process the medium case:
	size = 5;
	blurIteration(imageAccurate2_medium, imageAccurate1_medium, temp, size);
	blurIteration(imageAccurate1_medium, imageAccurate2_medium, temp, size);
	blurIteration(imageAccurate2_medium, imageAccurate1_medium, temp, size);
	blurIteration(imageAccurate1_medium, imageAccurate2_medium, temp, size);
	blurIteration(imageAccurate2_medium, imageAccurate1_medium, temp, size);
	
	AccurateImage *imageAccurate1_large = convertToAccurateImage(image);
	AccurateImage *imageAccurate2_large = convertToAccurateImage(image);
	
	// Do each color channel
	size = 8;
	blurIteration(imageAccurate2_large, imageAccurate1_large, temp, size);
	blurIteration(imageAccurate1_large, imageAccurate2_large, temp, size);
	blurIteration(imageAccurate2_large, imageAccurate1_large, temp, size);
	blurIteration(imageAccurate1_large, imageAccurate2_large, temp, size);
	blurIteration(imageAccurate2_large, imageAccurate1_large, temp, size);
	// calculate difference
	PPMImage *final_tiny = imageDifference(imageAccurate2_tiny, imageAccurate2_small);
    PPMImage *final_small = imageDifference(imageAccurate2_small, imageAccurate2_medium);
    PPMImage *final_medium = imageDifference(imageAccurate2_medium, imageAccurate2_large);
	// Save the images.
    if(argc > 1) {
        writePPM("flower_tiny.ppm", final_tiny);
        writePPM("flower_small.ppm", final_small);
        writePPM("flower_medium.ppm", final_medium);
    } else {
        writeStreamPPM(stdout, final_tiny);
        writeStreamPPM(stdout, final_small);
        writeStreamPPM(stdout, final_medium);
    }
	
}

