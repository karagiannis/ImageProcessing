#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} bmp_pixel;


typedef struct {
    unsigned short bfType;
    unsigned int bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int bfOffBits;
    // Add other fields as needed
} bmp_file_header;

typedef struct {
    unsigned int biSize;
    int biWidth;
    int biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
    // Add other fields as needed
} bmp_info_header;

void bmp_header_init_df(bmp_file_header* fileHeader, bmp_info_header* infoHeader, int width, int height) {
    // Initialize file header
    fileHeader->bfType = 0x4D42; // "BM" in ASCII
    fileHeader->bfSize = sizeof(bmp_file_header) + sizeof(bmp_info_header) + infoHeader->biSizeImage;
    fileHeader->bfReserved1 = 0;
    fileHeader->bfReserved2 = 0;
    fileHeader->bfOffBits = sizeof(bmp_file_header) + sizeof(bmp_info_header);

    // Initialize info header
    infoHeader->biSize = sizeof(bmp_info_header);
    infoHeader->biWidth = width;
    infoHeader->biHeight = height;
    // Other fields initialization...
}

bmp_pixel** allocateFrameMemory(int height, int width) {
    bmp_pixel** frame = malloc(height * sizeof(bmp_pixel*));
    for (int i = 0; i < height; ++i) {
        frame[i] = malloc(width * sizeof(bmp_pixel));
    }
    return frame;
}


void convertToGrayscale(int height, int width, bmp_pixel** pixels) {
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            // Calculate average of RGB values
            unsigned char avg = (pixels[i][j].red + pixels[i][j].green + pixels[i][j].blue) / 3;

            // Set all RGB values to the average
            pixels[i][j].red = avg;
            pixels[i][j].green = avg;
            pixels[i][j].blue = avg;
        }
    }
}

void flipVertical(int height, int width, bmp_pixel** pixels) {
    for (int i = 0; i < height / 2; ++i) {
        for (int j = 0; j < width; ++j) {
            // Swap pixels vertically
            bmp_pixel temp = pixels[i][j];
            pixels[i][j] = pixels[height - 1 - i][j];
            pixels[height - 1 - i][j] = temp;
        }
    }
}

void copyImage(int height, int width, bmp_pixel** image_orig, bmp_pixel** image_copied) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            // Copy pixel values from image_orig to image_copied
            image_copied[i][j] = image_orig[i][j];
        }
    }
}



// Function to add BMP header and save frame as BMP file
void saveFrameAsBMP(int height, int width, bmp_pixel** frame, FILE* bmpFile) {
    bmp_file_header fileHeader;
    bmp_info_header infoHeader;

    // Call the common initialization within bmp_header_init_df
    bmp_header_init_df(&fileHeader, &infoHeader, width, height);

    // Set size image for the info header
    infoHeader.biSizeImage = height * width * sizeof(bmp_pixel);

    // Write file header to file
    fwrite(&fileHeader, sizeof(bmp_file_header), 1, bmpFile);

    // Write info header to file
    fwrite(&infoHeader, sizeof(bmp_info_header), 1, bmpFile);

    // Write frame data
    for (int i = 0; i < height; ++i) {
        fwrite(frame[i], sizeof(bmp_pixel), width, bmpFile);
    }

    // Ensure to close the file when done
    fclose(bmpFile);
}


// Function to save all frames in the "frames" folder
void saveFrames(int numFrames, int height, int width, bmp_pixel*** frames) {
    // Create "frames" folder if it doesn't exist
    // Create "frames" folder if it doesn't exist
    if (system("mkdir -p frames") != 0) {
        perror("Error creating frames folder");
    }

    // Save each frame
    for (int i = 0; i < numFrames; ++i) {
        // Generate a unique file name for each frame
        char fileName[50];
        snprintf(fileName, sizeof(fileName), "frames/frame_%03d.bmp", i + 1);

        // Open the file for writing
        FILE* bmpFile = fopen(fileName, "wb");

        // Check if the file was opened successfully
        if (bmpFile == NULL) {
            perror("Error opening BMP file");
            return;
        }

        // Call saveFrameAsBMP with the file handle
        saveFrameAsBMP( height, width, frames[i], bmpFile);
    }
}




int main() {
    // Open the BMP file for reading
    FILE* bmpFile = fopen("480-360-sample.bmp", "rb");
    if (bmpFile == NULL) {
        perror("Error opening BMP file");
        return 1;
    }

    // Read the BMP file header
    bmp_file_header fileHeader;
    if (fread(&fileHeader, sizeof(bmp_file_header), 1, bmpFile) != 1) {
        perror("Error reading BMP file header");
        fclose(bmpFile);
        return 1;
    }

    // Read the BMP info header
    bmp_info_header infoHeader;
    if (fread(&infoHeader, sizeof(bmp_info_header), 1, bmpFile) != 1) {
        perror("Error reading BMP info header");
        fclose(bmpFile);
        return 1;
    }

    // Calculate the offset for the image data
    unsigned int imageDataOffset = fileHeader.bfOffBits;

    // Allocate memory for the image data
    bmp_pixel* imageData = malloc(infoHeader.biSizeImage);

    if (imageData == NULL) {
        perror("Error allocating memory for image data");
        fclose(bmpFile);
        return 1;
    }

    // Move to the beginning of the image data
    fseek(bmpFile, imageDataOffset, SEEK_SET);

    // Read the image data
    if (fread(imageData, 1, infoHeader.biSizeImage, bmpFile) != infoHeader.biSizeImage) {
        perror("Error reading image data");
        free(imageData);
        fclose(bmpFile);
        return 1;
    }

    // Now 'imageData' contains the raw pixel data
     const int numFrames = 30; // Set the number of frames you want

    bmp_pixel** frames[numFrames];


    int height = infoHeader.biHeight;
    int width =infoHeader.biWidth;

    // Initialize the first frame (you might load it from a file or create it)
    // For simplicity, let's assume you have a function createInitialFrame that allocates memory
    // Allocate memory for the first frame
   
    frames[0] = allocateFrameMemory(height, width);


    // Copy imageData to the first frame
    for (int i = 0; i < height; ++i) {
        memcpy(frames[0][i], imageData + i * width, width * sizeof(bmp_pixel));
    }


    // Apply transformations for the remaining frames
    for (int i = 1; i < numFrames; ++i) {
        // Allocate memory for the next frame
        // Allocate memory for frames[i]
        frames[i] = allocateFrameMemory(height, width);


        //Copy image
        copyImage(height, width, frames[i-1], frames[i]);

        // Apply transformations to create the next fram
        convertToGrayscale(height, width, frames[i]);
        flipVertical(height, width, frames[i]);
    }

    saveFrames(numFrames, height, width, frames);

    // Free allocated memory for all frames
    // Free memory for frames[i]
    for (int i = 0; i < height; ++i) {
        free(frames[i]);
    }



    // Proceed with your processing or transformations here

    // Cleanup
    free(imageData);
    fclose(bmpFile);

    return 0;
}
