#include "DittyWidget.hpp"

#include <Wt/WPanel.h>

DittyWidget::DittyWidget(const Ditty& info) :
    Wt::WContainerWidget(),
    mInfo(info)
{
  auto container = addNew<Wt::WContainerWidget>();

  auto songHash = mInfo.getField(Ditty::kID);
  auto panel = container->addNew<Wt::WPanel>();
  panel->setTitle(mInfo.getField(Ditty::kSINGER) + " : " + mInfo.getField(Ditty::kTITLE));

}

DittyWidget::~DittyWidget()
{
}

