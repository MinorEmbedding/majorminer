#include "super_vertex_reducer.hpp"

#include <common/cut_vertex.hpp>
#include <common/embedding_base.hpp>
#include <common/debug_utils.hpp>
#include <common/random_gen.hpp>
#include <common/csc_problem.hpp>

using namespace majorminer;

SuperVertexReducer::SuperVertexReducer(const EmbeddingBase& base, vertex_t sourceVertex)
  : m_embedding(base), m_sourceVertex(sourceVertex), m_done(false)
{ }

void SuperVertexReducer::setup()
{
  // prepare m_potentialNodes
  m_potentialNodes.insert(m_superVertex.begin(), m_superVertex.end());
  const auto& remaining = m_embedding.getRemainingTargetNodes();
  for (auto mapped : m_superVertex)
  {
    m_embedding.iterateTargetGraphAdjacent(mapped, [&](vertex_t targetAdj){
      if (remaining.contains(targetAdj)) m_potentialNodes.insert(targetAdj);
    });
  }

  if (!acceptOnlyReduction && (m_potentialNodes.size() == m_superVertex.size() || m_potentialNodes.size() <= 1))
  {
    m_done = true;
    return;
  }

  // prepare source vertices in m_sourceConnections
  const auto& mapping = m_embedding.getMapping();
  m_embedding.iterateSourceGraphAdjacent(m_sourceVertex,
    [&](vertex_t adjacent){
      if (mapping.contains(adjacent)) m_sourceConnections[adjacent] = 0;
  });

  // prepare m_adjacencies
  m_verticesList = std::make_unique<vertex_t[]>(m_potentialNodes.size());
  nodepairset_t adjacentSource;
  fuint32_t idx = 0;
  for (auto node : m_potentialNodes)
  {
    m_verticesList[idx++] = node;
    m_embedding.iterateTargetAdjacentReverseMapping(node, [&](vertex_t adjSource){
      if (m_sourceConnections.contains(adjSource)) adjacentSource.insert(std::make_pair(node, adjSource));
    });
  }
  for (auto& adj : adjacentSource) m_adjacencies.insert(adj);

  // prepare m_sourceConnections
  for (const auto& adj : m_adjacencies)
  {
    if (m_superVertex.contains(adj.first)) m_sourceConnections[adj.second]++;
  }
}

void SuperVertexReducer::initialize(const nodeset_t& currentMapping)
{
  const auto& mapping = m_embedding.getMapping();
  if (mapping.find(m_sourceVertex) != mapping.end())
  {
    throw std::runtime_error("Not a temporary mapping. Vertex was already mapped!");
  }
  m_superVertex.insert(currentMapping.begin(), currentMapping.end());
  setup();
}

void SuperVertexReducer::initialize()
{
  clear();
  const auto& mapping = m_embedding.getMapping();
  auto range = mapping.equal_range(m_sourceVertex);
  for (auto it = range.first; it != range.second; ++it)
  {
    m_superVertex.insert(it->second);
  }
  m_initialSuperVertex.insert(m_superVertex.begin(), m_superVertex.end());
  acceptOnlyReduction = true;
  setup();
}

void SuperVertexReducer::clear()
{
  m_initialSuperVertex.clear();
  m_superVertex.clear();
  m_potentialNodes.clear();
  m_adjacencies.clear();
  m_sourceConnections.clear();
  m_done = false;
}

