#include <acceleration/Acceleration.hpp>
#include "cplscheme/CouplingData.hpp"
#include "utils/Helpers.hpp"

namespace precice {
namespace acceleration {

void Acceleration::checkDataIDs(DataMap const &cplData) const
{
  for (int id : getDataIDs()) {
    bool valid = utils::contained(id, cplData);
    PRECICE_ASSERT(valid, "Data with ID " << id << " unknown.");
  }
}

} // namespace acceleration
} // namespace precice
