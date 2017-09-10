#pragma once

#include <QtCore/QUuid>
#include <QtWidgets/QGraphicsScene>

#include <unordered_map>
#include <tuple>
#include <memory>
#include <functional>

#include "QUuidStdHash.hpp"
#include "Export.hpp"
#include "ConnectionID.hpp"
#include "DataModelRegistry.hpp"

namespace QtNodes
{

class FlowSceneModel;
class ConnectionGraphicsObject;
class NodeGraphicsObject;

/// Scene holds connections and nodes.
class NODE_EDITOR_PUBLIC FlowScene
  : public QGraphicsScene
{
  Q_OBJECT
  
  friend NodeGraphicsObject;
  friend ConnectionGraphicsObject;
public:

  FlowScene(FlowSceneModel* model);

  ~FlowScene();

public:

  FlowSceneModel* model() const { return _model; }

  NodeGraphicsObject* nodeGraphicsObject(const NodeIndex& index);
  
  std::vector<NodeIndex> selectedNodes() const;

private slots:

  void nodeRemoved(const QUuid& id);
  void nodeAdded(const QUuid& newID);
  void nodePortUpdated(NodeIndex const& id);
  void nodeValidationUpdated(NodeIndex const& id);
  void connectionRemoved(NodeIndex const& leftNode, PortIndex leftPortID, NodeIndex const& rightNode, PortIndex rightPortID);
  void connectionAdded(NodeIndex const& leftNode, PortIndex leftPortID, NodeIndex const& rightNode, PortIndex rightPortID);
  void nodeMoved(NodeIndex const& index);

private:

  FlowSceneModel* _model;
  
  std::unordered_map<QUuid, NodeGraphicsObject*> _nodeGraphicsObjects;
  std::unordered_map<ConnectionID, ConnectionGraphicsObject*> _connGraphicsObjects;

  // This is for when you're creating a connection
  ConnectionGraphicsObject* _temporaryConn = nullptr;

};

NodeGraphicsObject*
locateNodeAt(QPointF scenePoint, FlowScene &scene,
             QTransform viewTransform);
}
