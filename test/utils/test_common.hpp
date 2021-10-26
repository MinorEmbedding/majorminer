#ifndef __MAJORMINER_TEST_COMMON_HPP_
#define __MAJORMINER_TEST_COMMON_HPP_

#include <gtest/gtest.h>
#include <tbb/parallel_for.h>
#include <majorminer.hpp>
#include "qubo_modelling.hpp"

namespace majorminer
{
  void containsEdges(const graph_t& graph, std::initializer_list<edge_t> edges);

  void printGraph(const graph_t& graph);

  void addEdges(graph_t& graph, std::initializer_list<edge_t> edges);

  void assertEquality1(fuint32_t n, qcoeff_t penalty);
  void assertLEQ1(fuint32_t n, qcoeff_t penalty);
  void assertAbsorber(fuint32_t n, qcoeff_t penalty);
  void assertGEQ1(fuint32_t n, qcoeff_t penalty);
}




#endif
