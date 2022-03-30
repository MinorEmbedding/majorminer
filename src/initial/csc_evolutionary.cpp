#include "initial/csc_evolutionary.hpp"

#include <common/utils.hpp>
#include <common/cut_vertex.hpp>
#include <common/embedding_state.hpp>

#define POPULATION_SIZE 10
#define ITERATION_LIMIT 10
#define MAX_NEW_VERTICES 10
#define REDUCE_ITERATION_COEFFICIENT 3

using namespace majorminer;

namespace
{
  template<typename T>
  void swapPointers(T*& from, T*& to)
  {
    T*& temp = to;
    to = from;
    from = temp;
  }
}

EvolutionaryCSCReducer::EvolutionaryCSCReducer(const EmbeddingState& state, fuint32_t sourceVertex)
  : m_state(state), m_sourceVertex(sourceVertex)
{
  initialize();
}

void EvolutionaryCSCReducer::optimize()
{
  Vector<CSCIndividual>* current = &m_populationA;
  Vector<CSCIndividual>* next = &m_populationB;

  for (fuint32_t iteration = 0; iteration < ITERATION_LIMIT; ++iteration)
  {
    optimizeIteration(*current);

    if (iteration + 1 != ITERATION_LIMIT)
    {
      bool success = createNextGeneration(*current, *next);
      if (!success) break;
      swapPointers(current, next);
    }
  }
}

void EvolutionaryCSCReducer::initialize()
{
  if (!canExpand()) return;

  const auto& mapping = m_state.getMapping();
  const auto& reverse = m_state.getReverseMapping();
  auto range = mapping.equal_range(m_sourceVertex);
  for (auto mappedIt = range.first; mappedIt != range.second; ++mappedIt)
  {
    m_bestSuperVertex.insert(mappedIt->second);
    m_vertexFitness.insert(std::make_pair(mappedIt->second,
      reverse.count(mappedIt->second) - 1));
  }
  m_preparedVertices.insert(m_bestSuperVertex.begin(), m_bestSuperVertex.end());

  // Prepare adjacent source vertices
  m_state.iterateSourceGraphAdjacent(m_sourceVertex, [&](vertex_t adjacentSource){
    if (mapping.contains(adjacentSource)) m_adjacentSourceVertices.insert(adjacentSource);
  });

  // Prepare intial "adjacentSource" adjacency list
  nodepairset_t targetAdjSourcePairs;
  for (auto target : m_bestSuperVertex)
  {
    m_state.iterateTargetAdjacentReverseMapping(target, [&](vertex_t adjSource){
      if (m_adjacentSourceVertices.contains(adjSource))
      {
        targetAdjSourcePairs.insert(std::make_pair(target, adjSource));
      }
    });
  }

  for (auto& adj : targetAdjSourcePairs) m_adjacentSources.insert(adj);

  m_bestFitness = getFitness(m_bestSuperVertex);
  initializePopulations();
}

void EvolutionaryCSCReducer::initializePopulations()
{
  m_populationA.resize(POPULATION_SIZE);
  m_populationB.resize(POPULATION_SIZE);

  for (auto& individual : m_populationA)
  {
    individual.initialize(*this, m_sourceVertex);
    individual.fromInitial(m_bestSuperVertex); // at this point m_bestSuperVertex is initial placement
  }
  for (auto& individual : m_populationA)
  {
    individual.initialize(*this, m_sourceVertex);
  }
}

bool EvolutionaryCSCReducer::canExpand()
{
  // Check whether there is some unoccupied adjacent target vertex
  auto range = m_state.getMapping().equal_range(m_sourceVertex);
  const auto& targetRemaining = m_state.getRemainingTargetNodes();
  m_expansionPossible = false;
  for (auto it = range.first; it != range.second; ++it)
  {
    m_state.iterateTargetGraphAdjacentBreak(it->second,
      [&](fuint32_t adjTarget){
        if (targetRemaining.contains(adjTarget))
        {
          m_expansionPossible = true;
        }
        return m_expansionPossible;
    });
    if (m_expansionPossible) return true;
  }
  return false;
}


