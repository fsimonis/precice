#pragma once

#include "MappingContext.hpp"
#include "SharedPointer.hpp"
#include "com/Communication.hpp"
#include "mapping/Mapping.hpp"
#include "mesh/SharedPointer.hpp"
#include "partition/ReceivedPartition.hpp"
#include "partition/SharedPointer.hpp"
#include <vector>

namespace precice
{
namespace impl
{

/// Stores a mesh and related objects and data.
struct MeshContext
{
  MeshContext(int dimensions)
      : localOffset(Eigen::VectorXd::Zero(dimensions))
  {}

  /// Mesh holding the geometry data structure.
  mesh::PtrMesh mesh;

  /// Data IDs of properties the geometry does possess.
  std::vector<int> associatedData;

  /// Determines which mesh type has to be provided by the accessor.
  mapping::Mapping::MeshRequirement meshRequirement = mapping::Mapping::UNDEFINED;

  /// Name of participant that creats the mesh.
  std::string receiveMeshFrom;

  /// bounding box to speed up decomposition of received mesh is increased by this safety factor
  double safetyFactor = -1;

  /// True, if accessor does create the mesh.
  bool provideMesh = false;

  /// type of geometric filter
  partition::ReceivedPartition::GeometricFilter geoFilter = partition::ReceivedPartition::GeometricFilter::UNDEFINED;

  /// Offset only applied to meshes local to the accessor.
  Eigen::VectorXd localOffset;

  /// Partition creating the parallel decomposition of the mesh
  partition::PtrPartition partition;

  /// Mapping used when mapping data from the mesh. Can be empty.
  MappingContext fromMappingContext;

  /// Mapping used when mapping data to the mesh. Can be empty.
  MappingContext toMappingContext;
};

} // namespace impl
} // namespace precice
