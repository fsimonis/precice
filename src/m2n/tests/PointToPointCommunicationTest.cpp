#ifndef PRECICE_NO_MPI

#include <vector>
#include "com/MPIDirectCommunication.hpp"
#include "com/MPIPortsCommunicationFactory.hpp"
#include "com/SocketCommunicationFactory.hpp"
#include "m2n/PointToPointCommunication.hpp"
#include "mesh/Mesh.hpp"
#include "testing/Testing.hpp"
#include "utils/MasterSlave.hpp"

using precice::testing::TestContext;
using precice::utils::MasterSlave;

using std::vector;

using namespace precice;
using namespace m2n;

BOOST_AUTO_TEST_SUITE(M2NTests)

void process(vector<double> &data)
{
  for (auto &elem : data) {
    elem += MasterSlave::getRank() + 1;
  }
}

void P2PComTest1(const TestContext &context, com::PtrCommunicationFactory cf)
{
  BOOST_TEST(context.hasSize(2));

  mesh::PtrMesh mesh(new mesh::Mesh("Mesh", 2, true, testing::nextMeshID()));

  m2n::PointToPointCommunication c(cf, mesh);

  vector<double> data;
  vector<double> expectedData;

  if (context.isNamed("A")) {
    if (context.isMaster()) {
      mesh->setGlobalNumberOfVertices(10);

      mesh->getVertexDistribution()[0].push_back(0);
      mesh->getVertexDistribution()[0].push_back(1);
      mesh->getVertexDistribution()[0].push_back(3);
      mesh->getVertexDistribution()[0].push_back(5);
      mesh->getVertexDistribution()[0].push_back(7);

      mesh->getVertexDistribution()[1].push_back(1);
      mesh->getVertexDistribution()[1].push_back(2);
      mesh->getVertexDistribution()[1].push_back(4);
      mesh->getVertexDistribution()[1].push_back(5);
      mesh->getVertexDistribution()[1].push_back(6);

      data         = {10, 20, 40, 60, 80};
      expectedData = {10 + 2, 4 * 20 + 3, 40 + 2, 4 * 60 + 3, 80 + 2};
    } else { // A Slave
      data         = {20, 30, 50, 60, 70};
      expectedData = {4 * 20 + 3, 30 + 1, 50 + 2, 4 * 60 + 3, 70 + 1};
    }
  } else {
    BOOST_TEST(context.isNamed("B"));
    if (context.isMaster()) {
      mesh->setGlobalNumberOfVertices(10);

      mesh->getVertexDistribution()[0].push_back(1);
      mesh->getVertexDistribution()[0].push_back(2);
      mesh->getVertexDistribution()[0].push_back(5);
      mesh->getVertexDistribution()[0].push_back(6);

      mesh->getVertexDistribution()[1].push_back(0);
      mesh->getVertexDistribution()[1].push_back(1);
      mesh->getVertexDistribution()[1].push_back(3);
      mesh->getVertexDistribution()[1].push_back(4);
      mesh->getVertexDistribution()[1].push_back(5);
      mesh->getVertexDistribution()[1].push_back(7);

      data.assign(4, -1);
      expectedData = {2 * 20, 30, 2 * 60, 70};

    } else {
      data.assign(6, -1);
      expectedData = {10, 2 * 20, 40, 50, 2 * 60, 80};
    }
  }

  if (context.isNamed("A")) {
    c.requestConnection("B", "A");

    c.send(data.data(), data.size());
    c.receive(data.data(), data.size());

    BOOST_TEST(data == expectedData);
  } else {
    c.acceptConnection("B", "A");

    c.receive(data.data(), data.size());
    BOOST_TEST(data == expectedData);
    process(data);
    c.send(data.data(), data.size());
  }
}

