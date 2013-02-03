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

#include "config.hpp"

using namespace std;
using namespace cv;

#define TRACE(x) cout << #x " = " << x << endl
#define FOR_EACH(it, V) for(__typeof((V).begin()) it = (V).begin(); it != (V).end(); ++it)

class MyBackgroundSubtractor : public cv::BackgroundSubtractorMOG2 {
public:
  MyBackgroundSubtractor() : cv::BackgroundSubtractorMOG2() {
    nShadowDetection = 0;
    bShadowDetection = true;
  }
};

int main(int argc, char* argv[]) {
  Config config;
  if (argc >= 2) {
    config.read(argv[1]);
  } else {
    config.read("config.cfg");
  }

  namedWindow("A");
  cvMoveWindow("A", 10, 10);
  namedWindow("B");
  cvMoveWindow("B", 800, 10);

  MyBackgroundSubtractor bg;

  vector<Mat> frames;
  Mat background;
  Mat frame;

  for (int ii = 0; ii < config.getN(); ++ii) {
    frame = imread(config.getNthName(ii).c_str());
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
