#ifndef WEBREMOTEAPP_HPP
#define WEBREMOTEAPP_HPP

#include <Wt/WApplication.h>
#include <Wt/WProgressBar.h>

#include "Authenticator.hpp"
#include "DlQueue.hpp"
#include "Library.hpp"
#include "Playlist.hpp"
#include "User.hpp"

class WebRemoteApp : public Wt::WApplication
{
public:
  WebRemoteApp(const Wt::WEnvironment& env, YoutubeDl::DlQueue& queue, Library& library, Authenticator& authenticator, Playlist& playlist);
  WebRemoteApp(const WebRemoteApp& env) = delete;
  WebRemoteApp& operator=(const WebRemoteApp& env) = delete;

  void showLoginDialog();
  void showRegisterDialog();

protected:
  static const std::string kCookieKey;
  static const int kMaxAge;

private:
  YoutubeDl::DlQueue& mDlQueue;
  Library& mLibrary;
  Authenticator& mAuthenticator;
  Playlist& mPlaylist;
  User* mUser;
  Wt::WDialog* mDialog;
  Wt::WMenuItem* mLoginItem;
};

#endif
