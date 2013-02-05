#ifndef __RACVID_CONFIG
#define __RACVID_CONFIG

#include <fstream>
#include <string>
#include <cassert>

class Config {
  std::string format;
  std::string dir;
  std::string output_dir;
  std::string cache_dir;
  int n;
  int numberOfSamples;
  std::string samples;
  std::string peopleResults;
  std::string luggageResults;
public:
  Config() {}

  int getN() const {
    return n;
  }
  
  int getNumberOfSamples() const {
    return numberOfSamples;
  }
  
  std::string getSamples() const {
    return samples;
  }
  
  std::string getPeopleResults() const {
    return peopleResults;
  }
  
  std::string getLuggageResults() const {
    return luggageResults;
  }

  std::string getNthName(int n) const {
    char filename[1024];
    sprintf(filename, (dir+format).c_str(), n);
    return filename;
  }

  std::string getNthCacheName(int n) const {
    char filename[1024];
    sprintf(filename, (cache_dir+"%05d.jpg").c_str(), n);
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
    ifs >> format >> dir >> n >> output_dir >> cache_dir;
    ifs >> numberOfSamples >> samples >> peopleResults >> luggageResults;
  }
};

#endif

