#include "test_common.hpp"

#include <fstream>

using namespace majorminer;

namespace
{
  void assertSimpleConstraint(fuint32_t n, qcoeff_t penalty, QConstraintType type)
  {
    QModel model{};
    auto vars = model.createBinaryVars(n);
    auto& cons = *model.createConstraint(type, 1.0, penalty);
    cons.addTerms(vars, 1.0);
    auto reformulated = model.reformulate();
    QEnumerationVerifier verifier{model, reformulated, penalty, true, true};
    ASSERT_TRUE(verifier.verify());
  }
}

void majorminer::containsEdges(const graph_t& graph, std::initializer_list<edge_t> edges)
{
  tbb::parallel_for_each(edges.begin(), edges.end(),
    [&graph](const edge_t& edge) {
      EXPECT_TRUE(graph.find(edge) != graph.end()
        || graph.find(std::make_pair(edge.second, edge.first)) != graph.end());
  });
}


void majorminer::addEdges(graph_t& graph, std::initializer_list<edge_t> edges)
{
  for (const auto& edge : edges)
  {
    graph.insert(edge);
  }
}


void majorminer::assertEquality1(fuint32_t n, qcoeff_t penalty)
{
  assertSimpleConstraint(n, penalty, QConstraintType::EQUAL);
}

void majorminer::assertLEQ1(fuint32_t n, qcoeff_t penalty)
{
  assertSimpleConstraint(n, penalty, QConstraintType::LOWER_EQUAL);
}

void majorminer::assertAbsorber(fuint32_t n, qcoeff_t penalty)
{
  QModel model{};
  auto vars = model.createBinaryVars(n);
  auto* absorber = model.createBinaryVar();
  auto& cons = *model.createConstraint(QConstraintType::EQUAL, 0.0, penalty);
  cons.addTerms(vars, 1.0);
  cons.addTerm(*absorber, -1.0);
  auto reformulated = model.reformulate();
  QEnumerationVerifier verifier{model, reformulated, penalty, true};
  ASSERT_TRUE(verifier.verify());
}

void majorminer::assertGEQ1(fuint32_t n, qcoeff_t penalty)
{
  assertSimpleConstraint(n, penalty, QConstraintType::GREATER_EQUAL);
}


void majorminer::dumpModel(const std::string& filename, const QPolynomial& poly)
{
  std::ofstream f(filename);
  if (!f.is_open()) throw std::runtime_error("Could not open file...");
  dumpPoly(f, poly);
  f.close();
}

nodeset_t majorminer::getNodeset(const graph_t& graph)
{
  nodeset_t nodes{};
  tbb::parallel_for_each(graph.begin(), graph.end(),
    [&nodes](const edge_t& edge){
      nodes.insert(edge.first);
      nodes.insert(edge.second);
  });
  return nodes;
}
