#ifndef PRECICE_NO_MPI
#include <Eigen/Core>
#include <algorithm>
#include <memory>
#include <mpi.h>
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
#include "testing/TestContext.hpp"
#include "testing/Testing.hpp"
#include "utils/MasterSlave.hpp"

using namespace precice;
using testing::TestContext;

struct ReinitTestFixture : testing::WhiteboxAccessor {
};

BOOST_AUTO_TEST_SUITE(PreciceTests)
BOOST_FIXTURE_TEST_SUITE(Reinit, ReinitTestFixture)

BOOST_AUTO_TEST_SUITE(Serial)


BOOST_AUTO_TEST_CASE(ResetInput)
{
  PRECICE_TEST("A"_on(1_rank), "B"_on(1_rank));

  constexpr double y  = 0.0;
  constexpr double y2 = 0.1; // after the reset

  std::vector<double> posBefore{0.0, y, 1.0, y};
  std::vector<double> posAfter{0.0, y2, 1.0, y2};
  const auto          sizeBefore = 2;
  const auto          sizeAfter  = 2;

  std::vector<double> valuesWrite0{0.01, 0.02};
  std::vector<double> valuesWrite1{0.11, 0.12};

  std::vector<double> valuesRead0{0.01, 0.02};
  std::vector<double> valuesRead1{0.11, 0.12};

  SolverInterface interface{context.name, "/precice/tests/reinit-parallel-explicit.xml"_src, context.rank, context.size};

  const int meshID = interface.getMeshID("M" + context.name);
  const int dataID = interface.getDataID("D", meshID);

  // A - Static Geometry
  if (context.isNamed("A")) {
    std::vector<int> vertexIDs(sizeBefore);
    auto             pVertexIDs = vertexIDs.data();

    interface.setMeshVertices(meshID, sizeBefore, posBefore.data(), pVertexIDs);
    interface.initialize();

    interface.writeBlockScalarData(dataID, sizeBefore, pVertexIDs, valuesWrite0.data());
    interface.advance(1.0);

    vertexIDs.resize(sizeAfter, -1);
    pVertexIDs = vertexIDs.data();
    interface.resetMesh(meshID);
    interface.setMeshVertices(meshID, sizeAfter, posAfter.data(), pVertexIDs);

    interface.writeBlockScalarData(dataID, sizeAfter, pVertexIDs, valuesWrite1.data());
    interface.advance(1.0);

    interface.finalize();
  }
  // B - Adaptive Geometry
  if (context.isNamed("B")) {
    std::vector<int> vertexIDs(sizeBefore, -1);
    auto             pVertexIDs = vertexIDs.data();

    interface.setMeshVertices(meshID, sizeBefore, posBefore.data(), pVertexIDs);
    interface.initialize();
    interface.advance(1.0);

    std::vector<double> readValues(sizeBefore, -1);
    auto                pValues = readValues.data();

    // empty data from initialization
    interface.readBlockScalarData(dataID, sizeBefore, pVertexIDs, pValues);
    BOOST_TEST_INFO("Before");
    BOOST_TEST(readValues == valuesRead0, boost::test_tools::per_element());

    interface.advance(1.0);

    interface.readBlockScalarData(dataID, sizeBefore, pVertexIDs, pValues);
    BOOST_TEST_INFO("After");
    BOOST_TEST(readValues == valuesRead1, boost::test_tools::per_element());

    interface.finalize();
  }
}

