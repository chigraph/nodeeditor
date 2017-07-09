#include "FlowSceneModel.hpp"
#include "NodeIndex.hpp"

namespace QtNodes {

FlowSceneModel::FlowSceneModel()
{
}

bool FlowSceneModel::removeNodeWithConnections(NodeIndex const& index) {
  auto& model = *index.model();
  
  // delete the conenctions that node has first
  auto deleteConnections = [&](PortType ty) -> bool {
    for (PortIndex portID = 0; portID < model.nodePortCount(index, ty); ++portID) {
      auto inputConnections = model.nodePortConnections(index, portID, ty);
      for (const auto& conn : inputConnections) {
        // try to remove it
        bool success;
        if (ty == PortType::In) {
          success = model.removeConnection(conn.first, conn.second, index, portID);
        } else {
          success = model.removeConnection(index, portID, conn.first, conn.second);
        }

        // failed, abort the node deletion
        if (!success) {
          return false;
        }
      }
    }
  };
  bool success = deleteConnections(PortType::In);
  if (!success) return false;
  
  success = deleteConnections(PortType::Out);
  if (!success) return false;
  
  // if we get here, then try to remove the node itsself
  return model.removeNode(index);
}

NodeIndex FlowSceneModel::createIndex(const QUuid& id, void* internalPointer)
{
  return NodeIndex(id, internalPointer, this);
}

} // namespace QtNodes

