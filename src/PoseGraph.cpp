#include "PoseGraph.hpp"

using namespace slam;

PoseGraph::PoseGraph()
{
	mNextVertexId = 0;
	mNextEdgeId = 0;
}

PoseGraph::~PoseGraph()
{
	
}

Vertex PoseGraph::addVertex(const VertexObject& object)
{
	Vertex n = boost::add_vertex(mGraph);
	mGraph[n] = object;
	mGraph[n].id = mNextVertexId;
	boost::put(boost::vertex_index_t(), mGraph, n, mNextVertexId);
	mNextVertexId++;
	return n;
}

Edge PoseGraph::addEdge(Vertex source, Vertex target, const EdgeObject& object)
{
	std::pair<Edge, bool> result = boost::add_edge(source, target, mGraph);
	Edge n = result.first;
	mGraph[n] = object;
	boost::put(boost::edge_index_t(), mGraph, n, mNextEdgeId);
	mNextEdgeId++;
	return n;
}

void PoseGraph::removeVertex(Vertex v)
{
	boost::clear_vertex(v, mGraph);
	boost::remove_vertex(v, mGraph);
}

void PoseGraph::removeEdge(Edge e)
{
	boost::remove_edge(e, mGraph);
}

AdjacencyRange PoseGraph::getAdjacentVertices(Vertex v)
{
	return boost::adjacent_vertices(v, mGraph);
}

Measurement* PoseGraph::getMeasurement(Vertex v)
{
	return mGraph[v].measurement;
}

void PoseGraph::dumpGraphViz(std::ostream& out)
{
	boost::write_graphviz(out, mGraph);
}
