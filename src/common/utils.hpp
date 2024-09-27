#ifndef __MAJORMINER_UTILS_HPP_
#define __MAJORMINER_UTILS_HPP_

#include <majorminer_types.hpp>
#include <common/debug_utils.hpp>
#include <type_traits>


namespace majorminer
{

  void convertToAdjacencyList(adjacency_list_t& adj, const graph_t& graph);

  template<typename K, typename V, typename Comparator = std::less<K>>
  std::pair<K,V> orderedPair(const K& e1, const V& e2)
  {
    static_assert(std::is_same_v<typename std::add_const<K>::type, typename std::add_const<V>::type>,
      "Ordered pair parameters must at most differ by their const classifier!");
    if (Comparator()(e1, e2)) return std::make_pair(e1, e2);
    else return std::make_pair(e2, e1);
  }

  template<typename K, typename V, typename Comparator = std::less<K>>
  std::pair<K,V> orderedPair(const std::pair<K, V>& p)
  {
    return orderedPair(p.first, p.second);
  }

  template<typename T>
  void setMin(T& val, const T& v) { if (v < val) val = v; }
  template<typename T>
  void setMax(T& val, const T& v) { if (v > val) val = v; }

  template<typename K, typename V>
  void eraseSinglePair(UnorderedMultiMap<K, V>& umap, const K& key, const V& val)
  {
    auto range = umap.equal_range(key);
    for (auto it = range.first; it != range.second; ++it)
    {
      if (it->second == val)
      {
        umap.unsafe_erase(it);
        return;
      }
    }
  }

  template<typename K, typename V>
  bool containsPair(const UnorderedMultiMap<K, V>& umap, const K& key, const V& val)
  {
    auto range = umap.equal_range(key);
    for (auto it = range.first; it != range.second; ++it)
    {
      if (it->second == val)
      {
        return true;
      }
    }
    return false;
  }

  void insertMappedTargetNodes(const EmbeddingBase& base, nodeset_t& nodes, fuint32_t sourceNode);

  inline bool isDefined(fuint32_t value) { return value != FUINT32_UNDEF; }
  inline bool isDefined(fuint32_pair_t& p) { return isDefined(p.first) && isDefined(p.second); }
  inline bool isDefined(NodePair& p) { return isDefined(p.source) && isDefined(p.target); }

  template<typename T>
  inline std::shared_ptr<T[]> make_shared_array(std::size_t size)
  {
      return std::shared_ptr<T[]>( new T[size], []( T *p ){ delete [] p; } );
  }

  template<typename T, typename Hash>
  inline bool overlappingSets(const UnorderedSet<T, Hash>& map1, const UnorderedSet<T, Hash>& map2)
  {
    for (const auto& key : map1)
    {
      if (map2.contains(key)) return true;
    }
    return false;
  }

  template<typename T>
  inline void clearStack(Stack<T>& s)
  {
    while(!s.empty()) s.pop();
  }

  nodeset_t getVertices(const graph_t& graph);

  fuint32_pair_t calculateOverlappingStats(const EmbeddingBase& base);

  // inefficient - just for visualization
  embedding_mapping_t replaceMapping(const embedding_mapping_t& mapping,
    const nodeset_t& targets, vertex_t source);

  template<typename K, typename V>
  inline std::pair<V, K> reversePair(const std::pair<K,V>& p)
  {
    return std::make_pair(p.second, p.first);
  }

  template<typename K, typename V, bool reverse = false>
  struct PairFirstKeySorter
  {
    bool operator()(const std::pair<K,V>& p1, const std::pair<K,V>& p2)
    {
      if(reverse) return p1.first < p2.first;
      else return p1.first > p2.first;
    }
  };

  // Calculate the fitness of an *unmapped* super vertex! Calculate number of overlappings
  fuint32_t calculateFitness(const EmbeddingBase& state, const nodeset_t& superVertex);

  bool containsEdge(const graph_t& graph, edge_t edge);

  template<typename A, typename B>
  inline bool empty_range(const std::pair<A,B>& range)
  {
    return range.first == range.second;
  }
}



#endif