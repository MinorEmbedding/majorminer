#include "embedding_state.hpp"

#include "common/utils.hpp"

using namespace majorminer;

EmbeddingState::EmbeddingState(const graph_t& sourceGraph, const graph_t& targetGraph, EmbeddingVisualizer* vis)
  : m_sourceGraph(&sourceGraph), m_targetGraph(&targetGraph), m_visualizer(vis)
{
  initialize();
}

void EmbeddingState::initialize()
{
  convertToAdjacencyList(m_source, *m_sourceGraph);
  convertToAdjacencyList(m_target, *m_targetGraph);
  for (const auto& arc : *m_targetGraph)
  {
    m_targetNodesRemaining.insert(arc.first);
    m_targetNodesRemaining.insert(arc.second);
  }
  for (const auto& arc : *m_sourceGraph)
  {
    m_nodesRemaining[arc.first] = 0;
    m_nodesRemaining[arc.second] = 0;
    m_sourceNeededNeighbors[arc.first]++;
    m_sourceNeededNeighbors[arc.second]++;
  }
  m_numberSourceVertices = m_nodesRemaining.size();
}

fint32_t EmbeddingState::getReverseMappedCnt(vertex_t target) const
{
  auto findIt = m_reverseCount.find(target);
  return findIt != m_reverseCount.end() ? findIt->second : 0;
}

fuint32_t EmbeddingState::getTrivialNode()
{ // TODO: assert
  auto node = *m_nodesRemaining.begin();
  m_nodesRemaining.unsafe_erase(m_nodesRemaining.begin());
  return node.first;
}

bool EmbeddingState::removeRemainingNode(vertex_t node)
{
  if (!m_nodesRemaining.contains(node)) return false;
  m_nodesRemaining.unsafe_erase(node);
  return true;
}

void EmbeddingState::unmapNode(vertex_t sourceVertex)
{
  auto range = m_mapping.equal_range(sourceVertex);
  for (auto mappedIt = range.first; mappedIt != range.second; ++mappedIt)
  {
    m_reverseCount[mappedIt->second]--;
    eraseSinglePair(m_reverseMapping, mappedIt->second, mappedIt->first);
  }
  m_mapping.unsafe_erase(sourceVertex);
}

void EmbeddingState::updateNeededNeighbors(vertex_t node)
{
  fuint32_t nbNodes = 0;
  iterateSourceGraphAdjacent(node, [&, this](vertex_t adjacentSource){
    if (isNodeMapped(adjacentSource))
    {
      nbNodes++;
      m_sourceNeededNeighbors[adjacentSource]--;
    }
  });
  m_sourceNeededNeighbors[node] -= nbNodes;
}


void EmbeddingState::updateConnections(vertex_t node, PrioNodeQueue& nodesToProcess)
{
  iterateSourceGraphAdjacent(node, [&](vertex_t adjacent){
    auto findIt = m_nodesRemaining.find(adjacent);
    if (findIt != m_nodesRemaining.end())
    {
      findIt->second += 1; // one of its neighbors is now embedded
      nodesToProcess.push(PrioNode{findIt->first, findIt->second});
    }
  });
}

int EmbeddingState::numberFreeNeighborsNeeded(vertex_t sourceNode) const
{ // TODO: rework
  // std::cout << "Source node " << sourceNode << " needs " << m_sourceNeededNeighbors[sourceNode].load() << " neighbors and has " << m_sourceFreeNeighbors[sourceNode].load() << std::endl;
  auto it = m_sourceNeededNeighbors.find(sourceNode);
  return 2 * (it == m_sourceNeededNeighbors.end() ? 0 : it->second.load())
    - std::max(getSourceNbFreeNeighbors(sourceNode), 0);
}

int EmbeddingState::getSourceNbFreeNeighbors(vertex_t sourceNode) const
{
  auto it = m_sourceFreeNeighbors.find(sourceNode);
  return it == m_sourceFreeNeighbors.end() ? 0 : it->second.load();
}


void EmbeddingState::mapNode(vertex_t source, vertex_t targetNode)
{
  m_reverseCount[targetNode]++;
  m_nodesOccupied.insert(targetNode);
  m_mapping.insert(std::make_pair(source, targetNode));
  m_reverseMapping.insert(std::make_pair(targetNode, source));
  m_targetNodesRemaining.unsafe_extract(targetNode);
  removeRemainingNode(source);
}

void EmbeddingState::mapNode(vertex_t source, const nodeset_t& targets)
{
  for (auto targetNode : targets)
  {
    m_reverseCount[targetNode]++;
    m_nodesOccupied.insert(targetNode);
    m_mapping.insert(std::make_pair(source, targetNode));
    m_reverseMapping.insert(std::make_pair(targetNode, source));
    m_targetNodesRemaining.unsafe_extract(targetNode);
  }
  removeRemainingNode(source);
}

