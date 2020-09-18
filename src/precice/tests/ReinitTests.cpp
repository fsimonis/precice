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
#include "utils/Parallel.hpp"

using namespace precice;
using testing::TestContext;

struct ReinitTestFixture : testing::WhiteboxAccessor {

  std::string _pathToTests;

  ParallelTestFixture()
  {
    _pathToTests = testing::getPathToSources() + "/precice/tests/";
  }
};

BOOST_AUTO_TEST_SUITE(PreciceTests)
BOOST_FIXTURE_TEST_SUITE(Reinit, ReinitTestFixture)

BOOST_AUTO_TEST_CASE(MeshReset, * testing::OnSize(4))
{
  std::string configFilename = _pathToTests + "config1.xml";
  config::Configuration config;

  // SolverOne - Static Geometry
  if(utils::Parallel::getProcessRank()<=1){
    utils::Parallel::splitCommunicator( "SolverOne" );
    utils::Parallel::clearGroups();
    xml::configure(config.getXMLTag(), configFilename);

    SolverInterface interface ( "SolverOne", utils::Parallel::getProcessRank(), 2 );
    impl(interface).configure(config.getSolverInterfaceConfiguration());
    int meshID = interface.getMeshID("MeshOne");
    int dataID = interface.getDataID("Data1", meshID);

    int vertexIDs[2];
    double xCoord = utils::Parallel::getProcessRank()*3;
    double positions[6] = {xCoord,0.0,0.0,xCoord+2.0,0.0,0.0};
    interface.setMeshVertices(meshID, 2, positions, vertexIDs);
    interface.initialize();
    double values[6] = {1.1, 1.2, 1.3, 3.1, 3.2, 3.3};
    interface.advance(1.0);
    interface.writeBlockVectorData(dataID, 2, vertexIDs, values);
    interface.advance(1.0);
    interface.writeBlockVectorData(dataID, 2, vertexIDs, values);
    interface.finalize();
  }
  // SolverTwo - Adaptive Geometry
  else {
    utils::Parallel::splitCommunicator( "SolverTwo" );
    utils::Parallel::clearGroups();
    xml::configure(config.getXMLTag(), configFilename);

    SolverInterface interface ( "SolverTwo", utils::Parallel::getProcessRank()-2, 2 );
    impl(interface).configure(config.getSolverInterfaceConfiguration());
    int meshID = interface.getMeshID("MeshTwo");
    int dataID = interface.getDataID("Data2", meshID);

    int vertexIDs[3];
    double xCoord = (utils::Parallel::getProcessRank()-2)*3;
    double positions[9] = {xCoord,0.0,0.0,xCoord+2.0,0.0,0.0,xCoord+1,0.0,0.0};
    interface.setMeshVertices(meshID, 2, positions, vertexIDs);
    interface.initialize();

    double values[9];
    interface.advance(1.0);
    interface.readBlockVectorData(dataID, 2, vertexIDs, values);

    interface.resetMesh(meshID);
    interface.setMeshVertices(meshID, 2, positions, vertexIDs);
    interface.advance(1.0);
    interface.readBlockVectorData(dataID, 2, vertexIDs, values);


    interface.finalize();
  }
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
#endif // PRECICE_NO_MPI
