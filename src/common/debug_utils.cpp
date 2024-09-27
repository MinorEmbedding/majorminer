#include <common/debug_utils.hpp>
#include <common/utils.hpp>

#include <algorithm>

using namespace majorminer;


void majorminer::printNodeset(const nodeset_t& nodeset)
{
  Vector<fuint32_t> nodes{};
  nodes.reserve(nodeset.size());
  nodes.assign(nodeset.begin(), nodeset.end());
  std::sort(nodes.begin(), nodes.end());

  std::cout << "{";
  for (auto node : nodes) std::cout << " " << node;
  std::cout << " }" << std::endl;
}


void majorminer::printAdjacencyList(const adjacency_list_t& adj)
{
  for (const auto& edge : adj)
  {
    std::cout << "(" << edge.first << ", " << edge.second << ")" << std::endl;
  }
}

void majorminer::printVertexNumberMap(const VertexNumberMap& m)
{
  for (const auto& p : m)
  {
    std::cout << "(" << p.first << ", " << p.second << ") ";
  }
  std::cout << std::endl;
}

embedding_mapping_t majorminer::getReverseMapping(const embedding_mapping_t& mapping)
{
  embedding_mapping_t reverse{};
  for (const auto& mapped : mapping) reverse.insert(reversePair(mapped));
  return reverse;
}

void majorminer::printEmbeddingOverlapStats(const embedding_mapping_t& mapping)
{
  VertexNumberMap overlapStat{};
  embedding_mapping_t reverse = getReverseMapping(mapping);
  for (const auto& rev : reverse)
  {
    if(reverse.count(rev.first) > 1) overlapStat[rev.second]++;
  }
  Vector<fuint32_pair_t> stats{};
  stats.reserve(overlapStat.size());
  for (const auto& p : overlapStat) stats.push_back(reversePair(p));
  std::sort(stats.begin(), stats.end(), PairFirstKeySorter<fuint32_t, fuint32_t, true>());
  for (const auto& stat : stats) std::cout << "Source vertex " << stat.second << " is mapped onto " << stat.first << " vertices." << std::endl;
}

void majorminer::printGraph(const graph_t& graph)
{
  for (const auto& p : graph)
  {
    std::cout << "(" << p.first << ", " << p.second << ")" << std::endl;
  }
}

