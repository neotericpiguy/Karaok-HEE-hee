#ifndef DLQUEUEWIDGET_HPP
#define DLQUEUEWIDGET_HPP

#include <Wt/WContainerWidget.h>

#include "DlInfoWidget.hpp"
#include "DlQueue.hpp"

class DlQueueWidget : public Wt::WContainerWidget
{
public:
  DlQueueWidget(YoutubeDl::DlQueue& mDlQueue);
  DlQueueWidget(const DlQueueWidget& env) = delete;
  DlQueueWidget& operator=(const DlQueueWidget& env) = delete;

  void updateStats();

private:
  std::map<std::string, DlInfoWidget*> mDlInfoWidgetMap;
  Wt::WText* mStatsText;
  YoutubeDl::DlQueue& mDlQueue;
};

#endif
