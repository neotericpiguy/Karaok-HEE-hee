#ifndef DITTYWIDGET_HPP
#define DITTYWIDGET_HPP

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WProgressBar.h>
#include <Wt/WText.h>

#include "Ditty.hpp"

class DittyWidget : public Wt::WContainerWidget
{
public:
  DittyWidget(const Ditty& ditty);
  DittyWidget(const DittyWidget& env) = delete;
  DittyWidget& operator=(const DittyWidget& env) = delete;
  ~DittyWidget();


private:
  const Ditty& mInfo;
};

#endif
