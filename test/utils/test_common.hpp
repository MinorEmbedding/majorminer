#ifndef __MAJORMINER_TEST_COMMON_HPP_
#define __MAJORMINER_TEST_COMMON_HPP_

#include <gtest/gtest.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_sort.h>
#include <sstream>

#include <majorminer.hpp>
#include <common/embedding_validator.hpp>

#include "qubo_modelling.hpp"

namespace majorminer
{
  void containsEdges(const graph_t& graph, std::initializer_list<edge_t> edges);

  void addEdges(graph_t& graph, std::initializer_list<edge_t> edges);

  void assertEquality1(fuint32_t n, qcoeff_t penalty);
  void assertLEQ1(fuint32_t n, qcoeff_t penalty);
  void assertAbsorber(fuint32_t n, qcoeff_t penalty);
  void assertGEQ1(fuint32_t n, qcoeff_t penalty);

  void dumpModel(const std::string& filename, const QPolynomial& poly);

  nodeset_t getNodeset(const graph_t& graph);

  void printNodeset(const nodeset_t& nodes);
}




#endif
