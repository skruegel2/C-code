
#include <math.h>
#include "tiff.h"
#include "allocate.h"
#include "randlib.h"
#include "typeutil.h"

#define FILTER_LENGTH 9

void error(char *name);

// Step through all rows and convolve
void ConvolveHoriz(int width, int height, int filt_len,
                      double** source_img, double* filt_array, double** dest_img);

// Step through all cols and convolve
void ConvolveVert(int width, int height, int filt_len,
                  double** source_img, double* filt_array, double** dest_img);

void ConvolveRow(int source_len, int filt_len,
                 double* source_row, double* filt_array, double* dest_row);

void ConvolveCol(int source_len, int filt_len,
                 double* source_col, double* filt_array, double* dest_col);

int main (int argc, char **argv)
{
//  double* source = malloc(3*sizeof(double));
//  source[0] = 2.0;
//  source[1] = 2.0;
//  source[2] = 2.0;
//  double* filt = malloc(2*sizeof(double));
//  filt[0] = 1.0;
//  filt[1] = 1.0;
//  double* dest = malloc(4*sizeof(double));
//  ConvolveRow(3, 2, source, filt, dest);
//  double** source_img = malloc(3*sizeof(double*));
//  for(int i=0; i< 3; i++)
//  {
//     source_img[i] = malloc(3*sizeof(double));
//  }
//  double** dest_img = malloc(4*sizeof(double*));
//  for(int i=0; i< 4; i++)
//  {
//     dest_img[i] = malloc(4*sizeof(double));
//  }
//  source_img[0][0] = 1;
//  source_img[0][1] = 1;
//  source_img[0][2] = 1;
//  source_img[1][0] = 2;
//  source_img[1][1] = 2;
//  source_img[1][2] = 2;
//  source_img[2][0] = 3;
//  source_img[2][1] = 3;
//  source_img[2][2] = 3;
//  double filter[2] = {1,1};
//  ConvolveHoriz(3,3, 2,source_img,&filter[0], dest_img);
  FILE *fp;
  struct TIFF_img input_img, color_img;
  double **img_red_array,**img_green_array, **img_blue_array;
  double **img_red_filt, **img_green_filt, **img_blue_filt;
  int i, j;
  int32_t pixel;
////  double filt_array[9] = {1.0/81.0,1.0/81.0,1.0/81.0,1.0/81.0,1.0/81.0,
////                        1.0/81.0,1.0/81.0,1.0/81.0};
  double filt_array[9] = {0.1111,0.1111,0.1111,0.1111,0.1111,0.1111,0.1111,0.1111,0.1111};
//  double filt_array[9] = {1,1,1,1,1,1,1,1,1};

  if ( argc != 2 ) error( argv[0] );

  /* open image file */
  if ( ( fp = fopen ( argv[1], "rb" ) ) == NULL ) {
    fprintf ( stderr, "cannot open file %s\n", argv[1] );
    exit ( 1 );
  }

  /* read image */
  if ( read_TIFF ( fp, &input_img ) ) {
    fprintf ( stderr, "error reading file %s\n", argv[1] );
    exit ( 1 );
  }

  /* close image file */
  fclose ( fp );

  /* check the type of image data */
  if ( input_img.TIFF_type != 'c' ) {
    fprintf ( stderr, "error:  image must be 24-bit color\n" );
    exit ( 1 );
  }

  /* Allocate image of double precision floats */
  /* Increase size of array in both dimensions for easy convolution. */

  img_red_array = (double **)get_img(input_img.width,
                                     input_img.height,
                                     sizeof(double));
////  img_green_array = (double **)get_img(input_img.width + (FILTER_LENGTH - 1) * 2,
////                                     input_img.height + (FILTER_LENGTH - 1) * 2,
////                                     sizeof(double));
////  img_blue_array = (double **)get_img(input_img.width + (FILTER_LENGTH - 1) * 2,
////                                     input_img.height + (FILTER_LENGTH - 1) * 2,
////                                     sizeof(double));
  img_red_filt = (double **)get_img(input_img.width + (FILTER_LENGTH - 1),
                                     input_img.height + (FILTER_LENGTH - 1),
                                     sizeof(double));
////  img_green_filt = (double **)get_img(input_img.width + (FILTER_LENGTH - 1) * 2,
////                                     input_img.height + FILTER_LENGTH - 1,
////                                     sizeof(double));
////  img_blue_filt = (double **)get_img(input_img.width + (FILTER_LENGTH - 1) * 2,
////                                     input_img.height + (FILTER_LENGTH - 1) * 2,
////                                     sizeof(double));
  /* copy all components to respective double array */
  for ( i = 0; i < input_img.height; i++ )
  {
    for ( j = 0; j < input_img.width; j++ )
    {
      img_red_array[i][j] = input_img.color[0][i][j];
  //    img_green_array[i+(FILTER_LENGTH - 1)][j+(FILTER_LENGTH - 1)] = input_img.color[1][i][j];
  //    img_blue_array[i+(FILTER_LENGTH - 1)][j+(FILTER_LENGTH - 1)] = input_img.color[2][i][j];
    }
  }
  ConvolveHoriz(input_img.width, input_img.height, FILTER_LENGTH,
                img_red_array, filt_array, img_red_filt);

//  /* Fill in boundary pixels */
////  for ( i = 0; i < input_img.height; i++ ) {
////    img2[i][0] = 0;
////    img2[i][input_img.width-1] = 0;
////  }
//
//
//  /* set up structure for output achromatic image */
//  /* to allocate a full color image use type 'c' */
////  get_TIFF ( &green_img, input_img.height, input_img.width, 'g' );

  /* set up structure for output color image */
  /* Note that the type is 'c' rather than 'g' */
  get_TIFF ( &color_img, input_img.height+(FILTER_LENGTH - 1),
            input_img.width+(FILTER_LENGTH - 1), 'c' );
////  get_TIFF ( &color_img, input_img.height,
////            input_img.width, 'c' );
//
//  /* copy green component to new images */
////  for ( i = 0; i < input_img.height; i++ )
////  for ( j = 0; j < input_img.width; j++ ) {
////    pixel = (int32_t)img2[i][j];
////    if(pixel>255) {
////      green_img.mono[i][j] = 255;
////    }
////    else {
////      if(pixel<0) green_img.mono[i][j] = 0;
////      else green_img.mono[i][j] = pixel;
////    }
////  }

  for ( i = 0; i < input_img.height+(FILTER_LENGTH - 1); i++ )
  {
      for ( j = 0; j < input_img.width+(FILTER_LENGTH - 1); j++ )
      {
          pixel = (int32_t) img_red_filt[i][j];
          if (pixel > 255)
          {
              color_img.color[0][i][j] = 255;
          }
          else if (pixel < 0)
          {
              color_img.color[0][i][j] = 0;
          }
          else
          {
              color_img.color[0][i][j] = pixel;
          }
//          color_img.color[1][i][j] = img_green_array[i][j];
//          color_img.color[2][i][j] = img_blue_array[i][j];
      }

  }

  /* open horiz image file */
  if ( ( fp = fopen ( "horiz.tif", "wb" ) ) == NULL ) {
    fprintf ( stderr, "cannot open file horiz.tif\n");
    exit ( 1 );
  }

  /* write horiz image */
  if ( write_TIFF ( fp, &color_img ) ) {
    fprintf ( stderr, "error writing TIFF file %s\n", argv[2] );
    exit ( 1 );
  }

  /* close horiz image file */
  fclose ( fp );


//  /* open color image file */
////  if ( ( fp = fopen ( "color.tif", "wb" ) ) == NULL ) {
////      fprintf ( stderr, "cannot open file color.tif\n");
////      exit ( 1 );
////  }
//
//  /* write color image */
////  if ( write_TIFF ( fp, &color_img ) ) {
////      fprintf ( stderr, "error writing TIFF file %s\n", argv[2] );
////      exit ( 1 );
////  }
//
//  /* close color image file */
////  fclose ( fp );
//
//  /* de-allocate space which was used for the images */
  free_TIFF ( &(input_img) );
////  free_TIFF ( &(green_img) );
  free_TIFF ( &(color_img) );
//
  free_img( (void**)img_red_array );
////  free_img( (void**)img_green_array );
////  free_img( (void**)img_blue_array );
  free_img( (void**)img_red_filt );
////  free_img( (void**)img_green_filt );
////  free_img( (void**)img_blue_filt );
////  free_img( (void**)img2 );

  return(0);
}

