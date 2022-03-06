#include "WebRemoteApp.hpp"

/*
 * The env argument contains information about the new session, and
 * the initial request. It must be passed to the WApplication
 * constructor so it is typically also an argument for your custom
 * application constructor.
 */
WebRemoteApp::WebRemoteApp(const Wt::WEnvironment& env) :
    WApplication(env),
    mNameEdit(nullptr),
    mGreeting(nullptr)
{
  setTitle("Web Remote");  // application title

  //  root()->addWidget(std::make_unique<Wt::WText>("Your name, please ? "));  // show some text
  //
  //  mNameEdit = root()->addWidget(std::make_unique<Wt::WLineEdit>());  // allow text input
  //  mNameEdit->setFocus();                                             // give focus
  //  root()->addWidget(std::make_unique<Wt::WBreak>());             // insert a line break

  auto screenOffButton = root()->addWidget(std::make_unique<Wt::WPushButton>("Screen off"));
  screenOffButton->clicked().connect([this]() {
    system("xset dpms force off");
  });

  auto muterButton = root()->addWidget(std::make_unique<Wt::WPushButton>("unmute"));
  muterButton->clicked().connect([this, muterButton]() {
    auto state = muterButton->text().toUTF8();
    std::string cmd = "amixer set Master " + state;
    state = (state == "unmute") ? "mute" : "unmute";
    muterButton->setText(state);
    system(cmd.c_str());
  });
}

