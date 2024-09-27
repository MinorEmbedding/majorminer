#include "initial/network_simplex.hpp"

#include <common/embedding_state.hpp>
#include <common/embedding_manager.hpp>
#include <common/utils.hpp>
#include <common/time_measurement.hpp>

using namespace majorminer;

#define PREVENT_TAKING 100
#define OCCUPIED 10
#define FREE 1


NetworkSimplexWrapper::NetworkSimplexWrapper(EmbeddingState& state, EmbeddingManager& embeddingManager)
  : m_state(state), m_embeddingManager(embeddingManager), m_initialized(false)
{ }

NetworkSimplexWrapper::capacity_t NetworkSimplexWrapper::getNumberAdjacentNodes(const adjacency_list_range_iterator_t& adjacentIt) const
{
  const auto& mapping = m_state.getMapping();
  capacity_t n = 0;
  for (auto adjacentNode = adjacentIt.first; adjacentNode != adjacentIt.second; ++adjacentNode)
  {
    if (mapping.contains(adjacentNode->second)) n++;
  }
  // if (n < 2) throw std::runtime_error("Invalid number of embedded adjacent nodes! < 2...");
  return n;
}

void NetworkSimplexWrapper::constructLemonGraph()
{
  // Construct base graph
  const auto& targetGraph = *m_state.getTargetGraph();
  for (const auto& arc : targetGraph)
  {
    auto uNode = createNode(arc.first);
    auto vNode = createNode(arc.second);
    LemonArc uv = m_graph.addArc(uNode, vNode);
    LemonArc vu = m_graph.addArc(vNode, uNode);
    m_edgeMap.insert(std::make_pair(arc, std::make_pair(uv, vu)));
  }
}

vertex_t NetworkSimplexWrapper::chooseSource(vertex_t source) const
{
  vertex_t bestFound = FUINT32_UNDEF;
  m_state.iterateSourceMappingAdjacent<true>(source,
    [&](vertex_t adjacent, vertex_t){
      bestFound = adjacent;
      return true;
  });
  if (isDefined(bestFound)) return bestFound;

  const auto& reverse = m_state.getReverseMapping();
  const auto& remaining = m_state.getRemainingTargetNodes();
  fuint32_t numberMapped = FUINT32_UNDEF;

  m_state.iterateSourceMappingAdjacent<false>(source,
    [&](vertex_t adjacent, vertex_t){
      if (remaining.contains(adjacent)) return false;
      fuint32_t nb = reverse.count(adjacent);
      if (nb < numberMapped)
      {
        numberMapped = nb;
        bestFound = adjacent;
      }
      return numberMapped == 1; // we can skip if we find a vertex
  });
  if (!isDefined(bestFound)) throw std::runtime_error("Isolated vertex in network simplex!");
  return bestFound;
}

NetworkSimplexWrapper::LemonNode& NetworkSimplexWrapper::getNextRootNode()
{
  if (m_rootCounter < m_rootVertices.size()) return m_rootVertices[m_rootCounter++];
  else
  {
    m_rootVertices.push_back(m_graph.addNode());
    LemonNode& root = m_rootVertices.back();
    createCheapArc(root, m_t, *m_costMap, *m_capMap, false);
    m_rootCounter++;
    return root;
  }
}

void NetworkSimplexWrapper::createCheapArc(LemonNode& from, LemonNode& to,
    LemonArcMap<cost_t>& costs, LemonArcMap<capacity_t>& caps, bool constructionArc, capacity_t capacity)
{
  auto temp = m_graph.addArc(from, to);
  costs[temp] = 0;
  caps[temp] = capacity;
  if (constructionArc) m_treeConstructionArcs.push_back(temp);
}

void NetworkSimplexWrapper::constructHelperNodes(LemonArcMap<cost_t>& costs, LemonArcMap<capacity_t>& caps,
    const adjacency_list_range_iterator_t& adjacentIt)
{
  // define nodes for construction
  const auto& mapping = m_state.getMapping();

  vertex_t adjacentCandidate = FUINT32_UNDEF;

  // sink vertices
  for (auto adjacentNode = adjacentIt.first; adjacentNode != adjacentIt.second; ++adjacentNode)
  {
    auto embeddingPath = mapping.equal_range(adjacentNode->second);
    if (embeddingPath.first == embeddingPath.second) continue;
    adjacentCandidate = adjacentNode->second;

    LemonNode& constructionNode = getNextRootNode();

    for (auto targetNode = embeddingPath.first; targetNode != embeddingPath.second; ++targetNode)
    {
      LemonNode& fromNode = m_nodeMap[targetNode->second];
      createCheapArc(fromNode, constructionNode, costs, caps, true);
    }
  }

  // source vertex
  vertex_t sVertex = chooseSource(adjacentCandidate);
  LemonNode& toNode = m_nodeMap[sVertex];
  m_sConnected = sVertex;
  createCheapArc(m_s, toNode, costs, caps, true, m_numberAdjacent);
  //adjustCosts(sVertex, costs);
}

