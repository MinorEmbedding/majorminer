#include <iostream>
#include <majorminer_lib.hpp>


using namespace majorminer;

int main()
{
  // Generate some source graph that should be embedded
  graph_t H = majorminer::generate_completegraph(8);

  // Generate the hardware graph
  graph_t G = majorminer::generate_chimera(3, 3);

  // For debugging purposes: Get us a visualizer
  ChimeraVisualizer vis{H, G, "imgs/example_majorminer/simple_example", 3, 3};

  // Embedd the problem
  EmbeddingSuite suite{H, G, &vis};
  embedding_mapping_t mapping = suite.find_embedding();

  // We can validate the embedding
  EmbeddingAnalyzer analyzer{mapping};
  std::cout << "Result has " << analyzer.getNbOverlaps() << " overlaps and uses "
    << analyzer.getNbUsedNodes() << " vertices." << std::endl;

  return 0;
}

