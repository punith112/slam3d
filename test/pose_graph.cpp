#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE slam3d

#include <boost/test/unit_test.hpp>
#include <PoseGraph.hpp>
#include <G2oSolver.hpp>

#include <iostream>

BOOST_AUTO_TEST_CASE(pose_graph_1)
{
	slam::Clock clock;
	slam::Logger logger(clock);
	
	slam::PoseGraph graph;
	slam::Measurement m1(1, clock.now(), "Sensor");
	slam::Measurement m2(2, clock.now(), "Sensor");
	slam::Measurement m3(3, clock.now(), "Sensor");

	// Create the vertices
	slam::VertexObject v_obj;
	v_obj.odometric_pose = Eigen::Translation<double, 3>(0,0,0);
	v_obj.corrected_pose = Eigen::Translation<double, 3>(0,0,0);	

	v_obj.measurement = &m1;
	slam::Vertex v1 = graph.addVertex(v_obj);

	v_obj.measurement = &m2;
	slam::Vertex v2 = graph.addVertex(v_obj);

	v_obj.measurement = &m3;
	slam::Vertex v3 = graph.addVertex(v_obj);
	
	// Create the edges
	slam::EdgeObject e_obj;
	e_obj.covariance = slam::Covariance::Identity();
	
	e_obj.transform = Eigen::Translation<double, 3>(1,0,0);
	graph.addEdge(v1, v2, e_obj);
	
	e_obj.transform = Eigen::Translation<double, 3>(1,0,0);
	graph.addEdge(v2, v3, e_obj);
	
	e_obj.transform = Eigen::Translation<double, 3>(-0.8, -0.7, 0.2);
	graph.addEdge(v3, v1, e_obj);
	
	BOOST_CHECK(graph.getMeasurement(v1) == &m1);
	BOOST_CHECK(graph.getMeasurement(v2) == &m2);
	BOOST_CHECK(graph.getMeasurement(v3) == &m3);
	
	BOOST_CHECK(graph.getMeasurement(v1) != &m2);
	BOOST_CHECK(graph.getMeasurement(v1) != &m3);
	
	// Test file output
	std::ofstream file;
	file.open("construction_test.dot");
	graph.dumpGraphViz(file);
	file.close();
	
	// Optimize the graph
	slam::Solver* solver = new slam::G2oSolver(&logger);
	graph.optimize(solver);

}

BOOST_AUTO_TEST_CASE(g2o_solver_1)
{
	slam::Clock clock;
	slam::Logger logger(clock);
	slam::G2oSolver* solver = new slam::G2oSolver(&logger);
	
	// Create the nodes
	slam::VertexObject v1, v2, v3;
	v1.id = 1;
	v2.id = 2;
	v3.id = 3;
	v1.corrected_pose = Eigen::Translation<double, 3>(0,0,0);
	v2.corrected_pose = Eigen::Translation<double, 3>(0,0,0);
	v3.corrected_pose = Eigen::Translation<double, 3>(0,0,0);
	solver->addNode(v1);
	solver->addNode(v2);
	solver->addNode(v3);
	BOOST_CHECK_THROW(solver->addNode(v3), slam::DuplicateVertex);

	// Create the edges
	slam::EdgeObject e1,e2, e3;
	e1.transform = Eigen::Translation<double, 3>(1,0,0);
	e2.transform = Eigen::Translation<double, 3>(0,1,0);
	e3.transform = Eigen::Translation<double, 3>(-0.8, -0.7, 0.2);
	e1.covariance = slam::Covariance::Identity();
	e2.covariance = slam::Covariance::Identity();
	e3.covariance = slam::Covariance::Identity();
	solver->addConstraint(e1, v1.id, v2.id);
	solver->addConstraint(e2, v2.id, v3.id);
	solver->addConstraint(e3, v3.id, v1.id);
	BOOST_CHECK_THROW(solver->addConstraint(e3, 3,4), slam::BadEdge);
	
	// Start the optimization
	solver->setFixed(v1.id);
	BOOST_CHECK_THROW(solver->setFixed(99), slam::UnknownVertex);
	
	solver->compute();
}