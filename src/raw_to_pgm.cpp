#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <bits/stdc++.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

uint8_t img_header[5];
#define col                     *(uint16_t *)(img_header)
#define row                     *(uint16_t *)(img_header + 2)

void list_dir(const char *path)
{
  struct dirent *entry;
  DIR *dir = opendir(path);

  if (dir == NULL) {
    return;
  }
  while ((entry = readdir(dir)) != NULL) {
  cout << entry->d_name << endl;
  }
  closedir(dir);
}

int main()
{
  if(mkdir("pgm_images", 0777)!=-1)
  {
    cout << "Created folder pgm_images to save output images!" << endl;
  }
  else
  {
    cout <<  "pgm_images folder create error: " << strerror(errno) << "!" << endl;
    return 1;
  }

  struct dirent *entry;
  DIR *dir = opendir("raw_images");

  if (dir == NULL)
  {
    cout << "Error: Cannot access folder raw_images!" << endl;
    return 1;
  }
  while((entry = readdir(dir)) != NULL)
  {
    // cout << entry->d_name << endl;
    string img_in = entry->d_name;
    int pos = img_in.find('.');
    if(img_in.substr(pos+1)=="raw")
    {
      string img_name = img_in.substr(0, pos);
      FILE *input;
    	uint8_t get_char;
    	input = fopen(("raw_images/"+img_in).c_str(),"rb");
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

      FILE* pgmimg;
      pgmimg = fopen(("pgm_images/"+img_name+".pgm").c_str(), "wb");
      fprintf(pgmimg, "P2\n");
      fprintf(pgmimg, "%d %d\n", col, row);
      fprintf(pgmimg, "255\n");
      for(int i = 0; i < row; i++)
      {
        for(int j = 0; j < col; j++)
        {
        	int temp = img[i][j];
        	if (temp>255)
        		temp = 255;
        	if (temp<0)
        		temp = 0;
            fprintf(pgmimg, "%d ", temp);
        }
        fprintf(pgmimg, "\n");
      }
      fclose(pgmimg);

    	cout << img_in << "(" << row << ", " << col << ")\t--->  pgm_images/" << img_name << ".pgm" << endl;
    }
  }
  closedir(dir);
  return 0;
}
