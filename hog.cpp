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

  HOGDescriptor hog;
  Mat orig = imread(config.getNthName(800));

  Mat img = orig.clone();
  vector<Rect> found;
  hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
  hog.detectMultiScale(img, found, 0, Size(), Size(), 1.05);
  
  TRACE(found.size());

  for (int i=0; i<(int)found.size(); i++) {
    Rect r = found[i];
    rectangle(img, r.tl(), r.br(), cv::Scalar(0, 255, 0), 2);
  }
  imshow("hog", img);
  imwrite("hog.jpeg", img);
  waitKey(-1);

  return 0;
}
