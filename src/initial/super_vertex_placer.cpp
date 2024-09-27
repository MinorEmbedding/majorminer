#include "super_vertex_placer.hpp"

#include "super_vertex_reducer.hpp"

#include <common/utils.hpp>
#include <common/embedding_manager.hpp>
#include <common/embedding_state.hpp>
#include <common/embedding_visualizer.hpp>
#include <common/debug_utils.hpp>
#include <initial/super_vertex_reducer.hpp>
#include <initial/csc_evolutionary.hpp>

using namespace majorminer;

SuperVertexPlacer::SuperVertexPlacer(EmbeddingState& state, EmbeddingManager& embeddingManager)
  : m_state(state), m_embeddingManager(embeddingManager)
{}

void SuperVertexPlacer::operator()()
{
  if (!m_nodesToProcess.empty())
  {
    if (!connectedNode()) return;
  }
  else
  {
    trivialNode();
  }
}

void SuperVertexPlacer::replaceOverlapping()
{
  nodeset_t overlapping{};
  // const auto& sourceGraph = m_state.getSourceAdjGraph();

  nodeset_t temporary{};

  for (fuint32_t idx = 0; idx < 20; ++idx)
  {
    // std::cout << "Idx: " << idx << " -> " 
    identifyOverlapping(overlapping);
    if (overlapping.empty()) break;
    for (vertex_t vertex : overlapping)
    {
      if(idx % 2 == 0) improveMapping(vertex);
      else replaceSuperVertex(vertex, temporary);

      // visualize(vertex, PlacedNodeType::COMPLEX, sourceGraph.count(vertex));
    }
    temporary.clear();
  }
}


void SuperVertexPlacer::identifyOverlapping(nodeset_t& overlapping)
{
  overlapping.clear();
  const auto& reverse = m_state.getReverseMapping();
  fuint32_pair_t lastPair = std::make_pair(FUINT32_UNDEF, FUINT32_UNDEF);
  for (auto reverseMapped : reverse)
  {
    if (isDefined(lastPair.first) && lastPair.first == reverseMapped.first)
    { // overlapping
      overlapping.insert(lastPair.second);
      overlapping.insert(reverseMapped.second);
    }
    lastPair = reverseMapped;
  }
}

void SuperVertexPlacer::improveMapping(vertex_t source)
{
  EvolutionaryCSCReducer reducer{m_state, source};
  const auto& sourceGraph = m_state.getSourceAdjGraph();
  reducer.optimize();
  if (reducer.foundBetter())
  {
    m_embeddingManager.unmapNode(source);
    const auto& superVertex = reducer.getPlacement();
    m_embeddingManager.mapNode(source, superVertex);
    // std::cout << "Improved through reducer. " << std::endl;
    if (m_state.hasVisualizer())
    {
      visualize(source, PlacedNodeType::COMPLEX, sourceGraph.count(source));
    }
  }
}

void SuperVertexPlacer::trivialNode()
{
  // just pick an arbitrary node and place it somewhere
  vertex_t node = m_state.getTrivialNode();
  // DEBUG(OUT_S << "Trivial node to embedd: " << node << std::endl;)
  embeddTrivialNode(node);

  if (m_state.hasVisualizer()) visualize(node, TRIVIAL);
}

bool SuperVertexPlacer::connectedNode()
{
  PrioNode node = m_nodesToProcess.top();
  m_nodesToProcess.pop();

  if (!m_state.removeRemainingNode(node.m_id)) return false;


  if (node.m_nbConnections > 1)
  {
    embeddNode(node.m_id);
    if (m_state.hasVisualizer()) visualize(node.m_id, COMPLEX, node.m_nbConnections);
  }
  else
  { // nbConnections = 1
    embeddSimpleNode(node.m_id);
    m_state.updateConnections(node.m_id, m_nodesToProcess);

    if (m_state.hasVisualizer()) visualize(node.m_id, SIMPLE);
  }
  return true;
}

void SuperVertexPlacer::embeddNode(vertex_t node)
{
  embeddNodeNetworkSimplex(node);
  m_state.updateConnections(node, m_nodesToProcess);
}



