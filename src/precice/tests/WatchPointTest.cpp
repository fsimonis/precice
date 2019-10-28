#include "../impl/WatchPoint.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/SharedPointer.hpp"
#include "mesh/Vertex.hpp"
#include "testing/Testing.hpp"
#include <iterator>
#include <vector>

using namespace precice;

BOOST_AUTO_TEST_SUITE(PreciceTests)

namespace {
    std::vector<double> readDoublesFromTXTFile(std::string const & filename, int skip = 0) {
        std::ifstream is{filename};
        if (skip > 0) {
            std::string ignore;
            while(skip--) {
                is >> ignore;
            }
        }
        return {std::istream_iterator<double>{is}, std::istream_iterator<double>{}};
    }
}

BOOST_AUTO_TEST_CASE(WatchPoint)
{
  using namespace mesh;
  int dim = 2;
  using Eigen::VectorXd;
  // Setup geometry
  std::string name("rectangle");
  bool flipNormals = false;
  PtrMesh mesh(new Mesh(name, dim, flipNormals));

  mesh::Vertex& v1 = mesh->createVertex(Eigen::Vector2d(1.0, 1.0));
  mesh::Vertex& v2 = mesh->createVertex(Eigen::Vector2d(2.0, 1.0));
  mesh::Vertex& v3 = mesh->createVertex(Eigen::Vector2d(1.0, 2.0));
  mesh::Vertex& v4 = mesh->createVertex(Eigen::Vector2d(2.0, 2.0));
  mesh->createEdge(v1,v2);
  mesh->createEdge(v1,v3);
  mesh->createEdge(v2,v4);
  mesh->createEdge(v3,v4);

  PtrData doubleData = mesh->createData("DoubleData", 1);
  PtrData vectorData = mesh->createData("VectorData", dim);
  auto &doubleValues = doubleData->values();
  auto &vectorValues = vectorData->values();
  mesh->computeState();
  mesh->allocateDataValues();
  doubleValues.setConstant(1.0);
  vectorValues.setConstant(1.0);


  std::string filename0("precice-WatchPointTest-output0.log");
  std::string filename1("precice-WatchPointTest-output1.log");

  // this scope forces the filestreams to be closed
  {
      // Create watchpoints
      Eigen::Vector2d pointToWatch0(1.0, 1.0);
      impl::WatchPoint watchpoint0(pointToWatch0, mesh, filename0);
      Eigen::Vector2d pointToWatch1(1.0, 1.5);
      impl::WatchPoint watchpoint1(pointToWatch1, mesh, filename1);

      // Initialize
      watchpoint0.initialize();
      watchpoint1.initialize();

      // Write output
      watchpoint0.exportPointData(0.0);
      watchpoint1.exportPointData(0.0);

      // Change data and write output again
      doubleValues.setConstant(3.0);
      vectorValues.setConstant(3.0);
      watchpoint0.exportPointData(1.0);
      watchpoint1.exportPointData(1.0);

      // Change Mesh and data
      mesh::Vertex& v5 = mesh->createVertex(Eigen::Vector2d(3.0, 2.0));
      mesh->createEdge(v4,v5);
      mesh->allocateDataValues();
      mesh->computeState();
      doubleValues.setConstant(5.0);
      vectorValues.setConstant(5.0);

      // Re-Initialize
      watchpoint0.initialize();
      watchpoint1.initialize();

      // Write output
      watchpoint0.exportPointData(2.0);
      watchpoint1.exportPointData(2.0);
  }
  
  // File Format: Time  Coordinate0  Coordinate1  DoubleData  VectorData0  VectorData1
  BOOST_TEST_CONTEXT("Validating watchpoint0")
  {
      auto result = readDoublesFromTXTFile(filename0, 6);
      auto expected = std::vector<double>{
          0.0, 1.0, 1.0, 1.0, 1.0, 1.0,
          1.0, 1.0, 1.0, 3.0, 3.0, 3.0,
          2.0, 1.0, 1.0, 5.0, 5.0, 5.0  
      };
      BOOST_TEST(result.size() == expected.size());
      for(size_t i = 0; i < result.size(); ++i) {
          BOOST_TEST_CONTEXT("entry index: " << i) {
              using testing::equals;
              BOOST_TEST(equals(result.at(i), expected.at(i)));
          }
      }
  }

  BOOST_TEST_CONTEXT("Validating watchpoint1")
  {
      auto result = readDoublesFromTXTFile(filename1, 6);
      auto expected = std::vector<double>{
          0.0, 1.0, 1.5, 1.0, 1.0, 1.0,
          1.0, 1.0, 1.5, 3.0, 3.0, 3.0,
          2.0, 1.0, 1.5, 5.0, 5.0, 5.0
      };
      BOOST_TEST(result.size() == expected.size());
      for(size_t i = 0; i < result.size(); ++i) {
          BOOST_TEST_CONTEXT("entry index: " << i) {
              using testing::equals;
              BOOST_TEST(equals(result.at(i), expected.at(i)));
          }
      }
  }
}

BOOST_AUTO_TEST_SUITE_END() // Precice
