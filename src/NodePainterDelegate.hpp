#pragma once

#include <QPainter>

#include "Export.hpp"

namespace QtNodes {

class NodeGraphicsObject;
  
/// Class to allow for custom painting
class NODE_EDITOR_PUBLIC NodePainterDelegate
{
public:

  virtual
  ~NodePainterDelegate() = default;

  virtual void
  paint(QPainter* painter,
        NodeGraphicsObject const& ngo) = 0;
};
}
