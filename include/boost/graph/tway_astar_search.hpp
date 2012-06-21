//
//=======================================================================
// Copyright 2010.
// Authors: S Roychowdhury
// Inherits: boost/graph/astar_search.hpp
//
// Copyright (c) 2004 Kristopher Beevers
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//

#ifndef BOOST_GRAPH_TWAY_ASTAR_SEARCH_HPP
#define BOOST_GRAPH_TWAY_ASTAR_SEARCH_HPP


#include <functional>
#include <boost/limits.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/pending/mutable_queue.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <boost/graph/exception.hpp>
#include <boost/graph/tway_color.hpp>
#include <boost/graph/tway_relax.hpp>
#include <boost/graph/tway_breadth_first_visit.hpp>

namespace boost {

/* Concepts definition start
 *
 */

template <class Heuristic, class Graph>
struct TwAStarHeuristicConcept {
	void constraints() {
		function_requires< CopyConstructibleConcept<Heuristic> >();
		h(u);
	}
	Heuristic h;
	typename graph_traits<Graph>::vertex_descriptor u;
};

template <class Graph, class CostType>
class tway_astar_heuristic
	: public std::unary_function<
		typename graph_traits<Graph>::vertex_descriptor, CostType> {
public:
	tway_astar_heuristic() {}
	CostType operator()(typename graph_traits<Graph>::vertex_descriptor u) {
		return static_cast<CostType>(0);
	}
};
/* Concepts definition end */

template <class Visitors = null_visitor>
class tway_astar_visitor : public tbfs_visitor<Visitors> {
public:
	tway_astar_visitor() {}
	tway_astar_visitor(Visitors vis)
		: tbfs_visitor<Visitors>(vis) {}

	template<class Edge, class Graph>
	void edge_relaxed(Edge e, Graph& g) {
		invoke_visitors(this->m_vis, e, g, ::boost::on_edge_relaxed());
	}
	template<class Edge, class Graph>
	void edge_not_relaxed(Edge e, Graph& g) {
		invoke_visitors(this->m_vis, e, g, ::boost::on_edge_not_relaxed());
	}

private:
	template <class Edge, class Graph>
	void tree_edge(Edge e, Graph& g) {}
	template <class Edge, class Graph>
	void non_tree_edge(Edge e, Graph& g) {}
};

template <class Visitors>
tway_astar_visitor<Visitors> make_tway_astar_visitor(Visitors vis)
{
	return tway_astar_visitor<Visitors>(vis);
}
typedef tway_astar_visitor<> default_tway_astar_visitor;


namespace detail {

template <class TwAStarHeuristic, class UniformCostVisitor,
         class UpdatableQueue, class PredecessorMap,
         class CostMap, class DistanceMap, class WeightMap,
         class ColorMap, class BinaryFunction,
         class BinaryPredicate>
struct tway_astar_tbfs_visitor {

	typedef typename property_traits<CostMap>::value_type C;
	typedef typename property_traits<ColorMap>::value_type ColorValue;
	typedef color_traits<ColorValue> Color;
	typedef typename property_traits<DistanceMap>::value_type distance_type;

	tway_astar_tbfs_visitor(
	    TwAStarHeuristic h_f,
	    TwAStarHeuristic h_r,
	    UniformCostVisitor vis,
	    UpdatableQueue& Q, PredecessorMap p,
	    CostMap c, DistanceMap d, WeightMap w,
	    ColorMap col, BinaryFunction combine,
	    BinaryPredicate compare, C zero, bool& is_fwd)
		: m_h_fwd(h_f), m_h_rev(h_r), m_vis(vis), m_Q(Q), m_predecessor(p), m_cost(c),
		  m_distance(d), m_weight(w), m_color(col),
		  m_combine(combine), m_compare(compare), m_zero(zero), is_fwd_(is_fwd) {}


#define TBFS_FUNCEVE_TWO_STUB(VertEdge,FuncEve)                \
    template<class VertEdge, class Graph>                      \
    void FuncEve ( VertEdge ve, Graph& g) {                    \
    m_vis.FuncEve(ve, g);                                      \
    }

	TBFS_FUNCEVE_TWO_STUB(Vertex,initialize_vertex)
	TBFS_FUNCEVE_TWO_STUB(Vertex,discover_vertex)
	TBFS_FUNCEVE_TWO_STUB(Vertex,examine_vertex)
	TBFS_FUNCEVE_TWO_STUB(Vertex,finish_vertex)

#undef TBFS_FUNCEVE_TWO_STUB

