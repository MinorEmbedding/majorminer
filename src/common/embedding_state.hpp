#ifndef __MAJORMINER_EMBEDDING_STATE_HPP_
#define __MAJORMINER_EMBEDDING_STATE_HPP_

#include <majorminer_types.hpp>
#include <common/embedding_base.hpp>
#include <common/thread_manager.hpp>
#include <lmrp/lmrp_subgraph.hpp>

namespace majorminer
{

  class EmbeddingState : public EmbeddingBase
  {
    friend EmbeddingManager;
    public:
      EmbeddingState(const graph_t& sourceGraph, const graph_t& targetGraph, EmbeddingVisualizer* vis);

      void mapNode(fuint32_t node, fuint32_t targetNode);
      void mapNode(fuint32_t source, const nodeset_t& targets);
      void updateNeededNeighbors(fuint32_t node);
      void updateConnections(fuint32_t node, PrioNodeQueue& nodesToProcess);
      int numberFreeNeighborsNeeded(fuint32_t sourceNode) const;
      fuint32_t getTrivialNode();

      bool removeRemainingNode(fuint32_t node);
      bool isNodeMapped(fuint32_t sourceNode) const { return !m_nodesRemaining.contains(sourceNode); }

      bool isNodeOccupied(fuint32_t node) const { return m_nodesOccupied.contains(node); }

      fuint32_t getSuperVertexSize(fuint32_t sourceNode) const { return m_mapping.count(sourceNode); }

      int getSourceNbFreeNeighbors(fuint32_t sourceNode) const;

      ThreadManager& getThreadManager() { return m_threadManager; }
      void setLMRPSubgraphGenerator(LMRPSubgraph* gen) { m_lmrpGen = gen; }
      fint32_t getReverseMappedCnt(vertex_t target) const;
      UnorderedMap<vertex_t, fint32_t>& getRevMappingCount() { return m_reverseCount; }

    public: // getter
      const graph_t* getSourceGraph() const override { return m_sourceGraph; }
      const graph_t* getTargetGraph() const override { return m_targetGraph; }
      const adjacency_list_t& getSourceAdjGraph() const override { return m_source; }
      const adjacency_list_t& getTargetAdjGraph() const override { return m_target; }
      const embedding_mapping_t& getMapping() const override { return m_mapping; }
      const embedding_mapping_t& getReverseMapping() const override { return m_reverseMapping; }
      const nodeset_t& getNodesOccupied() const override { return m_nodesOccupied; }
      const nodeset_t& getRemainingTargetNodes() const override { return m_targetNodesRemaining; }


      embedding_mapping_t& getMapping() { return m_mapping; }
      embedding_mapping_t& getReverseMapping() { return m_reverseMapping; }
      nodeset_t& getNodesOccupied() { return m_nodesOccupied; }
      nodeset_t& getRemainingTargetNodes() { return m_targetNodesRemaining; }
      const UnorderedMap<fuint32_t, fuint32_t>& getRemainingNodes() const { return m_nodesRemaining; }
      UnorderedMap<fuint32_t, fuint32_t>& getRemainingNodes() { return m_nodesRemaining; }

      EmbeddingVisualizer* getVisualizer() { return m_visualizer; }
      bool hasVisualizer() const { return m_visualizer != nullptr; }

      UnorderedMap<fuint32_t, std::atomic<int>>& getSourceFreeNeighbors() { return m_sourceFreeNeighbors; }
      UnorderedMap<fuint32_t, std::atomic<int>>& getSourceNeededNeighbors() { return m_sourceNeededNeighbors; }

      fuint32_t getNumberSourceVertices() const { return m_numberSourceVertices; }

      LMRPSubgraph* getSubgraphGen() { return m_lmrpGen; }

    private:
      void initialize();
      void unmapNode(vertex_t sourceVertex);

    private:
      const graph_t* m_sourceGraph;
      const graph_t* m_targetGraph;
      adjacency_list_t m_source;
      adjacency_list_t m_target;

      embedding_mapping_t m_mapping;
      embedding_mapping_t m_reverseMapping;
      nodeset_t m_nodesOccupied;
      nodeset_t m_targetNodesRemaining;

      UnorderedMap<fuint32_t, fuint32_t> m_nodesRemaining;
      UnorderedMap<fuint32_t, std::atomic<int>> m_sourceNeededNeighbors;
      UnorderedMap<fuint32_t, std::atomic<int>> m_sourceFreeNeighbors;
      nodeset_t m_sourceNodesAffected;
      fuint32_t m_numberSourceVertices;

      EmbeddingVisualizer* m_visualizer;

      LMRPSubgraph* m_lmrpGen;

      ThreadManager m_threadManager;

      UnorderedMap<vertex_t, fint32_t> m_reverseCount;
  };

}



#endif
