#include "FlowWidget.hpp"

#include "FlowScene.hpp"

namespace QtNodes {

FlowWidget::FlowWidget(std::shared_ptr<DataModelRegistry> registry) : FlowView{new FlowScene} {
  
}

} // namespace QtNodes
