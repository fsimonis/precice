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
BOOST_AUTO_TEST_SUITE(Parallel)

BOOST_AUTO_TEST_SUITE(Reset)

void runResetInput(testing::TestContext &context, const std::string &config)
{
  constexpr double y = 0.0;

  SolverInterface interface{context.name, config, context.rank, context.size};

  // A - Static Geometry
  if (context.isNamed("A")) {
    if (context.isMaster()) {
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
    } else {
      QuickTest(interface, "MA"_mesh, "D"_scalar)
          .setVertices({2.0, y, 3.0, y})
          .initialize()
          .write({1.01, 1.02})
          .advance()
          .resetMesh()
          .setVertices({2.0, y, 3.0, y})
          .write({1.11, 1.12})
          .advance()
          .finalize();
    }
  }
  // B - Adaptive Geometry
  if (context.isNamed("B")) {
    if (context.isMaster()) {
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
    } else {
      auto qt = QuickTest(interface, "MB"_mesh, "D"_scalar)
                    .setVertices({2.0, y, 3.0, y})
                    .initialize()
                    .advance();
      std::vector<double> expected0{1.01, 1.02};
      BOOST_TEST(qt.read() == expected0, boost::test_tools::per_element());
      qt.advance();

      std::vector<double> expected1{1.11, 1.12};
      BOOST_TEST(qt.read() == expected1, boost::test_tools::per_element());
      qt.finalize();
    }
  }
}

BOOST_AUTO_TEST_CASE(Input)
{
  PRECICE_TEST("A"_on(2_ranks), "B"_on(2_ranks));
  runResetInput(context, "/precice/tests/reinit-parallel-explicit.xml"_src);
}

BOOST_AUTO_TEST_CASE(Input2LI)
{
  PRECICE_TEST("A"_on(2_ranks), "B"_on(2_ranks));
  runResetInput(context, "/precice/tests/reinit-parallel-explicit-2li.xml"_src);
}

void runResetOutput(testing::TestContext &context, const std::string &config)
{
  constexpr double y = 0.0;

  SolverInterface interface{context.name, config, context.rank, context.size};

  // A - Static Geometry
  if (context.isNamed("A")) {
    if (context.isMaster()) {
      QuickTest(interface, "MA"_mesh, "D"_scalar)
          .setVertices({0.0, y, 1.0, y})
          .initialize()
          .write({0.01, 0.02})
          .advance()
          .write({0.11, 0.12})
          .advance()
          .finalize();
    } else {
      QuickTest(interface, "MA"_mesh, "D"_scalar)
          .setVertices({2.0, y, 3.0, y})
          .initialize()
          .write({1.01, 1.02})
          .advance()
          .write({1.11, 1.12})
          .advance()
          .finalize();
    }
  }
  // B - Adaptive Geometry
  if (context.isNamed("B")) {
    if (context.isMaster()) {
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
    } else {
      auto qt = QuickTest(interface, "MB"_mesh, "D"_scalar)
                    .setVertices({2.0, y, 3.0, y})
                    .initialize()
                    .advance();
      std::vector<double> expected0{1.01, 1.02};
      BOOST_TEST(qt.read() == expected0, boost::test_tools::per_element());
      qt.resetMesh()
          .setVertices({2.0, y, 3.0, y})
          .advance();
      std::vector<double> expected1{1.11, 1.12};
      BOOST_TEST(qt.read() == expected1, boost::test_tools::per_element());
      qt.finalize();
    }
  }
}

BOOST_AUTO_TEST_CASE(Output)
{
  PRECICE_TEST("A"_on(2_ranks), "B"_on(2_ranks));
  runResetOutput(context, "/precice/tests/reinit-parallel-explicit.xml"_src);
}

BOOST_AUTO_TEST_CASE(Output2LI)
{
  PRECICE_TEST("A"_on(2_ranks), "B"_on(2_ranks));
  runResetOutput(context, "/precice/tests/reinit-parallel-explicit-2li.xml"_src);
}

