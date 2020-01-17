#include "m2n/BoundM2N.hpp"
#include "com/Communication.hpp"
#include "m2n/M2N.hpp"

namespace precice {
namespace m2n {

void BoundM2N::prepareEstablishment()
{
  if (!isRequesting) {
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
  if (isRequesting) {
    m2n->requestSlavesConnection(remoteName, localName);
  } else {
    m2n->acceptSlavesConnection(localName, remoteName);
  }
}

void BoundM2N::cleanupEstablishment()
{
  if (!isRequesting) {
    m2n->cleanupEstablishment(localName, remoteName);
  }
}

} // namespace m2n
} // namespace precice
