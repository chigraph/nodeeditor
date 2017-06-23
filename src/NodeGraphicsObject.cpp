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
  , _proxyWidget(nullptr)
  , _locked(false)
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
    if (!_nodeIndex.model()->moveNode(_nodeIndex, pos())) {
      // set the location back
      setPos(_nodeIndex.model()->nodeLocation(_nodeIndex));
      moveConnections();
    }
    
  };
  connect(this, &QGraphicsObject::xChanged, this, onMoveSlot);
  connect(this, &QGraphicsObject::yChanged, this, onMoveSlot);
}


NodeGraphicsObject::
~NodeGraphicsObject()
{
  _scene.removeItem(this);
}


NodeIndex
NodeGraphicsObject::
index() const
{
  return _nodeIndex;
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
          con.second->getConnectionGraphicsObject().move();
      }
    };

  moveConnections(PortType::In);

  moveConnections(PortType::Out);
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

  NodePainter::paint(painter, _node, _scene);
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
      NodeGeometry & nodeGeometry = _node.nodeGeometry();

      // TODO do not pass sceneTransform
      int portIndex = nodeGeometry.checkHitScenePoint(portToCheck,
                                                      event->scenePos(),
                                                      sceneTransform());

      if (portIndex != INVALID)
      {
        NodeState const & nodeState = _node.nodeState();

        std::unordered_map<QUuid, Connection*> connections =
          nodeState.connections(portToCheck, portIndex);

        // start dragging existing connection
        if (!connections.empty() && portToCheck == PortType::In)
        {
          auto con = connections.begin()->second;

          NodeConnectionInteraction interaction(_node, *con, _scene);

          interaction.disconnect(portToCheck);
        }
        else // initialize new Connection
        {
          const auto outPolicy = _node.nodeDataModel()->portOutConnectionPolicy(portIndex);
          if (!connections.empty() &&
              portToCheck == PortType::Out &&
              outPolicy == NodeDataModel::ConnectionPolicy::One)
          {
            _scene.deleteConnection( *connections.begin()->second );
          }

          // todo add to FlowScene
          auto connection = _scene.createConnection(portToCheck,
                                                    _node,
                                                    portIndex);

          _node.nodeState().setConnection(portToCheck,
                                          portIndex,
                                          *connection);

          connection->getConnectionGraphicsObject().grabMouse();
        }
      }
    };

  clickPort(PortType::In);
  clickPort(PortType::Out);

  auto pos     = event->pos();
  auto & geom  = _node.nodeGeometry();
  auto & state = _node.nodeState();

  if (geom.resizeRect().contains(QPoint(pos.x(),
                                        pos.y())))
  {
    state.setResizing(true);
  }
}


void
NodeGraphicsObject::
mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
  auto & geom  = _node.nodeGeometry();
  auto & state = _node.nodeState();

  if (state.resizing())
  {
    auto diff = event->pos() - event->lastPos();

    if (auto w = _node.nodeDataModel()->embeddedWidget())
    {
      prepareGeometryChange();

      auto oldSize = w->size();

      oldSize += QSize(diff.x(), diff.y());

      w->setFixedSize(oldSize);

      _proxyWidget->setMinimumSize(oldSize);
      _proxyWidget->setMaximumSize(oldSize);
      _proxyWidget->setPos(geom.widgetPosition());

      geom.recalculateSize();
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
  auto & state = _node.nodeState();

  state.setResizing(false);

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

  _node.nodeGeometry().setHovered(true);
  update();
  _scene.nodeHovered(node(), event->screenPos());
  event->accept();
}


void
NodeGraphicsObject::
hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
  _node.nodeGeometry().setHovered(false);
  update();
  _scene.nodeHoverLeft(node());
  event->accept();
}


void
NodeGraphicsObject::
hoverMoveEvent(QGraphicsSceneHoverEvent * event)
{
  auto pos    = event->pos();
  auto & geom = _node.nodeGeometry();

  if (geom.resizeRect().contains(QPoint(pos.x(),
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

  _scene.nodeDoubleClicked(node());
}

} // namespace QtNodes
