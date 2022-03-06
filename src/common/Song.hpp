#ifndef SONG_HPP
#define SONG_HPP

#include <map>
#include <string>

#include "Record.hpp"

class Song : public CsvDb::Record
{
public:
  // TODO make a vector or map somehow to be able to iterate through
  static const std::string kID;
  static const std::string kTITLE;
  static const std::string kARTIST;
  static const std::string kTAGS;
  static const std::string kPATH;
};

#endif
