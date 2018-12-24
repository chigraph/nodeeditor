#pragma once

#include <QtCore/QUuid>

#include <QtWidgets/QGraphicsObject>

#include "ConnectionGeometry.hpp"
#include "ConnectionState.hpp"
#include "NodeData.hpp"
#include "NodeIndex.hpp"
#include "PortType.hpp"

class QGraphicsSceneMouseEvent;

namespace QtNodes {

class FlowScene;
struct ConnectionID;

/// Graphic Object for connection.
class ConnectionGraphicsObject : public QGraphicsObject
{
  Q_OBJECT

  /**\brief ConnectionGeometry set boundingRect for the class, so, every time,
   * when values, which neded for change boundingRect, changes - we have to call
   * prepareGeometryChange. So we can call this function in ConnectionGeometry -
   * this is simple and we can forget about do it by hand. And it fix problems
   */
  friend ConnectionGeometry;

public:
  ConnectionGraphicsObject(NodeIndex const& leftNode,
                           PortIndex leftPortIndex,
                           NodeIndex const& rightNode,
                           PortIndex rightPortIndex,
                           FlowScene& scene);

  virtual ~ConnectionGraphicsObject();

  enum
  {
    Type = UserType + 2
  };
  int type() const override { return Type; }

public:
  NodeIndex node(PortType pType) const
  {
    return pType == PortType::In ? _rightNode : _leftNode;
  }
  PortIndex portIndex(PortType pType) const
  {
    return pType == PortType::In ? _rightPortIndex : _leftPortIndex;
  }

  FlowScene& flowScene() const { return _scene; }

  ConnectionGeometry const& geometry() const { return _geometry; }
  ConnectionGeometry& geometry() { return _geometry; }

  ConnectionState const& state() const { return _state; }
  ConnectionState& state() { return _state; }

  QRectF boundingRect() const override;

  QPainterPath shape() const override;

  ConnectionID id() const;

  NodeDataType dataType(PortType ty) const;

  /// Updates the position of both ends
  void move();

  void lock(bool locked);

  const FlowScene& scene() const { return _scene; }

  FlowScene& scene() { return _scene; }

protected:
  void paint(QPainter* painter,
             QStyleOptionGraphicsItem const* option,
             QWidget* widget = 0) override;

  void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

  void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

  void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

  void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;

  void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
  void addGraphicsEffect();

private:
  FlowScene& _scene;

  ConnectionGeometry _geometry;
  ConnectionState _state;

  NodeIndex _leftNode;
  NodeIndex _rightNode;

  PortIndex _leftPortIndex;
  PortIndex _rightPortIndex;
};
} // namespace QtNodes