	template <class Edge, class Graph>
	void examine_edge(Edge e, Graph& g) {
		if (m_compare(get(m_weight, e), m_zero)) throw negative_edge();
		if ( (is_fwd_ && ( get(m_color, target(e, g)) == Color::gold()))
		        || ((!is_fwd_) && ( get(m_color, source(e, g)) == Color::silver())) )
			intersect_found(e, g);
		m_vis.examine_edge(e, g);
	}
	template <class Edge, class Graph>
	void non_tree_edge(Edge, Graph&) {}

#define TBFS_FUNCEVE_THR_STUB(VertEdge,FuncEve)                \
    template<class VertEdge, class Graph>                      \
    void FuncEve ( VertEdge ve, Graph& g) {                    \
    (is_fwd_)? FuncEve##_fwd(ve, g) : FuncEve##_rev(ve, g);    \
    }

	TBFS_FUNCEVE_THR_STUB(Edge,tree_edge)
	TBFS_FUNCEVE_THR_STUB(Edge,gray_target)
	TBFS_FUNCEVE_THR_STUB(Edge,black_target)
#undef TBFS_FUNCEVE_THR_STUB
	template <class Edge, class Graph>
	void intersect_found(Edge e, Graph& g) {
		typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
		/***
		 * Logic: use predecessor map as successor map and run while loop
		 */
		vertex_descriptor sval = source(e,g), tval=target(e,g);
		while (get(m_predecessor, tval) !=tval) {
			vertex_descriptor nval = get(m_predecessor, tval);
			put(m_predecessor, tval, sval);
			sval = tval;
			tval = nval;
		}
		put(m_predecessor, tval, sval);
		put(m_distance, tval, m_combine( m_combine( get(m_distance,source(e,g)), get(m_weight,e)), get(m_distance,target(e,g)) ) );
		m_vis.intersect_found(e, g);
	}

private:
	/** Implementation **/
	template <class Edge, class Graph>
	void tree_edge_fwd(Edge e, Graph& g) {
		m_decreased = tway_relax(e, g, m_weight, m_predecessor, m_distance, m_combine, m_compare,is_fwd_);

		if(m_decreased) {
			m_vis.edge_relaxed(e, g);
			put(m_cost, target(e, g), m_combine(get(m_distance, target(e, g)), m_h_fwd(target(e, g))));
		} else m_vis.edge_not_relaxed(e, g);
	}

	template <class Edge, class Graph>
	void tree_edge_rev(Edge e, Graph& g) {
		m_decreased = tway_relax(e, g, m_weight, m_predecessor, m_distance, m_combine, m_compare,is_fwd_);

		if(m_decreased) {
			m_vis.edge_relaxed(e, g);
			put(m_cost, source(e, g), m_combine(get(m_distance, source(e, g)), m_h_rev(source(e, g))));
		} else m_vis.edge_not_relaxed(e, g);
	}

	template <class Edge, class Graph>
	void gray_target_fwd(Edge e, Graph& g) {
		distance_type old_distance = get(m_distance, target(e, g));

		m_decreased = tway_relax(e, g, m_weight, m_predecessor, m_distance, m_combine, m_compare,is_fwd_);

		/* See comment in gray_target in regular astar */
		if(m_decreased && old_distance != get(m_distance, target(e, g))) {
			put(m_cost, target(e, g), m_combine(get(m_distance, target(e, g)), m_h_fwd(target(e, g))));
			m_Q.update(target(e, g));
			m_vis.edge_relaxed(e, g);
		} else
			m_vis.edge_not_relaxed(e, g);
	}

	template <class Edge, class Graph>
	void gray_target_rev(Edge e, Graph& g) {
		distance_type old_distance = get(m_distance, source(e, g));

		m_decreased = tway_relax(e, g, m_weight, m_predecessor, m_distance, m_combine, m_compare,is_fwd_);

		/* See comment in gray_target in regular astar */
		if(m_decreased && old_distance != get(m_distance, source(e, g))) {
			put(m_cost, source(e, g), m_combine(get(m_distance, source(e, g)), m_h_rev(source(e, g))));
			m_Q.update(source(e, g));
			m_vis.edge_relaxed(e, g);
		} else
			m_vis.edge_not_relaxed(e, g);
	}
	template <class Edge, class Graph>
	void black_target_fwd(Edge e, Graph& g) {
		distance_type old_distance = get(m_distance, target(e, g));

		m_decreased = tway_relax(e, g, m_weight, m_predecessor, m_distance,
		                         m_combine, m_compare,is_fwd_);

		/* See comment in gray_target in regular astar */
		if(m_decreased && old_distance != get(m_distance, target(e, g))) {
			m_vis.edge_relaxed(e, g);
			put(m_cost, target(e, g),
			    m_combine(get(m_distance, target(e, g)),
			              m_h_fwd(target(e, g))));
			m_Q.push(target(e, g));
			put(m_color, target(e, g), Color::silver_black());
			m_vis.black_target(e, g);
		} else
			m_vis.edge_not_relaxed(e, g);
	}

