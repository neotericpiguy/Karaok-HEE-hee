#include "KaraokeApp.hpp"

#include <Wt/WBootstrap5Theme.h>
#include <Wt/WEnvironment.h>
#include <Wt/WLabel.h>
#include <Wt/WMessageBox.h>
#include <Wt/WNavigationBar.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WPopupMenuItem.h>
#include <Wt/WPushButton.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WText.h>

#include "DlQueueWidget.hpp"
#include "LibraryWidget.hpp"
#include "PlaylistWidget.hpp"

const std::string KaraokeApp::kCookieKey = "login";
const int KaraokeApp::kMaxAge = 12 * 60 * 60;  // possibly seconds

KaraokeApp::KaraokeApp(const Wt::WEnvironment& env, YoutubeDl::DlQueue& queue, Library& library, Authenticator& authenticator, Playlist& playlist) :
    Wt::WApplication(env),
    mEnv(env),
    mDlQueue(queue),
    mLibrary(library),
    mAuthenticator(authenticator),
    mPlaylist(playlist),
    mUser(nullptr),
    mDialog(nullptr),
    mLoginItem(nullptr)
{
  setTitle("Karaok-HEE-hee");  // application title

  auto theme = std::make_shared<Wt::WBootstrap5Theme>();
  setTheme(theme);

  auto container = root()->addNew<Wt::WContainerWidget>();

  // Create a navigation bar with a link to a web page.
  Wt::WNavigationBar* navigation = container->addNew<Wt::WNavigationBar>();
  navigation->setResponsive(true);
  navigation->addStyleClass("navbar-dark bg-dark");
  navigation->setTitle("Karaok-HEE-hee", "#");

  Wt::WStackedWidget* contentsStack = container->addNew<Wt::WStackedWidget>();
  contentsStack->addStyleClass("contents");

  // Setup a Left-aligned menu.
  auto leftMenu = std::make_unique<Wt::WMenu>(contentsStack);
  auto leftMenu_ = navigation->addMenu(std::move(leftMenu));

  // TODO find a better way besides double pointers
  leftMenu_->addItem("Playlist", std::make_unique<PlaylistWidget>(playlist, &mUser));
  leftMenu_->addItem("Library", std::make_unique<LibraryWidget>(library, playlist, &mUser));
  leftMenu_->addItem("Download Queue", std::make_unique<DlQueueWidget>(queue));
  leftMenu_->addStyleClass("me-auto");

  // Add a Search control.
  auto editPtr = std::make_unique<Wt::WLineEdit>();
  auto edit = editPtr.get();
  edit->setPlaceholderText("Enter a search item");

  // navigation->addSearch(std::move(editPtr));

  // Setup a Right-aligned menu.
  auto rightMenu = std::make_unique<Wt::WMenu>();
  auto rightMenu_ = navigation->addMenu(std::move(rightMenu));

  auto loginItem = std::make_unique<Wt::WMenuItem>("Login");
  mLoginItem = loginItem.get();

  // Create a popup submenu for the Help menu.
  auto loginPtr = std::make_unique<Wt::WPopupMenu>();
  auto login = loginPtr.get();
  login->addItem("Sign in");
  login->addItem("Register");
  login->addItem("About");
  login->addSeparator();
  login->addItem("Logout");

  login->itemSelected().connect([this, login](Wt::WMenuItem* item) {
    auto itemSelection = item->text().toUTF8();

    if (itemSelection == "Sign in")
    {
      showLoginDialog();
      return;
    }
    else if (itemSelection == "Register")
    {
      showRegisterDialog();
      return;
    }
    else if (itemSelection == "Logout")
    {
      mLoginItem->setText("Sign in");
      removeCookie(kCookieKey);
      mUser->setAuthenticated(false);
      mUser = nullptr;
      return;
    }

    auto messageBox = login->addChild(
        std::make_unique<Wt::WMessageBox>("Help",
                                          Wt::WString("<p>Version: {1}</p>").arg(VERSION),
                                          Wt::Icon::Information,
                                          Wt::StandardButton::Ok));

    messageBox->buttonClicked().connect([=] {
      login->removeChild(messageBox);
    });
    messageBox->show();
  });

  mLoginItem->setMenu(std::move(loginPtr));
  rightMenu_->addItem(std::move(loginItem));

  if (env.supportsCookies())
  {
    if (auto loginCookie = env.getCookie(kCookieKey); loginCookie)
    {
      auto username = *loginCookie;
      if (auto record = mAuthenticator.getRecord(username); record)
      {
        if (auto user = dynamic_cast<User*>(record); user)
        {
          user->setAuthenticated(true);
          mLoginItem->setText(user->getUsername());
          mUser = user;
        }
      }
    }
  }
}

