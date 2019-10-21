#pragma once

#include "precice/impl/Participant.hpp"
#include "mesh/SharedPointer.hpp"
#include "mapping/SharedPointer.hpp"
#include "io/SharedPointer.hpp"
#include "logging/Logger.hpp"
#include "precice/config/ConfigurationListener.hpp"
#include <string>

namespace precice {
namespace config {

/**
 * @brief Performs XML configuration of a participant.
 */
class ParticipantConfiguration : public ConfigurationListener
{
public:

  ParticipantConfiguration (
    xml::XMLTag&                      parent,
    const ConfigurationContext&       context,
    const mesh::PtrMeshConfiguration& meshConfiguration);

  void setDimensions ( int dimensions );

  /**
   * @brief Callback function required for use of automatic configuration.
   *
   * @return True, if successful.
   */
  virtual void xmlTagCallback ( xml::XMLTag& callingTag );

  /**
   * @brief Callback function required for use of automatic configuration.
   *
   * @return True, if successful.
   */
  virtual void xmlEndTagCallback ( xml::XMLTag& callingTag );

  /**
   * @brief For manual configuration.
   */
  void addParticipant (
    const impl::PtrParticipant&             participant,
    const mapping::PtrMappingConfiguration& mappingConfig );

  /// Returns all configured participants.
  const std::vector<impl::PtrParticipant>& getParticipants() const;

private:

  struct WatchPointConfig
  {
    std::string name;
    std::string nameMesh;
    Eigen::VectorXd coordinates;
  };

  mutable logging::Logger _log{"config::ParticipantConfiguration"};

  const std::string TAG = "participant";
  const std::string TAG_WRITE = "write-data";
  const std::string TAG_READ = "read-data";
  const std::string TAG_DATA_ACTION = "data-action";
  const std::string TAG_USE_MESH = "use-mesh";
  const std::string TAG_WATCH_POINT = "watch-point";
  const std::string TAG_SERVER = "server";
  const std::string TAG_MASTER = "master";

  const std::string ATTR_NAME = "name";
  const std::string ATTR_SOURCE_DATA = "source-data";
  const std::string ATTR_TARGET_DATA = "target-data";
  const std::string ATTR_TIMING = "timing";
  const std::string ATTR_LOCAL_OFFSET = "offset";
  const std::string ATTR_ACTION_TYPE = "type";
  const std::string ATTR_FROM = "from";
  const std::string ATTR_SAFETY_FACTOR = "safety-factor";
  const std::string ATTR_GEOMETRIC_FILTER = "geometric-filter";
  const std::string ATTR_PROVIDE = "provide";
  const std::string ATTR_MESH = "mesh";
  const std::string ATTR_COORDINATE = "coordinate";
  const std::string ATTR_COMMUNICATION = "communication";
  const std::string ATTR_CONTEXT = "context";
  const std::string ATTR_NETWORK = "network";
  const std::string ATTR_EXCHANGE_DIRECTORY = "exchange-directory";

  const std::string VALUE_FILTER_FIRST = "filter-first";
  const std::string VALUE_BROADCAST_FILTER = "broadcast-filter";
  const std::string VALUE_NO_FILTER = "no-filter";

  const std::string VALUE_VTK = "vtk";

  int _dimensions = 0;

  mesh::PtrMeshConfiguration _meshConfig;

  mapping::PtrMappingConfiguration _mappingConfig;

  action::PtrActionConfiguration _actionConfig;

  io::PtrExportConfiguration _exportConfig;

  std::vector<impl::PtrParticipant> _participants;

  std::vector<WatchPointConfig> _watchPointConfigs;

  partition::ReceivedPartition::GeometricFilter getGeoFilter(const std::string& geoFilter) const;

  mesh::PtrMesh copy ( const mesh::PtrMesh& mesh ) const;

  const mesh::PtrData& getData (
    const mesh::PtrMesh& mesh,
    const std::string&   nameData ) const;

  mapping::PtrMapping getMapping ( const std::string& mappingName );

  void finishParticipantConfiguration ( const impl::PtrParticipant& participant );

};

}} // namespace precice, config
