#ifndef __MAJORMINER_GRAPH_GEN_HPP_
#define __MAJORMINER_GRAPH_GEN_HPP_

#include "majorminer_types.hpp"


namespace majorminer
{
  /// Generate a king's graph with number of rows and columns specified.
  /// Nodes are numbered row by row.
  graph_t generate_king(fuint32_t rows, fuint32_t cols);

  /// Generate a Chimera graph with rows x cols unit cells (each with 8 nodes).
  graph_t generate_chimera(fuint32_t rows, fuint32_t cols);

  /// TODO: Implement generate Pegasus graph
  graph_t generate_pegasus();

  /// Generate a simple cyclic graph C_n
  graph_t generate_cyclegraph(fuint32_t n);

  /// Generate a complete graph on n vertices
  graph_t generate_completegraph(fuint32_t n);

  /// Generate the well-studied Petersen graph
  graph_t generate_petersen();

  /// Import an edge list from a file.
  graph_t import_graph(const std::string& filename);

  /// Import an edge list from a character array.
  graph_t import_graph(const char* edgeList, size_t length);

  // Generate an erdös-renyi graph on n vertices. For each pair of vertices
  // create an edge between the pair with "probability"
  graph_t generate_erdosrenyi(fuint32_t n, float probability);

}


#endif