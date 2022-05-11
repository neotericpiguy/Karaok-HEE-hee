#include "LibraryWidget.hpp"

#include <Wt/WLabel.h>
#include <Wt/WLineEdit.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

#include "StringThings.hpp"

LibraryWidget::LibraryWidget(Library& library, Playlist& playlist, User** user) :
    mUser(user),
    mLibrary(library),
    mPlaylist(playlist),
    mSearchEdit(nullptr),
    mTableView(nullptr),
    mModel(),
    mTestText(),
    mContainer(),
    mLongPressDuration(nullptr),
    mCurrentIndex(),
    mDialog(nullptr)
{
  mContainer = this;

  mSearchEdit = mContainer->addNew<Wt::WLineEdit>();
  mSearchEdit->setPlaceholderText("Find Song");

  mSearchEdit->textInput().connect([this] {
    auto criteria = mSearchEdit->text().toUTF8();
    updateSearchTable(criteria);
  });

  mModel = std::make_shared<Wt::WStandardItemModel>();
  size_t column = 0;

  auto headers = mLibrary.getHeaders();
  mModel->insertColumns(0, headers.size());
  for (const auto& header : headers)
    mModel->setHeaderData(column++, Wt::WString(header));

  auto addPushButton = mContainer->addNew<Wt::WPushButton>("Add Song");
  addPushButton->setStyleClass("btn-primary");
  addPushButton->clicked().connect([this]() {
    if (!(*mUser))
    {
      auto messageBox = this->addChild(
          std::make_unique<Wt::WMessageBox>("Login!", "You need to login", Wt::Icon::Information,
                                            Wt::StandardButton::Ok));

      messageBox->buttonClicked().connect([messageBox, this] {
        this->removeChild(messageBox);
      });
      messageBox->show();
      return;
    }
    auto singer = (*mUser)->getUsername();
    if (singer == "dj")
    {
      showUserEntryDialog();
      return;
    }
    addToSongQueue(singer);
  });

  mSearchEdit->enterPressed().connect([this, addPushButton] {
    auto criteria = mSearchEdit->text().toUTF8();
    updateSearchTable(criteria);

    // Special case where a user hits enter and there is only 1 entry
    if (mLibrary.findSong(criteria).size() == 1)
    {
      Wt::WModelIndex index = mModel->index(0, 0);
      mTableView->select(index);
      Wt::WMouseEvent e;
      addPushButton->clicked().emit(e);
    }
  });

  auto editPushButton = mContainer->addNew<Wt::WPushButton>("Edit Song");
  editPushButton->clicked().connect([this] {
    auto indexes = mTableView->selectedIndexes();
    if (indexes.size() < 1)
      return;

    auto iter = indexes.begin();
    auto item = mModel->item(iter->row(), 0);
    auto id = item->text().toUTF8();

    auto songPtr = mLibrary.getSong(id);
    if (!songPtr)
      return;

    auto songWidget = mContainer->addNew<SongWidget>();
    songWidget->setSong(songPtr);
    songWidget->setSongChangedFunc([this]() {
      mLibrary.saveTable();
    });
    songWidget->showDialog();
  });

  mTableView = mContainer->addNew<SongTableView>();
  mTableView->setModel(mModel);
  mTableView->setColumnResizeEnabled(true);
  mTableView->setHeaderAlignment(0, Wt::AlignmentFlag::Left);
  mTableView->setAlternatingRowColors(true);
  mTableView->setRowHeight(50);
  mTableView->setHeaderHeight(50);
  mTableView->setSelectionMode(Wt::SelectionMode::Single);
  mTableView->setEditTriggers(Wt::EditTrigger::None);
  mTableView->setSortingEnabled(true);
  mTableView->doubleClicked().connect([this, addPushButton](const Wt::WModelIndex&, const Wt::WMouseEvent& e) {
    addPushButton->clicked().emit(e);
  });

  mLongPressDuration = mContainer->addChild(std::make_unique<Wt::WTimer>());
  mLongPressDuration->setInterval(std::chrono::milliseconds(2000));
  mLongPressDuration->setSingleShot(true);
  mLongPressDuration->timeout().connect([this, addPushButton] {
    mTableView->select(mCurrentIndex);
    Wt::WMouseEvent e;

    auto result = Wt::WMessageBox::show("Add song?", "You sure you want to add song?",
                                        Wt::StandardButton::Ok | Wt::StandardButton::Cancel);

    if (result == Wt::StandardButton::Ok)
      addPushButton->clicked().emit(e);
  });

  mTableView->touchStarted().connect([this, addPushButton](const std::vector<Wt::WModelIndex>& indexes, const Wt::WTouchEvent&) {
    auto indexIter = indexes.end();
    indexIter--;
    mCurrentIndex = *indexIter;
    mLongPressDuration->start();
  });

  mTableView->touchEnded().connect([this, addPushButton](const std::vector<Wt::WModelIndex>&, const Wt::WTouchEvent&) {
    mLongPressDuration->stop();
  });

  // Hide columns that users shouldn't see
  std::vector<std::string> columnsToHide = {Song::kID, Song::kTAGS, Song::kPATH};
  for (const auto& columnToHide : columnsToHide)
  {
    auto iter = std::find(headers.begin(), headers.end(), columnToHide);
    auto columnIndex = std::distance(headers.begin(), iter);
    mTableView->hideColumn(columnIndex);
  }

  // Sort by Title column
  auto titleColumnIndex = std::distance(headers.begin(), std::find(headers.begin(), headers.end(), Song::kTITLE));
  mTableView->sortByColumn(titleColumnIndex, Wt::SortOrder::Ascending);

  //// https://redmine.emweb.be/boards/2/topics/2659
  updateSearchTable("");
}

