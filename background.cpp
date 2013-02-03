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

const int start = 895;
const int skip = 50;


int main(int argc, char* argv[]) {
  Config config;
  if (argc >= 2) {
    config.read(argv[1]);
  } else {
    config.read("config.cfg");
  }

//  namedWindow("A");
//  cvMoveWindow("A", 10, 10);
//  namedWindow("B");
//  cvMoveWindow("B", 800, 10);

  MyBackgroundSubtractor bg;
  HOGDescriptor hog;
  hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

  vector<Mat> frames;
  Mat background;
  Mat frame;
  Mat foreground;

  for (int ii = start; ii < config.getN(); ++ii) {
    cout << "Frame: " << ii-start+1 << "; Image: " << ii << endl;
    frame = imread(config.getNthName(ii).c_str());
    Mat orig = frame.clone();

    bilateralFilter(orig, frame, 4, 20, 10);
    GaussianBlur(frame, frame, Size(9, 9), 3.0);

    Mat foregroundMask;
    bg(frame, foregroundMask);
    bg.getBackgroundImage(background);

    erode(foregroundMask, foregroundMask, Mat(), Point(-1, -1), 1);
    dilate(foregroundMask, foregroundMask, Mat(), Point(-1, -1), 1);
    if (ii - start < skip) continue;

    frame = orig.clone();
    for (int i = 0; i < foregroundMask.rows; ++i)
      for (int j = 0; j < foregroundMask.cols; ++j)
        if (!foregroundMask.at<char>(i, j))
          frame.at<Vec3b>(i, j) = 0;

    foreground = frame.clone();
    for (int i = 0; i < foregroundMask.rows; ++i) {
      for (int j = 0; j < foregroundMask.cols; ++j) {
        if (foregroundMask.at<char>(i, j)) {
          for (int k = max(i-8, 0); k < min(i+9, foregroundMask.rows); ++k) {
            for (int l = max(j-15, 0); l < min(j+16, foregroundMask.cols); ++l) {
              foreground.at<Vec3b>(k, l) = orig.at<Vec3b>(k, l);
            }
          }
        }
      }
    }


    Mat img = orig.clone();
    vector<Rect> found, found_filtered;
    hog.detectMultiScale(img, found, 0, Size(), Size(), 1.05, 2);

    size_t i, j;
    for (i=0; i<found.size(); i++) {
      Rect r = found[i];
      for (j=0; j<found.size(); j++)
        if (j!=i && (r & found[j])==r)
          break;
      if (j==found.size())
        found_filtered.push_back(r);
    }
    for (i=0; i<found_filtered.size(); i++) {
      Rect r = found_filtered[i];
	    rectangle(img, r.tl(), r.br(), cv::Scalar(0,255,0), 2);
    }


    hog.detectMultiScale(foreground, found, 0, Size(), Size(), 1.05, 2);
    found_filtered.clear();
    for (i=0; i<found.size(); i++) {
      Rect r = found[i];
      for (j=0; j<found.size(); j++)
        if (j!=i && (r & found[j])==r)
          break;
      if (j==found.size())
        found_filtered.push_back(r);
    }
    for (i=0; i<found_filtered.size(); i++) {
      Rect r = found_filtered[i];
	    rectangle(img, r.tl(), r.br(), cv::Scalar(255,0,0), 2);
    }

    imshow("video capture", img);
    imwrite(config.getNthOutputName(ii).c_str(), img);
    imshow("foreground", foreground);

    //    imshow("A", orig);
    //    imshow("B", frame);
    //    imshow("C", foregroundMask);
    //    imshow("back", background);

    if (waitKey(1) == 27) break;
  }

  imwrite("background.jpeg", background);
  imwrite("frame.jpeg", frame);
  return 0;
}
