#ifndef WEBREMOTEAPP_HPP
#define WEBREMOTEAPP_HPP

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
class WebRemoteApp : public Wt::WApplication
{
public:
  WebRemoteApp(const Wt::WEnvironment& env);
  WebRemoteApp(const WebRemoteApp& env) = delete;
  WebRemoteApp& operator=(const WebRemoteApp& env) = delete;

private:
  Wt::WLineEdit* mNameEdit;
  Wt::WText* mGreeting;
};

#endif
