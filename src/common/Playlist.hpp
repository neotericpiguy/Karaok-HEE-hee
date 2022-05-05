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
  enum State {
    UNKNOWN,
    INIT,
    SKIP,
    PLAYING,
    PAUSE,
  };

  static const std::map<State, std::string> stateMap;

public:
  Playlist(const Library& library);
  ~Playlist();

  size_t getLatestEnum() const;
  void setLatestEnum(size_t val);

  size_t loadTable() override;

  bool addDitty(const std::string& songHash, const std::string& singer);
  Ditty* getNextDitty();

  std::vector<std::string> songList() const;

  State getCurrentState() const;
  void setCurrentState(State val);

  const std::string& getCurrentSongPath() const;
  void setCurrentSongPath(const std::string& val);

  const std::string& getCurrentPoster() const;
  void setCurrentPoster(const std::string& val);

  std::string dittyPicture();
  void skip();
  void updateQueue(bool removeFirst = true);

private:
  const Library& mLibrary;
  size_t mLatestEnum;

  State mCurrentState;
  std::string mCurrentSongPath;
  std::string mCurrentPoster;
};

#endif
