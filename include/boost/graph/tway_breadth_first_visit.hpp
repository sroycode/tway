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

#ifndef BOOST_GRAPH_TWAY_BREADTH_FIRST_VISIT_HPP
#define BOOST_GRAPH_TWAY_BREADTH_FIRST_VISIT_HPP

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

namespace boost {

/* Concepts definition start
 *
 */
template <class Visitor, class Graph>
struct TBFSVisitorConcept {
	void constraints() {
		function_requires< CopyConstructibleConcept<Visitor> >();
		vis.initialize_vertex(u, g);
		vis.initialize_vertex(u, g);
		vis.discover_vertex(u, g);
		vis.examine_vertex(u, g);
		vis.examine_edge(e, g);
		vis.tree_edge(e, g);
		vis.non_tree_edge(e, g);
		vis.gray_target(e, g);
		vis.black_target(e, g);
		vis.finish_vertex(u, g);
	}
	Visitor vis;
	Graph g;
	typename graph_traits<Graph>::vertex_descriptor u;
	typename graph_traits<Graph>::edge_descriptor e;
};

/* Concepts definition end */

/*
 *  Reverse Enabled Breadth First Search Algorithm
 */


template <class BidirectionalGraph, class Buffer, class TBFSVisitor, class ColorMap>
void tway_breadth_first_visit(
    const BidirectionalGraph& g,
    typename graph_traits<BidirectionalGraph>::vertex_descriptor s,
    typename graph_traits<BidirectionalGraph>::vertex_descriptor t,
    Buffer& Q, TBFSVisitor vis, ColorMap color, bool& is_fwd)
{
	function_requires< BidirectionalGraphConcept<BidirectionalGraph> >();
	typedef graph_traits<BidirectionalGraph> GTraits;
	typedef typename GTraits::vertex_descriptor Vertex;
	typedef typename GTraits::edge_descriptor Edge;
	function_requires< TBFSVisitorConcept<TBFSVisitor, BidirectionalGraph> >();
	function_requires< ReadWritePropertyMapConcept<ColorMap, Vertex> >();
	typedef typename property_traits<ColorMap>::value_type ColorValue;
	typedef color_traits<ColorValue> Color;
	typedef typename GTraits::out_edge_iterator out_edge_iterator;
	typedef typename GTraits::in_edge_iterator in_edge_iterator;

	put(color, s, Color::silver_gray());
	vis.discover_vertex(s, g);
	Q.push(s);
	put(color, t, Color::gold_gray());
	vis.discover_vertex(t, g);
	Q.push(t);
	while (! Q.empty()) {
		Vertex u = Q.top();
		Q.pop();
		is_fwd = ( get(color, u) == Color::silver());
		vis.examine_vertex(u, g);
		if (is_fwd) {
			out_edge_iterator ei, ei_end;
			for (tie(ei, ei_end) = out_edges(u, g); ei != ei_end; ++ei) {
				Vertex v = target(*ei, g);
				vis.examine_edge(*ei, g);
				ColorValue v_color = get(color, v);
				if (v_color == Color::white()) {
					vis.tree_edge(*ei, g);
					put(color, v, Color::silver_gray());
					vis.discover_vertex(v, g);
					Q.push(v);
				} else {
					vis.non_tree_edge(*ei, g);
					if (v_color == Color::gray())
						vis.gray_target(*ei, g);
					else
						vis.black_target(*ei, g);
				}
			} // end for
		} else {
			in_edge_iterator ei, ei_end;
			for (tie(ei, ei_end) = in_edges(u, g); ei != ei_end; ++ei) {
				Vertex v = source(*ei, g);
				vis.examine_edge(*ei, g);
				ColorValue v_color = get(color, v);
				if (v_color == Color::white()) {
					vis.tree_edge(*ei, g);
					put(color, v, Color::gold_gray() );
					vis.discover_vertex(v, g);
					Q.push(v);
				} else {
					vis.non_tree_edge(*ei, g);
					if (v_color == Color::gray())
						vis.gray_target(*ei, g);
					else
						vis.black_target(*ei, g);
				}
			} // end for
		}
		put(color, u, (is_fwd)?Color::silver_black():Color::gold_black() );
		vis.finish_vertex(u, g);
	} // end while
} // tway_breadth_first_visit

template <class Visitors = null_visitor>
class tbfs_visitor {
public:
	tbfs_visitor() { }
	tbfs_visitor(Visitors vis) : m_vis(vis) { }

#define TBFS_FUNCEVE_ONE_STUB(VertEdge,FuncEve)             \
    template<class VertEdge, class Graph>                   \
    void FuncEve ( VertEdge ve, Graph& g) {                 \
    invoke_visitors(m_vis, ve, g, ::boost::on_##FuncEve()); \
    }

	TBFS_FUNCEVE_ONE_STUB(Vertex,initialize_vertex)
	TBFS_FUNCEVE_ONE_STUB(Vertex,discover_vertex)
	TBFS_FUNCEVE_ONE_STUB(Vertex,examine_vertex)
	TBFS_FUNCEVE_ONE_STUB(Edge,examine_edge)
	TBFS_FUNCEVE_ONE_STUB(Edge,tree_edge)
	TBFS_FUNCEVE_ONE_STUB(Edge,non_tree_edge)
	TBFS_FUNCEVE_ONE_STUB(Edge,gray_target)
	TBFS_FUNCEVE_ONE_STUB(Edge,black_target)
	TBFS_FUNCEVE_ONE_STUB(Vertex,finish_vertex)

#undef TBFS_FUNCEVE_ONE_STUB

	BOOST_GRAPH_EVENT_STUB(on_initialize_vertex,tbfs)
	BOOST_GRAPH_EVENT_STUB(on_discover_vertex,tbfs)
	BOOST_GRAPH_EVENT_STUB(on_examine_vertex,tbfs)
	BOOST_GRAPH_EVENT_STUB(on_examine_edge,tbfs)
	BOOST_GRAPH_EVENT_STUB(on_tree_edge,tbfs)
	BOOST_GRAPH_EVENT_STUB(on_non_tree_edge,tbfs)
	BOOST_GRAPH_EVENT_STUB(on_gray_target,tbfs)
	BOOST_GRAPH_EVENT_STUB(on_black_target,tbfs)
	BOOST_GRAPH_EVENT_STUB(on_finish_vertex,tbfs)


protected:
	Visitors m_vis;
};

template <class Visitors>
tbfs_visitor<Visitors> make_tbfs_visitor(Visitors vis)
{
	return tbfs_visitor<Visitors>(vis);
}
typedef tbfs_visitor<> default_tbfs_visitor;


// This version does not initialize colors, user has to.

template <class BidirectionalGraph, class P, class T, class R>
void tway_breadth_first_visit
(const BidirectionalGraph& g,
 typename graph_traits<BidirectionalGraph>::vertex_descriptor s,
 typename graph_traits<BidirectionalGraph>::vertex_descriptor t,
 const bgl_named_params<P, T, R>& params,bool& is_fwd)
{
	// The graph is passed by *const* reference so that graph adaptors
	// (temporaries) can be passed into this function. However, the
	// graph is not really const since we may write to property maps
	// of the graph.
	BidirectionalGraph& ng = const_cast<BidirectionalGraph&>(g);

	typedef graph_traits<BidirectionalGraph> Traits;
	// Buffer default
	typedef typename Traits::vertex_descriptor vertex_descriptor;
	typedef boost::queue<vertex_descriptor> queue_t;
	queue_t Q;
	// detail::wrap_ref<queue_t> Qref(Q);

	tway_breadth_first_visit
	(ng, s, t,
   choose_param(get_param(params, buffer_param_t()), boost::ref(Q)).get(),
	 choose_param(get_param(params, graph_visitor),
	              make_tbfs_visitor(null_visitor())),
	 choose_pmap(get_param(params, vertex_color), ng, vertex_color), is_fwd
	);
}

} // namespace boost

#endif // BOOST_GRAPH_TWAY_BREADTH_FIRST_VISIT_HPP
