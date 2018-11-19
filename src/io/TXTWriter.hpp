#pragma once

#include "logging/Logger.hpp"
#include <Eigen/Core>
#include <fstream>
#include <string>

namespace precice
{
namespace io
{

/**
 * @brief File writer for matrix in Matlab V7 ASCII format.
 */
class TXTWriter
{
public:
  /**
   * @brief Constructor, opens file and sets format.
   */
  explicit TXTWriter(const std::string &filename);

  /**
   * @brief Destructor, closes file.
   */
  ~TXTWriter();

  ///Writes (appends) the matrix to the file.
  void
  write(const Eigen::MatrixXd &matrix)
  {
    for (long i = 0; i < matrix.rows(); i++) {
      for (long j = 0; j < matrix.cols(); j++) {
        _file << matrix(i, j) << " ";
      }
    }
    _file << std::endl;
  }

private:
  logging::Logger _log{"io::TXTWriter"};

  // @brief Filestream.
  std::ofstream _file;
};

} // namespace io
} // namespace precice
