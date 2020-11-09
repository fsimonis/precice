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

void reinitParallelExplicit(const testing::TestContext& context)
{
  std::string configFilename = testing::getPathToSources() + "/precice/tests/reinit-parallel-explicit.xml";

  // SolverOne - Static Geometry
  if (context.isNamed("SolverOne")) {
    SolverInterface interface("SolverOne", configFilename, context.rank, context.size);

    const int meshID = interface.getMeshID("MeshOne");
    const int dataID = interface.getDataID("Data1", meshID);

    int    vertexIDs[2];
    double xCoord       = context.rank * 2.0;
    double positions[6] = {xCoord, 0.0, xCoord + 1.0, 0.0};
    interface.setMeshVertices(meshID, 2, positions, vertexIDs);
    interface.initialize();

    // value format <RANK>.<time><node>
    double valuest0[2] = {context.rank + 0.01,  context.rank + 0.02};
    interface.writeBlockScalarData(dataID, 2, vertexIDs, valuest0);

    interface.advance(1.0);
    double valuest1[2] = {context.rank + 0.11,  context.rank + 0.12};
    interface.writeBlockScalarData(dataID, 2, vertexIDs, valuest1);

    interface.advance(1.0);
    double valuest2[2] = {context.rank + 0.21,  context.rank + 0.22};
    interface.writeBlockScalarData(dataID, 2, vertexIDs, valuest2);

    interface.advance(1.0);
    interface.finalize();
  }
  // SolverTwo - Adaptive Geometry
  else {
    BOOST_REQUIRE(context.isNamed("SolverTwo"));
    SolverInterface interface("SolverTwo", configFilename, context.rank, context.size);

    const int meshID = interface.getMeshID("MeshTwo");
    const int dataID = interface.getDataID("Data1", meshID);

    int    vertexIDs[2];
    double xCoord       = context.rank * 2;
    double positions[4] = {xCoord, 0.0, xCoord + 1.0, 0.0};
    interface.setMeshVertices(meshID, 2, positions, vertexIDs);
    interface.initialize();

    double values[2];
    // empty data from initialization
    interface.readBlockScalarData(dataID, 2, vertexIDs, values);
    BOOST_REQUIRE(values[0] == 0.0);
    BOOST_REQUIRE(values[1] == 0.0);

    interface.advance(1.0);
    interface.readBlockScalarData(dataID, 2, vertexIDs, values);
    BOOST_TEST(values[0] == xCoord + 0.01);
    BOOST_TEST(values[1] == xCoord + 0.02);

    interface.advance(1.0);
    interface.readBlockScalarData(dataID, 2, vertexIDs, values);
    BOOST_TEST(values[0] == xCoord + 0.11);
    BOOST_TEST(values[1] == xCoord + 0.12);

    interface.resetMesh(meshID);
    double newpositions[4] = {xCoord, 0.2, xCoord + 1.0, 0.1};
    interface.setMeshVertices(meshID, 2, newpositions, vertexIDs);

    interface.advance(1.0);
    interface.readBlockScalarData(dataID, 2, vertexIDs, values);
    BOOST_TEST(values[0] == xCoord + 0.21);
    BOOST_TEST(values[1] == xCoord + 0.22);

    interface.finalize();
  }
}

BOOST_AUTO_TEST_CASE(ParallelExplicitSingle)
{
  PRECICE_TEST("SolverOne"_on(1_rank), "SolverTwo"_on(1_rank));
  reinitParallelExplicit(context);
}

BOOST_AUTO_TEST_CASE(ParallelExplicitDoubleOne)
{
  PRECICE_TEST("SolverOne"_on(2_ranks), "SolverTwo"_on(1_rank));
  reinitParallelExplicit(context);
}

BOOST_AUTO_TEST_CASE(ParallelExplicitDoubleTwo)
{
  PRECICE_TEST("SolverOne"_on(1_rank), "SolverTwo"_on(2_ranks));
  reinitParallelExplicit(context);
}

BOOST_AUTO_TEST_CASE(ParallelExplicitFullDouble)
{
  PRECICE_TEST("SolverOne"_on(2_ranks), "SolverTwo"_on(2_ranks));
  reinitParallelExplicit(context);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
#endif // PRECICE_NO_MPI
