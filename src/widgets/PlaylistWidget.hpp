#ifndef PLAYLISTWIDGET_HPP
#define PLAYLISTWIDGET_HPP

#include <Wt/WContainerWidget.h>

#include "DittyWidget.hpp"
#include "Playlist.hpp"
#include "User.hpp"

class PlaylistWidget : public Wt::WContainerWidget
{
public:
  PlaylistWidget(Playlist& playlist, User** user);
  PlaylistWidget(const PlaylistWidget& env) = delete;
  PlaylistWidget& operator=(const PlaylistWidget& env) = delete;

  Playlist& getPlaylist() const;
  void setPlaylist(Playlist& val);

  void stateMachine();

  void updateQueue(bool removeFirst = true);

  void setState(Playlist::State state);
  Playlist::State getState() const;

private:
  User** mUser;
  std::map<std::string, DittyWidget*> mDittyWidgetMap;
  Playlist& mPlaylist;
  Wt::WVideo* mVideo;
  bool mVideoStarted;

  std::string mPoster;
  Playlist::State mState;
};

#endif
