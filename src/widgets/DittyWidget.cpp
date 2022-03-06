#include "DittyWidget.hpp"

#include <Wt/WPanel.h>

DittyWidget::DittyWidget(const Ditty& info) :
    Wt::WContainerWidget(),
    mInfo(info),
    statsText(nullptr),
    linkText(nullptr),
    stateText(nullptr)
{
  auto container = addNew<Wt::WContainerWidget>();

  auto songHash = mInfo.getField(Ditty::kID);
  auto panel = container->addNew<Wt::WPanel>();
  panel->setTitle(mInfo.getField(Ditty::kSINGER) + " : " + mInfo.getField(Ditty::kTITLE));

  //  //  container->addNew<Wt::WText>(songHash);
  //  //  container->addNew<Wt::WBreak>();
  //  container->addNew<Wt::WText>(mInfo.getField(Ditty::kSINGER) + " : " +
  //                               mInfo.getField(Ditty::kTITLE) + " : " +
  //                               mInfo.getField(Ditty::kARTIST));
  //  container->addNew<Wt::WBreak>();
  //  container->addNew<Wt::WText>(mInfo.getField(Ditty::kTITLE));
  //  container->addNew<Wt::WBreak>();
  //  container->addNew<Wt::WText>(mInfo.getField(Ditty::kARTIST));
  //  container->addNew<Wt::WBreak>();

  //  stateText = container->addNew<Wt::WText>();
  //  statsText = container->addNew<Wt::WText>();
  //  container->addNew<Wt::WBreak>();
  //
  //  progressBar = container->addNew<Wt::WProgressBar>();
  //  progressBar->setRange(0, 100);
}

DittyWidget::~DittyWidget()
{
}

void DittyWidget::update()
{
  //  stateText->setText(mInfo.getState());
  //  progressBar->setValue(mInfo.getStatus());
  //
  //  if (mInfo.getState() == "Downloading")
  //    statsText->setText(" - Speed: " + mInfo.getSpeed() + " ETA: " + mInfo.getEta());
  //  else
  //    statsText->setText("");
  //
  //  if (!mInfo.getFilename().empty())
  //    linkText->setText(mInfo.getFilename());
}
