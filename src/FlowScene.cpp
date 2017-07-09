#include "FlowScene.hpp"
#include "NodeIndex.hpp"
#include "ConnectionGraphicsObject.hpp"
#include "NodeGraphicsObject.hpp"

namespace QtNodes {

FlowScene::FlowScene(FlowSceneModel* model) 
  : _model(model)
{
  Q_ASSERT(model != nullptr);

  connect(model, &FlowSceneModel::nodeRemoved, this, &FlowScene::nodeRemoved);
  connect(model, &FlowSceneModel::nodeAdded, this, &FlowScene::nodeAdded);
  connect(model, &FlowSceneModel::nodePortUpdated, this, &FlowScene::nodePortUpdated);
  connect(model, &FlowSceneModel::nodeValidationUpdated, this, &FlowScene::nodeValidationUpdated);
  connect(model, &FlowSceneModel::connectionRemoved, this, &FlowScene::connectionRemoved);
  connect(model, &FlowSceneModel::connectionAdded, this, &FlowScene::connectionAdded);
  connect(model, &FlowSceneModel::nodeMoved, this, &FlowScene::nodeMoved);
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

void 
FlowScene::
nodeRemoved(const QUuid& id)
{
  
}
void
FlowScene::
nodeAdded(const QUuid& newID)
{
  auto index = model()->nodeIndex(newID);

  auto ngo = std::make_unique<NodeGraphicsObject>(*this, index);

  _nodeGraphicsObjects[index.id()] = std::move(ngo);
}
void
FlowScene::
nodePortUpdated(NodeIndex const& id)
{

}
void
FlowScene::
nodeValidationUpdated(NodeIndex const& id)
{

}
void
FlowScene::
connectionRemoved(NodeIndex const& leftNode, PortIndex leftPortID, NodeIndex const& rightNode, PortIndex rightPortID)
{

}
void
FlowScene::
connectionAdded(NodeIndex const& leftNode, PortIndex leftPortID, NodeIndex const& rightNode, PortIndex rightPortID)
{

}
void
FlowScene::
nodeMoved(NodeIndex const& index)
{

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
