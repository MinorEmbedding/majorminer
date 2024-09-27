#include <gtest/gtest.h>
#include <majorminer.hpp>
#include <common/embedding_visualizer.hpp>
#include <common/embedding_analyzer.hpp>
#include <common/graph_gen.hpp>
#include <common/debug_utils.hpp>
#include <lmrp/lmrp_chimera_subgraph.hpp>
#include <lmrp/lmrp_king_subgraph.hpp>
#include <lmrp/lmrp_heuristic.hpp>

#include "utils/test_common.hpp"
#include "utils/state_gen.hpp"

using namespace majorminer;

namespace
{
  void validateLMRPResults(const LMRPHeuristic& lmrp)
  {
    ASSERT_TRUE(lmrp.componentsConnected());
    ASSERT_TRUE(lmrp.destroyedConnected());
    ASSERT_TRUE(lmrp.allEdgesEmbedded());
    ASSERT_TRUE(lmrp.allDestroyedEmbedded());
  }

  void visualizeKing(const KingGraphInfo& info, const embedding_mapping_t& original,
    const embedding_mapping_t& after, const graph_t& king,
    const graph_t& source, const std::string& filename)
  {
    KingsVisualizer vis{source, king, filename, info.getHeight(), info.getWidth()};
    vis.draw(original, "Before LMRP heuristic");
    vis.draw(after, "After LMRP heuristic");
  }
}


TEST(LMRPTest, SimpleChimera)
{
  auto chimera = majorminer::generate_chimera(3, 3);
  ChimeraGraphInfo info{3,3};
  ChimeraLMRPSubgraph subgraph{info};
  graph_t trivial{ { 0, 1 }, {0, 2} };
  StateGen gen{trivial, chimera};
  gen.addMapping(0, { 3, 7, 12, 13, 14, 15, 8, 32, 56, 62, 70});
  gen.addMapping(1, {37, 45});
  gen.addMapping(2, {36});
  auto state = gen.get();
  state->setLMRPSubgraphGenerator(&subgraph);
  LMRPHeuristic lmrp{*state, 32};
  lmrp.optimize();
  auto repaired = lmrp.getMapping();
  for (auto mapped : repaired)
  {
    std::cout << mapped.first << " --> " << mapped.second << std::endl;
  }
}


TEST(LMRPTest, SimpleKings)
{
  auto king = majorminer::generate_king(7, 7);
  KingGraphInfo info{7,7};
  KingLMRPSubgraph subgraph{info};
  graph_t trivial{{ 0, 1 }, {0, 2}, {1, 2}};
  StateGen gen{trivial, king};
  gen.addMapping(0, { 21,22,23,31,25,26,27});
  gen.addMapping(1, { 3, 10, 17, 24, 30, 38, 45});
  gen.addMapping(2, {0, 8, 16});
  auto state = gen.get();
  state->setLMRPSubgraphGenerator(&subgraph);
  LMRPHeuristic lmrp{*state, 24};
  lmrp.optimize();
  auto repaired = lmrp.getMapping();
  for (auto mapped : repaired)
  {
    std::cout << mapped.first << " --> " << mapped.second << std::endl;
  }
  visualizeKing(info, state->getMapping(), repaired, king, trivial, "imgs/LMRP_SimpleKings/SimpleKings");
  validateLMRPResults(lmrp);
}

TEST(LMRPTest, TrivialKings_SingleConnection)
{
  auto king = majorminer::generate_king(7, 7);
  KingGraphInfo info{7,7};
  KingLMRPSubgraph subgraph{info};
  graph_t trivial{ };
  StateGen gen{trivial, king};
  gen.addMapping(0, {21,22,23,31,25,26,27});
  auto state = gen.get();
  state->setLMRPSubgraphGenerator(&subgraph);
  LMRPHeuristic lmrp{*state, 24};
  lmrp.optimize();
  auto repaired = lmrp.getMapping();
  visualizeKing(info, state->getMapping(), repaired, king, trivial, "imgs/LMRP_TrivialKings_SingleConnection/TrivialKings_SingleConnection");
}

TEST(LMRPTest, TrivialKings_MultipleErdosRenyi)
{
  auto king = majorminer::generate_king(7, 7);
  KingGraphInfo info{7,7};
  KingLMRPSubgraph subgraph{info};
  graph_t random = majorminer::generate_erdosrenyi(10, 0.2);

  EmbeddingSuite suite{random, king};
  auto original = suite.find_embedding();
  StateGen gen{random, king};
  gen.addMapping(original);
  auto state = gen.get();
  state->setLMRPSubgraphGenerator(&subgraph);

  LMRPHeuristic lmrp{*state, 24};
  lmrp.optimize();
  visualizeKing(info, state->getMapping(), lmrp.getMapping(), king, random, "imgs/TrivialKings_MultipleErdosRenyi/TrivialKings_MultipleErdosRenyi");
  validateLMRPResults(lmrp);
}


TEST(LMRPTest, KingComplete6)
{
  auto king = majorminer::generate_king(7, 7);
  KingGraphInfo info{7,7};
  KingLMRPSubgraph subgraph{info};
  graph_t complete = majorminer::generate_completegraph(6);

  EmbeddingSuite suite{complete, king};
  auto original = suite.find_embedding();
  StateGen gen{complete, king};
  gen.addMapping(original);
  auto state = gen.get();
  state->setLMRPSubgraphGenerator(&subgraph);

  LMRPHeuristic lmrp{*state, 24};
  lmrp.optimize();
  visualizeKing(info, state->getMapping(), lmrp.getMapping(), king, complete, "imgs/LMRP_complete_6_king/LMRP_complete_6_king");
  // validateLMRPResults(lmrp);
}