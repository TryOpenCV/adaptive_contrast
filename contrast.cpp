#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <cmath>
#include <limits>

using namespace cv;
using namespace std;

int compare_ints(const void* a, const void* b)   // comparison function
{
  int arg1 = *reinterpret_cast<const int*>(a);
  int arg2 = *reinterpret_cast<const int*>(b);
  if(arg1 < arg2) return -1;
  if(arg1 > arg2) return 1;
  return 0;
}

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

double gain_contrast_local(double c, double nz)
{
  return pow(c, nz);
}

double gain_contrast_std(double c, double std_area, double std_min, double std_max, double a_min, double a_max)
{
  return pow(c, a_min + (a_max - a_min)*((std_area - std_min) / float(std_max - std_min)));
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

int find_l_min(int area[], int size)
{
  return area[0];
}

int find_l_max(int area[], int size)
{
  return area[size-1];
}

int find_h_max(int area[], int size)
{
  int result = 0;
  int k = 0;
  int uniq[size];
  for (int i = 0; i < size; i++)
  {
    if (area[i] != area[i-1])
    {
      uniq[k++] = area[i];
    }
  }
  int hist_size = k;
  int hist[hist_size];
  for (int i = 0; i < size; i++)
  {
    for (int k = 0; k < size; k++)
    {
      if (area[i] == uniq[k])
      {
        hist[k] += 1;
      }
    }
  }
  qsort(hist, hist_size, sizeof(int), compare_ints);
  return hist[hist_size-1];
}

double find_h_z(double l_max, double l_min, double h_max)
{
  return (l_max - l_min)/float(h_max);
}

double gain_contrast_hist(double c, double h_z, double a)
{
  return 1-exp(-(pow(h_z - a, 2)/0.0392));
}

int hist_l(int pixel, int area[], int size)
{
  int result = 0;
  for (int i = 0; i < size; i++)
  {
    if (pixel == area[i]) result++;
  }
  return result;
}

double get_entropy(int area[], int size)
{
  double sum = 0;
  for (int i = 0; i < size; i++)
  {
    double p = hist_l(area[i], area, size);
    sum += p*log2(p/size);
  }
  return -(sum/size)/(log2(size));
}

double find_min_max_entropy(Mat image, int area_size[4], double &entropy_min, double &entropy_max)
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

      double entropy = get_entropy(area, area_size);

      if (entropy < entropy_min)
      {
        entropy_min = entropy;
      }

      if (entropy > entropy_max)
      {
        entropy_max = entropy;
      }
    }
  }
}

double gain_contrast_entropy(double c, double p, double entropy_min, double entropy_max)
{
  return (p - entropy_min)/(entropy_max - entropy_min);
}

int main( int argc, char** argv )
{
  if( argc != 10)
  {
    cout <<" Usage: ./contrast image_path n1 n2 m1 m2 algorithm_type a_min a_max a" << endl;
    // ./contrast lenna.bmp 2 2 2 2 0.8 0.13 0.67 0.5
    // algorithm_types: std, entropy, hist, 0.1 ... 1.0
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

  double entropy_min = std::numeric_limits<float>::max();
  double entropy_max = std::numeric_limits<float>::min();

  double a_min = atof(argv[7]);
  double a_max = atof(argv[8]);
  double a = atof(argv[9]);

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
    find_min_max_entropy(image, area_size, entropy_min, entropy_max);
  }
  else
  {
    nz = atof(algorithm_type);
  }

  int width = image.rows;
  int height = image.cols;

  image_new = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE); // Create image copy

  for (int i = 0; i < width; i++)
  {
    for (int j = 0; j < height; j++)
    {
      Scalar pixel = image.at<uchar>(i,j);
      int pixel_brightness = pixel.val[0];

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

      qsort(area, area_size, sizeof(int), compare_ints);

      int zc = average_brightness(area, area_size);
      double c = contrast(pixel_brightness, zc);

      double gained_contrast = 0;

      if (strcmp(algorithm_type, "std") == 0)
      {
        double std_area = standart_deviation(area, area_size);
        gained_contrast = gain_contrast_std(c, std_area, std_min, std_max, a_min, a_max);
      }
      else if (strcmp(algorithm_type, "hist") == 0)
      {
        double l_max = find_l_max(area, area_size);
        double l_min = find_l_min(area, area_size);
        double h_max = find_h_max(area, area_size);
        double h_z = find_h_z(l_max, l_min, h_max);
        gained_contrast = gain_contrast_hist(c, h_z, a);
      }
      else if (strcmp(algorithm_type, "entropy") == 0)
      {
        double p = get_entropy(area, area_size);
        gained_contrast = gain_contrast_entropy(c, p, entropy_min, entropy_max);
      }
      else
      {
        gained_contrast = gain_contrast_local(c, nz);
      }

      int new_pixel = get_new_pixel(pixel_brightness, zc, gained_contrast);
      image_new.at<uchar>(i,j) = new_pixel;
    }
  }

  namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
  imshow( "Display window", image_new );                   // Show our image inside it.

  waitKey(0);                                          // Wait for a keystroke in the window
  return 0;
}
