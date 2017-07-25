#include "DataFlowScene.hpp"
#include "Connection.hpp"

#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>

namespace QtNodes {

DataFlowScene::DataFlowScene(std::shared_ptr<DataModelRegistry> registry) : FlowScene(new DataFlowModel(std::move(registry))) {
  _dataFlowModel = static_cast<DataFlowModel*>(model());
}


std::shared_ptr<Connection>
DataFlowScene::
restoreConnection(QJsonObject const &connectionJson)
{
  QUuid nodeInId  = QUuid(connectionJson["in_id"].toString());
  QUuid nodeOutId = QUuid(connectionJson["out_id"].toString());

  PortIndex portIndexIn  = connectionJson["in_index"].toInt();
  PortIndex portIndexOut = connectionJson["out_index"].toInt();


  ConnectionID connId;
  connId.lNodeID = nodeOutId;
  connId.rNodeID = nodeInId;
  
  connId.lPortID = portIndexOut;
  connId.rPortID = portIndexIn;
  
  if (!_dataFlowModel->addConnection(_dataFlowModel->nodeIndex(nodeOutId), connId.lPortID, _dataFlowModel->nodeIndex(nodeInId), connId.rPortID)) 
    return nullptr;

  return _dataFlowModel->_connections[connId];
  
}

Node&
DataFlowScene::
restoreNode(QJsonObject const& nodeJson)
{
  QString modelName = nodeJson["model"].toObject()["name"].toString();

  auto uuid = _dataFlowModel->addNode(modelName, {});
  
  if (uuid.isNull())
    throw std::logic_error(std::string("No registered model with name ") +
                           modelName.toLocal8Bit().data());

  auto& node = *_dataFlowModel->_nodes[uuid];
  node.restore(nodeJson);

  return node;
}



void 
DataFlowScene::
removeNode(Node& node) {
  model()->removeNodeWithConnections(model()->nodeIndex(node.id()));
}

void
DataFlowScene::
clearScene() {
  // delete all the nodes
  while(!_dataFlowModel->_nodes.empty()) {
    removeNode(*_dataFlowModel->_nodes.begin()->second);
  }
}

void
DataFlowScene::
save() const
{
  QString fileName =
    QFileDialog::getSaveFileName(nullptr,
                                 tr("Open Flow Scene"),
                                 QDir::homePath(),
                                 tr("Flow Scene Files (*.flow)"));

  if (!fileName.isEmpty())
  {
    if (!fileName.endsWith("flow", Qt::CaseInsensitive))
      fileName += ".flow";

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
    {
      file.write(saveToMemory());
    }
  }
}


void
DataFlowScene::
load()
{
  clearScene();

  //-------------

  QString fileName =
    QFileDialog::getOpenFileName(nullptr,
                                 tr("Open Flow Scene"),
                                 QDir::homePath(),
                                 tr("Flow Scene Files (*.flow)"));

  if (!QFileInfo::exists(fileName))
    return;

  QFile file(fileName);

  if (!file.open(QIODevice::ReadOnly))
    return;

  QByteArray wholeFile = file.readAll();

  loadFromMemory(wholeFile);
}


QByteArray
DataFlowScene::
saveToMemory() const
{
  QJsonObject sceneJson;

  QJsonArray nodesJsonArray;

  for (auto const & pair : _dataFlowModel->_nodes)
  {
    auto const &node = pair.second;

    nodesJsonArray.append(node->save());
  }

  sceneJson["nodes"] = nodesJsonArray;

  QJsonArray connectionJsonArray;
  for (auto const & pair : _dataFlowModel->_connections)
  {
    auto const &connection = pair.second;

    QJsonObject connectionJson = connection->save();

    if (!connectionJson.isEmpty())
      connectionJsonArray.append(connectionJson);
  }

  sceneJson["connections"] = connectionJsonArray;

  QJsonDocument document(sceneJson);

  return document.toJson();
}


void
DataFlowScene::
loadFromMemory(const QByteArray& data)
{
  QJsonObject const jsonDocument = QJsonDocument::fromJson(data).object();

  QJsonArray nodesJsonArray = jsonDocument["nodes"].toArray();

  for (int i = 0; i < nodesJsonArray.size(); ++i)
  {
    restoreNode(nodesJsonArray[i].toObject());
  }

  QJsonArray connectionJsonArray = jsonDocument["connections"].toArray();

  for (int i = 0; i < connectionJsonArray.size(); ++i)
  {
    restoreConnection(connectionJsonArray[i].toObject());
  }
}

DataFlowScene::DataFlowModel::DataFlowModel(std::shared_ptr<DataModelRegistry> registry) 
  : _registry(std::move(registry)) {
}


// FlowSceneModel read interface
QStringList DataFlowScene::DataFlowModel::modelRegistry() const {
  QStringList list;
  for (const auto& item : _registry->registeredModels()) {
    list << item.first;
  }
  return list;
}
QString DataFlowScene::DataFlowModel::nodeTypeCatergory(QString const& name) const {
  auto iter = _registry->registeredModelsCategoryAssociation().find(name);
  
  if (iter != _registry->registeredModelsCategoryAssociation().end()) {
    return iter->second;
  }
  return {};
}
QString DataFlowScene::DataFlowModel::converterNode(NodeDataType const& lhs, NodeDataType const& rhs) const {
  auto conv =  _registry->getTypeConverter(lhs.id, rhs.id);
  
  if (!conv) return {};
  
  return conv->name();
}
QList<QUuid> DataFlowScene::DataFlowModel::nodeUUids() const {
  QList<QUuid> ret;

  // extract the keys
  std::transform(_nodes.begin(), _nodes.end(), std::back_inserter(ret), [](const auto& pair) {
    return pair.first;
  });

  return ret;
}
NodeIndex DataFlowScene::DataFlowModel::nodeIndex(const QUuid& ID) const {
  auto iter = _nodes.find(ID);
  if (iter == _nodes.end()) return {};
  
  return createIndex(ID, iter->second.get());
}
QString DataFlowScene::DataFlowModel::nodeTypeIdentifier(NodeIndex const& index) const {
  Q_ASSERT(index.isValid());
  
  auto* node = static_cast<Node*>(index.internalPointer());
  
  return node->nodeDataModel()->name();
}
QString DataFlowScene::DataFlowModel::nodeCaption(NodeIndex const& index) const {
  Q_ASSERT(index.isValid());
  
  auto* node = static_cast<Node*>(index.internalPointer());
  
  if (!node->nodeDataModel()->captionVisible()) return {};
  
  return node->nodeDataModel()->caption();
}
QPointF DataFlowScene::DataFlowModel::nodeLocation(NodeIndex const& index) const {
  Q_ASSERT(index.isValid());
  
  auto* node = static_cast<Node*>(index.internalPointer());
  
  return node->position();
}
QWidget* DataFlowScene::DataFlowModel::nodeWidget(NodeIndex const& index) const {
  Q_ASSERT(index.isValid());
  
  auto* node = static_cast<Node*>(index.internalPointer());
  
  return node->nodeDataModel()->embeddedWidget();
}
bool DataFlowScene::DataFlowModel::nodeResizable(NodeIndex const& index) const {
  Q_ASSERT(index.isValid());
  
  auto* node = static_cast<Node*>(index.internalPointer());
  
  return node->nodeDataModel()->resizable();
}
NodeValidationState DataFlowScene::DataFlowModel::nodeValidationState(NodeIndex const& index) const {
  Q_ASSERT(index.isValid());
  
  auto* node = static_cast<Node*>(index.internalPointer());
  
  return node->nodeDataModel()->validationState();
}
QString DataFlowScene::DataFlowModel::nodeValidationMessage(NodeIndex const& index) const {
  Q_ASSERT(index.isValid());
  
  auto* node = static_cast<Node*>(index.internalPointer());
  
  return node->nodeDataModel()->validationMessage();
}
NodePainterDelegate* DataFlowScene::DataFlowModel::nodePainterDelegate(NodeIndex const& index) const {
  Q_ASSERT(index.isValid());
  
  auto* node = static_cast<Node*>(index.internalPointer());
  
  return node->nodeDataModel()->painterDelegate();
}
unsigned int DataFlowScene::DataFlowModel::nodePortCount(NodeIndex const& index, PortType portType) const {
  Q_ASSERT(index.isValid());
  
  auto* node = static_cast<Node*>(index.internalPointer());
  
  return node->nodeDataModel()->nPorts(portType);
}
QString DataFlowScene::DataFlowModel::nodePortCaption(NodeIndex const& index, PortIndex pIndex, PortType portType) const {
  Q_ASSERT(index.isValid());
  
  auto* node = static_cast<Node*>(index.internalPointer());
  
  return node->nodeDataModel()->portCaption(portType, pIndex);
}
NodeDataType DataFlowScene::DataFlowModel::nodePortDataType(NodeIndex const& index, PortIndex pIndex, PortType portType) const {
  Q_ASSERT(index.isValid());
  
  auto* node = static_cast<Node*>(index.internalPointer());
  
  return node->nodeDataModel()->dataType(portType, pIndex);
}
ConnectionPolicy DataFlowScene::DataFlowModel::nodePortConnectionPolicy(NodeIndex const& index, PortIndex pIndex, PortType portType) const {
  Q_ASSERT(index.isValid());
  
  auto* node = static_cast<Node*>(index.internalPointer());
  
  if (portType == PortType::In) {
    return ConnectionPolicy::One;
  }
  return node->nodeDataModel()->portOutConnectionPolicy(pIndex);
}
std::vector<std::pair<NodeIndex, PortIndex>> DataFlowScene::DataFlowModel::nodePortConnections(NodeIndex const& index, PortIndex id, PortType portType) const {
  Q_ASSERT(index.isValid());
  
  auto* node = static_cast<Node*>(index.internalPointer());
  
  std::vector<std::pair<NodeIndex, PortIndex>> ret;
  // construct connections
  for (const auto& conn : node->connections(portType, id)) {
    ret.emplace_back(nodeIndex(conn->getNode(oppositePort(portType))->id()), conn->getPortIndex(oppositePort(portType)));
  }
  return ret;
}

// FlowSceneModel write interface
bool DataFlowScene::DataFlowModel::removeConnection(NodeIndex const& leftNodeIdx, PortIndex leftPortID, NodeIndex const& rightNodeIdx, PortIndex rightPortID) {
  Q_ASSERT(leftNodeIdx.isValid());
  Q_ASSERT(rightNodeIdx.isValid());
  
  auto* leftNode = static_cast<Node*>(leftNodeIdx.internalPointer());
  auto* rightNode = static_cast<Node*>(rightNodeIdx.internalPointer());
  
  ConnectionID connID;
  connID.lNodeID = leftNodeIdx.id();
  connID.rNodeID = rightNodeIdx.id();
  connID.lPortID = leftPortID;
  connID.rPortID = rightPortID;
  
  // remove it from the nodes
  auto& leftConns = leftNode->connections(PortType::Out, leftPortID);
  auto iter = std::find_if(leftConns.begin(), leftConns.end(), [&](Connection* conn){ return conn->id() == connID; });
  Q_ASSERT(iter != leftConns.end());
  leftConns.erase(iter);
  
  auto& rightConns = rightNode->connections(PortType::In, rightPortID);
  iter = std::find_if(rightConns.begin(), rightConns.end(), [&](Connection* conn){ return conn->id() == connID; });
  Q_ASSERT(iter != rightConns.end());
  rightConns.erase(iter);
  
  // remove it from the map
  _connections.erase(connID);
  
  // tell the view
  emit connectionRemoved(leftNodeIdx, leftPortID, rightNodeIdx, rightPortID);
  
  return true;
}
bool DataFlowScene::DataFlowModel::addConnection(NodeIndex const& leftNodeIdx, PortIndex leftPortID, NodeIndex const& rightNodeIdx, PortIndex rightPortID) {
  Q_ASSERT(leftNodeIdx.isValid());
  Q_ASSERT(rightNodeIdx.isValid());
  
  auto* leftNode = static_cast<Node*>(leftNodeIdx.internalPointer());
  auto* rightNode = static_cast<Node*>(rightNodeIdx.internalPointer());
  
  ConnectionID connID;
  connID.lNodeID = leftNodeIdx.id();
  connID.rNodeID = rightNodeIdx.id();
  connID.lPortID = leftPortID;
  connID.rPortID = rightPortID;

  // create the connection
  auto conn = std::make_shared<Connection>(*rightNode, rightPortID, *leftNode, leftPortID);
  _connections[connID] = conn;
  
  // add it to the nodes
  leftNode->connections(PortType::Out, leftPortID).push_back(conn.get());
  rightNode->connections(PortType::In, rightPortID).push_back(conn.get());
  
  // tell the view the connection was added
  emit connectionAdded(leftNodeIdx, leftPortID, rightNodeIdx, rightPortID);
  
  return true;
}
bool DataFlowScene::DataFlowModel::removeNode(NodeIndex const& index) {
  Q_ASSERT(index.isValid());
  
  auto* node = static_cast<Node*>(index.internalPointer());
  
  // make sure there are no connections left
#ifndef NDEBUG
  for (PortIndex idx = 0; idx < node->nodeDataModel()->nPorts(PortType::In); ++idx) {
    Q_ASSERT(node->connections(PortType::In, idx).empty());
  }
  for (PortIndex idx = 0; idx < node->nodeDataModel()->nPorts(PortType::Out); ++idx) {
    Q_ASSERT(node->connections(PortType::Out, idx).empty());
  }
#endif

  // remove it from the map
  _nodes.erase(index.id());
  
  // tell the view
  emit nodeRemoved(index.id());

  return true;
}
QUuid DataFlowScene::DataFlowModel::addNode(const QString& typeID, QPointF const& location) {
  // create the NodeDataModel
  auto model = _registry->create(typeID);
  if (!model) {
    return {};
  }
  
  // create the UUID
  QUuid nodeid = QUuid::createUuid();
  
  // create a node
  auto node = std::make_unique<Node>(std::move(model), nodeid);
  
  // cache the pointer so the connection can be made
  auto nodePtr = node.get();
  
  // add it to the map
  _nodes[nodeid] = std::move(node);
  
  // connect to the geometry gets updated
  connect(nodePtr, &Node::positionChanged, this, [this, nodeid](QPointF const&){ nodeMoved(nodeIndex(nodeid)); });
  
  // tell the view
  emit nodeAdded(nodeid);
  
  return nodeid;
}
bool DataFlowScene::DataFlowModel::moveNode(NodeIndex const& index, QPointF newLocation) {
  Q_ASSERT(index.isValid());
  
  auto* node = static_cast<Node*>(index.internalPointer());
  node->setPosition(newLocation);
  
  // no need to emit, it's done by the function already
  return true;
}

} // namespace QtNodes