void SuperVertexPlacer::visualize(vertex_t node, PlacedNodeType type, fuint32_t nbConnections)
{
  auto& visualizer = *m_state.getVisualizer();
  const auto& mapping = m_state.getMapping();
  switch(type)
  {
    case TRIVIAL:
    {
      visualizer.draw(mapping,
        [&](std::ostream& os)
          { os << "Trivial node " << node; }
      );
      break;
    }
    case SIMPLE:
    {
      visualizer.draw(mapping,
        [&](std::ostream &os)
          { os << "Simple adjacent node " << node << " (1)."; }
      );
      break;
    }
    case COMPLEX:
    {
      visualizer.draw(mapping,
        [&](std::ostream &os)
          { os << "Complex adjacent node " << node
               << " (" << nbConnections << ")"; }
      );
      break;
    }
  }
}

void SuperVertexPlacer::replaceSuperVertex(vertex_t source, nodeset_t& svertex)
{
  svertex.clear();
  m_state.iterateSourceMapping(source, [&](vertex_t target) { svertex.insert(target); });
  m_embeddingManager.unmapNode(source);
  embeddNodeNetworkSimplex(source, &svertex);
}


void SuperVertexPlacer::embeddNodeNetworkSimplex(vertex_t node, const nodeset_t* oldMapping)
{
  if (m_nsWrapper.get() == nullptr) m_nsWrapper = std::make_unique<NetworkSimplexWrapper>(m_state, m_embeddingManager);
  m_nsWrapper->embeddNode(node);

  SuperVertexReducer reducer{m_state, node};
  reducer.initialize(m_nsWrapper->getMapped());
  reducer.optimize();
  const auto& superVertex = reducer.getBetterPlacement(m_nsWrapper->getMapped());
  fuint32_t fitness = calculateFitness(m_state, superVertex);
  if (oldMapping == nullptr)
  {
    m_embeddingManager.mapNode(node, superVertex);
    return;
  }
  fuint32_t oldFitness = calculateFitness(m_state, *oldMapping);
  if (std::make_pair(fitness, superVertex.size()) < std::make_pair(oldFitness, oldMapping->size()))
  {
    m_embeddingManager.mapNode(node, superVertex);
  }
  else m_embeddingManager.mapNode(node, *oldMapping);

  /*EvolutionaryCSCReducer reducer{m_state, m_nsWrapper->getMapped(), node};
  reducer.optimize();
  const auto& superVertex = reducer.getPlacement();
  m_embeddingManager.mapNode(node, superVertex);
  printNodeset(m_nsWrapper->getMapped());
  printNodeset(superVertex);*/
}

void SuperVertexPlacer::embeddSimpleNode(vertex_t node)
{
  // find a node that is adjacent to the node "adjacentNode"
  vertex_t adjacentNode = -1;
  m_state.iterateSourceGraphAdjacentBreak(node, [&](vertex_t adjacent){
    if (m_state.isNodeMapped(adjacent))
    { adjacentNode = adjacent; return true; }
    return false;
  });

  if (adjacentNode == (fuint32_t)-1) throw std::runtime_error("Could not find the adjacent node.");

  const auto& remaining = m_state.getRemainingTargetNodes();
  vertex_t bestNodeFound = FUINT32_UNDEF;
  m_state.iterateSourceMappingAdjacent<false>(adjacentNode,
    [&bestNodeFound, &remaining](vertex_t candidate, vertex_t){
    bestNodeFound = candidate;
    return remaining.contains(candidate);
  });
  if (!isDefined(bestNodeFound)) throw std::runtime_error("Isolated vertex.");

  // map "node" to "bestNodeFound"
  m_embeddingManager.mapNode(node, bestNodeFound);
  // m_state.updateNeededNeighbors(node);
}

void SuperVertexPlacer::embeddTrivialNode(vertex_t node)
{
  auto& remainingTargetNodes = m_state.getRemainingTargetNodes();
  if (!remainingTargetNodes.empty())
  {
    auto targetNode = *remainingTargetNodes.begin();
    remainingTargetNodes.unsafe_erase(remainingTargetNodes.begin());
    m_embeddingManager.mapNode(node, targetNode);
    m_state.updateNeededNeighbors(node);
    m_state.updateConnections(node, m_nodesToProcess);
  }
  else
  {
    throw std::runtime_error("Oooops...");
  }
}
