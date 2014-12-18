#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main( int argc, char** argv )
{
  if( argc != 2)
  {
    cout <<" Usage: contrast Image" << endl;
    return -1;
  }

  Mat image;
  Mat image_new;
  image = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);   // Read the file

  if(! image.data )                              // Check for invalid input
  {
    cout <<  "Could not open or find the image" << std::endl ;
    return -1;
  }

  int width = image.rows;
  int height = image.cols;

  std::cout << width << " " << height << endl;

  image_new = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE); // Create image copy

  for(int i = 0; i < width; i++)
  {
    for(int j = 0; j < height; j++)
    {
      Scalar pixel = image.at<uchar>(j, i);
      std::cout << pixel.val[0] << endl;
    }
  }

  namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
  imshow( "Display window", image );                   // Show our image inside it.

  waitKey(0);                                          // Wait for a keystroke in the window
  return 0;
}
