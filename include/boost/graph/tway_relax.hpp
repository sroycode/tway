//=======================================================================
// Copyright 2010.
// Authors: S Roychowdhury
// Inherits: boost/graph/relax.hpp
//
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#ifndef BOOST_GRAPH_TWAY_RELAX_HPP
#define BOOST_GRAPH_TWAY_RELAX_HPP

#include <boost/graph/relax.hpp>

namespace boost {
    template <class Graph, class WeightMap, 
            class PredecessorMap, class DistanceMap, 
            class BinaryFunction, class BinaryPredicate>
    bool tway_relax(typename graph_traits<Graph>::edge_descriptor e, 
               const Graph& g, const WeightMap& w, 
               PredecessorMap& p, DistanceMap& d, 
               const BinaryFunction& combine, const BinaryPredicate& compare,
               bool is_fwd=true)
    {
      typedef typename graph_traits<Graph>::directed_category DirCat;
      bool is_undirected = is_same<DirCat, undirected_tag>::value;
      typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
	    Vertex u = (is_fwd) ? source(e, g) : target(e,g);
	    Vertex v = (is_fwd) ? target(e, g) : source(e,g);
      typedef typename property_traits<DistanceMap>::value_type D;
      typedef typename property_traits<WeightMap>::value_type W;
      D d_u = get(d, u), d_v = get(d, v);
      W w_e = get(w, e);
      
      // The redundant gets in the return statements are to ensure that extra
      // floating-point precision in x87 registers does not lead to relax()
      // returning true when the distance did not actually change.
      if ( compare(combine(d_u, w_e), d_v) ) {
        put(d, v, combine(d_u, w_e));
        put(p, v, u);
        return true;
      } else if (is_undirected && compare(combine(d_v, w_e), d_u)) {
        put(d, u, combine(d_v, w_e));
        put(p, u, v);
        return true;
      } else
        return false;
    }
    
    template <class Graph, class WeightMap, 
      class PredecessorMap, class DistanceMap>
    bool tway_relax(typename graph_traits<Graph>::edge_descriptor e,
               const Graph& g, WeightMap w, PredecessorMap p, DistanceMap d,
               bool is_fwd=true)
    {
      typedef typename property_traits<DistanceMap>::value_type D;
      typedef closed_plus<D> Combine;
      typedef std::less<D> Compare;
      return tway_relax(e, g, w, p, d, Combine(), Compare());
    }

} // namespace boost
#endif /* BOOST_GRAPH_TWAY_RELAX_HPP */