void EvolutionaryCSCReducer::optimizeIteration(Vector<CSCIndividual>& parentPopulation)
{
  // optimize all in parent population
  for (auto& parent : parentPopulation) parent.optimize();

  // sort parent population
  std::sort(parentPopulation.begin(), parentPopulation.end(), std::less<CSCIndividual>());

  size_t newBestFitness = parentPopulation[0].getFitness();
  size_t size = parentPopulation[0].getSolutionSize();
  if (newBestFitness < m_bestFitness ||
    (newBestFitness == m_bestFitness && size < m_bestSuperVertex.size()))
  { // adopt new superior solution
    m_bestFitness = newBestFitness;
    m_bestSuperVertex.clear();
    const auto& newSuperVertex = parentPopulation[0].getSuperVertex();
    m_bestSuperVertex.insert(newSuperVertex.begin(), newSuperVertex.end());
  }
}

bool EvolutionaryCSCReducer::createNextGeneration(Vector<CSCIndividual>& parentPopulation,
  Vector<CSCIndividual>& childPopulation)
{
  fuint32_t remainingAttemps = 5 * POPULATION_SIZE;
  for (fuint32_t idx = 3; idx < POPULATION_SIZE && remainingAttemps > 0; --remainingAttemps)
  {
    const CSCIndividual* parentA = tournamentSelection(parentPopulation);
    const CSCIndividual* parentB = tournamentSelection(parentPopulation);
    bool success = childPopulation[idx].fromCrossover(*parentA, *parentB);
    if (success) idx++;
  }
  return remainingAttemps > 0;
}

const CSCIndividual* EvolutionaryCSCReducer::tournamentSelection(const Vector<CSCIndividual>& parentPopulation)
{
  fuint32_t max = POPULATION_SIZE - 1;
  const CSCIndividual* individualA = &parentPopulation[m_random.getRandomUint(max)];
  const CSCIndividual* individualB = &parentPopulation[m_random.getRandomUint(max)];
  return *individualA < *individualB ? individualA : individualB;
}


void EvolutionaryCSCReducer::prepareVertex(vertex_t target)
{
  m_temporary.clear();
  m_state.iterateTargetAdjacentReverseMapping(target, [&](fuint32_t adjacentSource){
    if (m_adjacentSourceVertices.contains(adjacentSource)) m_temporary.insert(adjacentSource);
  });
  for (auto source : m_temporary)
  {
    m_adjacentSources.insert(std::make_pair(target, source));
  }
}

void EvolutionaryCSCReducer::addConnectivity(VertexNumberMap& connectivity, vertex_t target)
{
  if (!m_preparedVertices.contains(target)) prepareVertex(target);

  auto range = m_adjacentSources.equal_range(target);
  for (auto it = range.first; it != range.second; ++it)
  {
    connectivity[it->second]++;
  }
}

size_t EvolutionaryCSCReducer::getFitness(const nodeset_t& placement) const
{
  size_t fitness = 0;
  VertexNumberMap::const_iterator findIt;
  for (auto vertex : placement)
  {
    findIt = m_vertexFitness.find(vertex);
    if (findIt != m_vertexFitness.end()) fitness += findIt->second;
  }
  return fitness;
}

size_t EvolutionaryCSCReducer::getFitness(vertex_t target) const
{
  auto findIt = m_vertexFitness.find(target);
  return findIt == m_vertexFitness.end() ? 0 : findIt->second;
}

bool EvolutionaryCSCReducer::isRemoveable(VertexNumberMap& connectivity, vertex_t target) const
{
  auto range = m_adjacentSources.equal_range(target);
  for (auto it = range.first; it != range.second; ++it)
  {
    if (connectivity[it->second] <= 1) return false;
  }
  return true;
}

void EvolutionaryCSCReducer::removeVertex(VertexNumberMap& connectivity, vertex_t target) const
{
  auto range = m_adjacentSources.equal_range(target);
  for (auto it = range.first; it != range.second; ++it)
  {
    connectivity[it->second]--;
  }
}

void CSCIndividual::initialize(EvolutionaryCSCReducer& reducer, vertex_t sourceVertex)
{
  m_reducer = &reducer;
  m_state = &reducer.m_state;
  m_sourceVertex = sourceVertex;
  m_random = std::make_unique<RandomGen>();
}



void CSCIndividual::fromInitial(const nodeset_t& placement)
{
  m_done = false;
  m_superVertex.clear();
  m_superVertex.insert(placement.begin(), placement.end());

  setupConnectivity();
}

