#include <gtest/gtest.h>
#include <majorminer.hpp>
#include <common/embedding_visualizer.hpp>
#include <common/embedding_analyzer.hpp>
#include <common/graph_gen.hpp>
#include <common/debug_utils.hpp>

#include "utils/test_common.hpp"
#include "utils/qubo_problems.hpp"

using namespace majorminer;

namespace
{
  coordinate_map_t getPetersenCoordinates()
  {
    coordinate_map_t coords{
      std::make_pair(0, Coordinate_t( 7.0, 0.0 )),
      std::make_pair(1, Coordinate_t( 13.657, 4.837 )),
      std::make_pair(2, Coordinate_t( 11.114, 12.663 )),
      std::make_pair(3, Coordinate_t( 2.8859999999999992, 12.663 )),
      std::make_pair(4, Coordinate_t( 0.34299999999999997, 4.837 )),
      std::make_pair(5, Coordinate_t( 7.0, 3.0 )),
      std::make_pair(6, Coordinate_t( 10.804, 5.763999999999999 )),
      std::make_pair(7, Coordinate_t( 9.350999999999999, 10.236 )),
      std::make_pair(8, Coordinate_t( 4.648999999999999, 10.236 )),
      std::make_pair(9, Coordinate_t( 3.1959999999999997, 5.763999999999999 ))
    };
    return coords;
  }

  void clique_test(fuint32_t n, fuint32_t x, fuint32_t y, std::string filename,
    bool fullValidation = false, bool visualize = true, bool printStats = false)
  {
    graph_t clique = generate_completegraph(n);
    graph_t chimera = generate_chimera(x, y);
    std::cout << filename << std::endl;
    std::unique_ptr<ChimeraVisualizer> visualizer;
    if (visualize) visualizer = std::make_unique<ChimeraVisualizer>(clique, chimera, filename, x, y);
    EmbeddingSuite suite{clique, chimera, visualize ? visualizer.get() : nullptr};
    auto embedding = suite.find_embedding();

    if (printStats) printEmbeddingOverlapStats(embedding);

    if (fullValidation) ASSERT_TRUE(suite.isValid());
    else ASSERT_TRUE(suite.connectsNodes());
  }
}

TEST(EmbeddingTest, Basic_Cycle_4)
{
  graph_t cycle = generate_cyclegraph(4);
  graph_t chimera = generate_chimera(1, 1);
  auto visualizer = std::make_unique<ChimeraVisualizer>(cycle, chimera, "imgs/Basic_Cycle_4/chimera_cycle_4", 1, 1);
  EmbeddingSuite suite{cycle, chimera, visualizer.get()};
  auto embedding = suite.find_embedding();
  ASSERT_TRUE(suite.isValid());
}


TEST(EmbeddingTest, Nikolaus)
{
  graph_t nikolaus = generate_completegraph(4);
  nikolaus.insert(edge_t{2,4});
  nikolaus.insert(edge_t{3,4});
  graph_t chimera = generate_chimera(2,2);
  auto visualizer = std::make_unique<ChimeraVisualizer>(nikolaus, chimera, "imgs/Nikolaus/Nikolaus", 2, 2);
  EmbeddingSuite suite{nikolaus, chimera, visualizer.get()};
  auto embedding = suite.find_embedding();
  ASSERT_TRUE(suite.isValid());
}

TEST(EmbeddingTest, Cycle_5_Extra_Edges)
{
  graph_t cycle = generate_cyclegraph(5);
  graph_t chimera = generate_chimera(1, 1);
  addEdges(cycle, { {0,2}, {2, 4}, {1,3}});
  auto visualizer = std::make_unique<ChimeraVisualizer>(cycle, chimera, "imgs/Cycle_5_Extra_Edges/chimera_cycle_5_ExtraEdges", 1, 1);
  EmbeddingSuite suite{cycle, chimera, visualizer.get()};
  auto embedding = suite.find_embedding();
  ASSERT_TRUE(suite.connectsNodes());
}

TEST(EmbeddingTest, Cycle_5_Extra_Edges_On_2_2_Chimera)
{
  graph_t cycle = generate_cyclegraph(5);
  graph_t chimera = generate_chimera(2, 2);
  addEdges(cycle, { {0,2}, {2, 4}, {1,3}});
  auto visualizer = std::make_unique<ChimeraVisualizer>(cycle, chimera, "imgs/Cycle_5_Extra_Edges_On_2_2_Chimera/chimera_cycle_5_ExtraEdges_Larger", 2, 2);
  EmbeddingSuite suite{cycle, chimera, visualizer.get()};
  auto embedding = suite.find_embedding();
  ASSERT_TRUE(suite.isValid());
}

TEST(EmbeddingTest, Complete_Graph_8_On_3_3_Chimera)
{
  clique_test(8, 3, 3, "imgs/Complete_Graph_8_On_3_3_Chimera/chimera_clique_8", false);
}


TEST(EmbeddingTest, Complete_Graph_12_On_5_5_Chimera)
{
  clique_test(12, 5, 5, "imgs/Complete_Graph_12_On_5_5_Chimera/chimera_clique_12", false);
}


TEST(EmbeddingTest, Complete_Graph_15_On_7_7_Chimera)
{
  clique_test(15, 7, 7, "imgs/Complete_Graph_15_On_7_7_Chimera/chimera_clique_15", false);
}


TEST(EmbeddingTest, Complete_Graph_18_On_9_9_Chimera)
{
  clique_test(18, 9, 9, "imgs/Complete_Graph_18_On_9_9_Chimera/chimera_clique_18", false);
}

