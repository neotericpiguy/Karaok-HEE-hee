#ifndef PLAYLIST_HPP
#define PLAYLIST_HPP

#include "Ditty.hpp"
#include "Library.hpp"
#include "Table.hpp"

// enum,Song_hash,singer
// 1,asdf124,ThisIsJonathan

class Playlist : public CsvDb::Table
{
public:
  Playlist(const Library& library);
  ~Playlist();

  size_t getLatestEnum() const;
  void setLatestEnum(size_t val);

  size_t loadTable() override;

  bool addDitty(const std::string& songHash, const std::string& singer);
  Ditty* getNextDitty();

  std::vector<std::string> songList() const;

private:
  const Library& mLibrary;
  size_t mLatestEnum;
};

#endif
