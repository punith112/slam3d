#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#define BOOST_TEST_MODULE slam3d

#include <boost/test/unit_test.hpp>
#include <PoseGraph.hpp>
#include <G2oSolver.hpp>

#include <iostream>

BOOST_AUTO_TEST_CASE(pose_graph_1)
{
	slam::Measurement m1(1);
	slam::Measurement m2(2);
	slam::Measurement m3(3);
	slam::PoseGraph graph;
	slam::Vertex v1 = graph.addVertex(&m1);
	slam::Vertex v2 = graph.addVertex(&m2);
	slam::Vertex v3 = graph.addVertex(&m3);
	
	slam::Edge e1 = graph.addEdge(v1, v2);
	slam::Edge e2 = graph.addEdge(v2, v3);
	slam::Edge e3 = graph.addEdge(v3, v1);
	
	BOOST_CHECK(graph.getMeasurement(v1) == &m1);
	BOOST_CHECK(graph.getMeasurement(v2) == &m2);
	BOOST_CHECK(graph.getMeasurement(v3) == &m3);
	
	BOOST_CHECK(graph.getMeasurement(v1) != &m2);
	BOOST_CHECK(graph.getMeasurement(v1) != &m3);
	
	slam::AdjacencyRange range = graph.getAdjacentVertices(v2);
	
	std::ofstream file;
	file.open("construction_test.dot");
	graph.dumpGraphViz(file);
	file.close();
}

BOOST_AUTO_TEST_CASE(g2o_solver_1)
{
	try
	{
		slam::G2oSolver* solver = new slam::G2oSolver();
	}
	catch(std::exception &e)
	{
		BOOST_ERROR(e.what());
	}
	slam::VertexObject v1, v2, v3;
	slam::EdgeObject e1,e2, e3;
	solver->addNode(v1, 1);
	solver->addNode(v2, 2);
	solver->addNode(v3, 3);
	
	solver->addConstraint(e1, 1,2);
	solver->addConstraint(e2, 2,3);
	BOOST_CHECK_THROW(solver.addConstraint(e3, 3,4), slam::BadEdge);
	
	delete solver;
}