#pragma once

#include <memory>

#include <QtCore/QObject>
#include <QtCore/QUuid>
#include <QVariant>

#include "PortType.hpp"
#include "NodeData.hpp"

#include "Serializable.hpp"
#include "ConnectionState.hpp"
#include "ConnectionGeometry.hpp"
#include "QUuidStdHash.hpp"
#include "Export.hpp"
#include "ConnectionID.hpp"

class QPointF;

namespace QtNodes
{

class Node;
class NodeData;
class ConnectionGraphicsObject;

///
class NODE_EDITOR_PUBLIC Connection
  : public QObject
  , public Serializable
{

  Q_OBJECT

public:

  Connection(Node& nodeIn,
             PortIndex portIndexIn,
             Node& nodeOut,
             PortIndex portIndexOut);

  Connection(const Connection&) = delete;
  Connection operator=(const Connection&) = delete;

  ~Connection();

public:

  QJsonObject
  save() const override;

public:
  Node*
  getNode(PortType portType) const;

  Node*&
  getNode(PortType portType);

  PortIndex
  getPortIndex(PortType portType) const;

  NodeDataType
  dataType() const;

  ConnectionID
  id() const;

public: // data propagation

  void
  propagateData(std::shared_ptr<NodeData> nodeData) const;

  void
  propagateEmptyData() const;

private:
  
  void 
  setNodeToPort(Node& node,
                PortType portType,
                PortIndex portIndex);

  Node* _outNode = nullptr;
  Node* _inNode  = nullptr;

  PortIndex _outPortIndex;
  PortIndex _inPortIndex;

private:

  ConnectionState    _connectionState;
  ConnectionGeometry _connectionGeometry;

  std::unique_ptr<ConnectionGraphicsObject> _connectionGraphicsObject;

signals:
  void
  updated(Connection& conn) const;
};
}
