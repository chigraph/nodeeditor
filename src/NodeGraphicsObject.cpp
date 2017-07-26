#include "NodeGraphicsObject.hpp"

#include <iostream>
#include <cstdlib>

#include <QtWidgets/QtWidgets>
#include <QtWidgets/QGraphicsEffect>

#include "ConnectionGraphicsObject.hpp"
#include "ConnectionState.hpp"

#include "FlowScene.hpp"
#include "NodePainter.hpp"

#include "NodeConnectionInteraction.hpp"

#include "StyleCollection.hpp"

#include "NodeIndex.hpp"
#include "FlowSceneModel.hpp"

namespace QtNodes {

NodeGraphicsObject::
NodeGraphicsObject(FlowScene& scene, const NodeIndex& index)
  : _scene(scene)
  , _nodeIndex(index)
  , _geometry(index)
  , _state(index)
  , _locked(false)
  , _proxyWidget(nullptr)
{
  _scene.addItem(this);

  setFlag(QGraphicsItem::ItemDoesntPropagateOpacityToChildren, true);
  setFlag(QGraphicsItem::ItemIsMovable, true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
  setFlag(QGraphicsItem::ItemIsSelectable, true);
  setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);

  setCacheMode( QGraphicsItem::DeviceCoordinateCache );

  auto const &nodeStyle = StyleCollection::nodeStyle();

  {
    auto effect = new QGraphicsDropShadowEffect;
    effect->setOffset(4, 4);
    effect->setBlurRadius(20);
    effect->setColor(nodeStyle.ShadowColor);

    setGraphicsEffect(effect);
  }

  setOpacity(nodeStyle.Opacity);

  setAcceptHoverEvents(true);

  setZValue(0);

  embedQWidget();

  // connect to the move signals
  auto onMoveSlot = [this] {
    
    // ask the model to move it
    if (!flowScene().model()->moveNode(_nodeIndex, pos())) {
      // set the location back
      setPos(_nodeIndex.model()->nodeLocation(_nodeIndex));
      moveConnections();
    }
    
  };
  connect(this, &QGraphicsObject::xChanged, this, onMoveSlot);
  connect(this, &QGraphicsObject::yChanged, this, onMoveSlot);
}

NodeGraphicsObject::
~NodeGraphicsObject() {
  if (_proxyWidget) {
    delete _proxyWidget->widget();
  }
}

NodeIndex
NodeGraphicsObject::
index() const
{
  return _nodeIndex;
}


FlowScene&
NodeGraphicsObject::
flowScene()
{
  return _scene;
}

FlowScene const&
NodeGraphicsObject::
flowScene() const {
  return _scene;
}

NodeGeometry&
NodeGraphicsObject::
geometry() {
  return _geometry;
}

NodeGeometry const&
NodeGraphicsObject::
geometry() const {
  return _geometry;
}


NodeState&
NodeGraphicsObject::
nodeState() {
  return _state;
}

NodeState const&
NodeGraphicsObject::
nodeState() const {
  return _state;
}

void
NodeGraphicsObject::
embedQWidget()
{

  if (auto w = _nodeIndex.model()->nodeWidget(_nodeIndex))
  {
    _proxyWidget = new QGraphicsProxyWidget(this);

    _proxyWidget->setWidget(w);

    _proxyWidget->setPreferredWidth(5);

    _geometry.recalculateSize();

    _proxyWidget->setPos(_geometry.widgetPosition());

    update();

    _proxyWidget->setOpacity(1.0);
    _proxyWidget->setFlag(QGraphicsItem::ItemIgnoresParentOpacity);
  }
}


QRectF
NodeGraphicsObject::
boundingRect() const
{
  return _geometry.boundingRect();
}


void
NodeGraphicsObject::
setGeometryChanged()
{
  prepareGeometryChange();
}


void
NodeGraphicsObject::
moveConnections() const
{
  auto moveConnections =
    [&](PortType portType)
    {
      auto const & connectionEntries =
        _state.getEntries(portType);

      for (auto const & connections : connectionEntries)
      {
        for (auto & con : connections)
          con->move();
      }
    };

  moveConnections(PortType::In);

  moveConnections(PortType::Out);
}



void
NodeGraphicsObject::
reactToPossibleConnection(PortType reactingPortType,

                          NodeDataType reactingDataType,
                          QPointF const &scenePoint)
{
  QTransform const t = sceneTransform();

  QPointF p = t.inverted().map(scenePoint);

  _geometry.setDraggingPosition(p);

  update();

  _state.setReaction(NodeState::REACTING,
                         reactingPortType,
                         reactingDataType);
}


void
NodeGraphicsObject::
resetReactionToConnection()
{
  _state.setReaction(NodeState::NOT_REACTING);
  update();
}

void NodeGraphicsObject::lock(bool locked)
{
  _locked = locked;
  setFlag(QGraphicsItem::ItemIsMovable, !locked);
  setFlag(QGraphicsItem::ItemIsFocusable, !locked);
  setFlag(QGraphicsItem::ItemIsSelectable, !locked);
}


void
NodeGraphicsObject::
paint(QPainter * painter,
      QStyleOptionGraphicsItem const* option,
      QWidget* )
{
  painter->setClipRect(option->exposedRect);

  NodePainter::paint(painter, *this);
}


QVariant
NodeGraphicsObject::
itemChange(GraphicsItemChange change, const QVariant &value)
{
  if (change == ItemPositionChange && scene())
  {
    moveConnections();
  }

  return QGraphicsItem::itemChange(change, value);
}


void
NodeGraphicsObject::
mousePressEvent(QGraphicsSceneMouseEvent * event)
{
  if(_locked) return;

  // deselect all other items after this one is selected
  if (!isSelected() &&
      !(event->modifiers() & Qt::ControlModifier))
  {
    _scene.clearSelection();
  }

  auto clickPort =
    [&](PortType portToCheck)
    {
      auto& model = *_nodeIndex.model();
      
      // TODO do not pass sceneTransform
      int portIndex = _geometry.checkHitScenePoint(portToCheck,
                                                      event->scenePos(),
                                                      sceneTransform());

      if (portIndex != INVALID)
      {

        NodeState::ConnectionPtrSet const& connections =
          _state.connections(portToCheck, portIndex);

        // start dragging existing connection if it's setup to only have one connection
        if (!connections.empty() && model.nodePortConnectionPolicy(_nodeIndex, portIndex, portToCheck) == ConnectionPolicy::One)
        {
          auto con = connections[0];
          
          // cache the nodes and port indicies because the connection will be deleted
          auto lNode = con->node(PortType::Out);
          auto rNode = con->node(PortType::In);
          auto lPortIdx = con->portIndex(PortType::Out);
          auto rPortIdx = con->portIndex(PortType::In);

          NodeConnectionInteraction interaction(_nodeIndex, *con);

          if (!interaction.disconnect(portToCheck)) {
            return;
          }
          
          // initialize a new connection
          if (portToCheck == PortType::In) {
            _scene._temporaryConn = std::make_unique<ConnectionGraphicsObject>(lNode, lPortIdx, NodeIndex{}, -1, _scene);
            _scene._temporaryConn->geometry().setEndPoint(PortType::In, event->scenePos());
          } else {
            _scene._temporaryConn = std::make_unique<ConnectionGraphicsObject>(NodeIndex{}, -1, rNode, rPortIdx, _scene);
            _scene._temporaryConn->geometry().setEndPoint(PortType::Out, event->scenePos());
          }
          
          _scene._temporaryConn->grabMouse();
        }
        else // initialize new Connection
        {
          if (portToCheck == PortType::In) {
            _scene._temporaryConn = std::make_unique<ConnectionGraphicsObject>(NodeIndex{}, -1, _nodeIndex, portIndex, _scene);
            _scene._temporaryConn->geometry().setEndPoint(PortType::Out, event->scenePos());
          } else {
            _scene._temporaryConn = std::make_unique<ConnectionGraphicsObject>(_nodeIndex, portIndex, NodeIndex{}, -1, _scene);
            _scene._temporaryConn->geometry().setEndPoint(PortType::In, event->scenePos());
          }
          
          _scene._temporaryConn->grabMouse();
        }
      }
    };

  clickPort(PortType::In);
  clickPort(PortType::Out);

  auto pos     = event->pos();
  
  if (_geometry.resizeRect().contains(QPoint(pos.x(),
                                        pos.y())))
  {
    _state.setResizing(true);
  }
}

void
NodeGraphicsObject::
mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{

  if (_state.resizing())
  {
    auto diff = event->pos() - event->lastPos();

    if (auto w = _nodeIndex.model()->nodeWidget(_nodeIndex))
    {
      prepareGeometryChange();

      auto oldSize = w->size();

      oldSize += QSize(diff.x(), diff.y());

      w->setFixedSize(oldSize);

      _proxyWidget->setMinimumSize(oldSize);
      _proxyWidget->setMaximumSize(oldSize);
      _proxyWidget->setPos(_geometry.widgetPosition());

      _geometry.recalculateSize();
      update();

      moveConnections();

      event->accept();
    }
  }
  else
  {
    QGraphicsObject::mouseMoveEvent(event);

    if (event->lastPos() != event->pos())
      moveConnections();

    event->ignore();
  }

  QRectF r = scene()->sceneRect();

  r = r.united(mapToScene(boundingRect()).boundingRect());

  scene()->setSceneRect(r);
}


void
NodeGraphicsObject::
mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  _state.setResizing(false);

