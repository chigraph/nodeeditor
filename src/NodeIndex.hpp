#pragma once

#include <cstddef>
#include <limits>

#include <QUuid>
#include <QtGlobal>

namespace QtNodes {

class FlowSceneModel;

class NodeIndex {
  friend FlowSceneModel;
public:

  /// Construct an invalid NodeIndex
  NodeIndex() = default;

private:
  /// Regular constructor
  NodeIndex(const QUuid& uuid, void* internalPtr, FlowSceneModel* model) 
    : _id {uuid}, _internalPointer{internalPtr}, _model{model} {
      Q_ASSERT(!_id.isNull());
      Q_ASSERT(_model != nullptr);
    }

public:

  /// Get the internal pointer
  void* internalPointer() const { return _internalPointer; }

  /// Get the owning model
  FlowSceneModel* model() const { return _model; }

  /// Get the id for the node
  QUuid id() const { return _id; }
  
  /// Test if it's valid
  bool isValid() const {
    return !id().isNull() && model() != nullptr;
  }


private:

  QUuid _id;
  void* _internalPointer = nullptr;
  FlowSceneModel* _model = nullptr;
};

}

