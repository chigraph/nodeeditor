#pragma once

#include "FlowScene.hpp"
#include "PortType.hpp"
#include "FlowSceneModel.hpp"
#include "QUuidStdHash.hpp"
#include "Node.hpp"
#include "ConnectionID.hpp"
#include "Export.hpp"

#include <memory>
#include <functional>
#include <unordered_map>

namespace QtNodes {
  
class Connection;
class DataModelRegistry;
class NodeDataModel;

class NODE_EDITOR_PUBLIC DataFlowScene : public FlowScene {
  Q_OBJECT

public:

  DataFlowScene(std::shared_ptr<DataModelRegistry> registry =
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
    
    DataFlowModel(std::shared_ptr<DataModelRegistry> reg);

    // FlowSceneModel read interface
    QStringList modelRegistry() const override;
    QString nodeTypeCatergory(QString const& /*name*/) const override;
    QString converterNode(NodeDataType const& /*lhs*/, NodeDataType const& ) const override;
    QList<QUuid> nodeUUids() const override;
    NodeIndex nodeIndex(const QUuid& ID) const override;
    QString nodeTypeIdentifier(NodeIndex const& index) const override;
    QString nodeCaption(NodeIndex const& index) const override;
    QPointF nodeLocation(NodeIndex const& index) const override;
    QWidget* nodeWidget(NodeIndex const& index) const override;
    bool nodeResizable(NodeIndex const& index) const override;
    NodeValidationState nodeValidationState(NodeIndex const& index) const override;
    QString nodeValidationMessage(NodeIndex const& index) const override;
    NodePainterDelegate* nodePainterDelegate(NodeIndex const& index) const override;
    unsigned int nodePortCount(NodeIndex const& index, PortType portType) const override;
    QString nodePortCaption(NodeIndex const& index, PortIndex pIndex, PortType portType) const override;
    NodeDataType nodePortDataType(NodeIndex const& index, PortIndex pIndex, PortType portType) const override;
    ConnectionPolicy nodePortConnectionPolicy(NodeIndex const& index, PortIndex pIndex, PortType portType) const override;
    std::vector<std::pair<NodeIndex, PortIndex>> nodePortConnections(NodeIndex const& index, PortIndex id, PortType portType) const override;

    // FlowSceneModel write interface
    bool removeConnection(NodeIndex const& leftNode, PortIndex leftPortID, NodeIndex const& rightNode, PortIndex rightPortID) override;
    bool addConnection(NodeIndex const& leftNode, PortIndex leftPortID, NodeIndex const& rightNode, PortIndex rightPortID) override;
    bool removeNode(NodeIndex const& index) override;
    QUuid addNode(const QString& typeID, QPointF const& location) override;
    bool moveNode(NodeIndex const& index, QPointF newLocation) override;

    using SharedConnection = std::shared_ptr<Connection>;
    using UniqueNode       = std::unique_ptr<Node>;

    std::unordered_map<ConnectionID, SharedConnection> _connections;
    std::unordered_map<QUuid, UniqueNode>              _nodes;
    std::shared_ptr<DataModelRegistry>                 _registry;
  };
  
  
  DataFlowModel* _dataFlowModel;

};

} // namespace QtNodes
