#include "edge_devernay.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

static FILE * xfopen(const char * path, const char * mode);
static int xfclose(FILE * f);
void write_curves_pdf(double * x, double * y, int * curve_limits, int M,
	char * filename, int X, int Y, double width);
void write_curves_txt(double * x, double * y, int * curve_limits, int M,
	char * filename);


using namespace std;
using namespace cv;

uint8_t img_header[5];
#define col                     *(uint16_t *)(img_header)
#define row                     *(uint16_t *)(img_header + 2)
#define pixel_size              *(uint8_t *)(img_header + 4)

int main(int argc, char **argv)
{
	if(argc<3)
	{
		printf("ERROR: Please specify path to input image and output image!!!\n");
		return 1;
	}

	char* img_in = argv[1];
	char* result = argv[2];
  //parameters setting
	double sigma = 1.0; /* default sigma=0 */
	double thre_high = 15.0; /* default th_h=0  */
	double thre_low = 5.0; /* default th_l=0  */
	double W = 1.0; /* default W=1.0   */
	bool save_edge_points = false;
	if(argc>3)
	{
		W = stof(argv[3]);
		if(W>2.0)
		{
			W = 2.0;
		}
		if(W<1.0)
		{
			W = 1.0;
		}
		if(argc>4)
		{
			sigma = stof(argv[4]);
			if(argc>5)
			{
				thre_high = stof(argv[5]);
				if(argc>6)
				{
					thre_low = stof(argv[6]);
					if(argc>7)
					{
						save_edge_points = true;
					}
				}
			}
		}
	}

  FILE *input;
	uint8_t get_char;
	input = fopen(img_in,"rb");
	if(input==NULL)
	{
		printf("ERROR: Cannot read image from %s!!!\n", img_in);
		return 1;
	}

	//read the image header
	for (int i = 0; i < 5; i++)
	{
    img_header[i] = fgetc(input);
	}
	printf("Loaded input image from %s, (row, col)=(%d, %d)\n", img_in, row, col);

  const int iHeight = row;
	const int iWidth = col;
	uchar* pSrc = new uchar[iHeight*iWidth];
	uchar* pDst = new uchar[iHeight*iWidth];
  for(int i=0; i<row*col; i++)
  {
    pSrc[i] = fgetc(input);
  }

	//kernel size
	double prec = 3.0;
	int offset = (int)ceil(sigma * sqrt(2.0 * prec * log(10.0)));
	int n = 1 + 2 * offset;

	printf("----Input parameters----\n");
	printf("-Gaussian kernel: sigma=%.1f, size=%d\n", sigma, n);
	printf("-Hysteresis thresholding: high_threshold=%.1f, low_threshold=%.1f\n", thre_high, thre_low);
	printf("-Subpixel scale: %.1f\n", W);
	printf("------------------------\n");


	double * x;          /* x[n] y[n] coordinates of result edge point n */
	double * y;
	int * curve_limits;  /* limits of the curves in the x[] and y[] */
	int N, M;         /* result: N edge points, forming M curves */
  devernay(&x, &y, &N, &curve_limits, &M, pSrc, pDst, iWidth, iHeight, sigma, thre_high, thre_low);
	printf("Successfully obtained edge points in subpixel accuracy!\n");

	int img_edge[int(W*iHeight)][int(W*iWidth)];
  for(int i=0; i<N; i++)
  {
    img_edge[int(W*y[i])][int(W*x[i])] = 255;
  }

  //save result
	FILE* pgmimg;
  pgmimg = fopen(result, "wb");
  fprintf(pgmimg, "P2\n");
  fprintf(pgmimg, "%d %d\n", int(W*col), int(W*row));
  fprintf(pgmimg, "255\n");
  int count = 0;
  for(int i = 0; i < int(W*row); i++)
  {
    for(int j = 0; j < int(W*col); j++)
    {
    	int temp = img_edge[i][j];
    	if (temp>255)
    		temp = 255;
    	if (temp<0)
    		temp = 0;
      fprintf(pgmimg, "%d ", temp);
    }
    fprintf(pgmimg, "\n");
  }
  fclose(pgmimg);
	printf("Saved edge image to %s, (row, col)=(%d, %d)!\n", result, int(W*row), int(W*col));

	if(save_edge_points)
	{
	  // write_curves_pdf(x, y, curve_limits, M, argv[3], iWidth, iHeight, W);
		write_curves_txt(x, y, curve_limits, M, argv[7]);
		printf("Saved coordinates of %d edge points to %s!\n", N, argv[7]);
	}

  return 0;
}



/* open file, print an error and exit if fail
*/
static FILE * xfopen(const char * path, const char * mode)
{
	FILE * f = fopen(path, mode);
	if (f == NULL)
	{
		fprintf(stderr, "error: unable to open file '%s'\n", path);
		exit(EXIT_FAILURE);
	}
	return f;
}

