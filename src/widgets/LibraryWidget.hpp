#ifndef LIBRARYWIDGET_HPP
#define LIBRARYWIDGET_HPP

#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WStandardItem.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WTableView.h>

#include "Library.hpp"
#include "Playlist.hpp"
#include "SongWidget.hpp"
#include "User.hpp"

class SongTableView : public Wt::WTableView
{
public:
  SongTableView()
  {
    setLayoutSizeAware(true);
  }
  virtual void layoutSizeChanged(const int width, const int /*height*/) override
  {
    for (int column = 0; (column < this->model()->columnCount()); column++)
    {
      setColumnWidth(column, Wt::WLength((((width - 3) / 2) - 7)));
    }
  }
};

class LibraryWidget : public Wt::WContainerWidget
{
public:
  LibraryWidget(Library& library, Playlist& playlist, User** user);
  LibraryWidget(const LibraryWidget& env) = delete;
  LibraryWidget& operator=(const LibraryWidget& env) = delete;

  void updateSearchTable(const std::string& criteria);

private:
  User** mUser;
  Library& mLibrary;
  Playlist& mPlaylist;
  Wt::WLineEdit* mSearchEdit;
  SongTableView* mTableView;
  std::shared_ptr<Wt::WStandardItemModel> mModel;
  Wt::WText* mTestText;
  Wt::WContainerWidget* mContainer;
};

#endif
