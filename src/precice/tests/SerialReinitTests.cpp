#include <Eigen/Core>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include "com/Communication.hpp"
#include "com/SharedPointer.hpp"
#include "logging/LogMacros.hpp"
#include "math/constants.hpp"
#include "mesh/Mesh.hpp"
#include "precice/SolverInterface.hpp"
#include "precice/config/Configuration.hpp"
#include "precice/impl/SolverInterfaceImpl.hpp"
#include "testing/QuickTest.hpp"
#include "testing/TestContext.hpp"
#include "testing/Testing.hpp"
#include "utils/MasterSlave.hpp"

using namespace precice;
using namespace precice::testing;
using testing::TestContext;

BOOST_AUTO_TEST_SUITE(PreciceTests)
BOOST_AUTO_TEST_SUITE(Reinit)
BOOST_AUTO_TEST_SUITE(Serial)

BOOST_AUTO_TEST_SUITE(Reset)

BOOST_AUTO_TEST_CASE(Input)
{
  PRECICE_TEST("A"_on(1_rank), "B"_on(1_rank));

  constexpr double y  = 0.0;

  SolverInterface interface{context.name, "/precice/tests/reinit-parallel-explicit.xml"_src, context.rank, context.size};

  // A - Static Geometry
  if (context.isNamed("A")) {
    QuickTest(interface, "MA"_mesh, "D"_scalar)
        .setVertices({0.0, y, 1.0, y})
        .initialize()
        .write({0.01, 0.02})
        .advance()
        .resetMesh()
        .setVertices({0.0, y, 1.0, y})
        .write({0.11, 0.12})
        .advance()
        .finalize();
  }
  // B - Adaptive Geometry
  if (context.isNamed("B")) {
    auto qt = QuickTest(interface, "MB"_mesh, "D"_scalar)
                  .setVertices({0.0, y, 1.0, y})
                  .initialize()
                  .advance();
    std::vector<double> expected0{0.01, 0.02};
    BOOST_TEST(qt.read() == expected0, boost::test_tools::per_element());
    qt.advance();

    std::vector<double> expected1{0.11, 0.12};
    BOOST_TEST(qt.read() == expected1, boost::test_tools::per_element());
    qt.finalize();
  }
}

BOOST_AUTO_TEST_CASE(Output)
{
  PRECICE_TEST("A"_on(1_rank), "B"_on(1_rank));

  constexpr double y  = 0.0;

  SolverInterface interface{context.name, "/precice/tests/reinit-parallel-explicit.xml"_src, context.rank, context.size};

  // A - Static Geometry
  if (context.isNamed("A")) {
    QuickTest(interface, "MA"_mesh, "D"_scalar)
        .setVertices({0.0, y, 1.0, y})
        .initialize()
        .write({0.01, 0.02})
        .advance()
        .write({0.11, 0.12})
        .advance()
        .finalize();
  }
  // B - Adaptive Geometry
  if (context.isNamed("B")) {
    auto qt = QuickTest(interface, "MB"_mesh, "D"_scalar)
                  .setVertices({0.0, y, 1.0, y})
                  .initialize()
                  .advance();
    std::vector<double> expected0{0.01, 0.02};
    BOOST_TEST(qt.read() == expected0, boost::test_tools::per_element());
    qt.resetMesh()
      .setVertices({0.0, y, 1.0, y})
      .advance();
    std::vector<double> expected1{0.11, 0.12};
    BOOST_TEST(qt.read() == expected1, boost::test_tools::per_element());
    qt.finalize();
  }
}