BOOST_AUTO_TEST_CASE(ResetOutput)
{
  PRECICE_TEST("A"_on(1_rank), "B"_on(1_rank));

  constexpr double y  = 0.0;
  constexpr double y2 = 0.1; // after the reset

  std::vector<double> posBefore{0.0, y, 1.0, y};
  std::vector<double> posAfter{0.0, y2, 1.0, y2};
  const auto          sizeBefore = 2;
  const auto          sizeAfter  = 2;

  std::vector<double> valuesWrite0{0.01, 0.02};
  std::vector<double> valuesWrite1{0.11, 0.12};

  std::vector<double> valuesRead0{0.01, 0.02};
  std::vector<double> valuesRead1{0.11, 0.12};

  SolverInterface interface{context.name, "/precice/tests/reinit-parallel-explicit.xml"_src, context.rank, context.size};

  const int meshID = interface.getMeshID("M" + context.name);
  const int dataID = interface.getDataID("D", meshID);

  // A - Static Geometry
  if (context.isNamed("A")) {
    std::vector<int> vertexIDs(sizeBefore);
    auto             pVertexIDs = vertexIDs.data();

    interface.setMeshVertices(meshID, sizeBefore, posBefore.data(), pVertexIDs);
    interface.initialize();

    interface.writeBlockScalarData(dataID, sizeBefore, pVertexIDs, valuesWrite0.data());
    interface.advance(1.0);

    interface.writeBlockScalarData(dataID, sizeBefore, pVertexIDs, valuesWrite1.data());
    interface.advance(1.0);

    interface.finalize();
  }
  // B - Adaptive Geometry
  if (context.isNamed("B")) {
    std::vector<int> vertexIDs(sizeBefore, -1);
    auto             pVertexIDs = vertexIDs.data();

    interface.setMeshVertices(meshID, sizeBefore, posBefore.data(), pVertexIDs);
    interface.initialize();
    interface.advance(1.0);

    std::vector<double> readValues(sizeBefore, -1);
    auto                pValues = readValues.data();

    // empty data from initialization
    interface.readBlockScalarData(dataID, sizeBefore, pVertexIDs, pValues);
    BOOST_TEST_INFO("Before");
    BOOST_TEST(readValues == valuesRead0, boost::test_tools::per_element());

    // Reset the Mesh
    vertexIDs.resize(sizeAfter, -1);
    pVertexIDs = vertexIDs.data();
    readValues.resize(sizeAfter, -1);
    pValues = readValues.data();
    interface.resetMesh(meshID);

    interface.setMeshVertices(meshID, sizeAfter, posAfter.data(), pVertexIDs);
    interface.advance(1.0);

    interface.readBlockScalarData(dataID, sizeAfter, pVertexIDs, pValues);
    BOOST_TEST_INFO("After");
    BOOST_TEST(readValues == valuesRead1, boost::test_tools::per_element());

    interface.finalize();
  }
}

BOOST_AUTO_TEST_CASE(ResetBoth)
{
  PRECICE_TEST("A"_on(1_rank), "B"_on(1_rank));

  constexpr double y  = 0.0;
  constexpr double y2 = 0.1; // after the reset

  std::vector<double> posBefore{0.0, y, 1.0, y};
  std::vector<double> posAfter{0.0, y2, 1.0, y2};
  const auto          sizeBefore = 2;
  const auto          sizeAfter  = 2;

  std::vector<double> valuesWrite0{0.01, 0.02};
  std::vector<double> valuesWrite1{0.11, 0.12};

  std::vector<double> valuesRead0{0.01, 0.02};
  std::vector<double> valuesRead1{0.11, 0.12};

  SolverInterface interface{context.name, "/precice/tests/reinit-parallel-explicit.xml"_src, context.rank, context.size};

  const int meshID = interface.getMeshID("M" + context.name);
  const int dataID = interface.getDataID("D", meshID);

  // A - Static Geometry
  if (context.isNamed("A")) {
    std::vector<int> vertexIDs(sizeBefore);
    auto             pVertexIDs = vertexIDs.data();

    interface.setMeshVertices(meshID, sizeBefore, posBefore.data(), pVertexIDs);
    interface.initialize();

    interface.writeBlockScalarData(dataID, sizeBefore, pVertexIDs, valuesWrite0.data());
    interface.advance(1.0);

    vertexIDs.resize(sizeAfter, -1);
    pVertexIDs = vertexIDs.data();
    interface.resetMesh(meshID);
    interface.setMeshVertices(meshID, sizeAfter, posAfter.data(), pVertexIDs);

    interface.writeBlockScalarData(dataID, sizeAfter, pVertexIDs, valuesWrite1.data());
    interface.advance(1.0);

    interface.finalize();
  }
  // B - Adaptive Geometry
  if (context.isNamed("B")) {
    std::vector<int> vertexIDs(sizeBefore, -1);
    auto             pVertexIDs = vertexIDs.data();

    interface.setMeshVertices(meshID, sizeBefore, posBefore.data(), pVertexIDs);
    interface.initialize();
    interface.advance(1.0);

    std::vector<double> readValues(sizeBefore, -1);
    auto                pValues = readValues.data();

    // empty data from initialization
    interface.readBlockScalarData(dataID, sizeBefore, pVertexIDs, pValues);
    BOOST_TEST_INFO("Before");
    BOOST_TEST(readValues == valuesRead0, boost::test_tools::per_element());

    // Reset the Mesh
    vertexIDs.resize(sizeAfter, -1);
    pVertexIDs = vertexIDs.data();
    readValues.resize(sizeAfter, -1);
    pValues = readValues.data();
    interface.resetMesh(meshID);

    interface.setMeshVertices(meshID, sizeAfter, posAfter.data(), pVertexIDs);
    interface.advance(1.0);

    interface.readBlockScalarData(dataID, sizeAfter, pVertexIDs, pValues);
    BOOST_TEST_INFO("After");
    BOOST_TEST(readValues == valuesRead1, boost::test_tools::per_element());

    interface.finalize();
  }
}

