#include "Node.hpp"

#include <QtCore/QObject>

#include <iostream>

#include "FlowScene.hpp"

#include "NodeGraphicsObject.hpp"
#include "NodeDataModel.hpp"

#include "ConnectionGraphicsObject.hpp"
#include "ConnectionState.hpp"

namespace QtNodes {

Node::
Node(std::unique_ptr<NodeDataModel> && dataModel, QUuid const& id)
  : _nodeDataModel(std::move(dataModel))
  , _index(id)
{
  // propagate data: model => node
  connect(_nodeDataModel.get(), &NodeDataModel::dataUpdated,
          this, &Node::onDataUpdated);

  _inConnections.resize(nodeDataModel()->nPorts(PortType::In));
  _outConnections.resize(nodeDataModel()->nPorts(PortType::Out));
}


Node::
~Node() {}

QJsonObject
Node::
save() const
{
  QJsonObject nodeJson;

  nodeJson["id"] = id().toString();

  nodeJson["model"] = _nodeDataModel->save();

  QJsonObject obj;
  obj["x"] = position().x();
  obj["y"] = position().y();
  nodeJson["position"] = obj;

  return nodeJson;
}


void
Node::
restore(QJsonObject const& json)
{
  _index = QUuid(json["id"].toString());

  QJsonObject positionJson = json["position"].toObject();
  QPointF     point(positionJson["x"].toDouble(),
                    positionJson["y"].toDouble());
  setPosition(point);

  _nodeDataModel->restore(json["model"].toObject());
}


QUuid
Node::
id() const
{
  return _index;
}


QPointF 
Node::
position() const {
  return _position;
}
void 
Node::
setPosition(QPointF const& newPos) {
  _position = newPos;
  
  // emit position changed signal
  emit positionChanged(newPos);
}

NodeDataModel*
Node::
nodeDataModel() const
{
  return _nodeDataModel.get();
}


std::vector<Connection*>&
Node::
connections(PortType pType, PortIndex idx)
{
  Q_ASSERT(pType == PortType::In ? idx < _inConnections.size() : idx < _outConnections.size());
  
  return pType == PortType::In ? _inConnections[idx] : _outConnections[idx];
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
}

} // namespace QtNodes
