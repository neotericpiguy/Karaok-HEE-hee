#ifndef DITTY_HPP
#define DITTY_HPP

#include <string>

#include "Song.hpp"

// enum,Song_hash,singer
// 1,asdf124,ThisIsJonathan

// class Ditty : public CsvDb::Record
class Ditty : public Song
{
public:
  static const std::string kORDER;
  static const std::string kSINGER;
};

#endif
