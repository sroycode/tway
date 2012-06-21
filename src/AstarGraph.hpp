//=======================================================================
// Copyright 2010.
// Authors: S Roychowdhury
// Project: tway
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#ifndef _TESTASTAR_ASTAR_GRAPH_HPP_
#define _TESTASTAR_ASTAR_GRAPH_HPP_
#include "Locals.hh"
#include <boost/tuple/tuple.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <boost/graph/astar_search.hpp>

namespace TestAstar {
/** xNode: struct to hold node */
struct xNode {
	xNode() : x(0),y(0) {}
	S_INT x;
	S_INT y;
};
/** xEdge: struct to hold edge */
struct xEdge {
	xEdge() : cost(0) {}
	U_INT cost;
};
/** found_goal:  Exception struct throwable*/
struct found_goal {};
/**
 * astar_goal_visitor: Visitor for astar, right now only termination
 */
template<typename VertexType>
struct astar_goal_visitor : public boost::default_astar_visitor {
public:
	astar_goal_visitor(VertexType goal) : m_goal(goal) {}
	template<typename GraphType>
	void examine_vertex(VertexType u, GraphType& g) {
		if (u == m_goal) throw found_goal();
	}
private:
	VertexType m_goal;
};

/**
 * distance_heuristic: astar distance calc heuristic, implements manhattan dist
 */
template<typename GraphType>
struct distance_heuristic : public boost::astar_heuristic<GraphType, U_INT> {
public:
	typedef typename boost::graph_traits<GraphType>::vertex_descriptor vertex_descriptor;
	distance_heuristic(GraphType& g, vertex_descriptor goal) : m_g(g), m_goal(goal) {}
	inline U_INT operator()(vertex_descriptor u) {
		return ( labs( m_g[m_goal].x - m_g[u].x) + labs( m_g[m_goal].y - m_g[u].y) )/2;
	}
private:
	GraphType& m_g;
	vertex_descriptor m_goal;
};
class AstarGraph {

public:
	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, xNode, xEdge> GraphT;
	AstarGraph() {}
	~AstarGraph() {}
	/**
	 * AddEdge: Add Graph Edge
	 */
	void AddEdge(U_INT src, U_INT trg, S_INT cost) {
		typedef boost::graph_traits<GraphT>::edge_descriptor edge_descriptor;
		bool Ins;
		edge_descriptor Edge;
		boost::tie(Edge, Ins) = boost::add_edge(src,trg,hGraph);
		if (!Ins) throw local_exception("Cannot insert graph data");
		hGraph[Edge].cost=cost;
	}
	/**
	 * Modify Node: Add x,y to node
	 */
	void ModifyNode(U_INT vid, S_INT x, S_INT y) {
		typedef boost::graph_traits<GraphT>::vertex_descriptor vertex_descriptor;
		if (vid>=num_vertices(hGraph))
			throw local_exception("Cannot insert graph data too large vid");
		vertex_descriptor v = vertex(vid, hGraph);
		hGraph[v].x = x;
		hGraph[v].y = y;
	}

	/**
	 * Search: Astar Search by source,target
	 */
	template <typename T>
	bool Search(U_INT src, U_INT trg, T& PathRes, U_INT& Cost) {
		typedef boost::graph_traits<GraphT>::vertex_descriptor vertex_descriptor;
		std::vector<vertex_descriptor> predecessors(num_vertices(hGraph));
		if (src>=num_vertices(hGraph) || trg>=num_vertices(hGraph)) return false;
		vertex_descriptor source_vertex = vertex(src, hGraph);
		vertex_descriptor target_vertex = vertex(trg, hGraph);
		std::vector<U_INT> distances(num_vertices(hGraph));
		typedef std::vector<boost::default_color_type> colormap_t;
		colormap_t colors(num_vertices(hGraph));
		try {
			boost::astar_search(
			    hGraph, source_vertex,
			    distance_heuristic<GraphT>(hGraph, target_vertex),
			    boost::predecessor_map(&predecessors[0]).
			    weight_map(get(( &xEdge::cost ), hGraph)).
			    distance_map(&distances[0]).
			    color_map(&colors[0]).
			    visitor(astar_goal_visitor<vertex_descriptor>(target_vertex)));
		} catch (found_goal fg) {
			Cost=distances[target_vertex];
			PathRes.clear();
			PathRes.push_front(target_vertex);
			size_t max=num_vertices(hGraph);
			while (target_vertex != source_vertex) {
				if (target_vertex == predecessors[target_vertex])
					return false;
				target_vertex = predecessors[target_vertex];
				PathRes.push_front(target_vertex);
				if (!max--)
					return false;
			}
			return true;
		}
		return false;
	}
private:
	GraphT hGraph;
};
} // namespace TestAstar
#endif
