#pragma once

#include <QtCore/QUuid>
#include <QtWidgets/QGraphicsScene>

#include <unordered_map>
#include <tuple>
#include <memory>
#include <functional>

#include "QUuidStdHash.hpp"
#include "Export.hpp"
#include "NodeGraphicsObject.hpp"
#include "DataModelRegistry.hpp"

namespace QtNodes
{

class FlowSceneModel;
class ConnectionGraphicsObject;

/// Scene holds connections and nodes.
class NODE_EDITOR_PUBLIC FlowScene
  : public QGraphicsScene
{
  Q_OBJECT
public:

  FlowScene(FlowSceneModel* model);

  ~FlowScene();

public:
  
  FlowSceneModel* model() const { return _model; }
  
  NodeGraphicsObject* nodeGraphicsObject(const NodeIndex& index);
  
private:
  FlowSceneModel* _model;
  
  std::unordered_map<QUuid, std::unique_ptr<NodeGraphicsObject>> _nodeGraphicsObjects;

  // This is for when you're creating a connection
  ConnectionGraphicsObject* _temporaryGraphics = nullptr;
  
};

NodeGraphicsObject*
locateNodeAt(QPointF scenePoint, FlowScene &scene,
             QTransform viewTransform);
}
