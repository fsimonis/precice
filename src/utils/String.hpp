#pragma once

#include <string>

namespace precice {
namespace utils {

std::string wrapText(
    const std::string &text,
    int linewidth,
    int indentation);

/**
 * @brief Checks if filename has the given extension, if not appends it.
 *
 * @return filename with extension.
 */
std::string &checkAppendExtension(std::string &filename, const std::string &extension);

/// Evaluates a string to find out if it represents a bool.
/**
 * Returns True if string is yes, true, 1 or on. Otherwise False.
 * This function is case-insensitive.
 */
bool convertStringToBool(std::string const &value);

} // namespace utils
} // namespace precice
