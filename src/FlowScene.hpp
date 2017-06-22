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

  FlowScene(FlowSceneModel* model);

  ~FlowScene();

public:
  
  FlowSceneModel* model() const { return _model; }
  
  
private:
  FlowSceneModel* _model;

};

Node*
locateNodeAt(QPointF scenePoint, FlowScene &scene,
             QTransform viewTransform);
}
