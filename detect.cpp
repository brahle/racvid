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

void floodFill(Mat frame, int x, int y) {
  static queue<pair<int, int> > Q;
  const static int dx[4] = {-1, 0, 1, 0};
  const static int dy[4] = {0, 1, 0, -1};
  while (!Q.empty()) Q.pop();

  for (Q.push(make_pair(x, y)); !Q.empty(); Q.pop()) {
    pair<int, int> ex = Q.front();
    frame.at<uchar>(ex.first, ex.second) = 0;
    for (int dir = 0; dir < 4; ++dir) {
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

void detect(Mat frame, vector<Rect>& found) {
  const int headX = 29;
  const int headY = 27;
  const double headThresh = 0.75;

  const int bodyX = 130;
  const int bodyY = 51;
  const double bodyThresh = 0.25;

  preprocess(frame);

  found.clear();
  for (int r = 0; r < frame.rows - headX+1; ++r)
    for (int s = 0; s < frame.cols - headY+1; ++s) {
      uchar ch = frame.at<uchar>(r, s);
      assert(ch == 0 || ch == 255);

      Rect head(s, r, headY, headX);
      double avgHead = getAvg(frame, head);

      if (avgHead > headThresh) {
        int bestDelta;
        double bestAvgBody = -1e100;

        for (int delta = -10; delta <= 10; ++delta) {
          Rect body(s+delta, r+headX, bodyY, bodyX);
          if (!isValid(frame, body)) continue;
          double avgBody = getAvg(frame, body);
          if (avgBody > bodyThresh && avgBody > bestAvgBody) {
            bestAvgBody = avgBody;
            bestDelta = delta;
          }
        }

        if (bestAvgBody > -1e100) {
          Rect body(s+bestDelta, r+headX, bodyY, bodyX);
          found.push_back(head);
          //          frame(head).setTo(0);

          found.push_back(body);
          //          frame(body).setTo(0);

          floodFill(frame, r+headX/2, s+headY/2);
          preprocess(frame);          
        }
      }
    }
}

int main(int argc, char* argv[])
{
  namedWindow("A");
  cvMoveWindow("A", 10, 10);
  namedWindow("B");
  cvMoveWindow("B", 800, 10);

  // ii = 1?
  for (int ii = 1400; ii < argc; ++ii) {
    Mat frame = imread(argv[ii], CV_LOAD_IMAGE_GRAYSCALE);
    threshold(frame, frame, 100, 255, THRESH_BINARY);
    imshow("A", frame);

    Mat tmp = frame.clone();
    vector<Rect> found; detect(tmp, found);

    cvtColor(frame, frame, CV_GRAY2BGR);
    for (int i = 0; i < (int)found.size(); ++i) {
      Rect r = found[i];
      rectangle(frame, r.tl(), r.br(), cv::Scalar(0, 255, 0), 2);
    }

    imshow("A", frame);
    imshow("B", tmp);
    if (waitKey(50) == 27) break;
  }

  waitKey(-1);
  return 0;
}
