#include "m2n/BoundM2N.hpp"
#include "com/Communication.hpp"
#include "m2n/M2N.hpp"

#include "logging/Logger.hpp"

namespace precice {
namespace m2n {

void BoundM2N::prepareEstablishment()
{
  if (isRequesting) {
    m2n->prepareEstablishment(remoteName, localName);
  } else {
    m2n->prepareEstablishment(localName, remoteName);
  }
}

void BoundM2N::connectMasters()
{
  std::string fullLocalName = localName;

  if (isRequesting) {
    m2n->requestMasterConnection(remoteName, fullLocalName);
  } else {
    m2n->acceptMasterConnection(fullLocalName, remoteName);
  }
}

void BoundM2N::connectSlaves()
{
  if (m2n->usesTwoLevelInitialization()) {
    PRECICE_DEBUG("Update slaves connections");
    m2n->completeSlavesConnection();
  } else {
    if (isRequesting) {
      PRECICE_DEBUG("Awaiting slaves connection from " << remoteName);
      m2n->requestSlavesConnection(remoteName, localName);
      PRECICE_DEBUG("Established slaves connection from " << remoteName);
    } else {
      PRECICE_DEBUG("Establishing slaves connection to " << remoteName);
      m2n->acceptSlavesConnection(localName, remoteName);
      PRECICE_DEBUG("Established  slaves connection to " << remoteName);
    }
  }
}

void BoundM2N::preConnectSlaves()
{
  if (not m2n->usesTwoLevelInitialization())
    return;

  PRECICE_WARN("Two-level initialization is still in beta testing. Several edge cases are known to fail. Please report problems nevertheless.");

  if (isRequesting) {
    PRECICE_DEBUG("Awaiting preliminary slaves connection from " << remoteName);
    m2n->requestSlavesPreConnection(remoteName, localName);
    PRECICE_DEBUG("Established preliminary slaves connection from " << remoteName);
  } else {
    PRECICE_DEBUG("Establishing preliminary slaves connection to " << remoteName);
    m2n->acceptSlavesPreConnection(localName, remoteName);
    PRECICE_DEBUG("Established preliminary slaves connection to " << remoteName);
  }
}

void BoundM2N::cleanupEstablishment()
{
  if (isRequesting) {
    m2n->cleanupEstablishment(remoteName, localName);
  } else {
    m2n->cleanupEstablishment(localName, remoteName);
  }
}

} // namespace m2n
} // namespace precice
