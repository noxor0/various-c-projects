#include <stdio.h>
#include <string.h>

#define HEADER_SIZE 54
#define BMP_EXT ".bmp"
#define TIGER_FILE "tiger"
#define TIGER_HEIGHT 240
#define TIGER_WIDTH 320

unsigned char bgrvalue(unsigned char current);
void colorinversion(FILE *file_ptr, int rgb_width, int height, unsigned char original_pixels[height][rgb_width], int pixels_size, char *header_arr);
void contrast(FILE *file_ptr, int rgb_width, int height, unsigned char original_pixels[height][rgb_width], int pixels_size, char *header_arr);
void mirrorflip(FILE *file_ptr, int rgb_width, int height, unsigned char original_pixels[height][rgb_width], int pixels_size, char *header_arr);
void scaledown(FILE *file_ptr, int rgb_width, int height, unsigned char original_pixels[height][rgb_width], int pixels_size, char *header_arr);

int main() {
  int debug = 1;

  char filename[] = TIGER_FILE;
  char extension[5] = BMP_EXT;
  int height = TIGER_HEIGHT;
  int width = TIGER_WIDTH;

  if (!debug) {
    printf("Enter the filename: ");
    scanf("%19s", filename);
    printf("\nEnter the height and width (in pixels): ");
    scanf("%d %d", &height, &width);
  }
  strcat(filename,extension);

  FILE *infile = fopen(filename, "rb");
  FILE *inversion_f = fopen("inversion.bmp", "wb");
  FILE *constrast_f = fopen("constrast.bmp", "wb");
  FILE *mirrorflip_f = fopen("mirrorflip.bmp", "wb");
  FILE *scaledown_f = fopen("scaledown.bmp", "wb");

  char header[HEADER_SIZE];
  int rgb_width = width * 3;

  unsigned char pixels[height][rgb_width];
  int pixels_size = sizeof(pixels);

  fread(header, 1, HEADER_SIZE, infile);
  fread(pixels, 1, height * rgb_width, infile);

  colorinversion(inversion_f, rgb_width, height, pixels, pixels_size, &header[0]);
  contrast      (constrast_f, rgb_width, height, pixels, pixels_size, &header[0]);
  mirrorflip    (mirrorflip_f, rgb_width, height, pixels, pixels_size, &header[0]);
  scaledown     (scaledown_f, rgb_width, height, pixels, pixels_size, &header[0]);

  fclose(infile);
  return 0;
}

// Color inversion method
void colorinversion(FILE *file_ptr, int rgb_width, int height, unsigned char original_pixels[height][rgb_width], int pixels_size, char *header_arr){
  unsigned char color_array[height][rgb_width];
  memcpy(color_array, original_pixels, pixels_size);

  int row_index, col_index;
  for(row_index = 0; row_index < height; row_index++) {
    for(col_index = 0; col_index < rgb_width; col_index++) {
      unsigned char temp = color_array[row_index][col_index];
      temp = ~temp;
      color_array[row_index][col_index] = temp;
    }
  }
  fwrite(header_arr, sizeof(char), HEADER_SIZE, file_ptr);
  fwrite(color_array, sizeof(char), height * rgb_width, file_ptr);
  fclose(file_ptr);
}

// Contrast method
void contrast(FILE *file_ptr, int rgb_width, int height, unsigned char original_pixels[height][rgb_width], int pixels_size, char *header_arr) {
  unsigned char contrast_array[height][rgb_width];
  memcpy(contrast_array, original_pixels, pixels_size);
  char temp_array[HEADER_SIZE];
  memcpy(temp_array, header_arr, HEADER_SIZE);

  int col_index, row_index;
  unsigned char blue, green, red;

  for(row_index = 0; row_index < height; row_index++) {
    for(col_index = 0; col_index < rgb_width; col_index+=3) {
        blue = contrast_array[row_index][col_index];
        contrast_array[row_index][col_index] = bgrvalue(blue);

        green = contrast_array[row_index][col_index+ 1];
        contrast_array[row_index][col_index + 1] = bgrvalue(green);

        red =  contrast_array[row_index][col_index+ 2];
        contrast_array[row_index][col_index + 2] = bgrvalue(red);
    }
  }

  fwrite(temp_array, sizeof(char), HEADER_SIZE, file_ptr);
  fwrite(contrast_array, sizeof(char), height * rgb_width, file_ptr);
  fclose(file_ptr);
}

