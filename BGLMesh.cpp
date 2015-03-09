/*
 * BGLMesh.cpp
 *
 *  Created on: Mar 9, 2015
 *      Author: jcassidy
 */


#include "graph.hpp"
#include "BGLMesh.hpp"

#include <boost/range.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/properties.hpp>

#include <utility>

using namespace std;

using std::begin;
using std::end;

//template<typename T>T begin(const std::pair<T,T>& p){ return p.first; }
//template<typename T>T   end(const std::pair<T,T>& p){ return p.second; }

struct dfs_trivial_vis {
	std::vector<unsigned> visited;
	unsigned Ne=0;

	dfs_trivial_vis(TetraGraph& G) : visited(num_vertices(G),-1U){}

	void initialize_vertex(TetraGraph::vertex_descriptor s,const TetraGraph& g){ visited[s]=0; }
	void start_vertex(TetraGraph::vertex_descriptor s,const TetraGraph& g){ cout << "Start " << s << endl; }
	void discover_vertex(TetraGraph::vertex_descriptor s,const TetraGraph& g){ cout << s << endl;}
	void examine_edge(TetraGraph::edge_descriptor e,const TetraGraph &g){}
	void tree_edge(TetraGraph::edge_descriptor e,const TetraGraph &g){}
	void back_edge(TetraGraph::edge_descriptor e,const TetraGraph &g){}
	void forward_or_cross_edge(TetraGraph::edge_descriptor e,const TetraGraph &g){}
	void finish_edge(TetraGraph::edge_descriptor e,const TetraGraph &g){}
	void finish_vertex(TetraGraph::vertex_descriptor u,const TetraGraph& g){ visited[u]=1; }

	unsigned N_visited() const { return std::count(visited.begin(),visited.end(),1); }
	unsigned N_unvisited() const { return std::count(visited.begin(),visited.end(),0); }
};

void dfs(const TetraGraph& G,unsigned start,unsigned D=2,unsigned d=0)
{
	cout << setw(d*2) << ' ' << start << endl;
	if (d < D)
		for(auto e = out_edges(start,G).first; e != out_edges(start,G).second; ++e)
			dfs(G,source(*e,G),D,d+1);
}

int main(int argc,char **argv)
{
	TetraMesh M("/home/jcassidy/src/FullMonteSW/data/mouse.mesh",TetraMesh::MatlabTP);

	TetraGraph G(M);

	dfs_trivial_vis vis(G);

	std::vector<boost::default_color_type> C(num_vertices(G));

	//unsigned
	//boost::property_map<TetraGraph, unsigned>::type vertex_id = get(boost::vertex_index(), G);

	boost::iterator_property_map<
		std::vector<boost::default_color_type>::iterator,
		boost::typed_identity_property_map<unsigned>,
		boost::default_color_type,
		boost::default_color_type&> imap(C.begin(),boost::typed_identity_property_map<unsigned>());

	cout << "Crude DFS: " << endl;

	dfs(G,10000,2);

	boost::depth_first_search(G,vis,imap);

	cout << "Events: " << vis.Ne << endl;

	cout << "Visited: " << vis.N_visited() << " unvisited: " << vis.N_unvisited() << " (total " << vis.N_unvisited()+vis.N_visited() << ")" << endl;
}
