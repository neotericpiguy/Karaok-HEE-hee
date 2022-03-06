#include "SongWidget.hpp"

#include <Wt/WDialog.h>
#include <Wt/WLabel.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPanel.h>
#include <Wt/WPushButton.h>

SongWidget::SongWidget() :
    Wt::WContainerWidget(),
    mSong(nullptr),
    mFieldEditMap(),
    mDialog(nullptr),
    mSongChangedFunc(nullptr)
{
}

SongWidget::~SongWidget()
{
}

Song* SongWidget::getSong() const
{
  return mSong;
}
void SongWidget::setSong(Song* val)
{
  mSong = val;
}
const std::function<void()>& SongWidget::getSongChangedFunc() const
{
  return mSongChangedFunc;
}
void SongWidget::setSongChangedFunc(const std::function<void()>& val)
{
  mSongChangedFunc = val;
}

void SongWidget::showDialog()
{
  auto id = mSong->getField(Song::kID);
  id = id.substr(0, 20);

  mDialog = this->addChild(std::make_unique<Wt::WDialog>("Title"));

  mDialog->contents()->addStyleClass("form-group");
  mDialog->setModal(true);

  setLayoutSizeAware(true);
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

  /* Process the mDialog result. */
  mDialog->finished().connect([this] {
    if (mDialog->result() == Wt::DialogCode::Accepted)
    {
      for (const auto& [field, value] : mFieldEditMap)
        mSong->setField(field, value->text().toUTF8());

      if (mSongChangedFunc)
        mSongChangedFunc();
    }

    removeChild(mDialog);
  });
  mDialog->setWindowTitle("Song Edit: " + id);

  mDialog->contents()->clear();

  auto fields = mSong->getFields();
  for (const auto& [field, value] : fields)
  {
    if (field == Song::kID || field == Song::kPATH)
      continue;

    auto label = mDialog->contents()->addNew<Wt::WLabel>(field);
    auto edit = mDialog->contents()->addNew<Wt::WLineEdit>(value);
    label->setBuddy(edit);

    mFieldEditMap[field] = edit;
  }

  mDialog->show();
  mDialog->raiseToFront();
}
