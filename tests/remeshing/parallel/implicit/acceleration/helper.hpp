#pragma once

#include <string>

#include <precice/Participant.hpp>
#include "testing/QuickTest.hpp"
#include "testing/TestContext.hpp"
#include "testing/Testing.hpp"

namespace precice::tests::remesh::parallelImplicit {

using testing::QuickTest;
using testing::operator""_mesh;
using testing::operator""_read;
using testing::operator""_write;

namespace acceleration {

inline void runResetA(testing::TestContext &context)
{
  constexpr double y = 0.0;

  BOOST_REQUIRE(context.size == 1);
  BOOST_REQUIRE(context.rank == 0);
  Participant p{context.name, context.config(), 0, 1};

  // The data format uses the following format:
  //  0 xpos . (00 | 10 | 11 )

  // A - Adaptive Geometry
  if (context.isNamed("A")) {
    QuickTest(p, "MA"_mesh, "DB"_read, "DA"_write)
        .setVertices({1.0, y, 2.0, y})
        .initialize()
        .expectWriteCheckpoint()
        .expectAll(00.00)
        .writeAll(40.00)
        .advance()

        .expectReadCheckpoint()
        .expectAll(20.00)
        .writeAll(30.00)
        .advance()

        .expectReadCheckpoint()
        .expectAll(25.00)
        .writeAll(15.00)
        .advance()

        .expectWriteCheckpoint()
        .expectAll(15.00)
        .resetMesh()
        .setVertices({2.0, y})
        .writeAll(40.00)
        .advance()

        .expectReadCheckpoint()
        .expectAll(20.00)
        .writeAll(30.00)
        .advance()

        .expectReadCheckpoint()
        .expectAll(25.00)
        .writeAll(15.00)
        .advance()

        .expectCouplingCompleted()
        .expectAll(15.00) // end sample
        .finalize();
  }
  // B - Changing Geometry
  if (context.isNamed("B")) {
    QuickTest(p, "MB"_mesh, "DA"_read, "DB"_write)
        .setVertices({1.0, y, 2.0, y})
        .initialize()
        .expectWriteCheckpoint()
        .expectAll(00.00)
        .writeAll(40.00)
        .advance()

        .expectReadCheckpoint()
        .expectAll(20.00)
        .writeAll(30.00)
        .advance()

        .expectReadCheckpoint()
        .expectAll(25.00)
        .writeAll(15.00)
        .advance()

        .expectWriteCheckpoint()
        .expectAll(15.00) // end of last tw
        .writeAll(40.00)
        .advance()

        .expectReadCheckpoint()
        .expectAll(20.00)
        .writeAll(30.00)
        .advance()

        .expectReadCheckpoint()
        .expectAll(25.00)
        .writeAll(15.00)
        .advance()

        .expectCouplingCompleted()
        .expectAll(15.00) // end sample
        .finalize();
  }
}

inline void runResetBoth(testing::TestContext &context)
{
  constexpr double y = 0.0;

  BOOST_REQUIRE(context.size == 1);
  BOOST_REQUIRE(context.rank == 0);
  Participant p{context.name, context.config(), 0, 1};

  // The data format uses the following format:
  //  0 xpos . (00 | 10 | 11 )

  // A - Adaptive Geometry
  if (context.isNamed("A")) {
    QuickTest(p, "MA"_mesh, "DB"_read, "DA"_write)
        .setVertices({1.0, y, 2.0, y})
        .initialize()
        .expectWriteCheckpoint()
        .expectAll(00.00)
        .writeAll(40.00)
        .advance()

        .expectReadCheckpoint()
        .expectAll(20.00)
        .writeAll(30.00)
        .advance()

        .expectReadCheckpoint()
        .expectAll(25.00)
        .writeAll(15.00)
        .advance()

        .expectWriteCheckpoint()
        .expectAll(15.00) // end of lw
        .resetMesh()
        .setVertices({2.0, y})
        .writeAll(40.00)
        .advance()

        .expectReadCheckpoint()
        .expectAll(20.00) // initial sample is 0
        .writeAll(30.00)
        .advance()

        .expectReadCheckpoint()
        .expectAll(25.00)
        .writeAll(15.00)
        .advance()

        .expectCouplingCompleted()
        .expectAll(15.00) // end sample
        .finalize();
  }
  // B - Changing Geometry
  if (context.isNamed("B")) {
    QuickTest(p, "MB"_mesh, "DA"_read, "DB"_write)
        .setVertices({1.0, y, 2.0, y})
        .initialize()
        .expectWriteCheckpoint()
        .expectAll(00.00)
        .writeAll(40.00)
        .advance()

        .expectReadCheckpoint()
        .expectAll(20.00)
        .writeAll(30.00)
        .advance()

        .expectReadCheckpoint()
        .expectAll(25.00)
        .writeAll(15.00)
        .advance()

        .expectWriteCheckpoint()
        .expectAll(15.00) // end of tw
        .resetMesh()
        .setVertices({2.0, y, 3.0, y})
        .writeAll(40.00)
        .advance()

        .expectReadCheckpoint()
        .expectAll(20.00) // initial sample is 0
        .writeAll(30.00)
        .advance()

        .expectReadCheckpoint()
        .expectAll(25.00)
        .writeAll(15.00)
        .advance()

        .expectCouplingCompleted()
        .expectAll(15.00) // end sample
        .finalize();
  }
}
} // namespace acceleration

} // namespace precice::tests::remesh::parallelImplicit
