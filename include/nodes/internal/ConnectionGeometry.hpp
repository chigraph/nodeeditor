#pragma once

#include "PortType.hpp"

#include <QtCore/QPointF>
#include <QtCore/QRectF>

#include <iostream>

namespace QtNodes {

class ConnectionGraphicsObject;

class ConnectionGeometry
{
  friend ConnectionGraphicsObject;

public:
  /**\brief this class change bounding rect of obj, so, it have to call
   * prepareGeometrychange for do this safely
   */
  explicit ConnectionGeometry(ConnectionGraphicsObject& obj);

public:
  QPointF const& getEndPoint(PortType portType) const;

  void setEndPoint(PortType portType, QPointF const& point);

  void moveEndPoint(PortType portType, QPointF const& offset);

  QRectF boundingRect() const;

  std::pair<QPointF, QPointF> pointsC1C2() const;

  QPointF source() const { return _out; }
  QPointF sink() const { return _in; }

  double lineWidth() const { return _lineWidth; }

  bool hovered() const { return _hovered; }
  void setHovered(bool hovered) { _hovered = hovered; }

private:
  // local object coordinates
  ConnectionGraphicsObject& _obj;
  QPointF _in;
  QPointF _out;

  // int _animationPhase;

  double _lineWidth;

  bool _hovered;
};
} // namespace QtNodes
