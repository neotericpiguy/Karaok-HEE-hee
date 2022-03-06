#include "DlInfoWidget.hpp"

#include <Wt/WPanel.h>

DlInfoWidget::DlInfoWidget(const YoutubeDl::DlInfo& info) :
    Wt::WContainerWidget(),
    mInfo(info),
    progressBar(nullptr),
    statsText(nullptr),
    linkText(nullptr),
    stateText(nullptr)
{
  auto container = addNew<Wt::WContainerWidget>();

  linkText = container->addNew<Wt::WText>(mInfo.getLink());
  container->addNew<Wt::WBreak>();

  stateText = container->addNew<Wt::WText>();
  statsText = container->addNew<Wt::WText>();
  container->addNew<Wt::WBreak>();

  progressBar = container->addNew<Wt::WProgressBar>();
  progressBar->setRange(0, 100);
}

DlInfoWidget::~DlInfoWidget()
{
}

void DlInfoWidget::update()
{
  stateText->setText(mInfo.getState());
  progressBar->setValue(mInfo.getStatus());

  if (mInfo.getState() == "Downloading")
    statsText->setText(" - Speed: " + mInfo.getSpeed() + " ETA: " + mInfo.getEta());
  else
    statsText->setText("");

  if (!mInfo.getFilename().empty())
    linkText->setText(mInfo.getFilename());
}
