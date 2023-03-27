#pragma once

#include <Eigen/Core>

namespace precice {
namespace utils {
namespace eigenio {

inline Eigen::IOFormat wkt()
{
  return Eigen::IOFormat(
      Eigen::StreamPrecision,
      Eigen::DontAlignCols,
      " ", // Coeff separator
      ","  // Row separator
  );
}

inline Eigen::IOFormat debug()
{
  return wkt();
}

inline Eigen::IOFormat vtkByDim(int dim)
{
  // Only 2D vectors need an additional 0 component
  auto rowSep = (dim == 2) ? " 0 " : " ";

  return Eigen::IOFormat(
      Eigen::StreamPrecision,
      Eigen::DontAlignCols,
      " ",   // Coeff separator
      rowSep // Row separator
  );
}

inline Eigen::IOFormat legacyVtkByDim(int dim)
{
  // Only 2D vectors need an additional 0 component
  auto rowSep = (dim == 2) ? " 0\n" : "\n";

  return Eigen::IOFormat(
      Eigen::StreamPrecision,
      Eigen::DontAlignCols,
      " ",   // Coeff separator
      rowSep // Row separator
  );
}

} // namespace eigenio
} // namespace utils
} // namespace precice
