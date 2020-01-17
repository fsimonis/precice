#pragma once
#include <string>
#include "logging/Logger.hpp"

namespace precice {
namespace com {

namespace impl {
/// Returns the file name for the connection information.
/**
   * It has the form first two letters from hash of 
   * (acceptorName, requesterName, mesh, rank)/rest of hash.
   */
std::string hashedFilePath(const std::string &acceptorName, const std::string &requesterName, const std::string &meshName, int rank);

/** Returns the local directory which is the root for storing connection information.
   * It has the form addressDirectory/precice-run/acceptorName-requesterName
   */
std::string localDirectory(const std::string &acceptorName, const std::string &requesterName, const std::string &addressDirectory);
} // namespace impl

class ConnectionInfoPublisher {
public:
  ConnectionInfoPublisher(std::string acceptorName,
                          std::string requesterName,
                          std::string meshName,
                          int         rank,
                          std::string addressDirectory) noexcept
      : acceptorName(std::move(acceptorName)),
        requesterName(std::move(requesterName)),
        meshName(std::move(meshName)),
        rank(rank),
        addressDirectory(std::move(addressDirectory))
  {
  }

  ConnectionInfoPublisher(std::string acceptorName,
                          std::string requesterName,
                          std::string meshName,
                          std::string addressDirectory) noexcept
      : acceptorName(std::move(acceptorName)),
        requesterName(std::move(requesterName)),
        meshName(std::move(meshName)),
        addressDirectory(std::move(addressDirectory))
  {
  }

protected:
  std::string const acceptorName;
  std::string const requesterName;
  std::string const meshName;
  int const         rank = -1;
  std::string const addressDirectory;

  /// Returns the local directory which is used to store the hashed part.
  std::string getLocalDirectory() const;

  /// Returns the full path to the hashed filename
  std::string getFilename() const;

  mutable logging::Logger _log{"com::ConnectionInfoPublisher"};
};

/// Reads the connection info for the given participant/rank information
class ConnectionInfoReader : public ConnectionInfoPublisher {
public:
  ConnectionInfoReader(std::string acceptorName,
                       std::string requesterName,
                       std::string meshName,
                       int         rank,
                       std::string addressDirectory) noexcept
      : ConnectionInfoPublisher(acceptorName, requesterName, meshName, rank, addressDirectory)
  {
  }

  ConnectionInfoReader(std::string acceptorName,
                       std::string requesterName,
                       std::string meshName,
                       std::string addressDirectory) noexcept
      : ConnectionInfoPublisher(acceptorName, requesterName, meshName, addressDirectory)
  {
  }

  /// Reads the info from the connection info file. Will block, if the the file is not present.
  std::string read() const;
};

/// Writes the connection info for the given participant/rank information.
/**
 * The file is removed, when the object is destroyed.
 */
class ConnectionInfoWriter : public ConnectionInfoPublisher {
public:
  ConnectionInfoWriter(std::string acceptorName,
                       std::string requesterName,
                       std::string meshName,
                       int         rank,
                       std::string addressDirectory) noexcept
      : ConnectionInfoPublisher(acceptorName, requesterName, meshName, rank, addressDirectory)
  {
  }

  ConnectionInfoWriter(std::string acceptorName,
                       std::string requesterName,
                       std::string meshName,
                       std::string addressDirectory) noexcept
      : ConnectionInfoPublisher(acceptorName, requesterName, meshName, addressDirectory)
  {
  }

  /// Removes the connection info file and the directories ./precice-run/[hash], is empty.
  ~ConnectionInfoWriter();

  /// Write the string info, e.g. IP:port to the connection info file
  /**
   * which is determined by acceptorName, requesterName, rank, addressDirectory
   * set at construction.
   */
  void write(std::string const &info) const;
};

} // namespace com
} // namespace precice
