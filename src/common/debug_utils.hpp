#ifndef __MAJORMINER_DEBUG_UTILS_HPP_
#define __MAJORMINER_DEBUG_UTILS_HPP_

#include <majorminer_types.hpp>

namespace majorminer
{

  #define TEST_OUTPUT(content) std::cout << #content << std::endl;

  void printAdjacencyList(const adjacency_list_t& adj);

  void printNodeset(const nodeset_t& nodeset);

  void printVertexNumberMap(const VertexNumberMap& m);

  embedding_mapping_t getReverseMapping(const embedding_mapping_t& mapping);

  void printEmbeddingOverlapStats(const embedding_mapping_t& mapping);

  void printGraph(const graph_t& graph);
}

#endif