/* close file, print an error and exit if fail
*/
static int xfclose(FILE * f)
{
	if (fclose(f) == EOF) error("unable to close file");
	return 0;
}
/*----------------------------------------------------------------------------*/
/* write curves into a PDF file. the output is PDF version 1.4 as described in
"PDF Reference, third edition" by Adobe Systems Incorporated, 2001
*/
void write_curves_pdf(double * x, double * y, int * curve_limits, int M,
	char * filename, int X, int Y, double width)
{
	FILE * pdf;
	long start1, start2, start3, start4, start5, startxref, stream_len;
	int i, j, k;

	/* check input */
	if (filename == NULL) error("invalid filename in write_curves_pdf");
	if (M > 0 && (x == NULL || y == NULL || curve_limits == NULL))
		error("invalid curves data in write_curves_pdf");
	if (X <= 0 || Y <= 0) error("invalid image size in write_curves_pdf");

	/* open file */
	pdf = xfopen(filename, "wb"); /* open to write as a binary file (b option).
								  otherwise, in some systems,
								  it may behave differently */

	/* PDF header */
	fprintf(pdf, "%%PDF-1.4\n");
	/* The following PDF comment contains characters with ASCII codes greater
	than 128. This helps to classify the file as containing 8-bit binary data.
	See "PDF Reference" p.63. */
	fprintf(pdf, "%%%c%c%c%c\n", 0xe2, 0xe3, 0xcf, 0xd3);

	/* Catalog, Pages and Page objects */
	start1 = ftell(pdf);
	fprintf(pdf, "1 0 obj\n<</Type /Catalog /Pages 2 0 R>>\n");
	fprintf(pdf, "endobj\n");
	start2 = ftell(pdf);
	fprintf(pdf, "2 0 obj\n<</Type /Pages /Kids [3 0 R] /Count 1 ");
	fprintf(pdf, "/Resources <<>> /MediaBox [0 0 %d %d]>>\nendobj\n", X, Y);
	start3 = ftell(pdf);
	fprintf(pdf, "3 0 obj\n");
	fprintf(pdf, "<</Type /Page /Parent 2 0 R /Contents 4 0 R>>\n");
	fprintf(pdf, "endobj\n");

	/* Contents object - graphic contents */
	start4 = ftell(pdf);
	fprintf(pdf, "4 0 obj\n<</Length 5 0 R>>\n"); /* indirect length in obj 5 */
	fprintf(pdf, "stream\n");
	stream_len = ftell(pdf);
	fprintf(pdf, "%.4f w\n", width); /* set line width */
	for (k = 0; k<M; k++) /* write curves */
	{
		/* an offset of 0.5,0.5 is added to point coordinates so that the
		drawing has the right positioning when superposed on the image
		drawn to the same size. in that case, pixels are drawn as squares
		of size 1,1 and the coordinate of the detected edge points are
		relative to the center of those squares. thus the 0.5, 0.5 offset.
		*/

		/* initate chain */
		i = curve_limits[k];
		fprintf(pdf, "%.4f %.4f m\n", x[i] + 0.5, Y - y[i] - 0.5); /* first point */

		/* add remaining points of the curve */
		for (j = i + 1; j<curve_limits[k + 1]; j++)
			fprintf(pdf, "%.4f %.4f l\n", x[j] + 0.5, Y - y[j] - 0.5);

		/* if the curve is closed, market as such */
		j = curve_limits[k + 1] - 1;
		if (x[i] == x[j] && y[i] == y[j]) fprintf(pdf, "h\n");

		/* end curve - stroke! */
		fprintf(pdf, "S\n");
	}
	stream_len = ftell(pdf) - stream_len; /* store stream length */
	fprintf(pdf, "\r\nendstream\n"); /* EOL must be CRLF before endstream */
	fprintf(pdf, "endobj\n");

	/* Contents' stream length object - the use of this indirect object
	for the stream length allows to generate the PDF file in a single
	pass, specifying the stream��s length only when its contents have
	been generated. See "PDF Reference" p.40. */
	start5 = ftell(pdf);
	fprintf(pdf, "5 0 obj\n%ld\nendobj\n", stream_len);

	/* PDF Cross-reference table */
	startxref = ftell(pdf);
	fprintf(pdf, "xref\n");
	fprintf(pdf, "0 6\n");
	fprintf(pdf, "0000000000 65535 f\r\n"); /* EOL must be CRLF in xref table */
	fprintf(pdf, "%010ld 00000 n\r\n", start1);
	fprintf(pdf, "%010ld 00000 n\r\n", start2);
	fprintf(pdf, "%010ld 00000 n\r\n", start3);
	fprintf(pdf, "%010ld 00000 n\r\n", start4);
	fprintf(pdf, "%010ld 00000 n\r\n", start5);

	/* PDF trailer */
	fprintf(pdf, "trailer <</Size 6 /Root 1 0 R>>\n");
	fprintf(pdf, "startxref\n");
	fprintf(pdf, "%ld\n", startxref);
	fprintf(pdf, "%%%%EOF\n");

	/* close file */
	xfclose(pdf);
}

/*----------------------------------------------------------------------------*/
/* write curves into a TXT file
*/
void write_curves_txt(double * x, double * y, int * curve_limits, int M,
	char * filename)
{
	FILE * txt;
	int i, k;

	/* check input */
	if (filename == NULL) error("invalid filename in write_curves_txt");
	if (M > 0 && (x == NULL || y == NULL || curve_limits == NULL))
		error("invalid curves data in write_curves_txt");

	/* open file */
	txt = xfopen(filename, "wb"); /* open to write as a binary file (b option).
								  otherwise, in some systems,
								  it may behave differently */

	/* write curves */
	for (k = 0; k<M; k++) /* write curves */
	{
		for (i = curve_limits[k]; i<curve_limits[k + 1]; i++)
			fprintf(txt, "%g %g\n", x[i], y[i]);
		fprintf(txt, "-1 -1\n"); /* end of chain */
	}

	/* close file */
	xfclose(txt);
}
