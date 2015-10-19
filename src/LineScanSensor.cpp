#include "LineScanSensor.hpp"

using namespace slam;

LineScanSensor::LineScanSensor(const std::string& n, Logger* l, const Transform& p)
 : Sensor(n, l, p)
{
	
}

LineScanSensor::~LineScanSensor()
{
	
}

TransformWithCovariance LineScanSensor::calculateTransform(Measurement* source, Measurement* target, Transform odometry) const
{
	// Transform guess in sensor frame
	Transform guess = source->getInverseSensorPose() * odometry * target->getSensorPose();
	
	// Cast to this sensors measurement type
	LineScanMeasurement* sourceScan = dynamic_cast<LineScanMeasurement*>(source);
	LineScanMeasurement* targetScan = dynamic_cast<LineScanMeasurement*>(target);
	if(!sourceScan || !targetScan)
	{
		mLogger->message(ERROR, "Measurement given to calculateTransform() is not a LineScan!");
		throw BadMeasurementType();
	}
	
	// Transform by odometry
	LineScan::Ptr shifted_target(new LineScan);
	pcl::transformPointCloud(*targetScan->getScan(), *shifted_target, guess.matrix());
	
	// Estimate Transformation
	ScanMatcher::Matrix4 match_result;
	mScanMatcher.estimateRigidTransformation(*shifted_target, *sourceScan->getScan(), match_result);
	
	Transform result = Transform(match_result) * guess;
	
	TransformWithCovariance twc;
	twc.transform = source->getSensorPose() * result * target->getInverseSensorPose();
	twc.covariance = Covariance::Identity();
	return twc;
}