void KaraokeApp::showLoginDialog()
{
  mDialog = this->addChild(std::make_unique<Wt::WDialog>("Login"));

  mDialog->contents()->addStyleClass("form-group");
  mDialog->setModal(true);
  mDialog->setResizable(true);

  // android seem to like it but apple not so much
  // Mozilla/5.0 (Linux; Android 10; Pixel 2) AppleWebKit/537.36 (KHTML, like
  // Gecko) Chrome/101.0.4951.41 Mobile Safari/537.36
  if (mEnv.userAgent().find("Chrome") != std::string::npos)
    mDialog->positionAt(mLoginItem);

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

  mDialog->setWindowTitle("Login");

  mDialog->contents()->clear();

  auto usernameLabel = mDialog->contents()->addNew<Wt::WLabel>("Username");
  auto usernameEdit = mDialog->contents()->addNew<Wt::WLineEdit>();
  usernameLabel->setBuddy(usernameEdit);

  auto passwordlabel = mDialog->contents()->addNew<Wt::WLabel>("Password");
  auto passwordEdit = mDialog->contents()->addNew<Wt::WLineEdit>("");
  passwordEdit->setEchoMode(Wt::EchoMode::Password);
  passwordlabel->setBuddy(passwordEdit);

  /* Process the mDialog result. */
  mDialog->finished().connect([this, passwordEdit, usernameEdit] {
    if (mDialog->result() == Wt::DialogCode::Accepted)
    {
      std::string passwordClear = passwordEdit->text().toUTF8();
      auto passwordHash = CsvDb::Table::sha256(passwordClear);

      User temp;
      temp.setUsername(usernameEdit->text().toUTF8());
      temp.setPasswordHash(passwordHash);
      if (mAuthenticator.authenticateUser(temp))
      {
        if (auto record = mAuthenticator.getRecord(temp.getUsername()); record)
        {
          if (auto user = dynamic_cast<User*>(record); user)
          {
            mUser = user;
            mLoginItem->setText(mUser->getUsername());
            setCookie(kCookieKey, mUser->getUsername(), kMaxAge);
          }
        }
      }
      else
      {
        mLoginItem->setText("Sign in");
        auto messageBox = this->addChild(
            std::make_unique<Wt::WMessageBox>("Login Failed", "Couldn't login", Wt::Icon::Information,
                                              Wt::StandardButton::Ok));

        messageBox->buttonClicked().connect([messageBox, this] {
          this->removeChild(messageBox);
        });
        messageBox->show();
        removeCookie(kCookieKey);
      }
    }

    removeChild(mDialog);
  });

  mDialog->show();
  mDialog->raiseToFront();
}

void KaraokeApp::showRegisterDialog()
{
  mDialog = this->addChild(std::make_unique<Wt::WDialog>("Register"));

  mDialog->contents()->addStyleClass("form-group");
  mDialog->setModal(true);
  mDialog->setResizable(true);
  mDialog->positionAt(mLoginItem);

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

  mDialog->setWindowTitle("Login");

  mDialog->contents()->clear();

  auto usernameLabel = mDialog->contents()->addNew<Wt::WLabel>("Username");
  auto usernameEdit = mDialog->contents()->addNew<Wt::WLineEdit>();
  usernameLabel->setBuddy(usernameEdit);

  auto passwordlabel = mDialog->contents()->addNew<Wt::WLabel>("Password");
  auto passwordEdit = mDialog->contents()->addNew<Wt::WLineEdit>("");
  passwordEdit->setEchoMode(Wt::EchoMode::Password);
  passwordlabel->setBuddy(passwordEdit);

  auto confirmPasswordlabel = mDialog->contents()->addNew<Wt::WLabel>("Confirm Password");
  auto confirmPasswordEdit = mDialog->contents()->addNew<Wt::WLineEdit>("");
  confirmPasswordEdit->setEchoMode(Wt::EchoMode::Password);
  confirmPasswordlabel->setBuddy(confirmPasswordEdit);

  /* Process the mDialog result. */
  mDialog->finished().connect([this, passwordEdit, usernameEdit, confirmPasswordEdit] {
    if (mDialog->result() == Wt::DialogCode::Accepted)
    {
      std::string passwordClear = passwordEdit->text().toUTF8();
      std::string confirmPasswordClear = confirmPasswordEdit->text().toUTF8();
      if (passwordClear != confirmPasswordClear)
      {
        auto messageBox = this->addChild(
            std::make_unique<Wt::WMessageBox>("Password",
                                              "Passwords don't match", Wt::Icon::Information,
                                              Wt::StandardButton::Ok));

        messageBox->buttonClicked().connect([messageBox, this] {
          this->removeChild(messageBox);
        });
        messageBox->show();
      }
      auto passwordHash = CsvDb::Table::sha256(passwordClear);

      User* temp = new User;
      temp->setUsername(usernameEdit->text().toUTF8());
      temp->setPasswordHash(passwordHash);
      // TODO wants to be DJ also or else segfault
      temp->setRoles({"Singer"});
      mAuthenticator.addRecord(temp);

      mUser = temp;
      mLoginItem->setText(mUser->getUsername());
      setCookie(kCookieKey, mUser->getUsername(), kMaxAge);

      mAuthenticator.saveTable();
    }

    removeChild(mDialog);
  });

  mDialog->show();
  mDialog->raiseToFront();
}