void SuperVertexReducer::optimize()
{
  if (m_done) return;
  RandomGen rand{};
  m_rand = &rand;
  fuint32_t maxIters = 8 * m_potentialNodes.size();
  fuint32_t halfMax = maxIters / 2;
  fuint32_t iteration = 0;
  for (; iteration <  halfMax; ++iteration)
  {
    fuint32_t flipIdx = rand.getRandomUint(m_potentialNodes.size() - 1);
    vertex_t target = m_verticesList[flipIdx];
    if (m_superVertex.contains(target)) removeNode(target);
    else addNode(target);
  }

  for (auto& node : m_potentialNodes)
  {
    if (m_superVertex.contains(node) && isBadNode(node))
    {
      removeNode(node);
    }
  }

  for (; iteration <  maxIters && m_superVertex.size() > 1; ++iteration)
  {
    fuint32_t flipIdx = rand.getRandomUint(m_potentialNodes.size() - 1);
    vertex_t target = m_verticesList[flipIdx];
    if (m_superVertex.contains(target)) removeNode(target);
  }

  for (fuint32_t idx = 0; idx < m_potentialNodes.size(); ++idx)
  {
    if (m_superVertex.contains(m_verticesList[idx])) removeNode(m_verticesList[idx]);
  }

  m_done = true;
}

bool SuperVertexReducer::isBadNode(vertex_t target) const
{
  bool initial = m_initialSuperVertex.contains(target);
  size_t count = m_embedding.getReverseMapping().count(target);
  return count >= (initial ? 2 : 1);
}

fint32_t SuperVertexReducer::getVertexFitness(vertex_t target) const
{
  bool initial = m_initialSuperVertex.contains(target);
  size_t count = m_embedding.getReverseMapping().count(target);
  return count > (initial ? 1 : 0) ? NONLINEAR((initial ? (count-1) : (count))) : 0;
}

void SuperVertexReducer::addNode(vertex_t target)
{
  if (m_superVertex.contains(target) || isBadNode(target) || !isConnected(target)) return;

  m_superVertex.insert(target);
  auto range = m_adjacencies.equal_range(target);
  for (auto it = range.first; it != range.second; ++it)
  {
    m_sourceConnections[it->second]++;
  }
}

void SuperVertexReducer::removeNode(vertex_t target)
{
  if (!m_superVertex.contains(target)) return;
  // 1. Check whether for each adjacent source vertex, there is another adjacent target node
  auto range = m_adjacencies.equal_range(target);

  for (auto it = range.first; it != range.second; ++it)
  {
    if (m_sourceConnections[it->second] <= 1) return;
  }

  // 2. Check whether cut vertex
  nodeset_t temp {};
  temp.insert(m_superVertex.begin(), m_superVertex.end());
  if (isCutVertex(m_embedding, temp, target)) return;

  // Now remove the vertex
  m_superVertex.unsafe_erase(target);
  for (auto it = range.first; it != range.second; ++it)
  {
    m_sourceConnections[it->second]--;
  }
}

bool SuperVertexReducer::isConnected(vertex_t target) const
{
  bool connected = false;
  m_embedding.iterateTargetGraphAdjacentBreak(target, [&](vertex_t adj){
    connected |= m_superVertex.contains(adj);
    return connected;
  });
  return connected;
}

fuint32_t SuperVertexReducer::checkScore(const nodeset_t& placement) const
{
  fuint32_t numberBad = 0;
  for (vertex_t node : placement) if(isBadNode(node)) numberBad++;
  return numberBad;
}

const nodeset_t& SuperVertexReducer::getBetterPlacement(const nodeset_t& previous) const
{
  fuint32_t scorePrevious = checkScore(previous);
  fuint32_t scoreOwn = checkScore(m_superVertex);
  return (scorePrevious < scoreOwn || (scorePrevious == scoreOwn && previous.size() < m_superVertex.size()))
    ? previous : m_superVertex;
}

bool SuperVertexReducer::improved() const
{
  // printNodeset(m_initialSuperVertex);
  // printNodeset(m_superVertex);
  fuint32_t scorePrevious = checkScore(m_initialSuperVertex);
  fuint32_t scoreOwn = checkScore(m_superVertex);
  return (scoreOwn < scorePrevious ||
      (scorePrevious == scoreOwn && m_superVertex.size() < m_initialSuperVertex.size()));
}

bool SuperVertexReducer::remainsValid(const EmbeddingManager& manager) const
{
  // m_superVertex is definitely connected, now check whether all
  // adjacent super vertices can be reached
  return improved() && connectsToAdjacentVertices(manager, m_superVertex, m_sourceVertex);
}