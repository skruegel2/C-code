
#include <math.h>
#include "tiff.h"
#include "allocate.h"
#include "randlib.h"
#include "typeutil.h"

#define FILTER_LENGTH 5

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

void ProcessRed(struct TIFF_img input_img, struct TIFF_img color_img);

void ProcessGreen(struct TIFF_img input_img, struct TIFF_img color_img);

void ProcessBlue(struct TIFF_img input_img, struct TIFF_img color_img);

int main (int argc, char **argv)
{
  FILE *fp_input;
  FILE *fp_color;
  struct TIFF_img input_img;
  struct TIFF_img color_img;
  if ( argc != 2 ) error( argv[0] );

  /* open image file */
  if ( ( fp_input = fopen ( argv[1], "rb" ) ) == NULL ) {
    fprintf ( stderr, "cannot open file %s\n", argv[1] );
    exit ( 1 );
  }

  /* read image */
  if ( read_TIFF ( fp_input, &input_img ) ) {
    fprintf ( stderr, "error reading file %s\n", argv[1] );
    exit ( 1 );
  }

  /* close image file */
  fclose ( fp_input );
  get_TIFF ( &color_img, input_img.height+(FILTER_LENGTH - 1),
            input_img.width+(FILTER_LENGTH - 1), 'c' );

  ProcessRed(input_img, color_img);
  ProcessGreen(input_img, color_img);
  ProcessBlue(input_img, color_img);

  if ( ( fp_color = fopen ( "Prob3Filt.tif", "wb" ) ) == NULL ) {
    fprintf ( stderr, "cannot open file horiz.tif\n");
    exit ( 1 );
  }

  // Write color image
  if ( write_TIFF ( fp_color, &color_img ) ) {
    fprintf ( stderr, "error writing TIFF file\n");
    exit ( 1 );
  }

  // Close color image file
  fclose ( fp_color );

  free_TIFF ( &(input_img) );
  free_TIFF ( &(color_img) );

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
  // Zero out destination
  for(int cur_el = 0; cur_el < source_len + filt_len; cur_el++)
  {
    dest_row[cur_el] = 0;
  }
  for(int i = 0; i < source_len; i++)
  {
    for(int j = 0; j < filt_len; j++)
    {
      dest_row[i+j] += source_row[i]*filt_array[j];
    }
  }
}

void ConvolveCol(int source_len, int filt_len,
                 double* source_col, double* filt_array, double* dest_col)
{
  // Zero out destination
  for(int cur_el = 0; cur_el < source_len + filt_len; cur_el++)
  {
    dest_col[cur_el] = 0;
  }

  for(int i = 0; i < source_len; i++)
  {
    for(int j = 0; j < filt_len; j++)
    {
      dest_col[i+j] += source_col[i]*filt_array[j];
    }
  }
}

void ProcessRed(struct TIFF_img input_img, struct TIFF_img color_img)
{
  double **img_red_array;
  double **img_red_filt;
  int i, j;
  int32_t pixel;
  double filt_array[5] = {0.2,0.2,0.2,0.2,0.2};

  img_red_array = (double **)get_img(input_img.width,
                                     input_img.height,
                                     sizeof(double));
  img_red_filt = (double **)get_img(input_img.width + (FILTER_LENGTH - 1),
                                     input_img.height + (FILTER_LENGTH - 1),
                                     sizeof(double));
  /* copy all components to respective double array */
  for ( i = 0; i < input_img.height; i++ )
  {
    for ( j = 0; j < input_img.width; j++ )
    {
      img_red_array[i][j] = input_img.color[0][i][j];
    }
  }
  ConvolveHoriz(input_img.width, input_img.height, FILTER_LENGTH,
                img_red_array, filt_array, img_red_filt);

  ConvolveVert(input_img.width, input_img.height, FILTER_LENGTH,
                img_red_array, filt_array, img_red_filt);

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
      }
  }

  free_img( (void**)img_red_array );
  free_img( (void**)img_red_filt );
}

void ProcessGreen(struct TIFF_img input_img, struct TIFF_img color_img)
{
  double **img_green_array;
  double **img_green_filt;
  int i, j;
  int32_t pixel;
  double filt_array[5] = {0.2,0.2,0.2,0.2,0.2};

  img_green_array = (double **)get_img(input_img.width,
                                     input_img.height,
                                     sizeof(double));
  img_green_filt = (double **)get_img(input_img.width + (FILTER_LENGTH - 1),
                                     input_img.height + (FILTER_LENGTH - 1),
                                     sizeof(double));
  /* copy all components to respective double array */
  for ( i = 0; i < input_img.height; i++ )
  {
    for ( j = 0; j < input_img.width; j++ )
    {
      img_green_array[i][j] = input_img.color[1][i][j];
    }
  }
  ConvolveHoriz(input_img.width, input_img.height, FILTER_LENGTH,
                img_green_array, filt_array, img_green_filt);

  ConvolveVert(input_img.width, input_img.height, FILTER_LENGTH,
                img_green_array, filt_array, img_green_filt);

  for ( i = 0; i < input_img.height+(FILTER_LENGTH - 1); i++ )
  {
      for ( j = 0; j < input_img.width+(FILTER_LENGTH - 1); j++ )
      {
          pixel = (int32_t) img_green_filt[i][j];
          if (pixel > 255)
          {
              color_img.color[1][i][j] = 255;
          }
          else if (pixel < 0)
          {
              color_img.color[1][i][j] = 0;
          }
          else
          {
              color_img.color[1][i][j] = pixel;
          }
      }
  }
  free_img( (void**)img_green_array );
  free_img( (void**)img_green_filt );
}

void ProcessBlue(struct TIFF_img input_img, struct TIFF_img color_img)
{
  double **img_blue_array;
  double **img_blue_filt;
  int i, j;
  int32_t pixel;
  double filt_array[5] = {0.2,0.2,0.2,0.2,0.2};

  img_blue_array = (double **)get_img(input_img.width,
                                     input_img.height,
                                     sizeof(double));
  img_blue_filt = (double **)get_img(input_img.width + (FILTER_LENGTH - 1),
                                     input_img.height + (FILTER_LENGTH - 1),
                                     sizeof(double));
  /* copy all components to respective double array */
  for ( i = 0; i < input_img.height; i++ )
  {
    for ( j = 0; j < input_img.width; j++ )
    {
      img_blue_array[i][j] = input_img.color[2][i][j];
    }
  }
  ConvolveHoriz(input_img.width, input_img.height, FILTER_LENGTH,
                img_blue_array, filt_array, img_blue_filt);

  ConvolveVert(input_img.width, input_img.height, FILTER_LENGTH,
                img_blue_array, filt_array, img_blue_filt);

  for ( i = 0; i < input_img.height+(FILTER_LENGTH - 1); i++ )
  {
      for ( j = 0; j < input_img.width+(FILTER_LENGTH - 1); j++ )
      {
          pixel = (int32_t) img_blue_filt[i][j];
          if (pixel > 255)
          {
              color_img.color[2][i][j] = 255;
          }
          else if (pixel < 0)
          {
              color_img.color[2][i][j] = 0;
          }
          else
          {
              color_img.color[2][i][j] = pixel;
          }
      }
  }

  free_img( (void**)img_blue_array );
  free_img( (void**)img_blue_filt );
}
