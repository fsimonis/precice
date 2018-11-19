#pragma once

#include "ConvergenceMeasure.hpp"
#include "logging/Logger.hpp"
#include "utils/MasterSlave.hpp"

namespace precice
{
namespace cplscheme
{
namespace tests
{
class AbsoluteConvergenceMeasureTest;
}
} // namespace cplscheme
} // namespace precice

namespace precice
{
namespace cplscheme
{
namespace impl
{

/**
 * @brief Measures the convergence from an old data set to a new one.
 *
 * The convergence is evaluated by looking at the two norm of the differences
 * between each data value from the new and old data set. If the two norm is
 * equal or below a given limit, convergence is achieved.
 *
 * For a description of how to perform the measurement, see class
 * ConvergenceMeasure.
 */
class AbsoluteConvergenceMeasure: public ConvergenceMeasure
{
public:
  explicit AbsoluteConvergenceMeasure(double convergenceLimit);

  virtual ~AbsoluteConvergenceMeasure(){};

  virtual void
  newMeasurementSeries()
  {
    _isConvergence = false;
  }

  virtual void
  measure(
      const Eigen::VectorXd &oldValues,
      const Eigen::VectorXd &newValues,
      const Eigen::VectorXd &designSpecification)
  {
    _normDiff      = utils::MasterSlave::l2norm((newValues - oldValues) - designSpecification);
    _isConvergence = _normDiff <= _convergenceLimit;
    //      INFO("Absolute convergence measure: "
    //                     << "two-norm differences = " << normDiff
    //                     << ", convergence limit = " << _convergenceLimit
    //                     << ", convergence = " << _isConvergence );
  }

  virtual bool
  isConvergence() const
  {
    return _isConvergence;
  }

  /// Adds current convergence information to output stream.
  virtual std::string
  printState()
  {
    std::ostringstream os;
    os << "absolute convergence measure: ";
    os << "two-norm diff = " << _normDiff;
    os << ", limit = " << _convergenceLimit;
    os << ", conv = ";
    if (_isConvergence)
      os << "true";
    else
      os << "false";
    return os.str();
  }

  virtual double
  getNormResidual()
  {
    return _normDiff;
  }

private:
  logging::Logger _log{"cplscheme::AbsoluteConvergenceMeasure"};

  double _convergenceLimit;

  double _normDiff = 0;

  bool _isConvergence = false;
};
} // namespace impl
} // namespace cplscheme
} // namespace precice
