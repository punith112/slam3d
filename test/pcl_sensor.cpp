#define BOOST_TEST_MODULE "PclSensorTest"

#include <PointCloudSensor.hpp>
#include <FileLogger.hpp>

#include <iostream>
#include <fstream>
#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>
#include <pcl/common/transforms.h>

using namespace slam3d;

PointCloud::Ptr loadFromFile(const std::string& filename)
{
	PointCloud::Ptr cloud(new PointCloud);


	// allocate 4 MB buffer (only ~130*4*4 KB are needed) and read in the file
	int32_t num = 1000000;
	float* data = new float[num];	
	
	// load point cloud
	FILE *stream = fopen(filename.c_str(),"rb");
	assert(stream);
	int32_t points = fread(data,sizeof(float),num,stream)/4;
	fclose(stream);
	
	 // pointers to coordiantes
	float *px = data+0;
	float *py = data+1;
	float *pz = data+2;
	float *pr = data+3;
	
	for(int32_t i = 0; i < points; i++)
	{
		PointType point;
		point.x = *px;
		point.y = *py;
		point.z = *pz;
//		point.intensity = *pr;
		cloud->push_back(point);
		px+=4; py+=4; pz+=4; pr+=4;
	}
	
	return cloud;
}

BOOST_AUTO_TEST_CASE(icp)
{
	Clock clock;
	FileLogger logger(clock, "pcl_sensor.log");
	logger.setLogLevel(DEBUG);
	
	Transform sensor_pose = Transform::Identity();
	PointCloudSensor pclSensor("TestPclSensor", &logger, sensor_pose);
	
	GICPConfiguration conf;
	conf.max_correspondence_distance = 2.0;
	conf.maximum_iterations = 200;
	pclSensor.setFineConfiguaration(conf);
	
	// How to access these files properly?
	PointCloud::Ptr cloud1 = loadFromFile("../test/cloud1.bin");
	PointCloud::Ptr cloud2 = loadFromFile("../test/cloud2.bin");
	PointCloud::Ptr cloud3 = loadFromFile("../test/cloud3.bin");
	PointCloud::Ptr cloud4 = loadFromFile("../test/cloud4.bin");
	
	PointCloudMeasurement::Ptr m1(new PointCloudMeasurement(cloud1, "r1", "pcl_sensor", sensor_pose));
	PointCloudMeasurement::Ptr m2(new PointCloudMeasurement(cloud2, "r1", "pcl_sensor", sensor_pose));
	PointCloudMeasurement::Ptr m3(new PointCloudMeasurement(cloud3, "r1", "pcl_sensor", sensor_pose));
	PointCloudMeasurement::Ptr m4(new PointCloudMeasurement(cloud4, "r1", "pcl_sensor", sensor_pose));

	Transform pose1 = Transform::Identity();
	
	Transform guess(Eigen::Translation<double, 3>(0.5,0,0));
	Measurement::Ptr m(new Measurement());
	BOOST_CHECK_THROW(pclSensor.calculateTransform(m1, m, guess), BadMeasurementType);
	
	TransformWithCovariance twc1 = pclSensor.calculateTransform(m1, m2, guess);
	Transform pose2 = twc1.transform * pose1;

//	logger.message(DEBUG, (boost::format("Estimated translation:\n %1%") % twc.transform.translation()).str());
//	logger.message(DEBUG, (boost::format("Estimated rotation:\n %1%") % twc.transform.rotation()).str());

	TransformWithCovariance twc2 = pclSensor.calculateTransform(m2, m3, guess);
	Transform pose3 = twc2.transform * pose2;
	
	TransformWithCovariance twc3 = pclSensor.calculateTransform(m3, m4, guess);
	Transform pose4 = twc3.transform * pose3;
	
	logger.message(INFO, (boost::format("Pose1:\n %1%") % pose1.translation()).str());
	logger.message(INFO, (boost::format("Pose2:\n %1%") % pose2.translation()).str());
	logger.message(INFO, (boost::format("Pose3:\n %1%") % pose3.translation()).str());
	logger.message(INFO, (boost::format("Pose4:\n %1%") % pose4.translation()).str());

	// Now calculate difference between pose 2 and 4
	logger.message(INFO, "Estimating 1 -> 3");
	guess = pose3 * pose1.inverse();
	pclSensor.calculateTransform(m1, m3, guess);
	
	logger.message(INFO, "Estimating 2 -> 3");
	guess = pose3 * pose2.inverse();
	pclSensor.calculateTransform(m2, m3, guess);
	
	logger.message(INFO, "Estimating 1 -> 4");
	guess = pose4 * pose1.inverse();
	pclSensor.calculateTransform(m1, m4, guess);
	
	logger.message(INFO, "Estimating 2 -> 4");
	guess = pose4 * pose2.inverse();
	pclSensor.calculateTransform(m2, m4, guess);


	// Test translation
	Transform tx(Eigen::Translation<double, 3>(1,0,0));
	Transform tx_inv(Eigen::Translation<double, 3>(-1,0,0));
	PointCloud::Ptr translated_cloud1(new PointCloud);
	pcl::transformPointCloud(*cloud1, *translated_cloud1, tx_inv.matrix());

	logger.message(INFO, "Test translation without estimation");
	PointCloudMeasurement::Ptr m1_tx(new PointCloudMeasurement(translated_cloud1, "r1", "pcl_sensor", sensor_pose));
	pclSensor.calculateTransform(m1, m1_tx, Transform::Identity());
	
	logger.message(INFO, "Test translation with estimation");
	pclSensor.calculateTransform(m1, m1_tx, tx);

	// test rotation
	Transform rx(Eigen::AngleAxisd(0.1, Eigen::Vector3d(1, 0, 0)));
	Transform rx_inv(Eigen::AngleAxisd(0.1, Eigen::Vector3d(-1, 0, 0)));
	PointCloud::Ptr rotated_cloud1(new PointCloud);
	pcl::transformPointCloud(*cloud1, *rotated_cloud1, rx_inv.matrix());
	
	logger.message(INFO, "Test rotarion without estimation");
	PointCloudMeasurement::Ptr m1_rx(new PointCloudMeasurement(rotated_cloud1, "r1", "pcl_sensor", sensor_pose));
	pclSensor.calculateTransform(m1, m1_rx, Transform::Identity());
	
	logger.message(INFO, "Test rotation with estimation");
	pclSensor.calculateTransform(m1, m1_rx, rx);
}