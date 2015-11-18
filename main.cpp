#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

using namespace cv;
using namespace std;

Mat src; Mat src_gray;
int thresh = 100;
int max_thresh = 255;
RNG rng(12345);

Mat keyboard_screen;

VideoCapture capture;
char* source_window1 = "Camera";
char* source_window2 = "Keyboard";

float wid_scr, hei_scr;
float wid_key = 200.0f, hei_key = 200.0f;

/// Function header

void detectAndDisplay(Mat src);
void average_contour(vector<Point> contour, Point2f& p);
void displayKeyboard(Point2f& p);
float wider_angle(float alpha_i, int _x, int _y);

/** @function main */
int main( int argc, char** argv )
{
  keyboard_screen = imread("keyboard.png");
  /// Load source image and convert it to gray
  capture.open( -1 );
  if ( ! capture.isOpened() ) { printf("--(!)Error opening video capture\n"); return -1; }
  waitKey(100);

  wid_scr = capture.get(CV_CAP_PROP_FRAME_WIDTH);
  hei_scr = capture.get(CV_CAP_PROP_FRAME_HEIGHT);

  wid_key = keyboard_screen.cols;
  hei_key = keyboard_screen.rows;
  // capture.read(src);
  // cvtColor( src, src_gray, CV_BGR2GRAY );
  // blur( src_gray, src_gray, Size(3,3) );

  /// Create Window

  while (  capture.read(src) )
  {
      if( src.empty() )
      {
          printf(" --(!) No captured frame -- Break!");
          break;
      }
      //-- 3. Apply the classifier to the frame
      detectAndDisplay( src );
      int c = waitKey(30);
      if( (char)c == 27 ) { break; } // escape
  }

  return 0;
}

void detectAndDisplay(Mat src)
{
  Mat hsv_image;
  cvtColor(src, hsv_image, cv::COLOR_BGR2HSV);
  Scalar color = Scalar(0, 0, 255);
  Scalar color2 = Scalar(0, 255, 0);

  Mat lower_color_hue_range;
  Mat upper_color_hue_range;
  inRange(hsv_image, Scalar(20, 100, 100), Scalar(30, 255, 255), lower_color_hue_range);
  //inRange(hsv_image, Scalar(55, 100, 100), Scalar(65, 255, 255), upper_color_hue_range);

  Mat color_hue_image;
  color_hue_image = lower_color_hue_range.clone();

  //circle(color_hue_image, Point(30, 30), 10, color);
  //addWeighted(lower_color_hue_range, 1.0, upper_color_hue_range, 1.0, 0.0, color_hue_image);
  //GaussianBlur(color_hue_image, color_hue_image, cv::Size(9, 9), 2, 2);

  Mat canny_output;
  int thresh = 100;
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;

  findContours(color_hue_image.clone(), contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

  int max_area = 0;
  int max_contour;

  for(int i=0; i < contours.size(); i++)
  {
    double area = contourArea(contours[i]);
    if (area > max_area)
    {
      max_area = area;
      max_contour = i;
    }
  }

  //printf("MAXAREA: %d, MAXCONTOUR: %d\n", max_area, max_contour);
  drawContours(src, contours, max_contour, color, 5, 8, hierarchy);

  Point2f middle_point;
  average_contour(contours[max_contour], middle_point);

  circle(src, middle_point, 10, color2, -1);

  //imshow(source_window1, color_hue_image);
  //imshow(source_window2, upper_color_hue_range);

  displayKeyboard(middle_point);
  imshow(source_window1, src);


}

void average_contour(vector<Point> contour, Point2f& p)
{
  p.x = 0;
  p.y = 0;

  for(int i=0; i < contour.size(); i++)
  {
    p.x += contour[i].x;
    p.y += contour[i].y;
  }

  p.x /= contour.size();
  p.y /= contour.size();
}

void displayKeyboard(Point2f& p)
{
  int _x = p.x - wid_scr / 2;
  int _y = hei_scr / 2 - p.y;

  int _r = sqrt(_x * _x + _y * _y);

  float alpha_i = asin(abs(_y) / (float) _r);
  float alpha = wider_angle(alpha_i, _x, _y);

  printf("DEBUG %f %f\n", alpha_i, alpha);

  float R = 0.4 * min(wid_key, hei_key);

  int _x_key = R * cos(alpha);
  int _y_key = R * sin(alpha);

  int x_key = _x_key + wid_key / 2;
  int y_key = hei_key / 2 - _y_key;

  Mat keyboard_screen_view = keyboard_screen.clone();
  Point touch_point(x_key, y_key);
  Point middle_screen(wid_key/2, hei_key/2);
  Scalar color = Scalar(0, 0, 255);

  line(keyboard_screen_view, touch_point, middle_screen, color, 5);

  imshow(source_window2, keyboard_screen_view);
}

float wider_angle(float alpha_i, int _x, int _y)
{
  float result;
  if (_x >= 0 && _y >= 0) result = alpha_i;
  if (_x < 0 && _y >= 0)  result = M_PI - alpha_i;
  if (_x < 0 && _y < 0)   result = M_PI + alpha_i;
  if (_x >= 0 && _y < 0)  result = 2*M_PI - alpha_i;

  return result;
}