void NetworkSimplexWrapper::initialCreation()
{
  m_costMap = std::make_unique<LemonArcMap<cost_t>>(m_graph);
  m_capMap = std::make_unique<LemonArcMap<capacity_t>>(m_graph);
  m_flowMap = std::make_unique<LemonArcMap<capacity_t>>(m_graph);
  constructLemonGraph();
  m_s = m_graph.addNode();
  m_t = m_graph.addNode();

  m_initialized = true;
}

void NetworkSimplexWrapper::embeddNode(vertex_t node)
{
  if (!m_initialized) initialCreation();
  clear();
  auto adjacentIt = m_state.getSourceAdjGraph().equal_range(node);
  m_numberAdjacent = getNumberAdjacentNodes(adjacentIt);

  setupCostsAndCaps();

  constructHelperNodes(*m_costMap, *m_capMap, adjacentIt);

  NetworkSimplex ns(m_graph);
  ns.costMap(*m_costMap).upperMap(*m_capMap).stSupply(m_s, m_t, m_numberAdjacent);
  NetworkSimplex::ProblemType status = ns.run();
  if (status == NetworkSimplex::OPTIMAL)
  {
    LemonArcMap<capacity_t>& flows = *m_flowMap;
    ns.flowMap(flows);
    for (const auto& arc : m_edgeMap)
    {
      if (flows[arc.second.first] > 0)
      {
        m_mapped.insert(arc.first.first);
      }
      if (flows[arc.second.second] > 0)
      {
        m_mapped.insert(arc.first.second);
      }
    }
  }
  else if(status == NetworkSimplex::INFEASIBLE)
  {
    throw std::runtime_error("Infeasible in NetworkSimplex...");
  }
  else
  {
    throw std::runtime_error("Unbounded in NetworkSimplex");
  }
}


void NetworkSimplexWrapper::adjustCosts(vertex_t node, LemonArcMap<cost_t>& costs)
{
  auto adjacentIt = m_state.getTargetAdjGraph().equal_range(node);
  const auto& targetNodesRemaining = m_state.getRemainingTargetNodes();
  for (auto it = adjacentIt.first; it != adjacentIt.second; ++it)
  {
    const auto& edges = getArcPair(node, it->second);
    if (targetNodesRemaining.contains(it->second))
    {
      costs[edges.first] = FREE;
      costs[edges.second] = FREE;
    }
    else
    {
      costs[edges.first] = PREVENT_TAKING;
      costs[edges.second] = PREVENT_TAKING;
    }
  }
}
const NetworkSimplexWrapper::LemonArcPair& NetworkSimplexWrapper::getArcPair(vertex_t n1, vertex_t n2)
{
  auto findIt = m_edgeMap.find(edge_t{n1, n2});
  if (findIt != m_edgeMap.end())
  {
    return findIt->second;
  }
  return m_edgeMap[edge_t{n2, n1}];
}

NetworkSimplexWrapper::LemonNode NetworkSimplexWrapper::createNode(vertex_t node)
{
  auto findIt = m_nodeMap.find(node);
  if (findIt == m_nodeMap.end())
  {
    auto it = m_nodeMap.insert(std::make_pair(node, m_graph.addNode()));
    return it.first->second;
  }
  return findIt->second;
}

NetworkSimplexWrapper::cost_t NetworkSimplexWrapper::determineCost(vertex_t node)
{
  // printAdjacencyList(m_state.getReverseMapping());
  auto mapped = m_state.getReverseMappedCnt(node);
  // auto nb = m_state.getReverseMapping().count(node);
  // std::cout << "Node " << node << ": " << mapped << " ?= " << nb << std::endl;
  // if (mapped != (fint32_t)nb)
  // {
  //   std::cout << mapped << " != " << nb << std::endl;
  //   throw std::runtime_error("error!");
  // }
  return mapped != 0 ? /*mapped **/ OCCUPIED : FREE;
}

void NetworkSimplexWrapper::setupCostsAndCaps()
{
  // set all costs of nonartificial arcs
  for (const auto& edgePair : m_edgeMap)
  {
    const auto& uv = edgePair.second.first;
    const auto& vu = edgePair.second.second;
    (*m_capMap)[uv] = m_numberAdjacent;
    (*m_capMap)[vu] = m_numberAdjacent;
    (*m_costMap)[uv] = determineCost(edgePair.first.first);
    (*m_costMap)[vu] = determineCost(edgePair.first.second);
  }
}

void NetworkSimplexWrapper::clear()
{
  for (auto& lemonArc : m_treeConstructionArcs)
  {
    (*m_capMap)[lemonArc] = 0;
  }
  m_mapped.clear();
  m_numberAdjacent = 0;
  m_sConnected = -1;

  m_rootCounter = 0;
}
