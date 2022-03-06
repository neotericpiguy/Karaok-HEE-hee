#ifndef DLENTRYWIDGET_HPP
#define DLENTRYWIDGET_HPP

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WProgressBar.h>
#include <Wt/WText.h>

#include "DlInfo.hpp"

class DlInfoWidget : public Wt::WContainerWidget
{
public:
  DlInfoWidget() = delete;
  DlInfoWidget(const YoutubeDl::DlInfo& info);
  DlInfoWidget(const DlInfoWidget& env) = delete;
  DlInfoWidget& operator=(const DlInfoWidget& env) = delete;
  ~DlInfoWidget();

  void update();

private:
  const YoutubeDl::DlInfo& mInfo;
  Wt::WProgressBar* progressBar;
  Wt::WText* statsText;
  Wt::WText* linkText;
  Wt::WText* stateText;
};

#endif
