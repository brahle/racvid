#include <algorithm>
#include <functional>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>
#include <vector>

#include <queue>
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

inline bool isValid(Mat frame, int x1, int y1, int x2, int y2) {
  if(x1 >= 0 && y1 >= 0)
    if (x2 <= frame.rows && y2 <= frame.cols)
      return true;
  return false;
}

inline bool isValid(Mat frame, int x, int y) {
  if (x < 0 || y < 0) return false;
  if (x >= frame.rows || y >= frame.cols) return false;
  return true;
}

inline bool isValid(Mat frame, const Rect& r) {
  return isValid(frame, r.y, r.x, r.y+r.height, r.x+r.width);
}

const int MAXN = 1000;
int prec[MAXN][MAXN];

void preprocess(Mat frame) {
  for (int i = 0; i <= max(frame.rows, frame.cols); ++i)
    prec[i][0] = prec[0][i] = 0;

  for (int i = 0; i < frame.rows; ++i)
    for (int j = 0; j < frame.cols; ++j) {
      prec[i+1][j+1] = (frame.at<uchar>(i, j) > 0) + prec[i][j+1] + prec[i+1][j] - prec[i][j];
    }
}

double getAvg(Mat frame, int x1, int y1, int x2, int y2) {
  assert(x1 >= 0 && y1 >= 0);
  assert(x2 <= frame.rows && y2 <= frame.cols);

  // int a = 0, b = 0;
  // for (int x = x1; x < x2; ++x)
  //   for (int y = y1; y < y2; ++y) {
  //     if (frame.at<uchar>(x, y)) {
  //       ++a;
  //     }
  //     ++b;
  //   }

  // assert(b > 0);

  int fast_a = prec[x2][y2] - prec[x1][y2] - prec[x2][y1] + prec[x1][y1];
  int fast_b = (x2-x1)*(y2-y1);

  return fast_a / double(fast_b);
}

double getAvg(Mat frame, const Rect& r) {
  return getAvg(frame, r.y, r.x, r.y+r.height, r.x+r.width);
}

inline bool isCloseEnough(pair<int, int> p, const Rect& roi) {
  int dist_s = 0;
  if (p.second <= roi.x) dist_s = roi.x - p.second;
  if (p.second >= roi.x+roi.width) dist_s = p.second - roi.x - roi.width;
  if (dist_s > 3) return false;

  int dist_r = 0;
  if (p.first <= roi.y) dist_r = roi.y - p.first;
  if (p.first >= roi.y+roi.height) dist_r = p.first - roi.y - roi.height;
  if (dist_r > 100) return false;
  return true;
}

void floodFill(Mat frame, int x, int y, Rect roi) {
  static queue<pair<int, int> > Q;
  const static int numDirs = 8;
  const static int dx[numDirs] = {-2, +2, +2, -2, -1, 0, 1, 0};
  const static int dy[numDirs] = {-2, +2, -2, +2, 0, 1, 0, -1};
  while (!Q.empty()) Q.pop();

  for (Q.push(make_pair(x, y)); !Q.empty(); Q.pop()) {
    pair<int, int> ex = Q.front();
    if (!isCloseEnough(ex, roi)) break;

    frame.at<uchar>(ex.first, ex.second) = 0;
    for (int dir = 0; dir < numDirs; ++dir) {
      pair<int, int> nx = ex;
      nx.first += dx[dir];
      nx.second += dy[dir];
      if (!isValid(frame, nx.first, nx.second)) continue;
      if (!frame.at<uchar>(nx.first, nx.second)) continue;
      frame.at<uchar>(nx.first, nx.second) = 0;
      Q.push(nx);
    }
  }
}

void clearRect(Mat frame, Rect roi) {
  for (int h = 0; h < roi.height; ++h)
    for (int w = 0; w < roi.width; ++w) {
      int r = roi.y + h;
      int s = roi.x + w;
      if (frame.at<uchar>(r, s))
        floodFill(frame, r, s, roi);
    }
}