BOOST_AUTO_TEST_CASE(Both)
{
  PRECICE_TEST("A"_on(1_rank), "B"_on(1_rank));

  constexpr double y  = 0.0;
  SolverInterface interface{context.name, "/precice/tests/reinit-parallel-explicit.xml"_src, context.rank, context.size};

  // A - Static Geometry
  if (context.isNamed("A")) {
    QuickTest(interface, "MA"_mesh, "D"_scalar)
        .setVertices({0.0, y, 1.0, y})
        .initialize()
        .write({0.01, 0.02})
        .advance()
        .resetMesh()
        .setVertices({0.0, y, 1.0, y})
        .write({0.11, 0.12})
        .advance()
        .finalize();
  }
  // B - Adaptive Geometry
  if (context.isNamed("B")) {
    auto qt = QuickTest(interface, "MB"_mesh, "D"_scalar)
                  .setVertices({0.0, y, 1.0, y})
                  .initialize()
                  .advance();
    std::vector<double> expected0{0.01, 0.02};
    BOOST_TEST(qt.read() == expected0, boost::test_tools::per_element());
    qt.resetMesh()
      .setVertices({0.0, y, 1.0, y})
      .advance();

    std::vector<double> expected1{0.11, 0.12};
    BOOST_TEST(qt.read() == expected1, boost::test_tools::per_element());
    qt.finalize();
  }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(ChangeMapping)

BOOST_AUTO_TEST_CASE(Input)
{
  PRECICE_TEST("A"_on(1_rank), "B"_on(1_rank));

  constexpr double y  = 0.0;

  SolverInterface interface{context.name, "/precice/tests/reinit-parallel-explicit.xml"_src, context.rank, context.size};

  // A - Static Geometry
  if (context.isNamed("A")) {
    QuickTest(interface, "MA"_mesh, "D"_scalar)
        .setVertices({0.0, y, 1.0, y})
        .initialize()
        .write({0.01, 0.02})
        .advance()
        .resetMesh()
        .setVertices({0.0, y, 0.5, y, 1.0, y})
        .write({0.11, 0.12, 0.13})
        .advance()
        .finalize();
  }
  // B - Adaptive Geometry
  if (context.isNamed("B")) {
    auto qt = QuickTest(interface, "MB"_mesh, "D"_scalar)
                  .setVertices({0.0, y, 1.0, y})
                  .initialize()
                  .advance();
    std::vector<double> expected0{0.01, 0.02};
    BOOST_TEST(qt.read() == expected0, boost::test_tools::per_element());
    qt.advance();

    std::vector<double> expected1{0.11, 0.13};
    BOOST_TEST(qt.read() == expected1, boost::test_tools::per_element());
    qt.finalize();
  }
}

BOOST_AUTO_TEST_CASE(Output)
{
  PRECICE_TEST("A"_on(1_rank), "B"_on(1_rank));

  constexpr double y  = 0.0;

  SolverInterface interface{context.name, "/precice/tests/reinit-parallel-explicit.xml"_src, context.rank, context.size};

  // A - Static Geometry
  if (context.isNamed("A")) {
    QuickTest(interface, "MA"_mesh, "D"_scalar)
        .setVertices({0.0, y, 1.0, y})
        .initialize()
        .write({0.01, 0.02})
        .advance()
        .write({0.11, 0.12})
        .advance()
        .finalize();
  }
  // B - Adaptive Geometry
  if (context.isNamed("B")) {
    auto qt = QuickTest(interface, "MB"_mesh, "D"_scalar)
                  .setVertices({0.0, y, 1.0, y})
                  .initialize()
                  .advance();
    std::vector<double> expected0{0.01, 0.02};
    BOOST_TEST(qt.read() == expected0, boost::test_tools::per_element());
    qt.resetMesh()
      .setVertices({0.0, y, 1.0, y, 2.0, y})
      .advance();
    std::vector<double> expected1{0.11, 0.12, 0.12};
    BOOST_TEST(qt.read() == expected1, boost::test_tools::per_element());
    qt.finalize();
  }
}

BOOST_AUTO_TEST_CASE(Both)
{
  PRECICE_TEST("A"_on(1_rank), "B"_on(1_rank));

  constexpr double y  = 0.0;
  SolverInterface interface{context.name, "/precice/tests/reinit-parallel-explicit.xml"_src, context.rank, context.size};

  // A - Static Geometry
  if (context.isNamed("A")) {
    QuickTest(interface, "MA"_mesh, "D"_scalar)
        .setVertices({0.0, y, 1.0, y})
        .initialize()
        .write({0.01, 0.02})
        .advance()
        .resetMesh()
        .setVertices({0.0, y, 1.0, y, 2.0, y})
        .write({0.11, 0.12, 0.13})
        .advance()
        .finalize();
  }
  // B - Adaptive Geometry
  if (context.isNamed("B")) {
    auto qt = QuickTest(interface, "MB"_mesh, "D"_scalar)
                  .setVertices({0.0, y, 1.0, y})
                  .initialize()
                  .advance();
    std::vector<double> expected0{0.01, 0.02};
    BOOST_TEST(qt.read() == expected0, boost::test_tools::per_element());
    qt.resetMesh()
      .setVertices({1.0, y, 2.0, y, 3.0, y})
      .advance();

    std::vector<double> expected1{0.12, 0.13, 0.13};
    BOOST_TEST(qt.read() == expected1, boost::test_tools::per_element());
    qt.finalize();
  }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
