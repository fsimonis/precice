#ifndef PRECICE_NO_MPI

#include "testing/Testing.hpp"

#include "../helper.hpp"

BOOST_AUTO_TEST_SUITE(Integration)
BOOST_AUTO_TEST_SUITE(Remeshing)
BOOST_AUTO_TEST_SUITE(Parallel)
BOOST_AUTO_TEST_SUITE(Explicit)
BOOST_AUTO_TEST_SUITE(Noop)
BOOST_AUTO_TEST_CASE(RemeshOutput2LI)
{
  PRECICE_TEST("A"_on(2_ranks), "B"_on(2_ranks));
  precice::tests::remesh::noop::runResetOutput(context);
}

BOOST_AUTO_TEST_SUITE_END() // Noop
BOOST_AUTO_TEST_SUITE_END() // Explicit
BOOST_AUTO_TEST_SUITE_END() // Parallel
BOOST_AUTO_TEST_SUITE_END() // Remeshing
BOOST_AUTO_TEST_SUITE_END() // Integration

#endif // PRECICE_NO_MPI