#ifndef PLAYLISTWIDGET_HPP
#define PLAYLISTWIDGET_HPP

#include <Wt/WContainerWidget.h>

#include "DittyWidget.hpp"
#include "Playlist.hpp"
#include "User.hpp"

class PlaylistWidget : public Wt::WContainerWidget
{
public:
  enum State {
    UNKNOWN,
    INIT,
    PLAYING,
    PAUSE,
  };

  std::map<State, std::string> stateMap = {
      {UNKNOWN, "UNKNOWN"},
      {INIT, "INIT"},
      {PLAYING, "PLAYING"},
      {PAUSE, "PAUSE"},
  };

  PlaylistWidget(Playlist& playlist, User** user);
  PlaylistWidget(const PlaylistWidget& env) = delete;
  PlaylistWidget& operator=(const PlaylistWidget& env) = delete;

  Playlist& getPlaylist() const;
  void setPlaylist(Playlist& val);

  std::string dittyPicture();
  void stateMachine();

  void updateQueue(bool removeFirst = true);

  void setState(State state);
  State getState() const;

private:
  User** mUser;
  std::map<std::string, DittyWidget*> mDittyWidgetMap;
  Playlist& mPlaylist;
  Wt::WVideo* mVideo;
  bool mVideoStarted;

  std::string mPoster;
  std::string mSongPath;
  State mState;

  // static vars are owned by class not instance use this map to pass vars
  // between instances of PlaylistWidget
  static State mCurrentState;
  static std::string mCurrentSongPath;
  static std::string mCurrentPoster;
};

#endif
