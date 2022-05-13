#include <algorithm>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/uuid/name_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <fstream>
#include <thread>

#include "com/ConnectionInfoPublisher.hpp"
#include "logging/LogMacros.hpp"
#include "precice/types.hpp"
#include "utils/assertion.hpp"

namespace bfs = boost::filesystem;

namespace precice {
namespace com {

std::string impl::hashedFilePath(const std::string &acceptorName, const std::string &requesterName, const std::string &tag, Rank rank)
{
  constexpr int                  firstLevelLen = 2;
  boost::uuids::string_generator ns_gen;
  auto                           ns = ns_gen("af7ce8f2-a9ee-46cb-38ee-71c318aa3580"); // md5 hash of precice.org as namespace

  boost::uuids::name_generator gen{ns};
  std::string const            s    = acceptorName + tag + requesterName + std::to_string(rank);
  std::string                  hash = boost::uuids::to_string(gen(s));
  hash.erase(std::remove(hash.begin(), hash.end(), '-'), hash.end());

  auto p = bfs::path(hash.substr(0, firstLevelLen)) / hash.substr(firstLevelLen);

  return p.string();
}

std::string impl::localDirectory(const std::string &acceptorName, const std::string &requesterName, const std::string &addressDirectory)
{
  std::string directional = acceptorName + "-" + requesterName;

  auto p = bfs::path(addressDirectory) / "precice-run" / directional;

  return p.string();
}

std::string ConnectionInfoPublisher::getLocalDirectory() const
{
  return impl::localDirectory(acceptorName, requesterName, addressDirectory);
}

std::string ConnectionInfoPublisher::getFilename() const
{
  auto local  = getLocalDirectory();
  auto hashed = impl::hashedFilePath(acceptorName, requesterName, tag, rank);
  auto p      = bfs::path(getLocalDirectory()) / hashed;

  return p.string();
}

std::string ConnectionInfoReader::read() const
{
  auto path = getFilename();

  PRECICE_DEBUG("Waiting for connection file {}", path);
  const auto waitdelay = std::chrono::milliseconds(1);
  while (!bfs::exists(path)) {
    std::this_thread::sleep_for(waitdelay);
  }
  PRECICE_ASSERT(bfs::exists(path));
  PRECICE_DEBUG("Found connection file {}", path);

  std::ifstream ifs(path);
  PRECICE_CHECK(ifs, "Unable to open connection file {}", path);
  std::string addressData;
  std::getline(ifs, addressData);
  PRECICE_CHECK(!addressData.empty(), "The connection file {} is empty. Please report this bug to the preCICE developers.", path);
  boost::algorithm::trim_right(addressData);
  return addressData;
}

ConnectionInfoWriter::~ConnectionInfoWriter()
{
  try {
    bfs::path p(getFilename());
    PRECICE_DEBUG("Deleting connection file {}", p.string());
    bfs::remove(p);
  } catch (const bfs::filesystem_error &e) {
    PRECICE_WARN("Unable to delete connection file due to error: {}", e.what());
  }
}

void ConnectionInfoWriter::write(std::string const &info) const
{
  auto path = getFilename();
  auto tmp  = bfs::path(path + "~");

  {
    auto message = "A {}connection file already exists at {}. "
                   "This is likely a leftover of a previous crash during communication build-up. "
                   "Please remove the \"precice-run\" directory and restart the simulation.";
    PRECICE_CHECK(!bfs::exists(path), message, "", path);
    PRECICE_CHECK(!bfs::exists(tmp), message, "temporary ")
  }

  PRECICE_DEBUG("Writing temporary connection file {}", tmp);
  bfs::create_directories(tmp.parent_path());
  {
    std::ofstream ofs(tmp.string());
    PRECICE_CHECK(ofs, "Unable to open temporary connection file {}", tmp);
    fmt::print(ofs,
               "{}\nAcceptor: {}, Requester: {}, Tag: {}, Rank: {}",
               info, acceptorName, requesterName, tag, rank);
  }
  PRECICE_CHECK(bfs::exists(tmp),
                "The temporary connection file {} doesn't exists. "
                "Please report this bug to the preCICE developers.",
                tmp);

  PRECICE_DEBUG("Publishing connection file {}", path);
  bfs::rename(tmp, path);
  /// @TODO Check if we need to sleep to let the underlying filesystem synchronize
  if (bfs::exists(tmp)) {
    PRECICE_WARN("The temporary connection file {} wasn't properly removed. "
                 "Make sure to delete the \"precice-run\" directory before restarting the simulation.",
                 tmp);
  }
  PRECICE_CHECK(bfs::exists(path),
                "The connection file {} was written, but doesn't exist on disk. "
                "Please report this bug to the preCICE developers.",
                path);
}

} // namespace com
} // namespace precice
