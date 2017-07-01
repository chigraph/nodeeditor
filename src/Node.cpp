#include "Node.hpp"

#include <QtCore/QObject>

#include <iostream>

#include "FlowScene.hpp"

#include "NodeGraphicsObject.hpp"
#include "NodeDataModel.hpp"

#include "ConnectionGraphicsObject.hpp"
#include "ConnectionState.hpp"

using QtNodes::Node;
using QtNodes::NodeGeometry;
using QtNodes::NodeState;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeGraphicsObject;
using QtNodes::PortIndex;
using QtNodes::PortType;

Node::
Node(std::unique_ptr<NodeDataModel> && dataModel)
  : _id(QUuid::createUuid())
  , _nodeDataModel(std::move(dataModel))
{
  // propagate data: model => node
  connect(_nodeDataModel.get(), &NodeDataModel::dataUpdated,
          this, &Node::onDataUpdated);
}


Node::
~Node() {}

QJsonObject
Node::
save() const
{
  QJsonObject nodeJson;

  nodeJson["id"] = _id.toString();

  nodeJson["model"] = _nodeDataModel->save();

  QJsonObject obj;
  obj["x"] = _nodeDataModel->position().x();
  obj["y"] = _nodeDataModel->position().y();
  nodeJson["position"] = obj;

  return nodeJson;
}


void
Node::
restore(QJsonObject const& json)
{
  _id = QUuid(json["id"].toString());

  QJsonObject positionJson = json["position"].toObject();
  QPointF     point(positionJson["x"].toDouble(),
                    positionJson["y"].toDouble());
  _nodeDataModel->setPosition(point);

  _nodeDataModel->restore(json["model"].toObject());
}


QUuid
Node::
id() const
{
  return _id;
}

NodeDataModel*
Node::
nodeDataModel() const
{
  return _nodeDataModel.get();
}


void
Node::
propagateData(std::shared_ptr<NodeData> nodeData,
              PortIndex inPortIndex) const
{
  _nodeDataModel->setInData(nodeData, inPortIndex);
}


void
Node::
onDataUpdated(PortIndex index)
{
  auto nodeData = _nodeDataModel->outData(index);

  auto connections =
    _nodeState.connections(PortType::Out, index);

  for (auto const & c : connections)
    c.second->propagateData(nodeData);
}
