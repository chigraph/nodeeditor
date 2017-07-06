#pragma once

#include <QtCore/QUuid>
#include <QtWidgets/QGraphicsScene>

#include <unordered_map>
#include <tuple>
#include <memory>
#include <functional>

#include "QUuidStdHash.hpp"
#include "Export.hpp"
#include "DataModelRegistry.hpp"

namespace QtNodes
{

class FlowSceneModel;

/// Scene holds connections and nodes.
class NODE_EDITOR_PUBLIC FlowScene
  : public QGraphicsScene
{
  Q_OBJECT
public:

  FlowScene(FlowSceneModel* model = nullptr);

  ~FlowScene();

public:
  
  FlowSceneModel* model() const { return _model; }
  
  NodeGraphicsObject* nodeGraphicsObject(const NodeIndex& index);
  
private:
  FlowSceneModel* _model;
  
  std::unordered_map<QUuid, NodeGraphicsObject*> _nodeGraphicsObjects;

};

NodeGraphicsObject*
locateNodeAt(QPointF scenePoint, FlowScene &scene,
             QTransform viewTransform);
}
