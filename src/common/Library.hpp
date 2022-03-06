#ifndef LIBRARY_HPP
#define LIBRARY_HPP

#include "Song.hpp"
#include "Table.hpp"

class Library : public CsvDb::Table
{
public:
  Library();
  ~Library();

  bool addSong(const std::string& filename);

  Song* getSong(const std::string& id);

  std::vector<const Song*> findSong(const std::string& criteria) const;
};

#endif