void runResetBoth(testing::TestContext &context, const std::string &config)
{
  constexpr double y = 0.0;
  SolverInterface  interface{context.name, config, context.rank, context.size};

  // A - Static Geometry
  if (context.isNamed("A")) {
    if (context.isMaster()) {
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
    } else {
      QuickTest(interface, "MA"_mesh, "D"_scalar)
          .setVertices({2.0, y, 3.0, y})
          .initialize()
          .write({1.01, 1.02})
          .advance()
          .resetMesh()
          .setVertices({2.0, y, 3.0, y})
          .write({1.11, 1.12})
          .advance()
          .finalize();
    }
  }
  // B - Adaptive Geometry
  if (context.isNamed("B")) {
    if (context.isMaster()) {
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
    } else {
      auto qt = QuickTest(interface, "MB"_mesh, "D"_scalar)
                    .setVertices({2.0, y, 3.0, y})
                    .initialize()
                    .advance();
      std::vector<double> expected0{1.01, 1.02};
      BOOST_TEST(qt.read() == expected0, boost::test_tools::per_element());
      qt.resetMesh()
          .setVertices({2.0, y, 3.0, y})
          .advance();

      std::vector<double> expected1{1.11, 1.12};
      BOOST_TEST(qt.read() == expected1, boost::test_tools::per_element());
      qt.finalize();
    }
  }
}

BOOST_AUTO_TEST_CASE(Both)
{
  PRECICE_TEST("A"_on(2_ranks), "B"_on(2_ranks));
  runResetBoth(context, "/precice/tests/reinit-parallel-explicit.xml"_src);
}