void detectSize(Mat& frame, vector<Rect>& heads, vector<Rect>& bodies, int headX, int headY, int bodyX, int bodyY) { 
  const double headThresh = 0.75;
  const double bodyThresh = 0.40;
  const double lowerBodyThresh = 0.25;

  for (int r = 0; r < frame.rows - headX+1; ++r)
    for (int s = 0; s < frame.cols - headY+1; ++s) {
      uchar ch = frame.at<uchar>(r, s);
      assert(ch == 0 || ch == 255);

      Rect head(s, r, headY, headX);
      double avgHead = getAvg(frame, head);

      if (avgHead > headThresh) {
        int bestDelta = -1;
        double bestAvgBody = -1e100;

        for (int delta = -10; delta <= 10; ++delta) {
          Rect body(s+delta, r+headX, bodyY, bodyX);
          if (!isValid(frame, body)) continue;
          double avgBody = getAvg(frame, body);
          Rect lowerBody(s+delta, r+headX+bodyX/2, bodyY, bodyX/2);
          double avgLowerBody = getAvg(frame, lowerBody);
          if (avgBody > bodyThresh && avgBody > bestAvgBody && avgLowerBody > lowerBodyThresh) {
            bestAvgBody = avgBody;
            bestDelta = delta;
          }
        }

        if (bestAvgBody > -1e100) {
          heads.push_back(head);
          //          frame(head).setTo(0);

          Rect body(s+bestDelta, r+headX, bodyY, bodyX);
          bodies.push_back(body);
          //          frame(body).setTo(0);

          clearRect(frame, body);
          preprocess(frame);          
        }
      }
    }
  
}

void detect(Mat& frame, vector<Rect>& heads, vector<Rect>& bodies) {
  Mat appended(frame.rows + 130, frame.cols, frame.type());
  appended.setTo(0);
  for (int r = 0; r < frame.rows; ++r)
    for (int s = 0; s < frame.cols; ++s)
      appended.at<uchar>(r, s) = frame.at<uchar>(r, s);
  frame = appended;
  preprocess(frame);

  heads.clear();
  bodies.clear();

  const int headX = 29;
  const int headY = 27;
  const int bodyX = 130;
  const int bodyY = 61;
  //  const double multi = 0.80;

  detectSize(frame, heads, bodies, headX, headY, bodyX, bodyY);
}

inline bool isRowOk(int l1, int r1, int l2, int r2) {
  int intersect = min(r1, r2) - max(l1, l2);
  if (intersect / double(r1-l1) < 0.5)
    return false;
  return true;
}

inline bool isColOk(int l1, int r1, int l2, int r2) {
  const int thresh = 3;
  if (l2-r1 >= 0 && r1-l2 <= thresh) return true;
  if (l1-r2 >= 0 && l1-r2 <= thresh) return true;
  return false;
}

bool isLuggage(Rect r, const vector<Rect>& bodies) {
  const int sizeThresh = 30;
  if (r.height < sizeThresh && r.width < sizeThresh) return false;
  FOR_EACH(it, bodies) {
    if (!isRowOk(r.y, r.y+r.height, it->y, it->y+it->height)) continue;
    if (!isColOk(r.x, r.x+r.width, it->x, it->x+it->width)) continue;
    return true;
  }
  return false;
}

void detectAndAnnotate(Mat orig, Mat frame) {
  Mat tmp = frame.clone();
  vector<Rect> peopleHead, peopleBody; detect(tmp, peopleHead, peopleBody);
  vector<Rect> people;
  FOR_EACH(it, peopleHead) people.push_back(*it);
  FOR_EACH(it, peopleBody) people.push_back(*it);
  if (people.empty()) return;

  vector<vector<Point> > contours;
  findContours(tmp, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

  for (int i = 0; i < (int)people.size(); ++i) {
    Rect r = people[i];
    rectangle(orig, r.tl(), r.br(), cv::Scalar(0, 255, 0), 2);
  }
  FOR_EACH(it, contours) {
    Rect r = boundingRect(*it);
    if (!isLuggage(r, peopleBody)) continue;
    rectangle(orig, r.tl(), r.br(), cv::Scalar(0, 0, 255), 2);
  }
}

int main(int argc, char* argv[]) {
  Config config;
  if (argc >= 2) {
    config.read(argv[1]);
  } else {
    config.read("config.cfg");
  }

  namedWindow("detektor");
  cvMoveWindow("detektor", 10, 10);
  namedWindow("silueta");
  cvMoveWindow("silueta", 800, 10);

  for (int ii = 0; ii < config.getN(); ++ii) {
    Mat frame = imread(config.getNthName(ii).c_str());
    Mat foregroundMask = imread(config.getNthCacheName(ii).c_str(), CV_LOAD_IMAGE_GRAYSCALE);
    threshold(foregroundMask, foregroundMask, 100, 255, THRESH_BINARY);

    detectAndAnnotate(frame, foregroundMask);

    imshow("detektor", frame);
    imshow("silueta", foregroundMask);

    int key = waitKey(30);
    if (key == 27) break;
    if (key == 's') {
      static int counter = 1;
      assert(imwrite(config.getNthOutputName(counter).c_str(), frame));
      ++counter;
    }
  }
  

  return 0;
}
