#pragma once

#include "FlowView.hpp"
#include "PortType.hpp"
#include "FlowSceneModel.hpp"
#include "QUuidStdHash.hpp"

#include <memory>
#include <functional>
#include <unordered_map>

namespace QtNodes {
  
class Connection;
class Node;
class DataModelRegistry;
class NodeDataModel;

class FlowWidget : public FlowView {
  Q_OBJECT

public:
  FlowWidget(std::shared_ptr<DataModelRegistry> registry =
              std::make_shared<DataModelRegistry>());

  std::shared_ptr<Connection>createConnection(PortType connectedPort,
                                              Node& node,
                                              PortIndex portIndex);

  std::shared_ptr<Connection>createConnection(Node& nodeIn,
                                              PortIndex portIndexIn,
                                              Node& nodeOut,
                                              PortIndex portIndexOut);

  std::shared_ptr<Connection>restoreConnection(QJsonObject const &connectionJson);

  void deleteConnection(Connection& connection);

  Node& createNode(std::unique_ptr<NodeDataModel> && dataModel);

  Node& restoreNode(QJsonObject const& nodeJson);

  void removeNode(Node& node);

  DataModelRegistry&registry() const;

  void setRegistry(std::shared_ptr<DataModelRegistry> registry);

  void iterateOverNodes(std::function<void(Node*)> visitor);

  void iterateOverNodeData(std::function<void(NodeDataModel*)> visitor);

  void iterateOverNodeDataDependentOrder(std::function<void(NodeDataModel*)> visitor);

  QPointF getNodePosition(const Node& node) const;

  void setNodePosition(Node& node, const QPointF& pos) const;

  QSizeF getNodeSize(const Node& node) const;
public:

  std::unordered_map<QUuid, std::unique_ptr<Node> > const &nodes() const;

  std::unordered_map<QUuid, std::shared_ptr<Connection> > const &connections() const;

  std::vector<Node*>selectedNodes() const;

public:

  void clearScene();

  void save() const;

  void load();

  QByteArray saveToMemory() const;

  void loadFromMemory(const QByteArray& data);

signals:

  void nodeCreated(Node &n);

  void nodeDeleted(Node &n);

  void connectionCreated(Connection &c);
  void connectionDeleted(Connection &c);

  void nodeMoved(Node& n, const QPointF& newLocation);

  void nodeDoubleClicked(Node& n);

  void connectionHovered(Connection& c, QPoint screenPos);

  void nodeHovered(Node& n, QPoint screenPos);

  void connectionHoverLeft(Connection& c);

  void nodeHoverLeft(Node& n);
  
private:

  // default model class
  class DataFlowModel : public FlowSceneModel {
  public:

    // FlowSceneModel read interface
    QStringList modelRegistry() const override;
    QList<QUuid> nodeUUids() const override;
    NodeIndex nodeIndex(const QUuid& ID) const override;
    QString nodeTypeIdentifier(const NodeIndex& index) const override;
    QString nodeCaption(const NodeIndex& index) const override;
    QPointF nodeLocation(const NodeIndex& index) const override;
    unsigned int nodePortCount(const NodeIndex& index, PortType portType) const override;
    QString nodePortCaption(const NodeIndex& index, PortIndex, PortType portType) const override;
    NodeDataType nodePortDataType(const NodeIndex& index, PortIndex, PortType portType) const override;
    ConnectionPolicy nodePortConnectionPolicy(const NodeIndex& index, PortIndex, PortType portType) const override;
    std::vector<std::pair<NodeIndex, PortIndex>> nodePortConnections(const NodeIndex& index, PortIndex, PortType portType, PortIndex) const override;

    // FlowSceneModel write interface
    bool removeConnection(const NodeIndex& leftNode, unsigned int leftPortID, const NodeIndex& rightNode, unsigned int rightPortID) override;
    bool addConnection(const NodeIndex& leftNode, unsigned int leftPortID, const NodeIndex& rightNode, unsigned int rightPortID) override;
    bool removeNode(const NodeIndex& index) override;
    bool addNode(const QString& typeID, std::size_t insertAt) override;
    bool moveNode(const NodeIndex& index, QPointF newLocation) override;

  private:
    using SharedConnection = std::shared_ptr<Connection>;
    using UniqueNode       = std::unique_ptr<Node>;

    std::unordered_map<QUuid, SharedConnection> _connections;
    std::unordered_map<QUuid, UniqueNode>       _nodes;
    std::shared_ptr<DataModelRegistry>          _registry;
  };
};

} // namespace QtNodes
