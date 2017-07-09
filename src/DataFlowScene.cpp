#include "DataFlowScene.hpp"
#include "Connection.hpp"

#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>

namespace QtNodes {

DataFlowScene::DataFlowScene(std::shared_ptr<DataModelRegistry> registry) : FlowScene(new DataFlowModel) {
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
  connId.lhs = _dataFlowModel->nodeIndex(nodeOutId);
  connId.rhs = _dataFlowModel->nodeIndex(nodeInId);
  
  connId.lPortID = portIndexOut;
  connId.rPortID = portIndexIn;
  
  if (!_dataFlowModel->addConnection(connId.lhs, connId.lPortID, connId.rhs, connId.rPortID)) 
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
  FlowSceneModel::removeNodeWithConnections(node.index());
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




// FlowSceneModel read interface
QStringList DataFlowScene::DataFlowModel::modelRegistry() const {

}
QList<QUuid> DataFlowScene::DataFlowModel::nodeUUids() const {

}
NodeIndex DataFlowScene::DataFlowModel::nodeIndex(const QUuid& ID) const {

}
QString DataFlowScene::DataFlowModel::nodeTypeIdentifier(NodeIndex const& index) const {

}
QString DataFlowScene::DataFlowModel::nodeCaption(NodeIndex const& index) const {

}
QPointF DataFlowScene::DataFlowModel::nodeLocation(NodeIndex const& index) const {

}
QWidget* DataFlowScene::DataFlowModel::nodeWidget(NodeIndex const& index) const {

}
bool DataFlowScene::DataFlowModel::nodeResizable(NodeIndex const& index) const {

}
NodeValidationState DataFlowScene::DataFlowModel::nodeValidationState(NodeIndex const& index) const {

}
QString DataFlowScene::DataFlowModel::nodeValidationMessage(NodeIndex const& index) const {

}
NodePainterDelegate* DataFlowScene::DataFlowModel::nodePainterDelegate(NodeIndex const& index) const {

}
unsigned int DataFlowScene::DataFlowModel::nodePortCount(NodeIndex const& index, PortType portType) const {

}
QString DataFlowScene::DataFlowModel::nodePortCaption(NodeIndex const& index, PortIndex, PortType portType) const {

}
NodeDataType DataFlowScene::DataFlowModel::nodePortDataType(NodeIndex const& index, PortIndex, PortType portType) const {

}
ConnectionPolicy DataFlowScene::DataFlowModel::nodePortConnectionPolicy(NodeIndex const& index, PortIndex, PortType portType) const {

}
std::vector<std::pair<NodeIndex, PortIndex>> DataFlowScene::DataFlowModel::nodePortConnections(NodeIndex const& index, PortIndex id, PortType portType) const {

}

// FlowSceneModel write interface
bool DataFlowScene::DataFlowModel::removeConnection(NodeIndex const& leftNode, PortIndex leftPortID, NodeIndex const& rightNode, PortIndex rightPortID) {

}
bool DataFlowScene::DataFlowModel::addConnection(NodeIndex const& leftNode, PortIndex leftPortID, NodeIndex const& rightNode, PortIndex rightPortID) {

}
bool DataFlowScene::DataFlowModel::removeNode(NodeIndex const& index) {

}
QUuid DataFlowScene::DataFlowModel::addNode(const QString& typeID, QPointF const& location) {

}
bool DataFlowScene::DataFlowModel::moveNode(NodeIndex const& index, QPointF newLocation) {

}

} // namespace QtNodes
