#pragma once

#include "com/SharedPointer.hpp"
#include <stdexcept>
#include <string>

namespace precice {
namespace com {
class CommunicationFactory {

public:
  virtual ~CommunicationFactory(){};

  virtual PtrCommunication newCommunication() = 0;

  virtual std::string addressDirectory() {
    throw std::runtime_error("Not available!");
  }
};
} // namespace com
} // namespace precice