BOOST_AUTO_TEST_CASE(Both2LI)
{
  PRECICE_TEST("A"_on(2_ranks), "B"_on(2_ranks));
  runResetBoth(context, "/precice/tests/reinit-parallel-explicit-2li.xml"_src);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(ChangeMapping)

void runResetInput(testing::TestContext &context, const std::string &config)
{
  constexpr double y = 0.0;

  SolverInterface interface{context.name, config, context.rank, context.size};

  // A - Static Geometry
  if (context.isNamed("A")) {
    if (context.isMaster()) {
      QuickTest(interface, "MA"_mesh, "D"_scalar)
          .setVertices({0.0, y, 1.0, y})
          .initialize()
          .write({0.01, 0.02})
          .advance()
          .resetMesh()
          .setVertices({1.0, y})
          .write({0.11})
          .advance()
          .finalize();
    } else {
      QuickTest(interface, "MA"_mesh, "D"_scalar)
          .setVertices({2.0, y, 3.0, y})
          .initialize()
          .write({1.01, 1.02})
          .advance()
          .write({1.11, 1.12})
          .advance()
          .finalize();
    }
  }
  // B - Adaptive Geometry
  if (context.isNamed("B")) {
    if (context.isMaster()) {
      auto qt = QuickTest(interface, "MB"_mesh, "D"_scalar)
                    .setVertices({0.0, y, 1.0, y})
                    .initialize()
                    .advance();
      std::vector<double> expected0{0.01, 0.02};
      BOOST_TEST(qt.read() == expected0, boost::test_tools::per_element());
      qt.advance();

      std::vector<double> expected1{0.11, 0.11};
      BOOST_TEST(qt.read() == expected1, boost::test_tools::per_element());
      qt.finalize();
    } else {
      auto qt = QuickTest(interface, "MB"_mesh, "D"_scalar)
                    .setVertices({2.0, y, 3.0, y})
                    .initialize()
                    .advance();
      std::vector<double> expected0{1.01, 1.02};
      BOOST_TEST(qt.read() == expected0, boost::test_tools::per_element());
      qt.advance();

      std::vector<double> expected1{1.11, 1.12};
      BOOST_TEST(qt.read() == expected1, boost::test_tools::per_element());
      qt.finalize();
    }
  }
}

BOOST_AUTO_TEST_CASE(Input)
{
  PRECICE_TEST("A"_on(2_ranks), "B"_on(2_ranks));
  runResetInput(context, "/precice/tests/reinit-parallel-explicit.xml"_src);
}

BOOST_AUTO_TEST_CASE(Input2LI)
{
  PRECICE_TEST("A"_on(2_ranks), "B"_on(2_ranks));
  runResetInput(context, "/precice/tests/reinit-parallel-explicit-2li.xml"_src);
}

void runResetOutput(testing::TestContext &context, const std::string &config)
{
  constexpr double y = 0.0;

  SolverInterface interface{context.name, config, context.rank, context.size};

  // A - Static Geometry
  if (context.isNamed("A")) {
    if (context.isMaster()) {
      QuickTest(interface, "MA"_mesh, "D"_scalar)
          .setVertices({0.0, y, 1.0, y})
          .initialize()
          .write({0.01, 0.02})
          .advance()
          .write({0.11, 0.12})
          .advance()
          .finalize();
    } else {
      QuickTest(interface, "MA"_mesh, "D"_scalar)
          .setVertices({2.0, y, 3.0, y})
          .initialize()
          .write({1.01, 1.02})
          .advance()
          .write({1.11, 1.12})
          .advance()
          .finalize();
    }
  }
  // B - Adaptive Geometry
  if (context.isNamed("B")) {
    if (context.isMaster()) {
      auto qt = QuickTest(interface, "MB"_mesh, "D"_scalar)
                    .setVertices({0.0, y, 1.0, y})
                    .initialize()
                    .advance();
      std::vector<double> expected0{0.01, 0.02};
      BOOST_TEST(qt.read() == expected0, boost::test_tools::per_element());
      qt.resetMesh()
          .setVertices({1.0, y})
          .advance();
      std::vector<double> expected1{0.12};
      BOOST_TEST(qt.read() == expected1, boost::test_tools::per_element());
      qt.finalize();
    } else {
      auto qt = QuickTest(interface, "MB"_mesh, "D"_scalar)
                    .setVertices({2.0, y, 3.0, y})
                    .initialize()
                    .advance();
      std::vector<double> expected0{1.01, 1.02};
      BOOST_TEST(qt.read() == expected0, boost::test_tools::per_element());
      qt.resetMesh()
          .setVertices({2.0, y})
          .advance();
      std::vector<double> expected1{1.11};
      BOOST_TEST(qt.read() == expected1, boost::test_tools::per_element());
      qt.finalize();
    }
  }
}

BOOST_AUTO_TEST_CASE(Output)
{
  PRECICE_TEST("A"_on(2_ranks), "B"_on(2_ranks));
  runResetOutput(context, "/precice/tests/reinit-parallel-explicit.xml"_src);
}

BOOST_AUTO_TEST_CASE(Output2LI)
{
  PRECICE_TEST("A"_on(2_ranks), "B"_on(2_ranks));
  runResetOutput(context, "/precice/tests/reinit-parallel-explicit-2li.xml"_src);
}

void runResetBoth(testing::TestContext &context, const std::string &config)
{
  constexpr double y = 0.0;
  SolverInterface  interface{context.name, config, context.rank, context.size};

  // A - Static Geometry
  if (context.isNamed("A")) {
    if (context.isMaster()) {
      QuickTest(interface, "MA"_mesh, "D"_scalar)
          .setVertices({0.0, y, 1.0, y})
          .initialize()
          .write({0.01, 0.02})
          .advance()
          .resetMesh()
          .setVertices({-1.0, y, 0, y})
          .write({0.11, 0.12})
          .advance()
          .finalize();
    } else {
      QuickTest(interface, "MA"_mesh, "D"_scalar)
          .setVertices({2.0, y, 3.0, y})
          .initialize()
          .write({1.01, 1.02})
          .advance()
          .resetMesh()
          .setVertices({3.0, y, 4.0, y})
          .write({1.11, 1.12})
          .advance()
          .finalize();
    }
  }
  // B - Adaptive Geometry
  if (context.isNamed("B")) {
    if (context.isMaster()) {
      auto qt = QuickTest(interface, "MB"_mesh, "D"_scalar)
                    .setVertices({0.0, y, 1.0, y})
                    .initialize()
                    .advance();
      std::vector<double> expected0{0.01, 0.02};
      BOOST_TEST(qt.read() == expected0, boost::test_tools::per_element());
      qt.resetMesh()
          .setVertices({0.0, y, 1.0, y})
          .advance();

      std::vector<double> expected1{0.12, 0.12};
      BOOST_TEST(qt.read() == expected1, boost::test_tools::per_element());
      qt.finalize();
    } else {
      auto qt = QuickTest(interface, "MB"_mesh, "D"_scalar)
                    .setVertices({2.0, y, 3.0, y})
                    .initialize()
                    .advance();
      std::vector<double> expected0{1.01, 1.02};
      BOOST_TEST(qt.read() == expected0, boost::test_tools::per_element());
      qt.resetMesh()
          .setVertices({2.0, y, 3.0, y})
          .advance();

      std::vector<double> expected1{1.11, 1.11};
      BOOST_TEST(qt.read() == expected1, boost::test_tools::per_element());
      qt.finalize();
    }
  }
}

BOOST_AUTO_TEST_CASE(Both)
{
  PRECICE_TEST("A"_on(2_ranks), "B"_on(2_ranks));
  runResetBoth(context, "/precice/tests/reinit-parallel-explicit.xml"_src);
}

BOOST_AUTO_TEST_CASE(Both2LI)
{
  PRECICE_TEST("A"_on(2_ranks), "B"_on(2_ranks));
  runResetBoth(context, "/precice/tests/reinit-parallel-explicit-2li.xml"_src);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(ChangePartitions)

void runOverlapBoth(testing::TestContext &context, const std::string &config)
{
  constexpr double y = 0.0;
  SolverInterface  interface{context.name, config, context.rank, context.size};

  // A - Static Geometry
  if (context.isNamed("A")) {
    if (context.isMaster()) {
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
    } else {
      QuickTest(interface, "MA"_mesh, "D"_scalar)
          .setVertices({2.0, y, 3.0, y})
          .initialize()
          .write({1.01, 1.02})
          .advance()
          .resetMesh()
          .setVertices({3.0, y})
          .write({1.11})
          .advance()
          .finalize();
    }
  }
  // B - Adaptive Geometry
  if (context.isNamed("B")) {
    if (context.isMaster()) {
      auto qt = QuickTest(interface, "MB"_mesh, "D"_scalar)
                    .setVertices({0.0, y, 1.0, y})
                    .initialize()
                    .advance();
      std::vector<double> expected0{0.01, 0.02};
      BOOST_TEST(qt.read() == expected0, boost::test_tools::per_element());
      qt.resetMesh()
          .setVertices({0.0, y})
          .advance();

      std::vector<double> expected1{0.11};
      BOOST_TEST(qt.read() == expected1, boost::test_tools::per_element());
      qt.finalize();
    } else {
      auto qt = QuickTest(interface, "MB"_mesh, "D"_scalar)
                    .setVertices({2.0, y, 3.0, y})
                    .initialize()
                    .advance();
      std::vector<double> expected0{1.01, 1.02};
      BOOST_TEST(qt.read() == expected0, boost::test_tools::per_element());
      qt.resetMesh()
          .setVertices({1.0, y, 2.0, y, 3.0, y})
          .advance();

      std::vector<double> expected1{0.12, 0.13, 1.11};
      BOOST_TEST(qt.read() == expected1, boost::test_tools::per_element());
      qt.finalize();
    }
  }
}

BOOST_AUTO_TEST_CASE(OverlapBoth)
{
  PRECICE_TEST("A"_on(2_ranks), "B"_on(2_ranks));
  runOverlapBoth(context, "/precice/tests/reinit-parallel-explicit.xml"_src);
}

BOOST_AUTO_TEST_CASE(OverlapBoth2LI)
{
  PRECICE_TEST("A"_on(2_ranks), "B"_on(2_ranks));
  runOverlapBoth(context, "/precice/tests/reinit-parallel-explicit-2li.xml"_src);
}

void runSwapOutputs(testing::TestContext &context, const std::string &config)
{
  constexpr double y = 0.0;

  SolverInterface interface{context.name, config, context.rank, context.size};

  // A - Static Geometry
  if (context.isNamed("A")) {
    if (context.isMaster()) {
      QuickTest(interface, "MA"_mesh, "D"_scalar)
          .setVertices({0.0, y, 1.0, y})
          .initialize()
          .write({0.01, 0.02})
          .advance()
          .write({0.11, 0.12})
          .advance()
          .finalize();
    } else {
      QuickTest(interface, "MA"_mesh, "D"_scalar)
          .setVertices({2.0, y, 3.0, y})
          .initialize()
          .write({1.01, 1.02})
          .advance()
          .write({1.11, 1.12})
          .advance()
          .finalize();
    }
  }
  // B - Adaptive Geometry
  if (context.isNamed("B")) {
    if (context.isMaster()) {
      auto qt = QuickTest(interface, "MB"_mesh, "D"_scalar)
                    .setVertices({0.0, y, 1.0, y})
                    .initialize()
                    .advance();
      std::vector<double> expected0{0.01, 0.02};
      BOOST_TEST(qt.read() == expected0, boost::test_tools::per_element());
      qt.resetMesh()
          .setVertices({2.0, y, 3.0, y})
          .advance();
      std::vector<double> expected1{1.11, 1.12};
      BOOST_TEST(qt.read() == expected1, boost::test_tools::per_element());
      qt.finalize();
    } else {
      auto qt = QuickTest(interface, "MB"_mesh, "D"_scalar)
                    .setVertices({2.0, y, 3.0, y})
                    .initialize()
                    .advance();
      std::vector<double> expected0{1.01, 1.02};
      BOOST_TEST(qt.read() == expected0, boost::test_tools::per_element());
      qt.resetMesh()
          .setVertices({0.0, y, 1.0, y})
          .advance();
      std::vector<double> expected1{0.11, 0.12};
      BOOST_TEST(qt.read() == expected1, boost::test_tools::per_element());
      qt.finalize();
    }
  }
}

BOOST_AUTO_TEST_CASE(SwapOutputs)
{
  PRECICE_TEST("A"_on(2_ranks), "B"_on(2_ranks));
  runSwapOutputs(context, "/precice/tests/reinit-parallel-explicit.xml"_src);
}

BOOST_AUTO_TEST_CASE(SwapOutputs2LI)
{
  PRECICE_TEST("A"_on(2_ranks), "B"_on(2_ranks));
  runSwapOutputs(context, "/precice/tests/reinit-parallel-explicit-2li.xml"_src);
}

void runPartitionScatterOutputs(testing::TestContext &context, const std::string &config)
{
  constexpr double y = 0.0;

  SolverInterface interface{context.name, config, context.rank, context.size};

  // A - Static Geometry
  if (context.isNamed("A")) {
    if (context.isMaster()) {
      QuickTest(interface, "MA"_mesh, "D"_scalar)
          .setVertices({0.0, y, 1.0, y})
          .initialize()
          .write({0.01, 0.02})
          .advance()
          .write({0.11, 0.12})
          .advance()
          .finalize();
    } else {
      QuickTest(interface, "MA"_mesh, "D"_scalar)
          .setVertices({2.0, y, 3.0, y})
          .initialize()
          .write({1.01, 1.02})
          .advance()
          .write({1.11, 1.12})
          .advance()
          .finalize();
    }
  }
  // B - Adaptive Geometry
  if (context.isNamed("B")) {
    if (context.isMaster()) {
      auto qt = QuickTest(interface, "MB"_mesh, "D"_scalar)
                    .setVertices({0.0, y, 1.0, y})
                    .initialize()
                    .advance();
      std::vector<double> expected0{0.01, 0.02};
      BOOST_TEST(qt.read() == expected0, boost::test_tools::per_element());
      qt.resetMesh()
          .setVertices({0.0, y, 2.0, y})
          .advance();
      std::vector<double> expected1{0.11, 1.11};
      BOOST_TEST(qt.read() == expected1, boost::test_tools::per_element());
      qt.finalize();
    } else {
      auto qt = QuickTest(interface, "MB"_mesh, "D"_scalar)
                    .setVertices({2.0, y, 3.0, y})
                    .initialize()
                    .advance();
      std::vector<double> expected0{1.01, 1.02};
      BOOST_TEST(qt.read() == expected0, boost::test_tools::per_element());
      qt.resetMesh()
          .setVertices({1.0, y, 3.0, y})
          .advance();
      std::vector<double> expected1{0.12, 1.12};
      BOOST_TEST(qt.read() == expected1, boost::test_tools::per_element());
      qt.finalize();
    }
  }
}

BOOST_AUTO_TEST_CASE(ScatterOutputs)
{
  PRECICE_TEST("A"_on(2_ranks), "B"_on(2_ranks));
  runPartitionScatterOutputs(context, "/precice/tests/reinit-parallel-explicit.xml"_src);
}

BOOST_AUTO_TEST_CASE(ScatterOutputs2LI)
{
  PRECICE_TEST("A"_on(2_ranks), "B"_on(2_ranks));
  runPartitionScatterOutputs(context, "/precice/tests/reinit-parallel-explicit-2li.xml"_src);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
