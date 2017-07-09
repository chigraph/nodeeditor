#pragma once

#include <memory>

#include <QtCore/QObject>
#include <QtCore/QUuid>

#include <QtCore/QJsonObject>

#include "PortType.hpp"

#include "Export.hpp"
#include "NodeState.hpp"
#include "NodeGeometry.hpp"
#include "NodeData.hpp"
#include "NodeGraphicsObject.hpp"
#include "ConnectionGraphicsObject.hpp"
#include "Serializable.hpp"

namespace QtNodes
{

class Connection;
class ConnectionState;
class NodeGraphicsObject;
class NodeDataModel;

class NODE_EDITOR_PUBLIC Node
  : public QObject
  , public Serializable
{
  Q_OBJECT

public:

  /// NodeDataModel should be an rvalue and is moved into the Node
  Node(std::unique_ptr<NodeDataModel> && dataModel, QUuid const& id);

  virtual
  ~Node();

public:

  QJsonObject
  save() const override;

  void
  restore(QJsonObject const &json) override;

public:

  QUuid
  id() const;

  
public:
  
  QPointF position() const;
  void setPosition(QPointF const& newPos);
  
public:

  NodeDataModel*
  nodeDataModel() const;
  
  std::vector<Connection*>&
  connections(PortType pType, PortIndex pIdx);

public slots: // data propagation

  /// Propagates incoming data to the underlying model.
  void
  propagateData(std::shared_ptr<NodeData> nodeData,
                PortIndex inPortIndex) const;

  /// Fetches data from model's OUT #index port
  /// and propagates it to the connection
  void
  onDataUpdated(PortIndex index);
  
signals:
  
  void positionChanged(QPointF const& newPos);

private:
  
  std::vector<std::vector<Connection*>> _inConnections, _outConnections;
  
  QPointF _position;

  // data
  std::unique_ptr<NodeDataModel> _nodeDataModel;
  QUuid _index;

};
}
