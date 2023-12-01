#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} bmp_pixel;


typedef struct {
    unsigned short bfType; //2 bytes
    unsigned int bfSize;   //4 bytes
    unsigned short bfReserved1; //2 bytes
    unsigned short bfReserved2;  //2 bytes
    unsigned int bfOffBits;     //4 bytes
    // Add other fields as needed
} bmp_file_header;

typedef struct {
    unsigned int biSize;         // 4 bytes
    unsigned int biWidth;        // 4 bytes
    unsigned int biHeight;       // 4 bytes
    unsigned short biPlanes;     // 2 bytes
    unsigned short biBitCount;   // 2 bytes
    unsigned int biCompression;  // 4 bytes
    unsigned int biSizeImage;    // 4 bytes
    unsigned int biXPelsPerMeter;// 4 bytes
    unsigned int biYPelsPerMeter;// 4 bytes
    unsigned int biClrUsed;      // 4 bytes
    unsigned int biClrImportant; // 4 bytes
    // Add other fields as needed
} bmp_info_header;


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
void saveFrameAsBMP(unsigned char* preamble, int preambleSize, int height, int width, bmp_pixel** frame, FILE* bmpFile) {
    // Write the stored preamble to the file
    fwrite(preamble, sizeof(unsigned char), preambleSize, bmpFile);

    // Write frame data
    for (int i = 0; i < height; ++i) {
        fwrite(frame[i], sizeof(bmp_pixel), width, bmpFile);
    }

    // Ensure to close the file when done
    fclose(bmpFile);
}



// Function to save all frames in the "frames" folder
void saveFrames(unsigned char* preamble, int preambleSize,int numFrames, int height, int width, bmp_pixel*** frames) {
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
        saveFrameAsBMP( preamble, preambleSize, height, width, frames[i], bmpFile);
    }
}

// Function to read BMP file header with proper endianness handling
int readBMPFileHeader(FILE *bmpFile, bmp_file_header *fileHeader) {
    if (fread(&fileHeader->bfType, sizeof(fileHeader->bfType), 1, bmpFile) != 1 ||
        fread(&fileHeader->bfSize, sizeof(fileHeader->bfSize), 1, bmpFile) != 1 ||
        fread(&fileHeader->bfReserved1, sizeof(fileHeader->bfReserved1), 1, bmpFile) != 1 ||
        fread(&fileHeader->bfReserved2, sizeof(fileHeader->bfReserved2), 1, bmpFile) != 1 ||
        fread(&fileHeader->bfOffBits, sizeof(fileHeader->bfOffBits), 1, bmpFile) != 1) {
        perror("Error reading BMP file header\n");
        return 1;
    }

    // Convert values to little-endian
    fileHeader->bfType = le16toh(fileHeader->bfType);
    fileHeader->bfSize = le32toh(fileHeader->bfSize);
    fileHeader->bfReserved1 = le16toh(fileHeader->bfReserved1);
    fileHeader->bfReserved2 = le16toh(fileHeader->bfReserved2);
    fileHeader->bfOffBits = le32toh(fileHeader->bfOffBits);

    return 0;
}

