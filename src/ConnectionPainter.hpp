#pragma once

#include <memory>

#include <QtGui/QPainter>

namespace QtNodes
{

class ConnectionGeometry;
class ConnectionGraphicsObject;

class ConnectionPainter
{
public:

  ConnectionPainter();

public:

  static
  QPainterPath
  cubicPath(ConnectionGeometry const& geom);

  static
  QPainterPath
  getPainterStroke(ConnectionGeometry const& geom);

  static
  void
  paint(QPainter* painter,
        ConnectionGraphicsObject const& cgo);
};
}