  QGraphicsObject::mouseReleaseEvent(event);

  // position connections precisely after fast node move
  moveConnections();
}


void
NodeGraphicsObject::
hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
  // bring all the colliding nodes to background
  QList<QGraphicsItem *> overlapItems = collidingItems();

  for (QGraphicsItem *item : overlapItems)
  {
    if (item->zValue() > 0.0)
    {
      item->setZValue(0.0);
    }
  }
  // bring this node forward
  setZValue(1.0);

  _geometry.setHovered(true);
  update();
  flowScene().model()->nodeHovered(_nodeIndex, event->screenPos(), true);
  event->accept();
}


void
NodeGraphicsObject::
hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
  _geometry.setHovered(false);
  update();
  flowScene().model()->nodeHovered(_nodeIndex, event->screenPos(), false);
  event->accept();
}

void
NodeGraphicsObject::
hoverMoveEvent(QGraphicsSceneHoverEvent * event)
{
  auto pos    = event->pos();

  if (_geometry.resizeRect().contains(QPoint(pos.x(),
                                        pos.y())))
  {
    setCursor(QCursor(Qt::SizeFDiagCursor));
  }
  else
  {
    setCursor(QCursor());
  }

  event->accept();
}


void
NodeGraphicsObject::
mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
  QGraphicsItem::mouseDoubleClickEvent(event);

  flowScene().model()->nodeDoubleClicked(_nodeIndex, event->screenPos());
}

} // namespace QtNodes