// Function to read BMP file header with proper endianness handling
int readBMPinfoHeader(FILE *bmpFile, bmp_info_header *infoHeader) {
    if (fread(&infoHeader->biSize, sizeof(infoHeader->biSize), 1, bmpFile) == 1 &&
        fread(&infoHeader->biWidth, sizeof(infoHeader->biWidth), 1, bmpFile) == 1 &&
        fread(&infoHeader->biHeight, sizeof(infoHeader->biHeight), 1, bmpFile) == 1 &&
        fread(&infoHeader->biPlanes, sizeof(infoHeader->biPlanes), 1, bmpFile) == 1 &&
        fread(&infoHeader->biBitCount, sizeof(infoHeader->biBitCount), 1, bmpFile) == 1 &&
        fread(&infoHeader->biCompression, sizeof(infoHeader->biCompression), 1, bmpFile) == 1 &&
        fread(&infoHeader->biSizeImage, sizeof(infoHeader->biSizeImage), 1, bmpFile) == 1 &&
        fread(&infoHeader->biXPelsPerMeter, sizeof(infoHeader->biXPelsPerMeter), 1, bmpFile) == 1 &&
        fread(&infoHeader->biYPelsPerMeter, sizeof(infoHeader->biYPelsPerMeter), 1, bmpFile) == 1 &&
        fread(&infoHeader->biClrUsed, sizeof(infoHeader->biClrUsed), 1, bmpFile) == 1 &&
        fread(&infoHeader->biClrImportant, sizeof(infoHeader->biClrImportant), 1, bmpFile) == 1) {
        // Convert values to little-endian
        infoHeader->biSize = le32toh(infoHeader->biSize);
        infoHeader->biWidth = le32toh(infoHeader->biWidth);
        infoHeader->biHeight = le32toh(infoHeader->biHeight);
        infoHeader->biPlanes = le16toh(infoHeader->biPlanes);
        infoHeader->biBitCount = le16toh(infoHeader->biBitCount);
        infoHeader->biCompression = le32toh(infoHeader->biCompression);
        infoHeader->biSizeImage = le32toh(infoHeader->biSizeImage);
        infoHeader->biXPelsPerMeter = le32toh(infoHeader->biXPelsPerMeter);
        infoHeader->biYPelsPerMeter = le32toh(infoHeader->biYPelsPerMeter);
        infoHeader->biClrUsed = le32toh(infoHeader->biClrUsed);
        infoHeader->biClrImportant = le32toh(infoHeader->biClrImportant);

        return 0;
    } else {
        perror("Error reading BMP info header\n");
        return 1;
    }
}


int main() {
    // Open the BMP file for reading
    FILE* bmpFile = fopen("/home/lasse/Documents/Image_processing/480-360-sample.bmp", "rb");
    if (bmpFile == NULL) {
        perror("Error opening BMP file\n");
        return 1;
    }

    // Set file pointer to the beginning
    fseek(bmpFile, 0, SEEK_SET);

     // Read the BMP file header
    bmp_file_header fileHeader;
    if (readBMPFileHeader(bmpFile, &fileHeader) != 0) {
        return 1;
    }


    // Read the BMP info header
    bmp_info_header infoHeader;
    if (readBMPinfoHeader(bmpFile, &infoHeader) != 0) {
        return 1;
    }

    // Calculate the offset for the image data
    unsigned int imageDataOffset = fileHeader.bfOffBits;


    // Allocate memory for the image data
    bmp_pixel* imageData = malloc(infoHeader.biSizeImage);
    if (imageData == NULL) {
        perror("Error allocating memory for image data\n");
        return 1;
    }

    // Move to the beginning of the image data
    fseek(bmpFile, imageDataOffset, SEEK_SET);

    // Read image data
    if (fread(imageData, sizeof(unsigned char), infoHeader.biSizeImage, bmpFile) != infoHeader.biSizeImage) {
        perror("Error reading image data\n");
        fprintf(stderr, "fread failed: %s\n", strerror(errno));  // Print more information about the error
        return 1;
    }
    // Rewind to the beginning of the file and store the preamble
    rewind(bmpFile);
    unsigned char preamble[imageDataOffset];
    if (fread(preamble, sizeof(unsigned char), imageDataOffset, bmpFile) != imageDataOffset) {
        perror("Error reading preamble\n");
        return 1;
    }
    fclose(bmpFile);

    // Now 'imageData' contains the raw pixel data
    const int numFrames = 30; // Set the number of frames you want
    bmp_pixel** frames[numFrames];

    int height = infoHeader.biHeight;
    int width = infoHeader.biWidth;

    // Initialize the first frame
    frames[0] = allocateFrameMemory(height, width);

    // Copy imageData to the first frame
    for (int i = 0; i < height; ++i) {
        memcpy(frames[0][i], imageData + i * width, width * sizeof(bmp_pixel));
    }

    // Apply transformations for the remaining frames
    for (int i = 1; i < numFrames; ++i) {
        // Allocate memory for the next frame
        frames[i] = allocateFrameMemory(height, width);

        // Copy image
        copyImage(height, width, frames[i - 1], frames[i]);

        // Apply transformations to create the next frame
        convertToGrayscale(height, width, frames[i]);
        flipVertical(height, width, frames[i]);
    }

    // Save frames to the "frames" folder
    saveFrames(preamble,imageDataOffset, numFrames, height, width, frames);

    // Free allocated memory for all frames
    for (int i = 0; i < numFrames; ++i) {
        free(frames[i]);
    }

    // Cleanup
    free(imageData);
    return 0;
}
