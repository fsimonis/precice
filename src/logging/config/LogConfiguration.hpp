#pragma once

#include "logging/LogConfiguration.hpp"
#include "precice/config/ConfigurationListener.hpp"

namespace precice {
namespace config {

/// Configures the log config file to use
class LogConfiguration : public config::ConfigurationListener
{
public:
  LogConfiguration(xml::XMLTag& parent, const config::ConfigurationContext& context);

  virtual void xmlTagCallback(xml::XMLTag& tag);

  virtual void xmlEndTagCallback(xml::XMLTag& tag);

private:
  precice::logging::Logger _log{"logging::config::LogConfiguration"};

  precice::logging::LoggingConfiguration _logconfig;
};

}} // namespace precice, config
