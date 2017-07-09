#include "Connection.hpp"

#include <iostream>
#include <math.h>

#include <QtWidgets/QtWidgets>
#include <QtGlobal>

#include "Node.hpp"
#include "FlowScene.hpp"
#include "FlowView.hpp"

#include "NodeGeometry.hpp"
#include "NodeGraphicsObject.hpp"
#include "NodeDataModel.hpp"

#include "ConnectionState.hpp"
#include "ConnectionGeometry.hpp"
#include "ConnectionGraphicsObject.hpp"

namespace QtNodes {

Connection::
Connection(Node& nodeIn,
           PortIndex portIndexIn,
           Node& nodeOut,
           PortIndex portIndexOut)
  : _outNode(&nodeOut)
  , _inNode(&nodeIn)
  , _outPortIndex(portIndexOut)
  , _inPortIndex(portIndexIn)
  , _connectionState()
{
  setNodeToPort(nodeIn, PortType::In, portIndexIn);
  setNodeToPort(nodeOut, PortType::Out, portIndexOut);
}


Connection::
~Connection()
{
  propagateEmptyData();
}


QJsonObject
Connection::
save() const
{
  QJsonObject connectionJson;

  if (_inNode && _outNode)
  {
    connectionJson["in_id"] = _inNode->id().toString();
    connectionJson["in_index"] = _inPortIndex;

    connectionJson["out_id"] = _outNode->id().toString();
    connectionJson["out_index"] = _outPortIndex;
  }

  return connectionJson;
}

void
Connection::
setNodeToPort(Node& node,
              PortType portType,
              PortIndex portIndex)
{
  auto& nodeWeak = getNode(portType);

  nodeWeak = &node;

  if (portType == PortType::Out)
    _outPortIndex = portIndex;
  else
    _inPortIndex = portIndex;

  _connectionState.setNoRequiredPort();

  updated(*this);
}


PortIndex
Connection::
getPortIndex(PortType portType) const
{
  PortIndex result = INVALID;

  switch (portType)
  {
    case PortType::In:
      result = _inPortIndex;
      break;

    case PortType::Out:
      result = _outPortIndex;

      break;

    default:
      break;
  }

  return result;
}

Node*
Connection::
getNode(PortType portType) const
{
  switch (portType)
  {
    case PortType::In:
      return _inNode;
      break;

    case PortType::Out:
      return _outNode;
      break;

    default:
      // not possible
      break;
  }
  Q_UNREACHABLE();
}


Node*&
Connection::
getNode(PortType portType)
{
  switch (portType)
  {
    case PortType::In:
      return _inNode;
      break;

    case PortType::Out:
      return _outNode;
      break;

    default:
      // not possible
      break;
  }
  Q_UNREACHABLE();
}

NodeDataType
Connection::
dataType() const
{
  Node* validNode;
  PortIndex index    = INVALID;
  PortType  portType = PortType::None;

  if ((validNode = _inNode))
  {
    index    = _inPortIndex;
    portType = PortType::In;
  }
  else if ((validNode = _outNode))
  {
    index    = _outPortIndex;
    portType = PortType::Out;
  }

  if (validNode)
  {
    auto const &model = validNode->nodeDataModel();

    return model->dataType(portType, index);
  }

  Q_UNREACHABLE();
}

ConnectionID
Connection::
id() const {
  ConnectionID ret;
  ret.lNodeID = getNode(PortType::Out)->id();
  ret.rNodeID = getNode(PortType::In)->id();
  
  ret.lPortID = getPortIndex(PortType::Out);
  ret.rPortID = getPortIndex(PortType::In);
  
  return ret;
}

void
Connection::
propagateData(std::shared_ptr<NodeData> nodeData) const
{
  if (_inNode)
  {
    _inNode->propagateData(nodeData, _inPortIndex);
  }
}


void
Connection::
propagateEmptyData() const
{
  std::shared_ptr<NodeData> emptyData;

  propagateData(emptyData);
}

} // namespace QtNodes
