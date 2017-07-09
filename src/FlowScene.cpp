#include "FlowScene.hpp"
#include "NodeIndex.hpp"
#include "ConnectionGraphicsObject.hpp"
#include "NodeGraphicsObject.hpp"

namespace QtNodes {

FlowScene::FlowScene(FlowSceneModel* model) 
  : _model(model) 
{
  
}

FlowScene::~FlowScene() = default;

NodeGraphicsObject* FlowScene::nodeGraphicsObject(const NodeIndex& index)
{
  auto iter = _nodeGraphicsObjects.find(index.id());
  if (iter == _nodeGraphicsObjects.end()) {
    return nullptr;
  }
  return iter->second.get();
}

NodeGraphicsObject*
locateNodeAt(QPointF scenePoint, FlowScene &scene,
             QTransform viewTransform)
{
  // items under cursor
  QList<QGraphicsItem*> items =
    scene.items(scenePoint,
                Qt::IntersectsItemShape,
                Qt::DescendingOrder,
                viewTransform);

  //// items convertable to NodeGraphicsObject
  std::vector<QGraphicsItem*> filteredItems;

  std::copy_if(items.begin(),
               items.end(),
               std::back_inserter(filteredItems),
               [] (QGraphicsItem * item)
    {
      return (dynamic_cast<NodeGraphicsObject*>(item) != nullptr);
    });

  if (!filteredItems.empty())
  {
    QGraphicsItem* graphicsItem = filteredItems.front();
    auto ngo = dynamic_cast<NodeGraphicsObject*>(graphicsItem);

    return ngo;
  }

  return nullptr;
}

} // namespace QtNodes
