#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <cmath>
#include <cfloat>

using namespace std;

uint8_t img_header[5];

#define col                     *(uint16_t *)(img_header)
#define row                     *(uint16_t *)(img_header + 2)
#define pixel_size              *(uint8_t *)(img_header + 4)

int main(int argc, char **argv)
{
	char* img_in = argv[1];
	char* result = argv[2];
	double sigma = stod(argv[3]);
	double thre_low = stod(argv[4]);
	double thre_high = stod(argv[5]);

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


	int kernel_size = (int)ceil(5*sigma);
	if(kernel_size%2==0) kernel_size+=1;
  int offset = (kernel_size-1)/2;
	float filter[kernel_size][kernel_size];
	int gaussian_mask[kernel_size][kernel_size];
	for(int x = -offset; x <= offset; x++)
  {
		for(int y = -offset; y <= offset; y++)
    {
			float expo =(x*x+y*y)/(2*sigma*sigma);
			filter[x+offset][y+offset] =  (exp(-expo))/(sigma*sigma*(2*M_PI));
		}
	}
	printf("  --Gaussian mask with sigma=%.2f, kernel_size=%d:\n", sigma, kernel_size);
	float temp = filter[0][0];
	int scale = 0;
	for (int i = 0; i < kernel_size; i++)
	{
		for (int j = 0; j < kernel_size; j++)
		{
			gaussian_mask[i][j] = round(filter[i][j] / temp);
			scale = scale + gaussian_mask[i][j];
			printf("%5d   ", gaussian_mask[i][j]);
		}
		printf(" \n");
	}
	printf("  --low threshold: %.1f, high threshold: %.1f.\n", thre_low, thre_high);

	int img_gaussian[row][col]; //Image after gaussian mask
	for(int i = offset; i < row-offset; i++)
	{
		for(int j = offset; j < col-offset; j++)
		{
			int sum = 0;
			for(int a = 0; a < kernel_size; a++)
			{
				for(int b = 0; b < kernel_size; b++)
				{
					sum = sum + gaussian_mask[a][b] * img[i-offset+a][j-offset+b];
				}
			}
			img_gaussian[i][j] = round(sum/scale);
		}
	}
	printf("  Guassian smoothing-----------------------DONE\n");


  int sobelx[3][3] = {{-1, 0, 1},{-2, 0, 2},{-1, 0, 1}};
  int sobely[3][3] = {{-1, -2, -1},{0, 0, 0},{1, 2, 1}};
	int G_x[row][col];  	//Image after sobel mask x
	int G_y[row][col];  	//Image after sobel mask y
  float G_strength[row][col];
  int G_orientation[row][col];
	for (int i = 1; i < row-1; i++)
	{
		for (int j = 1; j < col-1; j++)
		{
			int sum = 0;
			for (int a = 0; a < 3; a++)
			{
				for (int b = 0; b < 3; b++)
				{
					sum = sum + sobelx[a][b]*img_gaussian[i+a-1][j+b-1];
				}
			}
			G_x[i][j] = sum;
		}
	}
	for (int i = 1; i < row-1; i++)
	{
		for (int j = 1; j < col-1; j++)
		{
			int sum = 0;
			for (int a = 0; a < 3; a++)
			{
				for (int b = 0; b < 3; b++)
				{
					sum = sum + sobely[a][b]*img_gaussian[i+a-1][j+b-1];
				}
			}
			G_y[i][j] = sum;
		}
	}
  printf("  Compute derivatives----------------------DONE\n");

  for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
    {
      G_strength[i][j] = sqrt(pow(G_x[i][j],2)+pow(G_y[i][j],2));
      float angle = atan(float(G_y[i][j])/G_x[i][j])*180/M_PI;
			if(angle>-22.5 && angle<=22.5) G_orientation[i][j] = 0;
			else if(angle>22.5 && angle<=67.5) G_orientation[i][j] = 45;
			else if(angle>67.5 || angle<=-67.5) G_orientation[i][j] = 90;
			else G_orientation[i][j] = 135;
    }
	}
	printf("  Obtain strength and orientation----------DONE\n");


	for(int i=1; i<row-1; i++)
	{
		for(int j=1; j<col-1; j++)
		{
			if(G_orientation[i][j]==0)
			{
				G_strength[i][j] = ((G_strength[i][j]>=G_strength[i][j-1])&&(G_strength[i][j]>=G_strength[i][j+1]))?G_strength[i][j]:0;
			}
			else if(G_orientation[i][j]==90)
			{
				G_strength[i][j] = ((G_strength[i][j]>=G_strength[i-1][j])&&(G_strength[i][j]>=G_strength[i+1][j]))?G_strength[i][j]:0;
			}
			else if(G_orientation[i][j]==45)
			{
				G_strength[i][j] = ((G_strength[i][j]>=G_strength[i-1][j])&&(G_strength[i][j]>=G_strength[i][j+1]))?G_strength[i][j]:0;
			}
			else if(G_orientation[i][j]==135)
			{
				G_strength[i][j] = ((G_strength[i][j]>=G_strength[i][j-1])&&(G_strength[i][j]>=G_strength[i+1][j]))?G_strength[i][j]:0;
			}
		}
	}
	printf("  Non-max suppression----------------------DONE\n");


	int edge[row][col];
	int thre_edge[row][col];
	for(int i=0; i<row; i++)
	{
		for(int j=0; j<col; j++)
		{
			if(G_strength[i][j]>=thre_low)
			{
				thre_edge[i][j] = 1;
				if(G_strength[i][j]>=thre_high)
				{
					thre_edge[i][j] = 2;
				}
			}
		}
	}
	for(int i=0; i<row; i++)
	{
		for(int j=0; j<col; j++)
		{
			if(thre_edge[i][j]==2)
			{
				edge[i][j] = G_strength[i][j];
				if(thre_edge[i-1][j-1]==1) edge[i-1][j-1] = G_strength[i-1][j-1];
				if(thre_edge[i-1][j]==1) edge[i-1][j] = G_strength[i-1][j];
				if(thre_edge[i-1][j+1]==1) edge[i-1][j+1] = G_strength[i-1][j+1];
				if(thre_edge[i][j-1]==1) edge[i][j-1] = G_strength[i][j-1];
				if(thre_edge[i][j+1]==1) edge[i][j+1] = G_strength[i][j+1];
				if(thre_edge[i+1][j-1]==1) edge[i+1][j-1] = G_strength[i+1][j-1];
				if(thre_edge[i+1][j]==1) edge[i+1][j] = G_strength[i+1][j];
				if(thre_edge[i+1][j+1]==1) edge[i+1][j+1] = G_strength[i+1][j+1];
			}
			else edge[i][j] = 0;
		}
	}
	printf("  Hysteresis thresholding------------------DONE\n");



  //display img
	FILE* pgmimg;
  pgmimg = fopen(result, "wb");
  fprintf(pgmimg, "P2\n");
  fprintf(pgmimg, "%d %d\n", col, row);
  fprintf(pgmimg, "255\n");
  for(int i = 0; i < row; i++)
  {
    for(int j = 0; j < col; j++)
    {
    	int temp = edge[i][j];
    	if (temp>0)
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
