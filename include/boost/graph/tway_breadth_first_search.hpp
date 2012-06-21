//
//=======================================================================
// Copyright 2010.
// Authors: S Roychowdhury
// Inherits: boost/graph/breadth_first_search.hpp
//
//
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Copyright 2003 Bruce Barr
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//

#ifndef BOOST_GRAPH_TWAY_BREADTH_FIRST_SEARCH_HPP
#define BOOST_GRAPH_TWAY_BREADTH_FIRST_SEARCH_HPP

/*
  Tway Breadth First Search Algorithm modifies
  Breadth First Search Algorithm (Cormen, Leiserson, and Rivest p. 470)
*/

#include <boost/config.hpp>
#include <vector>
#include <boost/pending/queue.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/named_function_params.hpp>

#include <boost/graph/tway_color.hpp> // must include for defaults
#include <boost/graph/tway_breadth_first_visit.hpp>

namespace boost {

template <class VertexListGraph, class Buffer, class TBFSVisitor,
         class ColorMap>
void tway_breadth_first_search
(const VertexListGraph& g,
 typename graph_traits<VertexListGraph>::vertex_descriptor s,
 typename graph_traits<VertexListGraph>::vertex_descriptor t,
 Buffer& Q, TBFSVisitor vis, ColorMap color) {
	// Initialization
	typedef typename property_traits<ColorMap>::value_type ColorValue;
	typedef color_traits<ColorValue> Color;
	typename boost::graph_traits<VertexListGraph>::vertex_iterator i, i_end;
	for (tie(i, i_end) = vertices(g); i != i_end; ++i) {
		vis.initialize_vertex(*i, g);
		put(color, *i, Color::white());
	}
	bool is_fwd=true;
	tway_breadth_first_visit(g, s, t, Q, vis, color, is_fwd);
}

namespace detail {

template <class VertexListGraph, class ColorMap, class TBFSVisitor,
         class P, class T, class R>
void tbfs_helper
(VertexListGraph& g,
 typename graph_traits<VertexListGraph>::vertex_descriptor s,
 typename graph_traits<VertexListGraph>::vertex_descriptor t,
 ColorMap color,
 TBFSVisitor vis,
 const bgl_named_params<P, T, R>& params) {
	typedef graph_traits<VertexListGraph> Traits;
	// Buffer default
	typedef typename Traits::vertex_descriptor Vertex;
	typedef boost::queue<Vertex> queue_t;
	queue_t Q;
	detail::wrap_ref<queue_t> Qref(Q);
	tway_breadth_first_search
	(g, s, t,
	 choose_param(get_param(params, buffer_param_t()), Qref).ref,
	 vis, color);
}

//-------------------------------------------------------------------------
// Choose between default color and color parameters. Using
// function dispatching so that we don't require vertex index if
// the color default is not being used.

template <class ColorMap>
struct tbfs_dispatch {
	template <class VertexListGraph, class P, class T, class R>
	static void apply
	(VertexListGraph& g,
	 typename graph_traits<VertexListGraph>::vertex_descriptor s,
	 typename graph_traits<VertexListGraph>::vertex_descriptor t,
	 const bgl_named_params<P, T, R>& params,
	 ColorMap color) {
		tbfs_helper
		(g, s, t, color,
		 choose_param(get_param(params, graph_visitor),
		              make_tbfs_visitor(null_visitor())),
		 params);
	}
};

template <>
struct tbfs_dispatch<detail::error_property_not_found> {
	template <class VertexListGraph, class P, class T, class R>
	static void apply
	(VertexListGraph& g,
	 typename graph_traits<VertexListGraph>::vertex_descriptor s,
	 typename graph_traits<VertexListGraph>::vertex_descriptor t,
	 const bgl_named_params<P, T, R>& params,
	 detail::error_property_not_found) {
		std::vector<tway_color_type> color_vec(num_vertices(g));
		tway_color_type c; // default is white
		null_visitor null_vis;

		tbfs_helper
		(g, s, t,
		 make_iterator_property_map
		 (color_vec.begin(),
		  choose_const_pmap(get_param(params, vertex_index),
		                    g, vertex_index), c),
		 choose_param(get_param(params, graph_visitor),
		              make_tbfs_visitor(null_vis)),
		 params);
	}
};

} // namespace detail

// Named Parameter Variant
template <class VertexListGraph, class P, class T, class R>
void tway_breadth_first_search
(const VertexListGraph& g,
 typename graph_traits<VertexListGraph>::vertex_descriptor s,
 typename graph_traits<VertexListGraph>::vertex_descriptor t,
 const bgl_named_params<P, T, R>& params)
{
	// The graph is passed by *const* reference so that graph adaptors
	// (temporaries) can be passed into this function. However, the
	// graph is not really const since we may write to property maps
	// of the graph.
	VertexListGraph& ng = const_cast<VertexListGraph&>(g);
	typedef typename property_value< bgl_named_params<P,T,R>,
	        vertex_color_t>::type C;
	detail::tbfs_dispatch<C>::apply(ng, s, t, params,
	                                get_param(params, vertex_color));
}

} // namespace boost

#endif // BOOST_GRAPH_TWAY_BREADTH_FIRST_SEARCH_HPP