void LibraryWidget::updateSearchTable(const std::string& criteria)
{
  auto results = mLibrary.findSong(criteria);
  size_t row = 0;
  size_t column = 0;

  if (results.size() < static_cast<long unsigned int>(mModel->rowCount()))
    mModel->removeRows(results.size(), mModel->rowCount() - results.size());
  else
    mModel->insertRows(mModel->rowCount(), results.size() - mModel->rowCount());

  auto headers = mLibrary.getHeaders();
  for (const auto& result : results)
  {
    column = 0;
    for (const auto& header : headers)
    {
      if (!result->hasField(header))
        continue;

      auto item = std::make_unique<Wt::WStandardItem>();
      item->setText(result->getField(header));
      mModel->setItem(row, column++, std::move(item));
    }
    row++;
  }

  // Re sort
  auto titleColumnIndex = std::distance(headers.begin(), std::find(headers.begin(), headers.end(), Song::kTITLE));
  mTableView->sortByColumn(titleColumnIndex, Wt::SortOrder::Ascending);

  // Re draw table again
  mTableView->drawAgain();
}

void LibraryWidget::showUserEntryDialog()
{
  mDialog = this->addChild(std::make_unique<Wt::WDialog>("Login"));

  mDialog->contents()->addStyleClass("form-group");
  mDialog->setModal(true);
  mDialog->setResizable(true);

  Wt::WPushButton* ok = mDialog->footer()->addNew<Wt::WPushButton>("OK");
  ok->setDefault(true);

  Wt::WPushButton* cancel = mDialog->footer()->addNew<Wt::WPushButton>("Cancel");
  mDialog->rejectWhenEscapePressed();

  /* Accept the mDialog */
  ok->clicked().connect([this] {
    mDialog->accept();
  });

  /* Reject the mDialog */
  cancel->clicked().connect(mDialog, &Wt::WDialog::reject);

  mDialog->setWindowTitle("User Entry");

  mDialog->contents()->clear();

  auto usernameLabel = mDialog->contents()->addNew<Wt::WLabel>("Username");
  auto usernameEdit = mDialog->contents()->addNew<Wt::WLineEdit>();
  usernameLabel->setBuddy(usernameEdit);

  /* Process the mDialog result. */
  mDialog->finished().connect([this, usernameEdit] {
    if (mDialog->result() == Wt::DialogCode::Accepted)
    {
      std::string singer = usernameEdit->text().toUTF8();

      addToSongQueue(singer);
      removeChild(mDialog);
    }
  });

  mDialog->show();
  mDialog->raiseToFront();
}

void LibraryWidget::addToSongQueue(const std::string& singer)
{
  auto indexes = mTableView->selectedIndexes();
  if (indexes.size() < 1)
    return;

  auto iter = indexes.begin();
  auto item = mModel->item(iter->row(), 0);
  auto id = item->text().toUTF8();

  auto songPtr = mLibrary.getSong(id);
  if (!songPtr)
    return;

  mPlaylist.addDitty(id, singer);
  auto messageBox = this->addChild(
      std::make_unique<Wt::WMessageBox>(singer,
                                        songPtr->getField(Song::kTITLE) + " song added!",
                                        Wt::Icon::Information,
                                        Wt::StandardButton::Ok));

  messageBox->buttonClicked().connect([messageBox, this] {
    this->removeChild(messageBox);
  });
  messageBox->show();
}
