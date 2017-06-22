#pragma once

#include "PortType.hpp"
#include "Export.hpp"

#include <cstddef>

#include <QString>
#include <QPointF>
#include <QObject>
#include <QUuid>
#include <QList>


namespace QtNodes
{

class NodeIndex;
class NodeDataType;

enum class NodeConnectionPolicy {
  AllowOne,
  AllowMulti
};

enum class NodeValidationState
{
  Valid,
  Warning,
  Error
};


class NODE_EDITOR_PUBLIC FlowSceneModel : public QObject {
  Q_OBJECT

public:

  FlowSceneModel();

  // Scene specific functions
  virtual QStringList modelRegistry() const = 0;

  // Retrieval functions
  //////////////////////

  /// Get the list of node IDs
  virtual QList<QUuid> nodeUUids() const = 0;

  /// Create a NodeIndex for a node
  virtual NodeIndex nodeIndex(const QUuid& ID) const = 0;

  /// Get the type ID for the node
  virtual QString nodeTypeIdentifier(const NodeIndex& index) const = 0;

  /// Get the caption for the node
  virtual QString nodeCaption(const NodeIndex& index) const = 0;

  /// Get the location of a node
  virtual QPointF nodeLocation(const NodeIndex& index) const = 0;

  /// Get the embedded widget
  virtual QWidget* nodeWidget(const NodeIndex& index) const = 0;
  
  /// Get the validation state
  virtual NodeValidationState nodeValidationState(const NodeIndex& index) const = 0;
  
  /// Get the validation error/warning
  virtual QString nodeValidationMessage(const NodeIndex& index) const = 0;

  /// Get the count of DataPorts
  virtual unsigned int nodePortCount(const NodeIndex& index, PortType portType) const = 0;

  /// Get the port caption
  virtual QString nodePortCaption(const NodeIndex& index, unsigned int portID, PortType portType) const = 0;

  /// Get the port data type
  virtual NodeDataType nodePortDataType(const NodeIndex& index, unsigned int portID, PortType portType) const = 0;

  /// Port Policy
  virtual NodeConnectionPolicy nodePortConnectionPolicy(const NodeIndex& index, unsigned int portID, PortType portType) const = 0;

  /// Get the number of connections at a port
  virtual unsigned int nodePortConnectionCount(const NodeIndex& index, unsigned int portID, PortType portType) const = 0;

  /// Get a connection at a port
  virtual NodeIndex nodePortConnection(const NodeIndex& index, unsigned int portID, PortType portType, unsigned int connectionID) const = 0;


  // Mutation functions
  /////////////////////

  /// Remove a connection
  virtual bool removeConnection(const NodeIndex& /*leftNode*/, unsigned int /*leftPortID*/, const NodeIndex& /*rightNode*/, unsigned int /*rightPortID*/) { return false; }

  /// Add a connection
  virtual bool addConnection(const NodeIndex& /*leftNode*/, unsigned int /*leftPortID*/, const NodeIndex& /*rightNode*/, unsigned int /*rightPortID*/) { return false; }

  /// Remove a node
  virtual bool removeNode(const NodeIndex& /*index*/) { return false; }

  /// Add a node
  virtual bool addNode(const QString& /*typeID*/, std::size_t /*insertAt*/) { return false; }

  /// Move a node to a new location
  virtual bool moveNode(const NodeIndex& /*index*/, QPointF /*newLocation*/) { return false; }

signals:

  void nodeAboutToBeRemoved(const NodeIndex& index);
  void nodeRemoved(const QUuid& id);

  void nodeAdded(const QUuid& newID);

  void nodePortUpdated(const NodeIndex& id);
  
  void nodeValidationUpdated(const NodeIndex& id);

  void connectionRemoved(const NodeIndex& leftNode, unsigned int leftPortID, const NodeIndex& rightNode, unsigned int rightPortID);

  void connectionAdded(const NodeIndex& leftNode, unsigned int leftPortID, const NodeIndex& rightNode, unsigned int rightPortID);

  void nodeMoved(const NodeIndex& index, QPointF newLocation);

protected:

  NodeIndex createIndex(const QUuid& id, void* internalPointer);

};

} // namespace QtNodes
