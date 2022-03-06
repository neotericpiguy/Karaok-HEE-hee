#include "DlQueueWidget.hpp"

#include <Wt/WBootstrap5Theme.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WSlider.h>
#include <Wt/WTimer.h>

#include "StringThings.hpp"

DlQueueWidget::DlQueueWidget(YoutubeDl::DlQueue& queue) :
    mDlInfoWidgetMap(),
    mStatsText(nullptr),
    mDlQueue(queue)
{
  auto container = this;

  //  auto maxThreadSlider = container->addNew<Wt::WSlider>(Wt::Orientation::Horizontal);
  //  maxThreadSlider->resize(150, 50);
  //  maxThreadSlider->setTickPosition(Wt::WSlider::TicksBothSides);
  //  maxThreadSlider->setRange(1, 10);
  //  maxThreadSlider->setValue(mDlQueue.getMaxThreads());
  //  maxThreadSlider->valueChanged().connect([this, maxThreadSlider] {
  //    auto sliderValStr = maxThreadSlider->valueText().toUTF8();
  //    unsigned int val;
  //    if (!StringThings::strTo(val, sliderValStr))
  //      return;
  //    mDlQueue.setMaxThreads(val);
  //  });
  // container->addNew<Wt::WBreak>();

  mStatsText = container->addNew<Wt::WText>();

  auto linkEdit = container->addNew<Wt::WLineEdit>();
  linkEdit->setPlaceholderText("Youtube link");
  linkEdit->enterPressed().connect([this, container, linkEdit]() {
    auto link = linkEdit->text().toUTF8();
    linkEdit->setText("");

    if (link.empty())
      return;

    mDlQueue.addToQueue(link);
  });

  auto updateTimer = container->addChild(std::make_unique<Wt::WTimer>());
  updateTimer->setInterval(std::chrono::milliseconds(2000));
  updateTimer->timeout().connect([this, container] {
    // Remove widgets that are done by check if they are in dlInfoMap
    for (auto iter = mDlInfoWidgetMap.begin(); iter != mDlInfoWidgetMap.end();)
    {
      std::string link = iter->first;
      if (mDlQueue.getDlInfoMap().find(link) == mDlQueue.getDlInfoMap().end())
      {
        iter->second->removeFromParent();
        iter = mDlInfoWidgetMap.erase(iter);
      }
      else
        iter++;
    }

    // There are watch threads with out progress bars
    if (mDlInfoWidgetMap.size() != mDlQueue.getDlInfoMap().size())
    {
      for (auto& [link, info] : mDlQueue.getDlInfoMap())
      {
        if (mDlInfoWidgetMap.find(link) != mDlInfoWidgetMap.end())
          continue;

        if (info.getState() == "Complete")
          continue;

        mDlInfoWidgetMap[link] = container->addWidget(std::make_unique<DlInfoWidget>(info));
      }
    }

    // Update progressbars
    for (const auto& [link, dlInfoWidget] : mDlInfoWidgetMap)
      dlInfoWidget->update();

    updateStats();
  });
  updateTimer->start();

  // Add any progress bars that were made from resuming Downloads or other
  // webApp sessions
  for (const auto& [link, info] : mDlQueue.getDlInfoMap())
  {
    if (info.getState() == "Complete")
      continue;

    mDlInfoWidgetMap[link] = container->addWidget(std::make_unique<DlInfoWidget>(info));
  }
}

void DlQueueWidget::updateStats()
{
  // " OutputDir: " + std::to_string(mDlQueue.getOutputDirSize()) + "/" + std::to_string(mDlQueue.getMaxOutputDirSize()) + " " +
  mStatsText->setText("maxThreads: " + std::to_string(mDlQueue.getMaxThreads()) +
                      " Running: " + std::to_string(mDlQueue.getRunningThreadCount()) +
                      " Queue: " + std::to_string(mDlQueue.getLinkQuequeSize()) +
                      " Space: " + StringThings::fixed(mDlQueue.getOutputDirSize() * 100.0 / mDlQueue.getMaxOutputDirSize(), 2) + "%");
}
