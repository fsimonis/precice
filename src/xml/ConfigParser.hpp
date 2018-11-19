#pragma once

#include "logging/Logger.hpp"
#include "xml/XMLTag.hpp"
#include <map>
#include <string>
#include <vector>

namespace precice
{
namespace xml
{
class XMLTag; // forward declaration to resolve circular import

class ConfigParser
{
public:
  /// Struct holding the read tag from xml file
  struct CTag
  {
    std::string m_Name;
    std::string m_Prefix;
    bool        m_Used = false;

    using AttributePair = std::map<std::string, std::string>;
    AttributePair                      m_aAttributes;
    std::vector<std::shared_ptr<CTag>> m_aSubTags;
  };

  using CTagPtrVec = std::vector<std::shared_ptr<CTag>>;

private:
  static precice::logging::Logger _log;

  CTagPtrVec m_AllTags;
  CTagPtrVec m_CurrentTags;

  std::shared_ptr<precice::xml::XMLTag> m_pXmlTag;

  static void
  GenericErrorFunc(void *ctx, const char *msg, ...);

public:
  /// Parser ctor for Callback init
  ConfigParser(const std::string &filePath, std::shared_ptr<XMLTag> pXmlTag);

  /// Parser ctor without Callbacks
  ConfigParser(const std::string &filePath);

  /// Reads the xml file
  int
  readXmlFile(std::string const &filePath);

  /**
   * @brief Connects the actual tags of an xml layer with the predefined tags
   * @param DefTags predefined tags
   * @param SubTags actual tags from xml file
   */
  void
  connectTags(std::vector<std::shared_ptr<precice::xml::XMLTag>> &DefTags, CTagPtrVec &SubTags);

  /// Callback for Start-Tag
  void
  OnStartElement(
      std::string         localname,
      std::string         prefix,
      CTag::AttributePair attributes);

  /// Callback for End-Tag
  void
  OnEndElement();

  // Callback for text sections in xml file
  void
  OnTextSection(std::string ch);
};
} // namespace xml
} // namespace precice