BOOST_AUTO_TEST_SUITE_END()

using RankDistribution = std::vector<std::vector<int>>;

void reinitParallelExplicit(const testing::TestContext &context, const RankDistribution &rankDistribution)
{
  std::string      configFilename = testing::getPathToSources() + "/precice/tests/reinit-parallel-explicit.xml";
  constexpr double y              = 0.0;
  constexpr double y2             = 0.1; // after the reset

  // A - Static Geometry
  if (context.isNamed("A")) {
    SolverInterface interface("A", configFilename, context.rank, context.size);

    const int meshID = interface.getMeshID("MA");
    const int dataID = interface.getDataID("D", meshID);

    // global setup
    double                           positions[8] = {0.0, y, 1.0, y, 2.0, y, 3.0, y};
    std::vector<std::vector<double>> values{
        {context.rank + 0.01, context.rank + 0.02, context.rank + 0.03, context.rank + 0.04},
        {context.rank + 0.11, context.rank + 0.12, context.rank + 0.13, context.rank + 0.14},
        {context.rank + 0.21, context.rank + 0.22, context.rank + 0.23, context.rank + 0.24},
    };

    // local setup
    const int verticesPerRank = 4 / context.size;
    const int myOffset        = verticesPerRank * context.rank;
    double *  mypositions     = &positions[2 * verticesPerRank * context.rank];

    // setup mesh
    std::vector<int> vertexIDs(verticesPerRank);
    auto             pVertexIDs = vertexIDs.data();
    interface.setMeshVertices(meshID, verticesPerRank, mypositions, pVertexIDs);
    interface.initialize();

    // value format <RANK>.<time><node>
    interface.writeBlockScalarData(dataID, verticesPerRank, pVertexIDs, values.at(0).data() + myOffset);

    interface.advance(1.0);
    interface.writeBlockScalarData(dataID, verticesPerRank, pVertexIDs, values.at(1).data() + myOffset);

    interface.advance(1.0);
    interface.writeBlockScalarData(dataID, verticesPerRank, pVertexIDs, values.at(2).data() + myOffset);

    interface.advance(1.0);
    interface.finalize();
  }
  // B - Adaptive Geometry
  else {
    BOOST_REQUIRE(context.isNamed("B"));
    SolverInterface interface("B", configFilename, context.rank, context.size);

    const int meshID = interface.getMeshID("MB");
    const int dataID = interface.getDataID("D", meshID);

    // global setup
    std::vector<std::vector<double>> positions{
        {0.0, y, 1.0, y, 2.0, y, 3.0, y},    // Initial coords
        {0.0, y2, 1.0, y2, 2.0, y2, 3.0, y2} // Coords after reinit
    };

    // setup expected values
    // the rankpart is passed via rankDistribution
    std::vector<std::vector<double>> expected{
        {0.01, 0.02, 0.03, 0.04},
        {0.11, 0.12, 0.13, 0.14},
        {0.21, 0.22, 0.23, 0.24}};
    for (int t = 0; t < 3; ++t) {
      for (int n = 0; n < 3; ++n) {
        expected.at(t).at(n) += rankDistribution.at(t).at(n);
      }
    }

    // local setup
    const int verticesPerRank = 4 / context.size;
    const int myOffset        = verticesPerRank * context.rank;
    double *  mypositions0    = positions.at(0).data() + 2 * myOffset;
    double *  mypositions2    = positions.at(1).data() + 2 * myOffset;

    // setup mesh
    std::vector<int> vertexIDs(verticesPerRank);
    auto             pVertexIDs = vertexIDs.data();
    interface.setMeshVertices(meshID, verticesPerRank, mypositions0, pVertexIDs);
    interface.initialize();

    std::vector<double> readValues(verticesPerRank);
    auto                pValues = readValues.data();
    // empty data from initialization
    interface.readBlockScalarData(dataID, 2, pVertexIDs, pValues);
    BOOST_TEST_CONTEXT("Initial")
    {
      for (int idx = 0; idx != verticesPerRank; ++idx) {
        BOOST_TEST_CONTEXT("idx " << idx)
        {
          BOOST_TEST(pValues[idx] == 0.0);
        }
      }
    }

    interface.advance(1.0);
    interface.readBlockScalarData(dataID, verticesPerRank, pVertexIDs, pValues);
    BOOST_TEST_CONTEXT("Timestep 0")
    {
      for (int idx = 0; idx != verticesPerRank; ++idx) {
        BOOST_TEST_CONTEXT("idx " << idx)
        {
          BOOST_TEST(pValues[idx] == expected.at(0).at(myOffset + idx));
        }
      }
    }

    interface.advance(1.0);
    interface.readBlockScalarData(dataID, verticesPerRank, pVertexIDs, pValues);
    BOOST_TEST_CONTEXT("Timestep 1")
    {
      for (int idx = 0; idx != verticesPerRank; ++idx) {
        BOOST_TEST_CONTEXT("idx " << idx)
        {
          BOOST_TEST(pValues[idx] == expected.at(1).at(myOffset + idx));
        }
      }
    }

    interface.resetMesh(meshID);
    interface.setMeshVertices(meshID, verticesPerRank, mypositions2, pVertexIDs);

    interface.advance(1.0);
    interface.readBlockScalarData(dataID, verticesPerRank, pVertexIDs, pValues);
    BOOST_TEST_CONTEXT("Timestep 2 - after reinit")
    {
      for (int idx = 0; idx != verticesPerRank; ++idx) {
        BOOST_TEST(pValues[idx] == expected.at(2).at(myOffset + idx));
      }
    }

    interface.finalize();
  }
}

