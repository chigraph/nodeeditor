#include "NodeConnectionInteraction.hpp"

#include "ConnectionGraphicsObject.hpp"
#include "NodeGraphicsObject.hpp"
#include "NodeDataModel.hpp"
#include "DataModelRegistry.hpp"
#include "FlowScene.hpp"

namespace QtNodes {

NodeConnectionInteraction::
NodeConnectionInteraction(NodeIndex const& node,
                            ConnectionGraphicsObject& connection)
  : _node(node)
  , _connection(&connection)
{}

bool
NodeConnectionInteraction::
canConnect(PortIndex &portIndex, bool& typeConversionNeeded, QString& converterModel) const
{
  typeConversionNeeded = false;

  // 1) Connection requires a port

  PortType requiredPort = connectionRequiredPort();

  if (requiredPort == PortType::None)
  {
    return false;
  }

  // 2) connection point is on top of the node port

  QPointF connectionPoint = connectionEndScenePosition(requiredPort);

  portIndex = nodePortIndexUnderScenePoint(requiredPort,
                                           connectionPoint);

  if (portIndex == INVALID)
  {
    return false;
  }

  // 3) Node port is vacant

  // port should be empty
  if (!nodePortIsEmpty(requiredPort, portIndex))
    return false;

  // 4) Connection type equals node port type, or there is a registered type conversion that can translate between the two

  auto connectionDataType = _connection->dataType();

  auto const   &modelTarget = _node.model();
  NodeDataType candidateNodeDataType = modelTarget->nodePortDataType(_node, portIndex, requiredPort);

  // if the types don't match, try a conversion
  if (connectionDataType.id != candidateNodeDataType.id)
  {
    if (requiredPort == PortType::In)
    {
      return typeConversionNeeded = (converterModel = modelTarget->converterNode(connectionDataType, candidateNodeDataType)) != QString{};
    }
    return typeConversionNeeded = (converterModel = modelTarget->converterNode(candidateNodeDataType, connectionDataType)) != QString{};
  }

  return true;
}


bool
NodeConnectionInteraction::
tryConnect() const
{
  // 1) Check conditions from 'canConnect'
  PortIndex portIndex = INVALID;
  bool typeConversionNeeded = false;

  QString typeConverterModel;
  if (!canConnect(portIndex, typeConversionNeeded, typeConverterModel))
  {
    return false;
  }
  
  auto model = _connection->flowScene().model();
  
  
  //Determining port types
  PortType requiredPort = connectionRequiredPort();
  PortType connectedPort = oppositePort(requiredPort);
  
  //Get the node and port from where the connection starts
  auto outNode = _connection->node(connectedPort);
  Q_ASSERT(outNode.isValid());
  
  auto outNodePortIndex = _connection->portIndex(connectedPort);
  
  /// 1.5) If the connection is possible but a type conversion is needed, add a converter node to the scene, and connect it properly
  if (typeConversionNeeded)
  {

    
    // try to create the converter  node
    QUuid newNodeID = model->addNode(typeConverterModel, QPointF{});
    if (!newNodeID.isNull()) {
      // node was created!
      NodeIndex converterNode = model->nodeIndex(newNodeID);
      
      // get the graphics object
      auto convertedGraphics = _connection->flowScene().nodeGraphicsObject(converterNode);
      Q_ASSERT(convertedGraphics);
      
      auto thisNodeGraphics = _connection->flowScene().nodeGraphicsObject(_node);
      
      auto outGraphics = _connection->flowScene().nodeGraphicsObject(outNode);
      
      // move it
      auto converterNodePos = NodeGeometry::calculateNodePositionBetweenNodePorts(portIndex, requiredPort, *thisNodeGraphics, outNodePortIndex, connectedPort, *outGraphics, convertedGraphics->geometry());
      
      // if this fails, well at least we tried--keep on going
      model->moveNode(converterNode, converterNodePos);
      
      // connect it in all the right places
      
      //The connection order is different based on if the users connection was started from an input port, or an output port.
      if (requiredPort == PortType::In)
      {
        // hopefully this works...don't fail even if it doesn't
        model->addConnection(converterNode, 0, _node, portIndex);
        model->addConnection(outNode, outNodePortIndex, converterNode, 0);
      }
      else
      {
        // hopefully this works...don't fail even if it doesn't
        model->addConnection(converterNode, 0, outNode, outNodePortIndex);
        model->addConnection(_node, portIndex, converterNode, 0);
      }

    } else {
      // couln't create the node
      return false;
    }

    return true;
  }

  // 2) Assign node to required port in Connection
  if (connectionRequiredPort() == PortType::In) {
    return model->addConnection(outNode, outNodePortIndex, _node, portIndex);
  }
  return model->addConnection(_node, portIndex, outNode, outNodePortIndex);
}


/// 1) Node and Connection should be already connected
/// 2) If so, clear Connection entry in the NodeState
/// 3) Set Connection end to 'requiring a port'
bool
NodeConnectionInteraction::
disconnect(PortType portToDisconnect) const
{
  // try to disconnect it
  auto model = _connection->flowScene().model();
  
  return model->removeConnection(_connection->node(PortType::Out), _connection->portIndex(PortType::Out), 
                                 _connection->node(PortType::In), _connection->portIndex(PortType::In));
}


// ------------------ util fconnectionStateunctions below

PortType
NodeConnectionInteraction::
connectionRequiredPort() const
{
  auto const &state = _connection->state();

  return state.requiredPort();
}


QPointF
NodeConnectionInteraction::
connectionEndScenePosition(PortType portType) const
{
  ConnectionGeometry& geometry = _connection->geometry();

  QPointF endPoint = geometry.getEndPoint(portType);

  return _connection->mapToScene(endPoint);
}


QPointF
NodeConnectionInteraction::
nodePortScenePosition(PortType portType, PortIndex portIndex) const
{

  NodeGraphicsObject const& ngo = *_connection->flowScene().nodeGraphicsObject(_node);
  
  NodeGeometry const &geom = ngo.geometry();

  QPointF p = geom.portScenePosition(portIndex, portType);

  return ngo.sceneTransform().map(p);
}


PortIndex
NodeConnectionInteraction::
nodePortIndexUnderScenePoint(PortType portType,
                             QPointF const & scenePoint) const
{
  NodeGraphicsObject const& ngo = *_connection->flowScene().nodeGraphicsObject(_node);
  NodeGeometry const &nodeGeom = ngo.geometry();

  QTransform sceneTransform =
    ngo.sceneTransform();

  PortIndex portIndex = nodeGeom.checkHitScenePoint(portType,
                                                    scenePoint,
                                                    sceneTransform);
  return portIndex;
}


bool
NodeConnectionInteraction::
nodePortIsEmpty(PortType portType, PortIndex portIndex) const
{
  NodeState const & nodeState = _connection->flowScene().nodeGraphicsObject(_node)->nodeState();

  auto const & entries = nodeState.getEntries(portType);

  if (entries[portIndex].empty()) return true;

  const auto outPolicy = _node.model()->nodePortConnectionPolicy(_node, portIndex, portType);
  return outPolicy == ConnectionPolicy::Many;
}
} // namespace QtNodes
