#ifndef __RACVID_CONFIG
#define __RACVID_CONFIG

#include <fstream>
#include <string>

const char *CONFIG_DIR = "D:\\data\\S3-T7-A\\video\\pets2006\\S3-T7-A\\3\\";
const char *CONFIG_FORMAT = "S3-T7-A.%.5d.jpeg";

class Config {
  std::string format;
  std::string dir;
  int n;
public:
  Config() {}

  int getN() const {
    return n;
  }

  std::string getNthName(int n) const {
    char filename[1024];
    sprintf(filename, (dir+"//"+format).c_str(), n);
    return filename;
  }

  void read(const char *filename) {
    std::ifstream ifs(filename);
    ifs >> format >> dir >> n;
  }
};

#endif

