#include <algorithm>
#include <functional>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>
#include <vector>

#include <iostream>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

#define TRACE(x) cout << #x " = " << x << endl
#define FOR_EACH(it, V) for(__typeof((V).begin()) it = (V).begin(); it != (V).end(); ++it)

Mat GrayToDummyRgb(Mat gray) {
  Mat ret(gray.size(), CV_8UC3);
  for (int i = 0; i < gray.rows; ++i)
    for (int j = 0; j < gray.cols; ++j) {
      Vec3b& vec = ret.at<Vec3b>(i, j);
      uchar val = gray.at<uchar>(i, j);
      vec[0] = val;
      vec[1] = val;
      vec[2] = val;
    }
  return ret;
}

int main(int argc, char* argv[])
{
  assert(argc > 1);

  namedWindow("A");
  cvMoveWindow("A", 10, 10);
  namedWindow("B");
  cvMoveWindow("B", 800, 10);

  //  namedWindow("back");
  //  cvMoveWindow("back", 1600, 10);

  BackgroundSubtractorMOG2 bg;
  bg.nShadowDetection = 0; // pobojaj sjene u crno
  bg.bShadowDetection = true;

  vector<Mat> frames;
  Mat background;
  Mat frame;

  for (int arg = 700; arg < argc; ++arg) {
    if (arg % 100 == 0) {
      printf("Img %d/%d\n", arg, argc);
    }

    frame = imread(argv[arg]);
    Mat orig = frame.clone();

    bilateralFilter(orig, frame, 4, 20, 10);
    GaussianBlur(frame, frame, Size(9, 9), 3.0);

    Mat foregroundMask;
    bg(frame, foregroundMask);
    bg.getBackgroundImage(background);

    erode(foregroundMask, foregroundMask, Mat(), Point(-1, -1), 1);
    dilate(foregroundMask, foregroundMask, Mat(), Point(-1, -1), 1);

    frame = orig.clone();
    for (int i = 0; i < foregroundMask.rows; ++i)
      for (int j = 0; j < foregroundMask.cols; ++j)
        if (!foregroundMask.at<char>(i, j))
          frame.at<Vec3b>(i, j) = 0;

    imshow("A", orig);
    imshow("B", frame);
    //    imshow("back", background);
    
    if (waitKey(30) == 27) break;
  }

  imwrite("background.jpeg", background);  
  imwrite("frame.jpeg", frame);
  return 0;
}
