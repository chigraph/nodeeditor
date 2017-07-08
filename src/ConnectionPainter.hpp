#pragma once

#include <QtGui/QPainter>

#include <memory>


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
