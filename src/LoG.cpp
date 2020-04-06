#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <cmath>
#include <limits>

using namespace std;

uint8_t img_header[5];

#define col                     *(uint16_t *)(img_header)
#define row                     *(uint16_t *)(img_header + 2)
#define pixel_size              *(uint8_t *)(img_header + 4)


int main(int agrc, char **argv)
{
	char* img_in = argv[1];
	char* result = argv[2];
	float sigma = stod(argv[3]);
	int kernel_size = atoi(argv[4]);

	//Input the raw image, read the raw image to a matrix
	FILE *input;
	uint8_t get_char;
	input = fopen(img_in,"rb");
	//read the image header
	for (int i = 0; i < 5; i++)
	{
    img_header[i] = fgetc(input);
	}
	int img[row][col];
	//read the image in to 2d array
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			img[i][j] = fgetc(input);     //read the image to an array
		}
	}
	printf("**Loaded image from %s, (%d, %d)\n", img_in, row, col);


	// int kernel_size = (int)ceil(5*sigma);
	// if(kernel_size%2==0) kernel_size+=1;
	int offset = (kernel_size-1)/2;
	float LoG_kernel[kernel_size][kernel_size];
	float s2 = sigma*sigma;
	float s4 = sigma*sigma*sigma*sigma;
	float scale = 0.0;

	for(int x = -offset; x <= offset; x++)
	{
		for(int y = -offset; y <= offset; y++)
		{
			float val = (float)(-1.0f / (M_PI * (s4))) * (1.0f - ((x * x + y * y) / (2.0f * (s2)))) * (float)exp((-(x * x + y * y) / (2.0f * (s2))));
			LoG_kernel[x+offset][y+offset] = val;
			scale += val;
			printf("%10.5f", LoG_kernel[x+offset][y+offset]);
		}
		printf("  \n");
	}
	printf("  --LoG mask with sigma=%.2f, kernel_size=%d, scale=%.5f\n", sigma, kernel_size, scale);


	int img_LoG[row][col];
	for (int i = offset; i < row-offset; i++)
	{
		for (int j = offset; j < col-offset; j++)
		{
			float sum = 0;
			for (int a = 0; a < kernel_size; a++)
			{
				for (int b = 0; b < kernel_size; b++)
				{
					sum = sum + LoG_kernel[a][b] * img[i-offset+a][j-offset+b];
				}
			}
			img_LoG[i][j] = round(sum/scale);
		}
	}


	//display img
	FILE* pgmimg;
    pgmimg = fopen(result, "wb");
    fprintf(pgmimg, "P2\n");
    fprintf(pgmimg, "%d %d\n", col, row);
    fprintf(pgmimg, "255\n");
    int count = 0;
    for (int i = 0; i < row; i++)
		{
      for (int j = 0; j < col; j++)
			{
      	int temp = img_LoG[i][j];
      	if (temp>255)
      		temp = 255;
      	if (temp<0)
      		temp = 0;
        fprintf(pgmimg, "%d ", temp);
      }
      fprintf(pgmimg, "\n");
    }
    fclose(pgmimg);
		printf("**Saved canny edge to %s!\n", result);

	return 0;
}
