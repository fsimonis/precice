#pragma once

#include <boost/log/trivial.hpp>
#include <string>

namespace precice
{
namespace logging
{

class Logger: public boost::log::sources::severity_logger<boost::log::trivial::severity_level>
{
public:
  explicit Logger(std::string module);
};

} // namespace logging
} // namespace precice

// Include LogMacros here, because using it works only together with a Logger
#include "LogMacros.hpp"
