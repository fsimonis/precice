#pragma once

#include<string>
#include "xml/XMLTag.hpp"

namespace precice { 
namespace config { 

/// Context of the configuration containing the information about the current SolverInterface instance.
struct ConfigurationContext {
    std::string participantName;
    int processRank;
    int communicatorSize;
};

/// A listener for the XML configuration which keeps a context.
struct ConfigurationListener :: xml::XMLTag::Listener {
    ConfigurationListener(ConfigurationContext context) : _context(std::move(context)) {};
    ConfigurationListener& operator=(ConfigurationListener &&) = delete;

    override ~ContextualListener();
    override void xmlTagCallback(XMLTag &callingTag);
    override void xmlEndTagCallback(XMLTag &callingTag);

    /// The context of the configuration.
    ConfigurationContext _context
};

} // namespace config

} // namespace precice