// Helper method to find contrast
unsigned char bgrvalue(unsigned char current) {
  float ratio = 2.9695;
  int bgr_int;
  bgr_int = current - 128;
  bgr_int *= ratio;
  bgr_int += 128;

  if(bgr_int > 255) {
    bgr_int = 255;
  } else if (bgr_int < 0) {
    bgr_int = 0;
  }
  return (unsigned char) bgr_int;
}

// Mirrors and flips the image
void mirrorflip(FILE *file_ptr, int rgb_width, int height, unsigned char original_pixels[height][rgb_width], int pixels_size, char *header_arr) {
  unsigned char mirror_array[height][rgb_width];
  memcpy(mirror_array, original_pixels, pixels_size);

  char temp_array[HEADER_SIZE];
  memcpy(temp_array, header_arr, HEADER_SIZE);

  // Will always be whole number
  int pixel_count = rgb_width / 3;
  int start_pix = pixel_count / 2;
  if(start_pix % 2 == 1) {
    start_pix++;
  }

  int row_index, col_index, rgb_index;
  for(row_index = 0; row_index < height; row_index++) {
    for(col_index = start_pix * 3; col_index < rgb_width; col_index+=3) {
      int blue_val = rgb_width - col_index - 3;
      int mirror_corrector = 0;
      for(rgb_index = blue_val; rgb_index < blue_val + 3; rgb_index++){
        mirror_array[row_index][col_index + mirror_corrector] = mirror_array[row_index][rgb_index];
        mirror_corrector++;
      }
    }
  }

  int opposite_row = 0;
  for(row_index = height - 1; row_index >= height/2; row_index--) {
    for(col_index = 0; col_index < rgb_width; col_index++) {
        int temp = mirror_array[row_index][col_index];
        mirror_array[row_index][col_index] = mirror_array[opposite_row][col_index];
        mirror_array[opposite_row][col_index] = temp;
    }
    opposite_row++;
  }

  fwrite(temp_array, sizeof(char), HEADER_SIZE, file_ptr);
  fwrite(mirror_array, sizeof(char), height * rgb_width, file_ptr);
  fclose(file_ptr);
}

// Scales down image and sorts out R, G, B copies.
void scaledown(FILE *file_ptr, int rgb_width, int height, unsigned char original_pixels[height][rgb_width], int pixels_size, char *header_arr){
  char temp_array[HEADER_SIZE];
  memcpy(temp_array, header_arr, HEADER_SIZE);

  unsigned char scaledown_array[height][rgb_width];
  memcpy(scaledown_array, original_pixels, pixels_size);

  int col_index, row_index;
  int rgb_width_half = rgb_width / 2;
  int height_half = height / 2;
  int pix_width = rgb_width / 3;
  int pix_proc = 0;
  int curr_altered_row = 0;
  int curr_altered_col = 0;

  for(row_index = 0; row_index < height; row_index++) {
    for(col_index = 0; col_index < rgb_width; col_index+=3) {
      if (row_index % 2 == 1 && col_index % 2 == 1) {
        // Entire picture
        scaledown_array[curr_altered_row][rgb_width_half + curr_altered_col] = original_pixels[row_index][col_index];
        scaledown_array[curr_altered_row][rgb_width_half + curr_altered_col + 1] = original_pixels[row_index][col_index + 1];
        scaledown_array[curr_altered_row][rgb_width_half + curr_altered_col + 2] = original_pixels[row_index][col_index + 2];
        // B
        scaledown_array[curr_altered_row][curr_altered_col] = original_pixels[row_index][col_index];
        scaledown_array[curr_altered_row][curr_altered_col + 1] = 0;
        scaledown_array[curr_altered_row][curr_altered_col + 2] = 0;
        // G
        scaledown_array[curr_altered_row + height_half][rgb_width_half + curr_altered_col] = 0;
        scaledown_array[curr_altered_row + height_half][rgb_width_half + curr_altered_col + 1] = original_pixels[row_index][col_index + 1];
        scaledown_array[curr_altered_row + height_half][rgb_width_half + curr_altered_col + 2] = 0;
        // R
        scaledown_array[curr_altered_row + height_half][curr_altered_col] = 0;
        scaledown_array[curr_altered_row + height_half][curr_altered_col + 1] = 0;
        scaledown_array[curr_altered_row + height_half][curr_altered_col + 2] = original_pixels[row_index][col_index + 2];

        pix_proc++;
        curr_altered_row = (pix_proc*2) / pix_width;
        curr_altered_col = (pix_proc*3 % rgb_width_half);
      }
    }
  }

  fwrite(temp_array, sizeof(char), HEADER_SIZE, file_ptr);
  fwrite(scaledown_array, sizeof(char), height * rgb_width, file_ptr);
  fclose(file_ptr);
}