	template <class Edge, class Graph>
	void black_target_rev(Edge e, Graph& g) {
		distance_type old_distance = get(m_distance, source(e, g));

		m_decreased = tway_relax(e, g, m_weight, m_predecessor, m_distance, m_combine, m_compare,is_fwd_);

		/* See comment in gray_target in regular astar */
		if(m_decreased && old_distance != get(m_distance, source(e, g))) {
			m_vis.edge_relaxed(e, g);
			put(m_cost, source(e, g),
			    m_combine(get(m_distance, source(e, g)),
			              m_h_rev(source(e, g))));
			m_Q.push(source(e, g));
			put(m_color, source(e, g), Color::gold_black());
			m_vis.black_target(e, g);
		} else
			m_vis.edge_not_relaxed(e, g);
	}
	/** Implementation **/



	TwAStarHeuristic m_h_fwd;
	TwAStarHeuristic m_h_rev;
	UniformCostVisitor m_vis;
	UpdatableQueue& m_Q;
	PredecessorMap m_predecessor;
	CostMap m_cost;
	DistanceMap m_distance;
	WeightMap m_weight;
	ColorMap m_color;
	BinaryFunction m_combine;
	BinaryPredicate m_compare;
	bool m_decreased;
	C m_zero;
	const bool& is_fwd_;
};


} // namespace detail

template <typename VertexListGraph, typename TwAStarHeuristic,
         typename TwAStarVisitor, typename PredecessorMap,
         typename CostMap, typename DistanceMap,
         typename WeightMap, typename ColorMap,
         typename VertexIndexMap,
         typename CompareFunction, typename CombineFunction,
         typename CostInf, typename CostZero>
inline void tway_astar_search_no_init (VertexListGraph &g,
                                       typename graph_traits<VertexListGraph>::vertex_descriptor s,
                                       typename graph_traits<VertexListGraph>::vertex_descriptor t,
                                       TwAStarHeuristic h_f,
                                       TwAStarHeuristic h_r,
                                       TwAStarVisitor vis,
                                       PredecessorMap predecessor, CostMap cost,
                                       DistanceMap distance, WeightMap weight,
                                       ColorMap color, VertexIndexMap index_map,
                                       CompareFunction compare, CombineFunction combine,
                                       CostInf inf, CostZero zero)
{
	typedef typename graph_traits<VertexListGraph>::vertex_descriptor Vertex;
	typedef indirect_cmp<CostMap, CompareFunction> IndirectCmp;
	IndirectCmp icmp(cost, compare);


	typedef mutable_queue<Vertex, std::vector<Vertex>, IndirectCmp, VertexIndexMap> MutableQueue;
	MutableQueue Q(num_vertices(g), icmp, index_map);

	bool is_fwd=true;
	detail::tway_astar_tbfs_visitor<TwAStarHeuristic, TwAStarVisitor,
	       MutableQueue, PredecessorMap, CostMap, DistanceMap,
	       WeightMap, ColorMap, CombineFunction, CompareFunction>
	       tbfs_vis(h_f,h_r, vis, Q, predecessor, cost, distance, weight,
	                color, combine, compare, zero, is_fwd);

	tway_breadth_first_visit(g, s, t, Q, tbfs_vis, color, is_fwd);
}

// Non-named parameter interface
template <typename VertexListGraph, typename TwAStarHeuristic,
         typename TwAStarVisitor, typename PredecessorMap,
         typename CostMap, typename DistanceMap,
         typename WeightMap, typename VertexIndexMap,
         typename ColorMap,
         typename CompareFunction, typename CombineFunction,
         typename CostInf, typename CostZero>
inline void tway_astar_search (VertexListGraph &g,
                               typename graph_traits<VertexListGraph>::vertex_descriptor s,
                               typename graph_traits<VertexListGraph>::vertex_descriptor t,
                               TwAStarHeuristic h_f,
                               TwAStarHeuristic h_r,
                               TwAStarVisitor vis,
                               PredecessorMap predecessor, CostMap cost,
                               DistanceMap distance, WeightMap weight,
                               VertexIndexMap index_map, ColorMap color,
                               CompareFunction compare, CombineFunction combine,
                               CostInf inf, CostZero zero)
{

	typedef typename property_traits<ColorMap>::value_type ColorValue;
	typedef color_traits<ColorValue> Color;
	typename graph_traits<VertexListGraph>::vertex_iterator ui, ui_end;
	for (tie(ui, ui_end) = vertices(g); ui != ui_end; ++ui) {
		put(color, *ui, Color::white());
		put(distance, *ui, inf);
		put(cost, *ui, inf);
		put(predecessor, *ui, *ui);
		vis.initialize_vertex(*ui, g);
	}
	put(distance, s, zero);
	put(cost, s, h_f(s));
	put(color, s, Color::silver());
	put(distance, t, zero);
	put(cost, t, h_r(t));
	put(color, t, Color::gold());

	tway_astar_search_no_init
	(g, s, t, h_f,h_r, vis, predecessor, cost, distance, weight,
	 color, index_map, compare, combine, inf, zero);

}



namespace detail {
template <class VertexListGraph, class TwAStarHeuristic,
         class CostMap, class DistanceMap, class WeightMap,
         class IndexMap, class ColorMap, class Params>
inline void tway_astar_dispatch2 (VertexListGraph& g,
                                  typename graph_traits<VertexListGraph>::vertex_descriptor s,
                                  typename graph_traits<VertexListGraph>::vertex_descriptor t,
                                  TwAStarHeuristic h_f,
                                  TwAStarHeuristic h_r,
                                  CostMap cost, DistanceMap distance,
                                  WeightMap weight, IndexMap index_map, ColorMap color,
                                  const Params& params) {
	dummy_property_map p_map;
	typedef typename property_traits<CostMap>::value_type C;
	tway_astar_search
	(g, s, t, h_f,h_r,
	 choose_param(get_param(params, graph_visitor),
	              make_tway_astar_visitor(null_visitor())),
	 choose_param(get_param(params, vertex_predecessor), p_map),
	 cost, distance, weight, index_map, color,
	 choose_param(get_param(params, distance_compare_t()),
	              std::less<C>()),
	 choose_param(get_param(params, distance_combine_t()),
	              closed_plus<C>()),
	 choose_param(get_param(params, distance_inf_t()),
	              std::numeric_limits<C>::max BOOST_PREVENT_MACRO_SUBSTITUTION ()),
	 choose_param(get_param(params, distance_zero_t()),
	              C()));
}

template <class VertexListGraph, class TwAStarHeuristic,
         class CostMap, class DistanceMap, class WeightMap,
         class IndexMap, class ColorMap, class Params>
inline void tway_astar_dispatch1 (VertexListGraph& g,
                                  typename graph_traits<VertexListGraph>::vertex_descriptor s,
                                  typename graph_traits<VertexListGraph>::vertex_descriptor t,
                                  TwAStarHeuristic h_f,
                                  TwAStarHeuristic h_r,
                                  CostMap cost, DistanceMap distance,
                                  WeightMap weight, IndexMap index_map, ColorMap color,
                                  const Params& params) {
	typedef typename property_traits<WeightMap>::value_type D;
	typename std::vector<D>::size_type
	n = is_default_param(distance) ? num_vertices(g) : 1;
	std::vector<D> distance_map(n);
	n = is_default_param(cost) ? num_vertices(g) : 1;
	std::vector<D> cost_map(n);
	std::vector<tway_color_type> color_map(num_vertices(g));
	tway_color_type c; // default init white

	detail::tway_astar_dispatch2
	(g, s, t, h_f,h_r,
	 choose_param(cost, make_iterator_property_map
	              (cost_map.begin(), index_map,
	               cost_map[0])),
	 choose_param(distance, make_iterator_property_map
	              (distance_map.begin(), index_map,
	               distance_map[0])),
	 weight, index_map,
	 choose_param(color, make_iterator_property_map
	              (color_map.begin(), index_map, c)),
	 params);
}
} // namespace detail


// Named parameter interface
template <typename VertexListGraph,
         typename TwAStarHeuristic,
         typename P, typename T, typename R>
void tway_astar_search (VertexListGraph &g,
                        typename graph_traits<VertexListGraph>::vertex_descriptor s,
                        typename graph_traits<VertexListGraph>::vertex_descriptor t,
                        TwAStarHeuristic h_f,
                        TwAStarHeuristic h_r,
                        const bgl_named_params<P, T, R>& params)
{

	detail::tway_astar_dispatch1
	(g, s, t, h_f, h_r,
	 get_param(params, vertex_rank),
	 get_param(params, vertex_distance),
	 choose_const_pmap(get_param(params, edge_weight), g, edge_weight),
	 choose_const_pmap(get_param(params, vertex_index), g, vertex_index),
	 get_param(params, vertex_color),
	 params);

}

} // namespace boost

#endif // BOOST_GRAPH_TWAY_ASTAR_SEARCH_HPP