/// a very similar test, but with a vertex that has been completely filtered out
void P2PComTest2(const TestContext &context, com::PtrCommunicationFactory cf)
{
  BOOST_TEST(context.hasSize(2));
  mesh::PtrMesh mesh(new mesh::Mesh("Mesh", 2, true, testing::nextMeshID()));

  m2n::PointToPointCommunication c(cf, mesh);

  vector<double> data;
  vector<double> expectedData;

  if (context.isNamed("A")) {
    if (context.isMaster()) {
      mesh->setGlobalNumberOfVertices(10);

      mesh->getVertexDistribution()[0].push_back(0);
      mesh->getVertexDistribution()[0].push_back(1);
      mesh->getVertexDistribution()[0].push_back(3);
      mesh->getVertexDistribution()[0].push_back(5);
      mesh->getVertexDistribution()[0].push_back(7);

      mesh->getVertexDistribution()[1].push_back(1);
      mesh->getVertexDistribution()[1].push_back(2);
      mesh->getVertexDistribution()[1].push_back(4);
      mesh->getVertexDistribution()[1].push_back(5);
      mesh->getVertexDistribution()[1].push_back(6);

      data         = {10, 20, 40, 60, 80};
      expectedData = {10 + 2, 4 * 20 + 3, 2 * 40 + 3, 4 * 60 + 3, 80 + 2};
    } else {
      data         = {20, 30, 50, 60, 70};
      expectedData = {4 * 20 + 3, 0, 50 + 2, 4 * 60 + 3, 70 + 1};
    }
  } else {
    BOOST_TEST(context.isNamed("B"));
    if (context.isMaster()) {
      mesh->setGlobalNumberOfVertices(10);

      mesh->getVertexDistribution()[0].push_back(1);
      mesh->getVertexDistribution()[0].push_back(3);
      mesh->getVertexDistribution()[0].push_back(5);
      mesh->getVertexDistribution()[0].push_back(6);

      mesh->getVertexDistribution()[1].push_back(0);
      mesh->getVertexDistribution()[1].push_back(1);
      mesh->getVertexDistribution()[1].push_back(3);
      mesh->getVertexDistribution()[1].push_back(4);
      mesh->getVertexDistribution()[1].push_back(5);
      mesh->getVertexDistribution()[1].push_back(7);

      data.assign(4, -1);
      expectedData = {2 * 20, 40, 2 * 60, 70};

    } else {
      data.assign(6, -1);
      expectedData = {10, 2 * 20, 40, 50, 2 * 60, 80};
    }
  }

  if (context.isNamed("A")) {
    c.requestConnection("B", "A");

    c.send(data.data(), data.size());
    c.receive(data.data(), data.size());
    BOOST_TEST(data == expectedData);
  } else {
    c.acceptConnection("B", "A");

    c.receive(data.data(), data.size());
    BOOST_TEST(data == expectedData);
    process(data);
    c.send(data.data(), data.size());
  }
}

void connectionTest(const TestContext &context, com::PtrCommunicationFactory cf)
{

  BOOST_TEST(context.hasSize(2));

  int           dimensions  = 2;
  bool          flipNormals = false;
  mesh::PtrMesh mesh(new mesh::Mesh("Mesh", dimensions, flipNormals, testing::nextMeshID()));

  std::vector<std::string> conections = {"same", "cross"};

  for (auto &connectionType : conections) {

    if (context.isNamed("A")) {
      if (context.isMaster()) {

        if (connectionType == "same") {
          mesh->getConnectedRanks().push_back(0);
        } else {
          mesh->getConnectedRanks().push_back(1);
        }
      } else {

        if (connectionType == "same") {
          mesh->getConnectedRanks().push_back(1);
        } else {
          mesh->getConnectedRanks().push_back(0);
        }
      }
    } else {
      BOOST_TEST(context.isNamed("B"));
      if (context.isMaster()) {

        if (connectionType == "same") {
          mesh->getConnectedRanks().push_back(0);
        } else {
          mesh->getConnectedRanks().push_back(1);
        }
      } else {

        if (connectionType == "same") {
          mesh->getConnectedRanks().push_back(1);
        } else {
          mesh->getConnectedRanks().push_back(0);
        }
      }
    }

    m2n::PointToPointCommunication c(cf, mesh);

    std::vector<int> receiveData;

    if (context.isNamed("A")) {
      if (context.isMaster()) {

        c.requestPreConnection("Solid", "Fluid");
        int sendData = 5;
        c.broadcastSend(sendData);

      } else {

        c.requestPreConnection("Solid", "Fluid");
        int sendData = 10;
        c.broadcastSend(sendData);
      }
    } else {
      c.acceptPreConnection("Solid", "Fluid");
      c.broadcastReceiveAll(receiveData);

      if (context.isMaster()) {
        if (connectionType == "same") {
          BOOST_TEST(receiveData[0] == 5);
        } else {
          BOOST_TEST(receiveData[1] == 10);
        }
      } else {
        if (connectionType == "same") {
          BOOST_TEST(receiveData[0] == 10);
        } else {
          BOOST_TEST(receiveData[1] == 5);
        }
      }
    }

    mesh::Data::resetDataCount();
  }
}

