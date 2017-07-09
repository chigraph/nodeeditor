#pragma once

#include "NodeIndex.hpp"
#include "PortType.hpp"

#include <utility>

namespace QtNodes {
struct ConnectionID {
  NodeIndex lhs;
  NodeIndex rhs;

  PortIndex lPortID;
  PortIndex rPortID;
  
};

inline bool operator==(ConnectionID const& lhs, ConnectionID const& rhs) {
  return lhs.lhs     == rhs.lhs &&
         lhs.rhs     == rhs.rhs &&
         lhs.lPortID == rhs.lPortID && 
         lhs.rPortID == rhs.rPortID;
}

} // namespace QtNodes

// hash for ConnectionID
namespace std {

template<>
struct hash<::QtNodes::ConnectionID> {
  size_t operator()(::QtNodes::ConnectionID const& toHash) const {
    return qHash(toHash.lhs.id()) ^ qHash(toHash.rhs.id()) ^ std::hash<QtNodes::PortIndex>()(toHash.lPortID) ^ std::hash<QtNodes::PortIndex>()(toHash.rPortID);
  }
};

} // namespace std

