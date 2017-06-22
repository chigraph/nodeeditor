#include "FlowSceneModel.hpp"
#include "NodeIndex.hpp"

namespace QtNodes {

FlowSceneModel::FlowSceneModel()
{
}

NodeIndex FlowSceneModel::createIndex(const QUuid& id, void* internalPointer)
{
  return NodeIndex(id, internalPointer, this);
}

} // namespace QtNodes

