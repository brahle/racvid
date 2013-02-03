#ifndef __RACVID_CONFIG
#define __RACVID_CONFIG

#include <fstream>
#include <string>
#include <cassert>

class Config {
  std::string format;
  std::string dir;
  std::string output_dir;
  int n;
public:
  Config() {}

  int getN() const {
    return n;
  }

  std::string getNthName(int n) const {
    char filename[1024];
    sprintf(filename, (dir+format).c_str(), n);
    return filename;
  }

  std::string getNthOutputName(int n) const {
    char filename[1024];
    sprintf(filename, (output_dir+"%05d.jpg").c_str(), n);
    return filename;
  }

  void read(const char *filename) {
    std::ifstream ifs(filename);
    assert(ifs);
    ifs >> format >> dir >> n >> output_dir;
  }
};

#endif

