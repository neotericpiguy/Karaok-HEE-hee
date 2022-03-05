/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

/*
 * A simple hello world application class which demonstrates how to react
 * to events, read input, and give feed-back.
 */
class HelloApplication : public Wt::WApplication
{
public:
  HelloApplication(const Wt::WEnvironment& env);
  HelloApplication(const HelloApplication& env) = delete;
  HelloApplication& operator=(const HelloApplication& env) = delete;

private:
  Wt::WLineEdit* mNameEdit;
  Wt::WText* mGreeting;
};

/*
 * The env argument contains information about the new session, and
 * the initial request. It must be passed to the WApplication
 * constructor so it is typically also an argument for your custom
 * application constructor.
 */
HelloApplication::HelloApplication(const Wt::WEnvironment& env) :
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
    std::cout << "Action of some sort" << std::endl;
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

int main(int argc, char** argv)
{
  /*
   * Your main method may set up some shared resources, but should then
   * start the server application (FastCGI or httpd) that starts listening
   * for requests, and handles all of the application life cycles.
   *
   * The last argument to WRun specifies the function that will instantiate
   * new application objects. That function is executed when a new user surfs
   * to the Wt application, and after the library has negotiated browser
   * support. The function should return a newly instantiated application
   * object.
   */
  return Wt::WRun(argc, argv, [](const Wt::WEnvironment& env) {
    /*
     * You could read information from the environment to decide whether
     * the user has permission to start a new application
     */
    return std::make_unique<HelloApplication>(env);
  });
}