void emptyConnectionTest(const TestContext &context, com::PtrCommunicationFactory cf)
{
  BOOST_TEST(context.hasSize(2));

  int           dimensions  = 2;
  bool          flipNormals = false;
  mesh::PtrMesh mesh(new mesh::Mesh("Mesh", dimensions, flipNormals, testing::nextMeshID()));

  if (context.isNamed("A")) {
    if (context.isMaster()) {

      mesh->getConnectedRanks().push_back(0);

    } else {
    }
  } else {
    BOOST_TEST(context.isNamed("B"));
    if (context.isMaster()) {
      mesh->getConnectedRanks().push_back(0);

    } else {
    }
  }

  m2n::PointToPointCommunication c(cf, mesh);

  std::vector<int> receiveData;

  if (context.isNamed("A")) {
    c.requestPreConnection("Solid", "Fluid");
    int sendData = 5;
    c.broadcastSend(sendData);
  } else {
    c.acceptPreConnection("Solid", "Fluid");
    c.broadcastReceiveAll(receiveData);

    if (context.isMaster()) {
      BOOST_TEST(receiveData[0] == 5);

    } else {
      BOOST_TEST(receiveData.size() == 0);
    }
  }

  mesh::Data::resetDataCount();
}

void P2PMeshBroadcastTest(const TestContext &context, com::PtrCommunicationFactory cf)
{
  BOOST_TEST(context.hasSize(2));

  int           dimensions  = 2;
  bool          flipNormals = false;
  mesh::PtrMesh mesh(new mesh::Mesh("Mesh", dimensions, flipNormals, testing::nextMeshID()));

  if (context.isNamed("A")) {
    if (context.isMaster()) {
      Eigen::VectorXd position(dimensions);
      position << 5.5, 0.0;
      mesh::Vertex &v1 = mesh->createVertex(position);
      position << 1.0, 2.0;
      mesh::Vertex &v2 = mesh->createVertex(position);
      mesh->createEdge(v1, v2);

      mesh->getConnectedRanks().push_back(0);

    } else {
      Eigen::VectorXd position(dimensions);
      position << 1.5, 0.0;
      mesh::Vertex &v1 = mesh->createVertex(position);
      position << 1.5, 2.0;
      mesh::Vertex &v2 = mesh->createVertex(position);
      mesh->createEdge(v1, v2);

      mesh->getConnectedRanks().push_back(1);
    }
  } else {
    BOOST_TEST(context.isNamed("B"));
    if (context.isMaster()) {
      mesh->getConnectedRanks().push_back(0);

    } else {
      mesh->getConnectedRanks().push_back(1);
    }
  }

  m2n::PointToPointCommunication c(cf, mesh);

  if (context.isNamed("A")) {

    c.requestPreConnection("Solid", "Fluid");
    c.broadcastSendMesh();
  } else {

    c.acceptPreConnection("Solid", "Fluid");
    c.broadcastReceiveMesh();

    if (context.isMaster()) {
      // This rank should receive the mesh from rank 0 (fluid master)
      BOOST_TEST(mesh->vertices().size() == 2);
      BOOST_TEST(mesh->vertices()[0].getCoords()[0] == 5.50);
      BOOST_TEST(mesh->vertices()[0].getCoords()[1] == 0.0);
      BOOST_TEST(mesh->vertices()[1].getCoords()[0] == 1.0);
      BOOST_TEST(mesh->vertices()[1].getCoords()[1] == 2.0);
    } else {
      // This rank should receive the mesh from rank 1 (fluid slave)
      BOOST_TEST(mesh->vertices().size() == 2);
      BOOST_TEST(mesh->vertices()[0].getCoords()[0] == 1.50);
      BOOST_TEST(mesh->vertices()[0].getCoords()[1] == 0.0);
      BOOST_TEST(mesh->vertices()[1].getCoords()[0] == 1.50);
      BOOST_TEST(mesh->vertices()[1].getCoords()[1] == 2.0);
    }
  }

  mesh::Data::resetDataCount();
}

