
#include <math.h>
#include "tiff.h"
#include "allocate.h"
#include "randlib.h"
#include "typeutil.h"

#define FILTER_LENGTH 9
#define HALF_FILT 4

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

void ProcessSingleColor(int color, struct TIFF_img input_img, struct TIFF_img color_img, double** filt);

// Multiply one pixel element by filter
double MultiplyOnePixelEl(int source_height, int source_width,
                         int src_cur_row, int src_cur_col,
                         double** source_img, double** filt);

// Multiply pixel by one filter element, considering boundaries.
double MultiplyOneFilterEl(int source_height, int source_width,
                         int src_cur_row, int src_cur_col, int filt_cur_row,
                         int filt_cur_col, double** source_img, double** filt);

// Init filter
void InitFilter(double** filt)
{
  for(int cur_row = 0; cur_row < FILTER_LENGTH; cur_row++)
  {
    for(int cur_col = 0; cur_col < FILTER_LENGTH; cur_col++)
    {
      filt[cur_row][cur_col] = 1.0/81.0;
    }
  }
}

int main (int argc, char **argv)
{
  FILE *fp_input;
  FILE *fp_color;
  struct TIFF_img input_img;
  struct TIFF_img color_img;
  double** test_src_img;
  double** filt;

  test_src_img = (double **)get_img(2,
                                    2,
                                     sizeof(double));
  filt = (double **)get_img(FILTER_LENGTH,
                            FILTER_LENGTH,
                            sizeof(double));
  InitFilter(filt);
//  double test_val;
//  test_src_img[0][0] = 1;
//  test_src_img[0][1] = 1;
//  test_src_img[1][0] = 1;
//  test_src_img[1][1] = 1;
//
//  test_val = MultiplyOnePixelEl(2,2,0,0,test_src_img,filt);
//  test_val = MultiplyOnePixelEl(2,2,0,1,test_src_img,filt);
//  test_val = MultiplyOnePixelEl(2,2,1,0,test_src_img,filt);
//  test_val = MultiplyOnePixelEl(2,2,1,1,test_src_img,filt);

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
  get_TIFF ( &color_img, input_img.height,
            input_img.width, 'c' );

  // Process red, green, blue
  for(int color = 0; color < 3; color++)
  {
    ProcessSingleColor(color, input_img, color_img, filt);
  }

  if ( ( fp_color = fopen ( "Prob3Filt.tif", "wb" ) ) == NULL ) {
    fprintf ( stderr, "cannot open file Prob3Filt.tif\n");
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

// Multiply pixel by one filter element, considering boundaries.
double MultiplyOneFilterEl(int source_height, int source_width,
                         int src_cur_row, int src_cur_col, int filt_cur_row,
                         int filt_cur_col, double** source_img, double** filt)
{
  double ret_val;
  // Check for out of bounds to the left
  if (src_cur_col + filt_cur_col < 0)
  {
    ret_val = 0;
  }
  // Check for out of bounds to the right
  else if (src_cur_col + filt_cur_col > (source_width - 1))
  {
    ret_val = 0;
  }
  // Check for out of bounds on top
  else if (src_cur_row + filt_cur_row < 0)
  {
    ret_val = 0;
  }
  // Check for out of bounds at bottom
  else if (src_cur_row + filt_cur_row > (source_height - 1))
  {
    ret_val = 0;
  }
  else
  {
    ret_val = source_img[src_cur_row][src_cur_col]*filt[filt_cur_row+HALF_FILT][filt_cur_col+HALF_FILT];
  }
  return ret_val;
}

// Multiply one pixel element by filter
double MultiplyOnePixelEl(int source_height, int source_width,
                         int src_cur_row, int src_cur_col,
                         double** source_img, double** filt)
{
  double ret_val = 0;   // Init before accumulating
  for(int filt_row = -HALF_FILT; filt_row <= HALF_FILT; filt_row++)
  {
    for (int filt_col = -HALF_FILT; filt_col <= HALF_FILT; filt_col++)
    {
      ret_val += MultiplyOneFilterEl(source_height,source_width,src_cur_row, src_cur_col,
                                    filt_row, filt_col, source_img, filt);
    }
  }

  return ret_val;
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

//// Step through all rows and convolve
//void ConvolveHoriz(int width, int height, int filt_len,
//                  double** source_img, double* filt_array, double** dest_img)
//{
//  for(int current_row = 0; current_row < height; current_row++)
//  {
//    ConvolveRow(width, filt_len, source_img[current_row], &filt_array[0], dest_img[current_row]);
//  }
//}
//
//// Step through all cols and convolve
//void ConvolveVert(int width, int height, int filt_len,
//                  double** source_img, double* filt_array, double** dest_img)
//{
//  for(int current_col = 0; current_col < width; current_col++)
//  {
//    ConvolveCol(height, filt_len, source_img[current_col], &filt_array[0], dest_img[current_col]);
//  }
//}
//
//void ConvolveRow(int source_len, int filt_len,
//                 double* source_row, double* filt_array, double* dest_row)
//{
//  // Zero out destination
//  for(int cur_el = 0; cur_el < source_len + filt_len; cur_el++)
//  {
//    dest_row[cur_el] = 0;
//  }
//  for(int i = 0; i < source_len; i++)
//  {
//    for(int j = 0; j < filt_len; j++)
//    {
//      dest_row[i+j] += source_row[i]*filt_array[j];
//    }
//  }
//}
//
//void ConvolveCol(int source_len, int filt_len,
//                 double* source_col, double* filt_array, double* dest_col)
//{
//  // Zero out destination
//  for(int cur_el = 0; cur_el < source_len + filt_len; cur_el++)
//  {
//    dest_col[cur_el] = 0;
//  }
//
//  for(int i = 0; i < source_len; i++)
//  {
//    for(int j = 0; j < filt_len; j++)
//    {
//      dest_col[i+j] += source_col[i]*filt_array[j];
//    }
//  }
//}

void ProcessSingleColor(int color, struct TIFF_img input_img, struct TIFF_img color_img, double** filt)
{
  double **img_orig;
  double **img_filt;
  int cur_row, cur_col;
  int32_t pixel;

  img_orig = (double **)get_img(input_img.width,
                                     input_img.height,
                                     sizeof(double));
  img_filt = (double **)get_img(input_img.width,
                                     input_img.height,
                                     sizeof(double));
  double test_val;
  // Copy all components to respective double array
  for ( cur_row = 0; cur_row < input_img.height; cur_row++ )
  {
    for ( cur_col = 0; cur_col < input_img.width; cur_col++ )
    {
      test_val = input_img.color[color][cur_row][cur_col];
      img_orig[cur_row][cur_col] = input_img.color[color][cur_row][cur_col];
    }
  }

  // Step through rows and columns applying filter
  for ( cur_row = 0; cur_row < input_img.height; cur_row++ )
  {
    for ( cur_col = 0; cur_col < input_img.width; cur_col++ )
    {
      test_val = MultiplyOnePixelEl(input_img.height,
                                                      input_img.width,
                                                      cur_row,cur_col,
                                                      img_orig, filt);
      img_filt[cur_row][cur_col] = MultiplyOnePixelEl(input_img.height,
                                                      input_img.width,
                                                      cur_row,cur_col,
                                                      img_orig, filt);
    }
  }

  for ( cur_row = 0; cur_row < input_img.height; cur_row++ )
  {
      for ( cur_col = 0; cur_col < input_img.width; cur_col++ )
      {
          pixel = (int32_t) img_filt[cur_row][cur_col];
          if (pixel > 255)
          {
              color_img.color[color][cur_row][cur_col] = 255;
          }
          else if (pixel < 0)
          {
              color_img.color[color][cur_row][cur_col] = 0;
          }
          else
          {
              color_img.color[color][cur_row][cur_col] = pixel;
          }
      }
  }

  free_img( (void**)img_orig );
  free_img( (void**)img_filt );
}
