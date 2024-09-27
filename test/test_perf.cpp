#include <gtest/gtest.h>
#include <majorminer.hpp>
#include <common/embedding_analyzer.hpp>
#include <common/graph_gen.hpp>

#include "utils/test_common.hpp"
#include "utils/qubo_problems.hpp"

using namespace majorminer;

namespace
{
  graph_t getPegasusGraph()
  {
    return majorminer::import_graph("test/data/sample_edgelists/pegasus.txt");
  }

  void runFrequencyTest_Complete(const graph_t& hardware, fuint32_t upTo)
  {
    Vector<fuint32_pair_t> valids{};
    for (fuint32_t n = 4; n < upTo; ++n)
    {
      graph_t clique = generate_completegraph(n);
      fuint32_t nbValid = 0;
      for (fuint32_t i = 0; i < 50;)
      {
        std::cout << "n=" << n << "; iter=" << i << std::endl;
        EmbeddingSuite suite{clique, hardware};
        auto embedding = suite.find_embedding();
        if (suite.connectsNodes())
        {
          i++; // in case of a bug, skip this attempt
          if (suite.isValid()) nbValid++;
        }
      }
      std::cout << n << ":" << nbValid << std::endl;
      valids.push_back(fuint32_pair_t{n, nbValid});
      for (const auto& p : valids) std::cout << p.first << ":" << p.second << std::endl;
    }
  }
}


TEST(PerfTest, DISABLED_CompleteLarge_21)
{
  graph_t clique = generate_completegraph(21);
  graph_t chimera = generate_chimera(11, 11);
  EmbeddingSuite suite{clique, chimera};
  auto embedding = suite.find_embedding();
  ASSERT_TRUE(suite.connectsNodes());
}



TEST(PerfTest, DISABLED_CompleteGraphTest_Frequencies_Chimera)
{
  graph_t chimera = generate_chimera(8, 8);
  runFrequencyTest_Complete(chimera, 30);
}

TEST(PerfTest, DISABLED_CompleteGraphTest_Frequencies_Pegasus)
{
  graph_t pegasus = getPegasusGraph();
  runFrequencyTest_Complete(pegasus, 60);
}

TEST(PerfTest, DISABLED_TSP_Pegasus)
{
  auto pegasus = majorminer::import_graph("test/data/sample_edgelists/pegasus.txt");
  auto tsp = majorminer::quboTSP(8, [](vertex_t, vertex_t){ return 1; });
  fuint32_t nbValid = 0;
  for (fuint32_t iter = 0; iter < 50;)
  {
    std::cout << "Iter " << iter << std::endl;
    EmbeddingSuite suite{tsp, pegasus};
    suite.find_embedding();
    if (suite.connectsNodes())
    {
      iter++;
      if (suite.isValid())
      {
        std::cout << "Valid" << std::endl;
        nbValid++;
      }
    }
  }
  std::cout << "Nb valid: " << nbValid << std::endl;
}

TEST(PerfTest, DISABLED_ErdosRenyi)
{
  graph_t chimera = generate_chimera(16, 16);
  Vector<fuint32_pair_t> valids{};
  for (fuint32_t n = 4; n < 60; ++n)
  {
    fuint32_t nbValid = 0;
    for (fuint32_t i = 0; i < 50;)
    {
      graph_t erdos = generate_erdosrenyi(n, 0.1);
      EmbeddingSuite suite{erdos, chimera};
      auto embedding = suite.find_embedding();
      if (suite.connectsNodes())
      {
        std::cout << "n=" << n << "; iter=" << i << std::endl;
        i++; // in case of a bug, skip this attempt
        if (suite.isValid()) nbValid++;
      }
    }
    std::cout << n << ":" << nbValid << std::endl;
    valids.push_back(fuint32_pair_t{n, nbValid});
    for (const auto& p : valids) std::cout << p.first << ":" << p.second << std::endl;
  }
}
