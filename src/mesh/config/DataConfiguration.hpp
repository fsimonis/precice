#pragma once

#include <string>
#include <vector>
#include "logging/Logger.hpp"
#include "mesh/Data.hpp"
#include "time/Time.hpp"
#include "utils/ManageUniqueIDs.hpp"
#include "xml/XMLTag.hpp"

namespace precice {
namespace mesh {

/// Performs and provides configuration for Data objects from XML files.
class DataConfiguration : public xml::XMLTag::Listener {
public:
  enum struct SizeType : bool {
    Rank  = false, /// Size is tensor rank
    Fixed = true   /// Size is fixed
  };

  struct ConfiguredData {
    std::string name;
    int         size;
    SizeType    type;
    int         waveformDegree;

    ConfiguredData(
        const std::string &name,
        int                size,
        SizeType           type,
        int                waveformDegree)
        : name(name), size(size), type(type), waveformDegree(waveformDegree) {}

    int dimensionsFor(int meshDimensions) const
    {
      if (type == SizeType::Fixed) {
        return size;
      }
      return std::pow(meshDimensions, size); // N^rank
    }
  };

  DataConfiguration(xml::XMLTag &parent);

  const std::vector<ConfiguredData> &data() const;

  ConfiguredData getRecentlyConfiguredData() const;

  virtual void xmlTagCallback(
      const xml::ConfigurationContext &context,
      xml::XMLTag &                    callingTag);

  virtual void xmlEndTagCallback(
      const xml::ConfigurationContext &context,
      xml::XMLTag &                    callingTag);

  void addFixedData(const std::string &name,
                    int                size,
                    int                waveformDegree = time::Time::DEFAULT_WAVEFORM_DEGREE);

  void addTensorData(const std::string &name,
                     int                rank,
                     int                waveformDegree = time::Time::DEFAULT_WAVEFORM_DEGREE);

private:
  mutable logging::Logger _log{"mesh::DataConfiguration"};

  const std::string TAG          = "data";
  const std::string ATTR_DEGREE  = "waveform-degree";
  const std::string ATTR_NAME    = "name";
  const std::string ATTR_RANK    = "rank";
  const std::string ATTR_SIZE    = "size";
  const std::string VALUE_CUSTOM = "custom";
  const std::string VALUE_SCALAR = "scalar";
  const std::string VALUE_TENSOR = "tensor";
  const std::string VALUE_VECTOR = "vector";

  std::vector<ConfiguredData> _data;

  int _indexLastConfigured = -1;
};

} // namespace mesh
} // namespace precice