bool CSCIndividual::fromCrossover(const CSCIndividual& individualA, const CSCIndividual& individualB)
{
  m_done = false;
  m_superVertex.clear();
  const auto& superVertexA = individualA.getSuperVertex();
  const auto& superVertexB = individualB.getSuperVertex();
  if (!overlappingSets(superVertexA, superVertexB)
    && !areSetsConnected(*m_state, superVertexA, superVertexB))
  {
    return false;
  }
  m_superVertex.insert(superVertexA.begin(), superVertexA.end());
  m_superVertex.insert(superVertexB.begin(), superVertexB.end());

  setupConnectivity();
  return true;
}

void CSCIndividual::setupConnectivity()
{
  const auto& adjacentSourceVertices = m_reducer->getAdjacentSourceVertices();
  for (auto adjSource : adjacentSourceVertices)
  {
    m_connectivity[adjSource] = 0;
  }

  for (auto target : m_superVertex)
  {
    m_reducer->addConnectivity(m_connectivity, target);
  }
}

void CSCIndividual::optimize()
{
  if (m_done) return;
  mutate();
  reduce();
  m_fitness = m_reducer->getFitness(m_superVertex);
  m_done = true;
}


size_t CSCIndividual::getSolutionSize() const
{
  return m_superVertex.size();
}

size_t CSCIndividual::getFitness() const
{
  return m_fitness;
}

void CSCIndividual::mutate()
{
  m_temporarySet.clear();
  for (vertex_t vertex : m_superVertex)
  {
    m_state->iterateFreeTargetAdjacent(vertex,
      [&](vertex_t adjacentTarget){
        m_temporarySet.insert(adjacentTarget);
    });
  }
  if (m_temporarySet.empty()) return;

  vertex_t startVertex = m_random->getRandomVertex(m_temporarySet);
  if (!isDefined(startVertex)) return;
  m_temporarySet.clear();
  clearStack(m_iteratorStack);

  const auto& targetGraph = m_state->getTargetAdjGraph();
  const auto& remaining = m_state->getRemainingTargetNodes();

  fuint32_t numerAdded = 1;
  addVertex(m_sourceVertex);
  m_iteratorStack.push(targetGraph.equal_range(startVertex));
  while(!m_iteratorStack.empty() && numerAdded < MAX_NEW_VERTICES)
  {
    auto& top = m_iteratorStack.top();
    if (top.first == top.second)
    {
      m_iteratorStack.pop();
      continue;
    }
    vertex_t adjacent = top.first->second;
    if (remaining.contains(adjacent) && !m_superVertex.contains(adjacent))
    { // add node
      top.first++;
      addVertex(adjacent);
      m_iteratorStack.push(targetGraph.equal_range(adjacent));
    }
    else
    {
      top.first++;
    }
  }
}

void CSCIndividual::reduce()
{
  if (m_superVertex.size() <= 1) return;
  m_vertexVector.resize(m_superVertex.size());
  fuint32_t idx = 0;
  for (vertex_t vertex : m_superVertex) m_vertexVector[idx++] = vertex;
  fuint32_t vectorSize = m_vertexVector.size();

  // Try greedily reducing overlap vertices
  for (idx = 0; idx < vectorSize;)
  {
    vertex_t* current = &m_vertexVector[idx];
    if (m_reducer->getFitness(*current) != 0 && tryRemove(*current))
    {
      *current = m_vertexVector.back();
      m_vertexVector.resize(--vectorSize);
    }
    else idx++;
  }

  // Try reducing all other vertices
  fuint32_t maxIterations = REDUCE_ITERATION_COEFFICIENT * m_vertexVector.size();
  for (fuint32_t iteration = 0; idx < maxIterations; ++iteration)
  {
    fuint32_t randomIdx = m_random->getRandomUint(vectorSize - 1);
    vertex_t* current = &m_vertexVector[randomIdx];
    if (tryRemove(*current))
    {
      *current = m_vertexVector.back();
      m_vertexVector.resize(--vectorSize);
    }
  }

  for (idx = 0; idx < vectorSize; ++idx) tryRemove(m_vertexVector[idx]);
}

void CSCIndividual::addVertex(vertex_t target)
{
  m_superVertex.insert(target);
  m_reducer->addConnectivity(m_connectivity, target);
}

bool CSCIndividual::tryRemove(vertex_t target)
{
  // Remove if not a cut vertex
  if (m_reducer->isRemoveable(m_connectivity, target)
    && !isCutVertex(*m_state, m_superVertex, target))
  {
    m_reducer->removeVertex(m_connectivity, target);
    m_superVertex.unsafe_erase(target);
    return true;
  }
  return false;
}

