#ifndef __MAJORMINER_LMRP_HEURISTIC_HPP_
#define __MAJORMINER_LMRP_HEURISTIC_HPP_

#include <majorminer_types.hpp>
#include <lmrp/lmrp_types.hpp>

namespace majorminer
{

  class LMRPHeuristic
  {
    public:
      LMRPHeuristic(EmbeddingState& state, vertex_t target);
      void optimize();
      const embedding_mapping_t getMapping() const { return m_mapping; }

    private:
      void buildBorder();
      void identifyEdges();
      void identifyEdgesFrom(const nodeset_t& from);
      void identifyComponents();
      void buildSubgraphs(graph_t& borderMapped, graph_t& subgraph);
      void calculatePreviousFitness();
      void identifyDestroyed();
      void solve();

      void connectComponent(ConnectedList& component, fuint32_t componentIdx);
      void embeddDestroyed();
      void embeddSingleDestroyed(vertex_t source);

      void addReachableComponent(vertex_t target, vertex_t source);
      void initializeDijkstraData();
      void resetDijkstra();
      void addSingleVertexNeighbors(vertex_t target, fuint32_t overlaps, fuint32_t length);
      void runDijkstraToTarget(nodeset_t& targets, vertex_t root);
      vertex_t checkConnectedTo(const nodeset_t& wantedTargets, vertex_t target);
      void addEmbeddedPath(vertex_t leaf);
      void connectAdjacentComponents(nodeset_t& adjacent);
      bool checkConnectedToSource(nodeset_t& wantedSources, vertex_t target);
      void addAllMapped(vertex_t source);

      void mapVertex(vertex_t source, vertex_t target);
      void addBorderToMapping();
      fuint32_pair_t getLeastMappedNeighbor(vertex_t source);
      void dijkstraDestroyed(vertex_t source, vertex_t neighbor);
      void mapToFreeVertex();
      void mapToSingleAdjacent(vertex_t neighbor);
      void addAdjacentVertices(vertex_t source, nodeset_t& adjacent);

    public:
      // validation functions
      bool componentsConnected() const;
      bool destroyedConnected() const;
      bool allEdgesEmbedded() const;
      bool allDestroyedEmbedded() const;
      void removeEdges(const nodeset_t& fromSet, graph_t& remaining) const;
      void connectivityDFS(nodeset_t& connected, nodeset_t& mapped,
        Stack<adjacency_list_range_iterator_t>& stack) const;

    private:
      const EmbeddingState& m_state;
      bool m_done;
      fuint32_t m_numberOverlaps;
      fuint32_t m_numberMapped;

      nodeset_t m_crater;
      nodeset_t m_border;
      nodeset_t m_completelyDestroyed;
      graph_t m_edges;
      adjacency_list_t m_sourceAdjacencies;

      Vector<ConnectedList> m_componentsList;
      Vector<vertex_t> m_componentVertices;

      embedding_mapping_t m_mapping;
      embedding_mapping_t m_reverse;
      graph_t m_superVertices;

      PriorityQueue<DijkstraVertex, std::greater<DijkstraVertex>> m_dijkstraQueue;
      UnorderedMap<vertex_t, DijkstraVertex> m_bestPaths;
      vertex_t m_currentSource;
  };


}

#endif
