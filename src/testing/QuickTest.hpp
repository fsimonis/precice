#pragma once

#include <cmath>
#include <precice/SolverInterface.hpp>
#include <string>
#include <vector>
#include "testing/TestContext.hpp"

namespace precice {
namespace testing {

struct QuickTest {

  struct Mesh {
    std::string name;
  };

  struct Data {
    std::string name;
    bool        vectorial;
  };

  QuickTest(SolverInterface &si, Mesh m, Data d)
      : interface(&si), dims(si.getDimensions()), meshID(si.getMeshID(m.name)), dataID(si.getDataID(d.name, meshID)), vectorial(d.vectorial)
  {
  }

  QuickTest &setVertices(const std::vector<double> &pos)
  {
    auto n = pos.size() / dims;
    vertexIDs.resize(n, -1);
    interface->setMeshVertices(meshID, n, pos.data(), vertexIDs.data());
    return *this;
  }

  QuickTest &initialize()
  {
    interface->initialize();
    return *this;
  }

  QuickTest &resetMesh()
  {
    interface->resetMesh(meshID);
    return *this;
  }

  void finalize()
  {
    interface->finalize();
  }

  QuickTest &advance(double dt = 1.0)
  {
    interface->advance(dt);
    return *this;
  }

  QuickTest &write(const std::vector<double> &data)
  {
    if (vectorial) {
      auto n = data.size() / dims;
      interface->writeBlockVectorData(dataID, n, vertexIDs.data(), data.data());
    } else {
      interface->writeBlockScalarData(dataID, data.size(), vertexIDs.data(), data.data());
    }
    return *this;
  }

  std::vector<double> read()
  {
    std::vector<double> result;
    if (vectorial) {
      auto n = vertexIDs.size() * dims;
      result.resize(n, -0.0);
      interface->readBlockVectorData(dataID, n, vertexIDs.data(), result.data());
    } else {
      auto n = vertexIDs.size();
      result.resize(n, -0.0);
      interface->readBlockScalarData(dataID, n, vertexIDs.data(), result.data());
    }
    return result;
  }

  SolverInterface *interface;
  int              dims;
  int              meshID;
  int              dataID;
  bool             vectorial;
  std::vector<int> vertexIDs;
};

inline QuickTest::Mesh operator""_mesh(const char *name, std::size_t)
{
  return {name};
}

inline QuickTest::Data operator""_scalar(const char *name, std::size_t)
{
  return {name, false};
}

inline QuickTest::Data operator""_vector(const char *name, std::size_t)
{
  return {name, true};
}

} // namespace testing
} // namespace precice