void P2PComLCMTest(const TestContext &context, com::PtrCommunicationFactory cf)
{
  BOOST_TEST(context.hasSize(2));

  int                             dimensions  = 2;
  bool                            flipNormals = false;
  mesh::PtrMesh                   mesh(new mesh::Mesh("Mesh", dimensions, flipNormals, testing::nextMeshID()));
  const auto                      expectedId = mesh->getID();
  std::map<int, std::vector<int>> localCommunicationMap;

  if (context.isNamed("A")) {
    if (context.isMaster()) {

      // The numbers are chosen in this way to make it easy to test weather
      // correct values are communicated or not!
      mesh->getConnectedRanks().push_back(0);
      localCommunicationMap[0].push_back(102);
      localCommunicationMap[0].push_back(1022);
      localCommunicationMap[0].push_back(10222);
      localCommunicationMap[1].push_back(103);
      localCommunicationMap[1].push_back(1033);
      localCommunicationMap[1].push_back(10333);

    } else {

      // The numbers are chosen in this way to make it easy to test weather
      // correct values are communicated or not!
      mesh->getConnectedRanks().push_back(1);
      localCommunicationMap[0].push_back(112);
      localCommunicationMap[0].push_back(1122);
      localCommunicationMap[0].push_back(11222);
      localCommunicationMap[1].push_back(113);
      localCommunicationMap[1].push_back(1133);
      localCommunicationMap[1].push_back(11333);
    }
  } else {
    BOOST_TEST(context.isNamed("B"));
    if (context.isMaster()) {

      mesh->getConnectedRanks().push_back(0);

    } else {

      mesh->getConnectedRanks().push_back(1);
    }
  }

  m2n::PointToPointCommunication c(cf, mesh);

  if (context.isNamed("A")) {

    c.requestPreConnection("Solid", "Fluid");
    c.broadcastSendLCM(localCommunicationMap);
    BOOST_TEST(mesh->getID() == expectedId);

  } else {
    c.acceptPreConnection("Solid", "Fluid");
    c.broadcastReceiveLCM(localCommunicationMap);
    BOOST_TEST(mesh->getID() == expectedId);
  }

  if (context.isNamed("B")) {
    if (context.isMaster()) {
      // The numbers are chosen in this way to make it easy to test weather
      // correct values are communicated or not!
      BOOST_TEST(localCommunicationMap.size() == 1);
      BOOST_TEST(localCommunicationMap[0].size() == 3);
      BOOST_TEST(localCommunicationMap[0][0] == 102);
      BOOST_TEST(localCommunicationMap[0][1] == 1022);
      BOOST_TEST(localCommunicationMap[0][2] == 10222);

    } else {
      // The numbers are chosen in this way to make it easy to test weather
      // correct values are communicated or not!
      BOOST_TEST(localCommunicationMap.size() == 1);
      BOOST_TEST(localCommunicationMap[1].size() == 3);
      BOOST_TEST(localCommunicationMap[1][0] == 113);
      BOOST_TEST(localCommunicationMap[1][1] == 1133);
      BOOST_TEST(localCommunicationMap[1][2] == 11333);
    }
  }

  mesh::Data::resetDataCount();
}

BOOST_AUTO_TEST_CASE(SocketCommunication)
{
  PRECICE_TEST("A"_on(2_ranks).setupMasterSlaves(), "B"_on(2_ranks).setupMasterSlaves(), Require::Events);
  com::PtrCommunicationFactory cf(new com::SocketCommunicationFactory);
  P2PComTest1(context, cf);
  P2PComTest2(context, cf);
  connectionTest(context, cf);
  emptyConnectionTest(context, cf);
  P2PMeshBroadcastTest(context, cf);
  P2PComLCMTest(context, cf);
}

BOOST_AUTO_TEST_CASE(MPIPortsCommunication, *boost::unit_test::label("MPI_Ports"))
{
  PRECICE_TEST("A"_on(2_ranks).setupMasterSlaves(), "B"_on(2_ranks).setupMasterSlaves(), Require::Events);
  com::PtrCommunicationFactory cf(new com::MPIPortsCommunicationFactory);
  P2PComTest1(context, cf);
  P2PComTest2(context, cf);
  connectionTest(context, cf);
  emptyConnectionTest(context, cf);
}

BOOST_AUTO_TEST_SUITE_END()

#endif // not PRECICE_NO_MPI
