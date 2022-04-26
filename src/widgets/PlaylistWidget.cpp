#include "PlaylistWidget.hpp"

#include <Wt/WBootstrap5Theme.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WMediaPlayer.h>
#include <Wt/WPushButton.h>
#include <Wt/WSlider.h>
#include <Wt/WText.h>
#include <Wt/WTimer.h>
#include <Wt/WVideo.h>

#include "StringThings.hpp"

PlaylistWidget::PlaylistWidget(Playlist& playlist, User** user) :
    mUser(user),
    mDittyWidgetMap(),
    mPlaylist(playlist),
    mVideo(nullptr),
    mVideoStarted(false),
    mPoster(""),
    mSongPath(""),
    mState(Playlist::UNKNOWN)
{
  auto container = this;

  auto updateTimer = container->addChild(std::make_unique<Wt::WTimer>());
  updateTimer->setInterval(std::chrono::milliseconds(1000));
  updateTimer->timeout().connect([this, container] {
    // Remove widgets that are done by check
    for (auto iter = mDittyWidgetMap.begin(); iter != mDittyWidgetMap.end();)
    {
      std::string link = iter->first;
      if (mPlaylist.getTableRecords().find(link) == mPlaylist.getTableRecords().end())
      {
        iter->second->removeFromParent();
        iter = mDittyWidgetMap.erase(iter);
      }
      else
        iter++;
    }

    // Add new widgets from added song by outside users
    for (const auto& [order, record] : mPlaylist.getTableRecords())
    {
      if (mDittyWidgetMap.find(order) == mDittyWidgetMap.end())
      {
        if (auto ditty = dynamic_cast<Ditty*>(record); ditty)
        {
          mDittyWidgetMap[order] = container->addWidget(std::make_unique<DittyWidget>(*ditty));
        }
      }
    }

    // Add video preview if the User that is logged in has the "DJ" Role
    if (*mUser)
    {
      if ((*mUser)->hasRole("DJ") && !mVideo)
      {
        auto video = std::make_unique<Wt::WVideo>();
        mVideo = video.get();
        insertWidget(0, std::move(video));

        mVideo->setOptions(Wt::PlayerOption::Controls);
        mVideo->resize(640, 360);

        mVideo->setPreloadMode(Wt::MediaPreloadMode::Auto);

        mVideo->playbackStarted().connect([this] {});
        mVideo->ended().connect([this] {
          if (mPlaylist.getCurrentState() == Playlist::Playlist::INIT)
            return;

          mPlaylist.setCurrentState(Playlist::Playlist::INIT);
          updateQueue();
        });

        auto nextPushButton = std::make_unique<Wt::WPushButton>("Next");
        nextPushButton->clicked().connect([this] {
          if (mPlaylist.getCurrentState() == Playlist::Playlist::INIT)
            return;

          mPlaylist.setCurrentState(Playlist::Playlist::INIT);
          updateQueue();
        });
        insertWidget(1, std::move(nextPushButton));

        auto pausePushButton = std::make_unique<Wt::WPushButton>("Pause");
        pausePushButton->clicked().connect([this] { mPlaylist.setCurrentState(Playlist::Playlist::PAUSE); });
        insertWidget(1, std::move(pausePushButton));

        auto startPushButton = std::make_unique<Wt::WPushButton>("Start");
        startPushButton->clicked().connect([this] { mPlaylist.setCurrentState(Playlist::Playlist::PLAYING); });
        insertWidget(1, std::move(startPushButton));

        if (mPlaylist.getCurrentState() != Playlist::Playlist::INIT && mPlaylist.getCurrentSongPath().empty())
        {
          mPlaylist.setCurrentState(Playlist::Playlist::INIT);
          updateQueue(false);
        }
      }

      if (mState != mPlaylist.getCurrentState())
        setState(mPlaylist.getCurrentState());

      // service changes in state video state machine
      stateMachine();
    }
  });
  updateTimer->start();

  // Add initial set of songs
  for (const auto& [order, record] : mPlaylist.getTableRecords())
  {
    if (mDittyWidgetMap.find(order) == mDittyWidgetMap.end())
    {
      if (auto ditty = dynamic_cast<Ditty*>(record); ditty)
      {
        mDittyWidgetMap[order] = container->addWidget(std::make_unique<DittyWidget>(*ditty));
      }
    }
  }
}

std::string PlaylistWidget::dittyPicture()
{
  std::string targetPath = "/usr/share/Wt";
  std::string targetFile = "pics/target" + std::to_string(rand()) + ".jpg";
  std::string targetFilename = targetPath + "/" + targetFile;

  auto results = mPlaylist.songList();

  static std::string dittyText;
  static std::string prevTargetFile;

  std::string text = StringThings::vecToStr(results, "\n");
  if (text.empty())
    text = "Empty Playlist...";

  if (text != dittyText)
  {
    std::string cmd = "convert -size 2560x1080 xc:black " + targetFilename;
    system(cmd.c_str());

    cmd = "convert -pointsize 80 -fill white -draw 'text 60,100 \"" + text + "\"' " + targetFilename + " " + targetFilename;
    system(cmd.c_str());

    dittyText = text;
    prevTargetFile = targetFile;
  }

  return prevTargetFile;
}

void PlaylistWidget::stateMachine()
{
  // If you are a DJ instance.
  if ((*mUser)->hasRole("DJ") && mVideo)
  {
    if (getState() == Playlist::INIT)
    {
      mVideo->pause();

      if (mPlaylist.getCurrentSongPath().empty())
        updateQueue(false);
      mSongPath = mPlaylist.getCurrentSongPath();
      mVideo->clearSources();
      mVideo->addSource(Wt::WLink(mSongPath));
      mVideoStarted = false;
      mVideo->setPoster(mPlaylist.getCurrentPoster());
    }

    // Not also a singer
    if (!(*mUser)->hasRole("Singer"))
    {
      if (getState() == Playlist::PLAYING && !mVideoStarted)
      {
        if (!mVideo->playing())
        {
          mVideo->play();
          mVideoStarted = true;
        }
      }
      else if (getState() == Playlist::PAUSE)
      {
        if (mVideo->playing())
        {
          mVideo->pause();
          mVideoStarted = false;
        }
      }
    }
  }
}

void PlaylistWidget::setState(Playlist::State state)
{
  switch (state)
  {
    case Playlist::INIT:
    case Playlist::PLAYING:
      // State initial conditions
      mVideoStarted = false;

      break;
    case Playlist::PAUSE:
      // State initial conditions
      mVideoStarted = true;

      break;
    default:
      mState = Playlist::UNKNOWN;
  }

  std::cout << Playlist::stateMap.at(mState) << " to " << Playlist::stateMap.at(state) << std::endl;
  mState = state;
}

Playlist::State PlaylistWidget::getState() const
{
  return mState;
}

void PlaylistWidget::updateQueue(bool removeFirst)
{
  auto nextDitty = mPlaylist.getNextDitty();

  if (removeFirst)
  {
    mPlaylist.removeRecord(*nextDitty);
    nextDitty = mPlaylist.getNextDitty();
  }

  mPlaylist.setCurrentPoster(dittyPicture());

  if (!nextDitty)
  {
    mPlaylist.setCurrentSongPath("");
    return;
  }

  mPlaylist.setCurrentSongPath(nextDitty->getField(Song::kPATH));
}
