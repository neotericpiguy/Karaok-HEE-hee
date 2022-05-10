#ifndef KARAOKEAPP_HPP
#define KARAOKEAPP_HPP

#include <Wt/WApplication.h>
#include <Wt/WProgressBar.h>

#include "Authenticator.hpp"
#include "DlQueue.hpp"
#include "Library.hpp"
#include "Playlist.hpp"
#include "User.hpp"

class KaraokeApp : public Wt::WApplication
{
public:
  KaraokeApp(const Wt::WEnvironment& env, YoutubeDl::DlQueue& queue, Library& library, Authenticator& authenticator, Playlist& playlist);
  KaraokeApp(const KaraokeApp& env) = delete;
  KaraokeApp& operator=(const KaraokeApp& env) = delete;

  void showLoginDialog();
  void showRegisterDialog();

protected:
  static const std::string kCookieKey;
  static const int kMaxAge;

private:
  const Wt::WEnvironment& mEnv;
  YoutubeDl::DlQueue& mDlQueue;
  Library& mLibrary;
  Authenticator& mAuthenticator;
  Playlist& mPlaylist;
  User* mUser;
  Wt::WDialog* mDialog;
  Wt::WMenuItem* mLoginItem;
};

#endif
