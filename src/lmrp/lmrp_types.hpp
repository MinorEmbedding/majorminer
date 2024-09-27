#ifndef __MAJORMINER_LMRP_TYPES_HPP_
#define __MAJORMINER_LMRP_TYPES_HPP_

#include <majorminer_types.hpp>

namespace majorminer
{
  struct ConnectedList
  {
    ConnectedList(vertex_t source, fuint32_t idx, fuint32_t nbMapped);

    bool wasSatisfied() const { return m_satisfied; }
    void satisfied() { m_satisfied = true; }

    friend bool operator<(const ConnectedList& c1, const ConnectedList& c2)
    { return c1.m_source < c2.m_source || (c1.m_source == c2.m_source
      && c1.m_nbMapped < c2.m_nbMapped); }

    friend std::ostream& operator<<(std::ostream& os, const ConnectedList& component)
    {
      os << "Component{source=" << component.m_source << "; nbMapped="
         << component.m_nbMapped << "; idx=" << component.m_idx << "}";
      return os;
    }

    vertex_t m_source;
    fuint32_t m_idx;
    fuint32_t m_nbMapped;
    bool m_satisfied;
  };

  struct DijkstraVertex
  {
    public:
      DijkstraVertex(){}
      DijkstraVertex(vertex_t target);
      DijkstraVertex(vertex_t target, vertex_t parent,
        fuint32_t overlapCnt, fuint32_t nonOverlapCnt);

      void reset();

      bool visited();

      bool wasVisited() const { return m_visited; }

      bool lowerTo(fuint32_t parent, fuint32_t overlap, fuint32_t nonOverlap);

      friend bool operator>(const DijkstraVertex& v1, const DijkstraVertex& v2)
      {
        return v1.m_overlapCnt > v2.m_overlapCnt ||
          ( v1.m_overlapCnt == v2.m_overlapCnt
            && v1.m_nonOverlapCnt > v2.m_nonOverlapCnt);
      }

      friend bool operator==(const DijkstraVertex& v1, const DijkstraVertex& v2)
      { return v1.m_target == v2.m_target; }

      friend std::ostream& operator<<(std::ostream& os, const DijkstraVertex& v)
      {
        os << "DijkstraVertex{target=" << v.m_target << "; fitness=(" << v.m_overlapCnt << ","
           << v.m_nonOverlapCnt << "); nbMapped=" << v.m_visited << "; parent="
           << v.m_parent << " }";
        return os;
      }


      vertex_t m_target;
      vertex_t m_parent;
      fuint32_t m_overlapCnt;
      fuint32_t m_nonOverlapCnt;
      bool m_visited;
  };
}



#endif