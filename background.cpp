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

  namedWindow("filter");
  cvMoveWindow("filter", 10, 10);
  namedWindow("fore");
  cvMoveWindow("fore", 800, 10);

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

    Mat foreground;
    bg(frame, foreground);
    bg.getBackgroundImage(background);

    erode(foreground, foreground, Mat(), Point(-1, -1), 1);
    dilate(foreground, foreground, Mat(), Point(-1, -1), 1);

    for (int i = 0; i < foreground.rows; ++i)
      for (int j = 0; j < foreground.cols; ++j)
        if (!foreground.at<char>(i, j))
          frame.at<Vec3b>(i, j) = 0;

    // maskirani frame
    Mat tmp = frame.clone();

    cvtColor(tmp, tmp, CV_RGB2GRAY);
    erode(tmp, tmp, Mat());
    threshold(tmp, tmp, 1, 255, THRESH_BINARY);

    //vector<vector<Point> > contours;
    //findContours(tmp, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
    //drawContours(frame, contours, -1, 255, 3);

    // SimpleBlobDetector::Params params;
    // params.minThreshold = 1;
    // params.maxThreshold = 100; // ?
    // params.filterByColor = false;
    // params.filterByArea = false;
    // params.filterByCircularity = false;
    // params.filterByInertia = false;
    // params.filterByConvexity = false;
    // SimpleBlobDetector blobs(params); // http://docs.opencv.org/modules/features2d/doc/common_interfaces_of_feature_detectors.html#SimpleBlobDetector : public FeatureDetector

    // vector<KeyPoint> keypoints;
    // blobs.detect(frame, keypoints);
    // cv::drawKeypoints(frame, keypoints, frame);

    //    imshow("orig", orig);
    imshow("filter", orig);
    imshow("fore", frame);
    //    imshow("back", background);
    
    if (waitKey(30) == 27) break;
  }

  imwrite("background.jpeg", background);  
  imwrite("frame.jpeg", frame);
  return 0;
}