BOOST_AUTO_TEST_CASE(ParallelExplicit_1_1)
{
  PRECICE_TEST("A"_on(1_rank), "B"_on(1_rank));
  RankDistribution rd{{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
  reinitParallelExplicit(context, rd);
}

BOOST_AUTO_TEST_CASE(ParallelExplicit_2_1)
{
  PRECICE_TEST("A"_on(2_ranks), "B"_on(1_rank));
  RankDistribution rd{{0, 0, 1, 1}, {0, 0, 1, 1}, {0, 0, 1, 1}, {0, 0, 1, 1}};
  reinitParallelExplicit(context, rd);
}

BOOST_AUTO_TEST_CASE(ParallelExplicit_1_2)
{
  PRECICE_TEST("A"_on(1_rank), "B"_on(2_ranks));
  RankDistribution rd{{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
  reinitParallelExplicit(context, rd);
}

BOOST_AUTO_TEST_CASE(ParallelExplicit_2_2)
{
  PRECICE_TEST("A"_on(2_ranks), "B"_on(2_ranks));
  RankDistribution rd{{0, 0, 1, 1}, {0, 0, 1, 1}, {0, 0, 1, 1}, {0, 0, 1, 1}};
  reinitParallelExplicit(context, rd);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
#endif // PRECICE_NO_MPI