TEST(EmbeddingTest, Complete_Graph_21_On_11_11_Chimera)
{
  clique_test(21, 11, 11, "imgs/Complete_Graph_21_On_11_11_Chimera/chimera_clique_21", false);
}

TEST(EmbeddingTest, Complete_Graph_25_On_16_16_Chimera)
{
  clique_test(25, 16, 16, "imgs/Complete_Graph_25_On_16_16_Chimera/chimera_clique_25", false);
}


TEST(EmbeddingTest, Complete_Graph_28_On_16_16_Chimera)
{
  clique_test(28, 16, 16, "imgs/Complete_Graph_28_On_16_16_Chimera/chimera_clique_28", false);
}


TEST(EmbeddingTest, Complete_Graph_31_On_16_16_Chimera)
{
  clique_test(31, 16, 16, "imgs/Complete_Graph_31_On_16_16_Chimera/chimera_clique_31", false, false);
}

TEST(EmbeddingTest, Complete_Graph_33_On_16_16_Chimera)
{
  clique_test(33, 16, 16, "imgs/Complete_Graph_33_On_16_16_Chimera/chimera_clique_33", false, false, false);
}

TEST(EmbeddingTest, Complete_Graph_34_On_16_16_Chimera)
{
  clique_test(34, 16, 16, "imgs/Complete_Graph_34_On_16_16_Chimera/chimera_clique_34", false, false);
}


TEST(EmbeddingTest, Complete_Graph_25_On_8_8_Chimera)
{
  clique_test(25, 8, 8, "imgs/Complete_Graph_25_On_8_8_Chimera/chimera_clique_25", false, false);
}

TEST(EmbeddingTest, Basic_Cycle_8_Visualization)
{
  graph_t cycle = generate_cyclegraph(8);
  graph_t chimera = generate_chimera(2, 2);
  auto visualizer = std::make_unique<ChimeraVisualizer>(cycle, chimera, "imgs/Basic_Cycle_8_Visualization/chimera_cycle_8", 2, 2);
  EmbeddingSuite suite{cycle, chimera, visualizer.get()};
  auto embedding = suite.find_embedding();
  ASSERT_TRUE(suite.isValid());
}

TEST(EmbeddingTest, Petersen_Chimera)
{
  graph_t petersen = generate_petersen();
  graph_t chimera = generate_chimera(2, 2);
  auto visualizer = std::make_unique<ChimeraVisualizer>(petersen, chimera, "imgs/Petersen_Chimera/chimera_petersen", 2, 2);
  EmbeddingSuite suite{petersen, chimera, visualizer.get()};
  auto embedding = suite.find_embedding();
  ASSERT_TRUE(suite.connectsNodes());
}


TEST(EmbeddingTest, Petersen_KingsGraph)
{
  graph_t petersen = generate_petersen();
  graph_t king = generate_king(10, 10);
  auto visualizer = std::make_unique<KingsVisualizer>(petersen, king, "imgs/Petersen_KingsGraph/king_petersen", 10, 10);
  EmbeddingSuite suite{petersen, king, visualizer.get()};
  auto embedding = suite.find_embedding();
  ASSERT_TRUE(suite.connectsNodes());
}

TEST(EmbeddingTest, DISABLED_TSP_7)
{
  graph_t tsp = majorminer::quboTSP(7, [](fuint32_t, fuint32_t){ return 1;});
  printGraph(tsp);
  graph_t chimera = generate_chimera(16, 16);
  auto visualizer = std::make_unique<ChimeraVisualizer>(tsp, chimera, "imgs/TSP_7/tsp_7_chimera_2000x", 16, 16);
  EmbeddingSuite suite{tsp, chimera, visualizer.get()};
  auto embedding = suite.find_embedding();
  EmbeddingAnalyzer analyzer{embedding};
  std::cout << analyzer.getNbOverlaps() << " nodes overlap and "
            << analyzer.getNbUsedNodes()
            << " nodes were needed." << std::endl;
}

TEST(EmbeddingTest, K33_On_Petersen_Kuratowski)
{
  graph_t petersen = majorminer::generate_petersen();
  graph_t k33{};
  addEdges(k33, {
    {0,3}, {0,4}, {0,5},
    {1,3}, {1,4}, {1,5},
    {2,3}, {2,4}, {2,5}
  });
  auto coords = getPetersenCoordinates();
  auto visualizer = std::make_unique<GenericVisualizer>(k33, petersen, "imgs/PetersenNotPlanar/petersen_kuratowski", coords, 14, 14);
  EmbeddingSuite suite{k33, petersen, visualizer.get()};
  auto embedding = suite.find_embedding();
  ASSERT_TRUE(suite.connectsNodes());
}


TEST(EmbeddingTest, ErdosRenyi_Chimera_7_7)
{
  graph_t erdos = generate_erdosrenyi(35, 0.1);
  graph_t chimera = generate_chimera(7, 7);
  auto visualizer = std::make_unique<ChimeraVisualizer>(erdos, chimera, "imgs/ErdosRenyi_Chimera_7_7/ErdosRenyi_Chimera_7_7", 7, 7);
  EmbeddingSuite suite { erdos, chimera, visualizer.get() };
  suite.find_embedding();
  ASSERT_TRUE(suite.connectsNodes());
}

TEST(EmbeddingTest, DISABLED_RunMultipleTimes)
{
  for (int i = 0; i < 100; ++i)
  {
    clique_test(33, 16, 16, "imgs/Complete_Graph_33_On_16_16_Chimera/chimera_clique_33", false, false, false);
  }
}