/*
 * BGLMesh.hpp
 *
 *  Created on: Mar 9, 2015
 *      Author: jcassidy
 */

#ifndef BGLMESH_HPP_
#define BGLMESH_HPP_

#include <boost/graph/graph_traits.hpp>
#include <boost/iterator/counting_iterator.hpp>

#include <array>

class TetraGraph;

namespace boost {

template<>struct graph_traits<TetraGraph> {
	typedef unsigned 	vertex_descriptor;
	typedef int 		edge_descriptor;

	typedef boost::disallow_parallel_edge_tag 	edge_parallel_category;
	typedef boost::directed_tag 				directed_category;

	typedef boost::incidence_graph_tag traversal_category;
	typedef std::array<int,4>::const_iterator 	out_edge_iterator;
	typedef boost::counting_iterator<unsigned>	vertex_iterator;

	typedef unsigned degree_size_type;

	static vertex_descriptor null_vertex(){ return 0; }
};

}

class TetraGraph {
	const TetraMesh& M_;
public:

	TetraGraph(const TetraMesh& M) : M_(M){}

	typedef unsigned vertex_descriptor;
	typedef int edge_descriptor;

	typedef unsigned degree_size_type;
	typedef std::array<int,4>::const_iterator 	out_edge_iterator;

	vertex_descriptor source(TetraGraph::edge_descriptor e) const
	{
		return e < 0 ? M_.vecFaceID_Tetra[-e].second : M_.vecFaceID_Tetra[e].first;
	}

	vertex_descriptor target(TetraGraph::edge_descriptor e) const
	{
		return e < 0 ? M_.vecFaceID_Tetra[-e].first : M_.vecFaceID_Tetra[e].second;
	}

	// TetraGraph out edges are faces (descriptor is int; -ve if down-face, +ve if up-face)
	std::pair<out_edge_iterator,out_edge_iterator> out_edges(TetraGraph::edge_descriptor e) const
	{
		return std::make_pair(M_.T_f[e].begin(), M_.T_f[e].end());

	}

	boost::graph_traits<TetraGraph>::degree_size_type out_degree(TetraGraph::vertex_descriptor u) const
	{
		return u==0 ? 0 : 4;
	}


	size_t num_vertices() const
	{
		return M_.getNt()+1;
	}

	size_t num_edges() const
	{
		return M_.getNf()+1;
	}
};

size_t num_vertices(const TetraGraph& G)
{
	return G.num_vertices();
}

size_t num_edges(const TetraGraph& G)
{
	return G.num_edges();
}

TetraGraph::vertex_descriptor source(TetraGraph::edge_descriptor e,const TetraGraph& g)
{
	return g.source(e);
}

TetraGraph::vertex_descriptor target(TetraGraph::edge_descriptor e,const TetraGraph& g)
{
	return g.target(e);
}

// TetraGraph out edges are faces (descriptor is int; -ve if down-face, +ve if up-face)
std::pair<TetraGraph::out_edge_iterator,TetraGraph::out_edge_iterator> out_edges(TetraGraph::edge_descriptor e,const TetraGraph& g)
{
	return g.out_edges(e);
}

std::pair<boost::counting_iterator<unsigned>,boost::counting_iterator<unsigned>> vertices(const TetraGraph& G)
{
	return std::make_pair(
			boost::counting_iterator<unsigned>(0U),
			boost::counting_iterator<unsigned>(num_vertices(G)));
}

boost::graph_traits<TetraGraph>::degree_size_type out_degree(TetraGraph::vertex_descriptor u,const TetraGraph& g)
{
	return g.out_degree(u);
}


#endif /* BGLMESH_HPP_ */
