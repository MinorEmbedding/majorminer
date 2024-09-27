#ifndef __MAJORMINER_TYPES_HPP_
#define __MAJORMINER_TYPES_HPP_

#include "common/config.hpp"

#include <tbb/concurrent_set.h>
#include <tbb/concurrent_map.h>
#include <tbb/concurrent_unordered_set.h>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_priority_queue.h>
#include <tbb/concurrent_lru_cache.h>
#include <tbb/concurrent_queue.h>
#include <tbb/parallel_for_each.h>

#include <stack>
#include <vector>
#include <queue>

#include <cinttypes>
#include <atomic>
#include <memory>
#include <thread>
#include <mutex>
#include <random>
#include <shared_mutex>


namespace majorminer
{
  typedef int_fast32_t fint32_t;
  typedef uint_fast32_t fuint32_t;
  typedef std::pair<fuint32_t, fuint32_t> fuint32_pair_t;
  typedef fuint32_t vertex_t;

  const static fuint32_t FUINT32_UNDEF = (fuint32_t)-1;

  template<typename K, typename V = K>
  struct PairHashFunc
  {
    size_t operator()(const std::pair<K,V>& pair) const
    {
      size_t first = std::hash<K>()(pair.first);
      size_t second = std::hash<V>()(pair.second);
      return first ^ (second << 32) ^ (second >> 32);
    }
  };

  struct PrioNode
  {
    PrioNode() : m_id(-1), m_nbConnections(0) {}
    PrioNode(vertex_t id, fuint32_t nbConnections = 0)
      : m_id(id), m_nbConnections(nbConnections) {}

    friend bool operator<(const PrioNode& n1, const PrioNode& n2)
    { return n1.m_nbConnections < n2.m_nbConnections; }

    vertex_t m_id;
    fuint32_t m_nbConnections;
  };

  struct NodePair
  {
    NodePair() : source(FUINT32_UNDEF), target(FUINT32_UNDEF) {}
    NodePair(vertex_t s, vertex_t t) : source(s), target(t) {}
    NodePair(const fuint32_pair_t& p): source(p.first), target(p.second) {}

    friend bool operator==(const NodePair& p1, const NodePair& p2)
    { return p1.source == p2.source && p1.target == p2.target; }

    vertex_t source;
    vertex_t target;
  };

  template<typename T, typename Allocator = std::allocator<T>>
  using Vector = std::vector<T, Allocator>;

  template<class T, typename HashFunc = std::hash<T>, typename Allocator = std::allocator<T>>
  using UnorderedSet = tbb::concurrent_unordered_set<T, HashFunc, std::equal_to<T>, Allocator>;

  template<typename K, typename V, typename HashFunc = std::hash<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
  using UnorderedMap = tbb::concurrent_unordered_map<K, V, HashFunc, std::equal_to<K>, Allocator>;

  template<typename K, typename V, typename HashFunc = std::hash<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
  using UnorderedMultiMap = tbb::concurrent_unordered_multimap<K, V, HashFunc, std::equal_to<K>, Allocator>;

  template<typename T, typename Comparator = std::less<T>>
  using PriorityQueue = std::priority_queue<T, Vector<T>, Comparator>;

  template<typename T, typename Comparator = std::less<T>>
  using ConcurrentPriorityQueue = tbb::concurrent_priority_queue<T, Comparator>;


  template<typename T>
  using Queue = tbb::concurrent_queue<T>;

  template<typename T>
  using BoundedQueue = tbb::concurrent_bounded_queue<T>;

  template<typename T>
  using Stack = std::stack<T, std::vector<T>>;

  template<typename K, typename V>
  using Cache = tbb::concurrent_lru_cache<K, V>;

  typedef fuint32_pair_t edge_t;

  typedef UnorderedMap<vertex_t, fuint32_t> VertexNumberMap;
  typedef UnorderedSet<edge_t, PairHashFunc<vertex_t>> graph_t;
  typedef UnorderedMultiMap<vertex_t, vertex_t> adjacency_list_t;
  typedef adjacency_list_t embedding_mapping_t;
  typedef UnorderedSet<vertex_t> nodeset_t;
  typedef UnorderedSet<fuint32_pair_t, PairHashFunc<vertex_t, vertex_t>> nodepairset_t;
  typedef nodepairset_t coordinateset_t;
  typedef PriorityQueue<PrioNode, std::less<PrioNode>> PrioNodeQueue;
  typedef std::pair<adjacency_list_t::const_iterator, adjacency_list_t::const_iterator> adjacency_list_range_iterator_t;

  typedef std::pair<vertex_t, std::shared_ptr<fuint32_pair_t[]>> ShiftingCandidates;
  typedef Cache<vertex_t, ShiftingCandidates> CandidateCache;

  struct ChimeraGraphInfo;
  class EmbeddingVisualizer;
  class EmbeddingSuite;
  class EmbeddingBase;
  class EmbeddingState;
  class EmbeddingManager;
  class SuperVertexPlacer;
  class SuperVertexReducer;
  class EvolutionaryCSCReducer;
  class GenericMutation;
  class MutationExtend;
  class MuationFrontierShifting;
  class MutationReduceOverlap;
  class MutationManager;
  class NetworkSimplexWrapper;
  class RandomGen;
  class ThreadManager;
  class LMRPSubgraph;
}


#endif