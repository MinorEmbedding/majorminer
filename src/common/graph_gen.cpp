#include "graph_gen.hpp"

#include <fstream>
#include <string>
#include <memory>
#include <iostream>

#include <common/random_gen.hpp>

using namespace majorminer;

graph_t majorminer::generate_chimera(fuint32_t rows, fuint32_t cols)
{
  graph_t graph{};
  fuint32_t currentNode = 0;
  fuint32_t lowerUnitCellOffset = cols * 8;
  for (fuint32_t row = 0; row < rows; ++row)
  {

    for (fuint32_t col = 0; col < cols; ++col)
    {
      fuint32_t iOffset = currentNode;
      fuint32_t jOffset = currentNode + 4;

      // add edges within unit cell
      for (fuint32_t i = 0; i < 4; ++i)
      {
        for (fuint32_t j = 0; j < 4; ++j)
        {
          graph.insert(std::make_pair(iOffset + i, jOffset + j));
        }
      }

      // add edges to right unit cell
      if (col + 1 != cols)
      {
        graph.insert(std::make_pair(jOffset, jOffset + 8));
        graph.insert(std::make_pair(jOffset + 1, jOffset + 9));
        graph.insert(std::make_pair(jOffset + 2, jOffset + 10));
        graph.insert(std::make_pair(jOffset + 3, jOffset + 11));
      }

      // add edges to unit cell below
      if (row + 1 != rows)
      {
        graph.insert(std::make_pair(iOffset, iOffset + lowerUnitCellOffset));
        graph.insert(std::make_pair(iOffset + 1, iOffset + lowerUnitCellOffset + 1));
        graph.insert(std::make_pair(iOffset + 2, iOffset + lowerUnitCellOffset + 2));
        graph.insert(std::make_pair(iOffset + 3, iOffset + lowerUnitCellOffset + 3));
      }

      currentNode += 8;
    }

  }

  return graph;
}


graph_t majorminer::generate_king(fuint32_t rows, fuint32_t cols)
{
  graph_t graph{};
  if (rows == 0 || cols == 0) return graph;
  fuint32_t currentNode = 0;
  fuint32_t lowerLeftOffset = cols - 1;
  fuint32_t lowerRightOffset = cols + 1;
  for (fuint32_t row = 0; row < rows; ++row)
  {
    for (fuint32_t col = 0; col < cols; ++col)
    {
      if (col + 1 != cols) graph.insert(std::make_pair(currentNode, currentNode + 1));
      if (row + 1 != rows)
      {
        graph.insert(std::make_pair(currentNode, currentNode + cols));
        if (col > 0) graph.insert(std::make_pair(currentNode, currentNode + lowerLeftOffset));
        if (col + 1 != cols) graph.insert(std::make_pair(currentNode, currentNode + lowerRightOffset));
      }
      currentNode++;
    }
  }

  return graph;
}


graph_t majorminer::import_graph(const std::string& filename)
{
  std::ifstream file(filename);
  if (!file.is_open()) throw std::runtime_error("File not found.");

  file.seekg(0, std::ios::end);
  size_t length = file.tellg();

  file.seekg(0, std::ios::beg);

  auto content = std::make_unique<char[]>(length);
  file.read(content.get(), length);
  return import_graph(content.get(), length);
}

enum ImportState
{
  INITIAL,
  OUTER_LIST,
  INNER_LIST
};

graph_t majorminer::import_graph(const char* edgeList, size_t length)
{
  graph_t graph{};
  ImportState state = INITIAL;

  fuint32_t readIdx = 0;
  fuint32_t nodes[] = { 0, 0 };
  fuint32_t nrEnds = 0;
  fuint32_t currentVal = 0;
  bool done = false;
  bool readNb = false;
  while(readIdx < length && !done)
  {
    char sym = edgeList[readIdx];
    switch(state)
    {
      case INITIAL:
        if (sym == '[') state = OUTER_LIST;
        break;
      case OUTER_LIST:
      {
        if (sym == '[') state = INNER_LIST;
        else if (sym == ']') done = true;
        else if (sym != ' ' && sym != ',') throw std::runtime_error("Bad edge list.");
        break;
      }
      case INNER_LIST:
      {
        if (std::isdigit(sym))
        {
          currentVal = currentVal * 10 + (sym - '0');
          readNb = true;
        }
        else if ((sym == ',' || sym == ']') && readNb)
        {
          readNb = false;
          if (nrEnds < 2) nodes[nrEnds++] = currentVal;
          else throw std::runtime_error("Invalid edge list.");
          currentVal = 0;
        }
        if (sym == ']')
        {
          if (nrEnds != 2) throw std::runtime_error("Invalid edge list. Wrong number of ends.");
          graph.insert(std::make_pair(nodes[0], nodes[1]));
          nrEnds = 0;
          state = OUTER_LIST;
          nodes[0] = 0;
          nodes[1] = 0;
        }
        break;
      }
    }
    readIdx++;
  }
  return graph;
}


graph_t majorminer::generate_cyclegraph(fuint32_t n)
{
  graph_t cycle{};
  if (n < 2) return cycle;
  tbb::parallel_for( tbb::blocked_range<fuint32_t>(0, n),
    [&cycle, n](const tbb::blocked_range<fuint32_t>& range) {
      for (auto i = range.begin(); i != range.end(); ++i)
      {
        cycle.insert(std::make_pair(i, (i +1) % n));
      }
  });
  return cycle;
}

graph_t majorminer::generate_completegraph(fuint32_t n)
{
  graph_t clique{};
  if (n < 2) return clique;
  tbb::parallel_for( tbb::blocked_range<fuint32_t>(0, n),
    [&clique, n](const tbb::blocked_range<fuint32_t>& range) {
      for (auto i = range.begin(); i != range.end(); ++i)
      {
        for (fuint32_t j = i + 1; j < n; ++j)
        {
          clique.insert(std::make_pair(i, j));
        }
      }
  });
  return clique;
}

graph_t majorminer::generate_petersen()
{
  graph_t petersen{
    {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 0},
    {0, 5}, {1, 6}, {2, 7}, {3, 8}, {4, 9},
    {5, 7}, {7, 9}, {9, 6}, {6, 8}, {8, 5}
  };
  return petersen;
}


graph_t majorminer::generate_erdosrenyi(fuint32_t n, float probability)
{
  ProbabilisticDecision<float> rand{};
  graph_t erdos{};
  for (fuint32_t i = 0; i < n; ++i)
  {
    for (fuint32_t j = i + 1; j < n; ++j)
    {
      if (rand(probability)) erdos.insert(std::make_pair(i,j));
    }
  }
  return erdos;
}

