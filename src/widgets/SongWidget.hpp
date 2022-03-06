#ifndef SONGWIDGET_HPP
#define SONGWIDGET_HPP

#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>

#include "Song.hpp"

class SongWidget : public Wt::WContainerWidget
{
public:
  SongWidget();
  SongWidget(const SongWidget& env) = delete;
  SongWidget& operator=(const SongWidget& env) = delete;
  ~SongWidget();

  Song* getSong() const;
  void setSong(Song* val);
  void showDialog();

  const std::function<void()>& getSongChangedFunc() const;
  void setSongChangedFunc(const std::function<void()>& val);

private:
  Song* mSong;
  std::map<std::string, Wt::WLineEdit*> mFieldEditMap;
  Wt::WDialog* mDialog;
  std::function<void()> mSongChangedFunc;
};

#endif
