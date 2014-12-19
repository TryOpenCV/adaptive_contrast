#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <cmath>
#include <limits>

using namespace cv;
using namespace std;

int average_brightness(int area[], int len)
{
  int sum = 0;
  for (int i = 0; i < len; i++)
  {
    sum += area[i];
  }
  return round(sum/float(len));
}

double contrast(int z, int zc)
{
  return abs(z-zc)/float(z+zc);
}

double gain_contrast(double c, char algorithm_type[], double nz, double std_area, double std_min, double std_max, double a_min, double a_max)
{
  if (strcmp(algorithm_type, "std") == 0)
  {
    return pow(c, a_min + (a_max - a_min)*((std_area - std_min) / float(std_max - std_min)));
  }
  else if (strcmp(algorithm_type, "entropy") == 0)
  {

  }
  else
  {
    return pow(c, nz);
  }
}

int get_new_pixel(int z, int zc, double c)
{
  if (z < zc)
  {
    return int(zc*(1-c)/float(1+c));
  }
  else
  {
    return int(zc*(1+c)/float(1-c));
  }
}

double standart_deviation(int area[], int len)
{
  int average = average_brightness(area, len);
  double sum = 0;
  for (int i = 0; i < len; i++)
  {
    sum += pow(area[i] - average, 2);
  }
  return sqrt(sum/float(len));
}

void find_min_max_std(Mat image, int area_size[4], double &std_min, double &std_max)
{
  int width = image.rows;
  int height = image.cols;

  int n1 = area_size[0], n2 = area_size[1];
  int m1 = area_size[2], m2 = area_size[3];

  for (int i = 0; i < width; i++)
  {
    for (int j = 0; j < height; j++)
    {
      int i_min = i-n1, i_max = i+n2, j_min = j-m1, j_max = j+m2;

      if (i_min < 0)
      {
        i_min = 0;
      }
      if (i_max > width)
      {
        i_max = width;
      }
      if (j_min < 0)
      {
        j_min = 0;
      }
      if (j_max > height)
      {
        j_max = height;
      }

      int area_size = (i_max - i_min)*(j_max - j_min);
      int area[area_size];

      int k = 0;
      for (int x = i_min; x < i_max; x++)
      {
        for (int y = j_min; y < j_max; y++)
        {
          Scalar area_pixel = image.at<uchar>(x,y);
          area[k++] = area_pixel.val[0];
        }
      }

      double std_area = standart_deviation(area, area_size);

      if (std_area < std_min)
      {
        std_min = std_area;
      }

      if (std_area > std_max)
      {
        std_max = std_area;
      }
    }
  }
}

int main( int argc, char** argv )
{
  if( argc != 9)
  {
    cout <<" Usage: ./contrast image_path n1 n2 m1 m2 algorithm_type a_min a_max" << endl;
    // ./contrast lenna.bmp 2 2 2 2 0.8 0.13 0.67
    return -1;
  }

  Mat image;
  Mat image_new;
  image = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);   // Read the file

  int n1 = atoi(argv[2]);
  int n2 = atoi(argv[3]);
  int m1 = atoi(argv[4]);
  int m2 = atoi(argv[5]);

  int area_size[4] = { n1, n2, m1, m2 };

  char algorithm_type[255];
  strcpy(algorithm_type, argv[6]);
  double nz = 0;

  double std_min = std::numeric_limits<float>::max();
  double std_max = std::numeric_limits<float>::min();

  double a_min = atof(argv[7]);
  double a_max = atof(argv[8]);

  if(! image.data )                              // Check for invalid input
  {
    cout <<  "Could not open or find the image" << std::endl ;
    return -1;
  }

  if (strcmp(algorithm_type, "std") == 0)
  {
    find_min_max_std(image, area_size, std_min, std_max);
  }
  else if (strcmp(algorithm_type, "entropy") == 0)
  {

  }
  else
  {
    nz = atof(algorithm_type);
  }

  int width = image.rows;
  int height = image.cols;

  //std::cout << width << " " << height << endl;

  image_new = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE); // Create image copy

  for (int i = 0; i < width; i++)
  {
    for (int j = 0; j < height; j++)
    {
      Scalar pixel = image.at<uchar>(i,j);
      int pixel_brightness = pixel.val[0];
      //std::cout << pixel_brightness << endl;

      int i_min = i-n1, i_max = i+n2, j_min = j-m1, j_max = j+m2;

      if (i_min < 0)
      {
        i_min = 0;
      }
      if (i_max > width)
      {
        i_max = width;
      }
      if (j_min < 0)
      {
        j_min = 0;
      }
      if (j_max > height)
      {
        j_max = height;
      }

      int area_size = (i_max - i_min)*(j_max - j_min);
      int area[area_size];

      int k = 0;
      for (int x = i_min; x < i_max; x++)
      {
        for (int y = j_min; y < j_max; y++)
        {
          Scalar area_pixel = image.at<uchar>(x,y);
          area[k++] = area_pixel.val[0];
        }
      }

      int zc = average_brightness(area, area_size);
      double c = contrast(pixel_brightness, zc);

      double std_area = 0;
      if (strcmp(algorithm_type, "std") == 0)
      {
        std_area = standart_deviation(area, area_size);
      }

      double gained_contrast = gain_contrast(c, algorithm_type, nz, std_area, std_min, std_max, a_min, a_max);
      int new_pixel = get_new_pixel(pixel_brightness, zc, gained_contrast);
      image_new.at<uchar>(i,j) = new_pixel;
    }
  }

  namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
  imshow( "Display window", image_new );                   // Show our image inside it.

  waitKey(0);                                          // Wait for a keystroke in the window
  return 0;
}
