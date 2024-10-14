#ifndef PRECICE_NO_MPI

#include "helper.hpp"
#include "testing/Testing.hpp"

BOOST_AUTO_TEST_SUITE(Integration)
BOOST_AUTO_TEST_SUITE(Remeshing)
BOOST_AUTO_TEST_SUITE(Parallel)
BOOST_AUTO_TEST_SUITE(Implicit)
BOOST_AUTO_TEST_SUITE(Acceleration)
BOOST_AUTO_TEST_CASE(RemeshFirstSerial)
{
  PRECICE_TEST("A"_on(1_rank), "B"_on(1_rank));
  precice::tests::remesh::parallelImplicit::acceleration::runResetA(context);
}

BOOST_AUTO_TEST_SUITE_END() // Convergence
BOOST_AUTO_TEST_SUITE_END() // Implicit
BOOST_AUTO_TEST_SUITE_END() // Parallel
BOOST_AUTO_TEST_SUITE_END() // Remeshing
BOOST_AUTO_TEST_SUITE_END() // Integration

#endif // PRECICE_NO_MPI