void error(char *name)
{
    printf("usage:  %s  image.tiff \n\n",name);
    printf("this program reads in a 24-bit color TIFF image.\n");
    printf("It then horizontally filters the green component, adds noise,\n");
    printf("and writes out the result as an 8-bit image\n");
    printf("with the name 'green.tiff'.\n");
    printf("It also generates an 8-bit color image,\n");
    printf("that swaps red and green components from the input image");
    exit(1);
}
//    ConvolveHoriz(i, input_img.width, FILTER_LENGTH,
//                  img_red_array, filt_array, img_red_filt);

// Step through all rows and convolve
void ConvolveHoriz(int width, int height, int filt_len,
                  double** source_img, double* filt_array, double** dest_img)
{
  for(int current_row = 0; current_row < height; current_row++)
  {
    ConvolveRow(width, filt_len, source_img[current_row], &filt_array[0], dest_img[current_row]);
  }
}

// Step through all cols and convolve
void ConvolveVert(int width, int height, int filt_len,
                  double** source_img, double* filt_array, double** dest_img)
{
  for(int current_col = 0; current_col < width; current_col++)
  {
    ConvolveCol(height, filt_len, source_img[current_col], &filt_array[0], dest_img[current_col]);
  }
}

void ConvolveRow(int source_len, int filt_len,
                 double* source_row, double* filt_array, double* dest_row)
{
  int k;
  // Zero out destination
  for(int cur_el = 0; cur_el < source_len + filt_len; cur_el++)
  {
    dest_row[cur_el] = 0;
  }
  double source[3];
  double filt[2];
  for(k = 0; k < 3; k++)
  {
    source[k] = source_row[k];
  }
  for(k = 0; k < 2; k++)
  {
    filt[k] = filt_array[k];
  }

  for(int i = 0; i < source_len; i++)
  {
    for(int j = 0; j < filt_len; j++)
    {
      dest_row[i+j] += source_row[i]*filt_array[j];
    }
  }
  double dest[4];
  for(k = 0; k < 4; k++)
  {
    dest[k] = dest_row[k];
  }

}

void ConvolveCol(int source_len, int filt_len,
                 double* source_col, double* filt_array, double* dest_col)
{
  int k;

  // Zero out destination
  for(int cur_el = 0; cur_el < source_len + filt_len; cur_el++)
  {
    dest_col[cur_el] = 0;
  }

  double source[3];
  double filt[2];
  for(k = 0; k < 3; k++)
  {
    source[k] = source_col[k];
  }
  for(k = 0; k < 2; k++)
  {
    filt[k] = filt_array[k];
  }

  for(int i = 0; i < source_len; i++)
  {
    for(int j = 0; j < filt_len; j++)
    {
      dest_col[i+j] += source_col[i]*filt_array[j];
    }
  }
  double dest[4];
  for(k = 0; k < 4; k++)
  {
    dest[k] = dest_col[k];
  }

}
