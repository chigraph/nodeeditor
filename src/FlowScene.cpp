#include "FlowScene.hpp"
#include "NodeIndex.hpp"

namespace QtNodes {


NodeGraphicsObject* FlowScene::nodeGraphicsObject(const NodeIndex& index) {
  auto iter = _nodeGraphicsObjects.find(index.id());
  if (iter == _nodeGraphicsObjects.end()) {
    return nullptr;
  }
  return iter->second;
}

} // namespace QtNodes
