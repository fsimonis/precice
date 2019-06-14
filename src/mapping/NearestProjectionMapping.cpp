#include "NearestProjectionMapping.hpp"
#include "query/FindClosest.hpp"
#include <Eigen/Core>
#include "utils/Event.hpp"
#include "mesh/RTree.hpp"
#include <stdexcept>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

namespace precice {
extern bool syncMode;

namespace mapping {

NearestProjectionMapping:: NearestProjectionMapping
(
  Constraint constraint,
  int        dimensions)
:
  Mapping(constraint, dimensions)
{
  if (constraint == CONSISTENT){
    setInputRequirement(Mapping::MeshRequirement::FULL);
    setOutputRequirement(Mapping::MeshRequirement::VERTEX);
  }
  else {
    assertion(constraint == CONSERVATIVE, constraint);
    setInputRequirement(Mapping::MeshRequirement::VERTEX);
    setOutputRequirement(Mapping::MeshRequirement::FULL);
  }
}

namespace{
    struct MatchType {
        double distance;
        int index;

        MatchType() = default;
        MatchType(double d, int i):distance(d), index(i) {};
        constexpr bool operator<(MatchType const & other) const { return distance < other.distance; };
    };
}

void NearestProjectionMapping:: computeMapping()
{
    TRACE(input()->vertices().size(), output()->vertices().size());
    precice::utils::Event e("map.np.computeMapping.From" + input()->getName() + "To" + output()->getName(), precice::syncMode);

    // Setup Direction of Mapping
    mesh::PtrMesh from, to;
    if (getConstraint() == CONSISTENT){
        DEBUG("Compute consistent mapping");
        from = output();
        to   = input();
    } else {
        DEBUG("Compute conservative mapping");
        from = input();
        to   = output();
    }

    const auto &fVertices = from->vertices();
    const auto &tVertices = to->vertices();
    const auto &tEdges    = to->edges();

    _weights.resize(fVertices.size());

    constexpr int nnearest = 4;

    if (getDimensions() == 2) {
        //CHECK(fVertices.empty() || !tEdges.empty(), "Mesh \"" << to->getName() << "\" does not contain Edges to project onto.");

        auto indexEdges = mesh::rtree::getEdgeRTree(to);
        auto indexVertices = mesh::rtree::getVertexRTree(to);

        std::vector<MatchType> matches;
        matches.reserve(nnearest);
        for (size_t i = 0; i < fVertices.size(); i++) {
            const Eigen::VectorXd &coords = fVertices[i].getCoords();
            // Search for the from vertex inside the destination meshes edges
            matches.clear();
            indexEdges->query(bg::index::nearest(coords, nnearest),
                    boost::make_function_output_iterator([&](int match) {
                        matches.emplace_back(bg::distance(coords, tEdges[match]), match);
                        }));
            std::sort(matches.begin(), matches.end());
            bool found = false;
            for (const auto& match: matches) {
                auto weights = query::generateInterpolationElements(fVertices[i], tEdges[match.index]);
                if (std::all_of(weights.begin(), weights.end(), [](query::InterpolationElement const & elem){ return elem.weight >= 0.0; })) {
                    _weights[i] = std::move(weights);
                    found = true;
                    break;
                }
            }

            if (not found) {
                // Search for the from vertex inside the destination meshes vertices
                indexVertices->query(bg::index::nearest(coords, 1), 
                        boost::make_function_output_iterator([&](int match) {
                            _weights[i] = query::generateInterpolationElements(fVertices[i], tVertices[match]);
                            }));
            }
        }
    } else {
        const auto &tTriangles = to->triangles();
        //CHECK(fVertices.empty() || !tTriangles.empty(), "Mesh \"" << to->getName() << "\" does not contain Triangles to project onto.");

        auto indexTriangles = mesh::rtree::getTriangleRTree(to);
        auto indexEdges     = mesh::rtree::getEdgeRTree(to);
        auto indexVertices  = mesh::rtree::getVertexRTree(to);

        std::vector<MatchType> matches;
        matches.reserve(nnearest);
        for (size_t i = 0; i < fVertices.size(); i++) {
            const Eigen::VectorXd &coords = fVertices[i].getCoords();

            // Search for the vertex inside the destination meshes triangles
            matches.clear();
            indexTriangles->query(bg::index::nearest(coords, nnearest),
                    boost::make_function_output_iterator([&](mesh::rtree::triangle_traits::IndexType const & match) {
                        matches.emplace_back(bg::distance(coords, tTriangles[match.second]), match.second);
                        }));
            std::sort(matches.begin(), matches.end());
            bool found = false;
            for (const auto& match: matches) {
                auto weights = query::generateInterpolationElements(fVertices[i], tTriangles[match.index]);
                if (std::all_of(weights.begin(), weights.end(), [](query::InterpolationElement const & elem){ return elem.weight >= 0.0; })) {
                    _weights[i] = std::move(weights);
                    found = true;
                    break;
                }
            }

            if (not found) {
                // Search for the vertex inside the destination meshes edges
                matches.clear();
                indexEdges->query(bg::index::nearest(coords, nnearest),
                        boost::make_function_output_iterator([&](int match) {
                            matches.emplace_back(bg::distance(coords, tEdges[match]), match);
                            }));
                std::sort(matches.begin(), matches.end());
                for (const auto& match: matches) {
                    auto weights = query::generateInterpolationElements(fVertices[i], tEdges[match.index]);
                    if (std::all_of(weights.begin(), weights.end(), [](query::InterpolationElement const & elem){ return elem.weight >= 0.0; })) {
                        _weights[i] = std::move(weights);
                        found = true;
                        break;
                    }
                }
            }

            if (not found) {
                // Search for the vertex inside the destination meshes vertices
                indexVertices->query(bg::index::nearest(coords, 1), 
                        boost::make_function_output_iterator([&](int match) {
                            _weights[i] = query::generateInterpolationElements(fVertices[i], tVertices[match]);
                            }));
            }
        }
    }
    _hasComputedMapping = true;
}

bool NearestProjectionMapping:: hasComputedMapping() const
{
  return _hasComputedMapping;
}

void NearestProjectionMapping:: clear()
{
  TRACE();
  _weights.clear();
  _hasComputedMapping = false;
  if (getConstraint() == CONSISTENT){
    mesh::rtree::clear(*input()); 
  } else {
    mesh::rtree::clear(*output()); 
  }
}

void NearestProjectionMapping:: map
(
  int inputDataID,
  int outputDataID )
{
  TRACE(inputDataID, outputDataID);

  precice::utils::Event e("map.np.mapData.From" + input()->getName() + "To" + output()->getName(), precice::syncMode);

  mesh::PtrData inData = input()->data(inputDataID);
  mesh::PtrData outData = output()->data(outputDataID);
  const Eigen::VectorXd& inValues = inData->values();
  Eigen::VectorXd& outValues = outData->values();

  int dimensions = inData->getDimensions();
  assertion(dimensions == outData->getDimensions());

  if (getConstraint() == CONSISTENT){
    DEBUG("Map consistent");
    assertion(_weights.size() == output()->vertices().size(),
               _weights.size(), output()->vertices().size());
    for (size_t i=0; i < output()->vertices().size(); i++){
      InterpolationElements& elems = _weights[i];
      size_t outOffset = i * dimensions;
      for (query::InterpolationElement& elem : elems) {
        size_t inOffset = (size_t)elem.element->getID() * dimensions;
        for (int dim=0; dim < dimensions; dim++){
          assertion(outOffset + dim < (size_t)outValues.size());
          assertion(inOffset + dim < (size_t)inValues.size());
          outValues(outOffset + dim) += elem.weight * inValues(inOffset + dim);
        }
      }
    }
  }
  else {
    assertion(getConstraint() == CONSERVATIVE, getConstraint());
    DEBUG("Map conservative");
    assertion(_weights.size() == input()->vertices().size(),
               _weights.size(), input()->vertices().size());
    for (size_t i=0; i < input()->vertices().size(); i++){
      size_t inOffset = i * dimensions;
      InterpolationElements& elems = _weights[i];
      for (query::InterpolationElement& elem : elems) {
        size_t outOffset = (size_t)elem.element->getID() * dimensions;
        for ( int dim=0; dim < dimensions; dim++ ){
          assertion(outOffset + dim < (size_t)outValues.size());
          assertion(inOffset + dim < (size_t)inValues.size());
          outValues(outOffset + dim) += elem.weight * inValues(inOffset + dim);
        }
      }
    }
  }
}

void NearestProjectionMapping::tagMeshFirstRound()
{
  TRACE();
  DEBUG("Compute Mapping for Tagging");

  computeMapping();
  DEBUG("Tagging First Round");

  std::set<int> tagged;

  if (getConstraint() == CONSISTENT){
    for(mesh::Vertex& v : input()->vertices()){
      for (size_t i=0; i < output()->vertices().size(); i++) {
        const InterpolationElements& elems = _weights[i];
        for (const query::InterpolationElement& elem : elems) {
          if (elem.element->getID()==v.getID() && !math::equals(elem.weight,0.0)) {
            v.tag();
            tagged.insert(v.getID());
          }
        }
      }
    }
    DEBUG("First Round Tagged " << tagged.size() << "/" << input()->vertices().size() << " Vertices");
  }
  else {
    assertion(getConstraint() == CONSERVATIVE, getConstraint());
    for(mesh::Vertex& v : output()->vertices()){
      for (size_t i=0; i < input()->vertices().size(); i++) {
        const InterpolationElements& elems = _weights[i];
        for (const query::InterpolationElement& elem : elems) {
          if (elem.element->getID()==v.getID() && !math::equals(elem.weight,0.0)) {
            v.tag();
            tagged.insert(v.getID());
          }
        }
      }
    }
    DEBUG("First Round Tagged " << tagged.size() << "/" << output()->vertices().size() << " Vertices");
  }

  clear();
}

void NearestProjectionMapping::tagMeshSecondRound()
{
  TRACE();
  // for NP mapping no operation needed here
}

}} // namespace precice, mapping
