// slam3d - Frontend for graph-based SLAM
// Copyright (C) 2017 S. Kasperski
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SLAM_SENSOR_HPP
#define SLAM_SENSOR_HPP

#include "Types.hpp"

namespace slam3d
{	
	/**
	 * @class BadMeasurementType
	 * @brief Exception thrown when measurement types do not match.
	 * @details This can happen when a sensor is given a measurement that
	 * originates from a different sensor. In general the sensor is supposed
	 * to operate on its own measurements only. However special implementations
	 * could allow to register their readings with those from selected other
	 * sensors. This case should be documented in the specialization class.
	 */
	class BadMeasurementType: public std::exception
	{
	public:
		BadMeasurementType(){}
		virtual const char* what() const throw()
		{
			return "Measurement type does not match sensor type!";
		}
	};
	
	/**
	 * @class NoMatch
	 * @brief Exception thrown when two measurements could not be matched.
	 * @details This is not necessarily a problem if it happens once in a while.
	 * But when matching fails regularly, the mapping can fail and matching
	 * parameters should be changed by the user.
	 */
	class NoMatch: public std::exception
	{
	public:
		NoMatch(const std::string& msg):message(msg){}
		virtual ~NoMatch() throw() {}
		virtual const char* what() const throw()
		{
			return message.c_str();
		}
		
		std::string message;
	};
	
	class Mapper;
	class Graph;
	class Logger;
	
	/**
	 * @class Sensor
	 * @brief Abstract base class for a sensor used in the mapping process.
	 * @details The sensor is responsible for adding new measurements to the
	 * graph holding its measurements and for creating representations (maps)
	 * from all readings using the corrected poses from SLAM.
	 */
	class Sensor
	{
	public:
		Sensor(const std::string& n, Logger* l, const Transform& p)
		 :mMapper(NULL), mGraph(NULL), mLogger(l), mName(n), mSensorPose(p), mLastVertex(0){}
		virtual ~Sensor(){}
		
		/**
		 * @brief Set the mapper and graph that this sensor is used by.
		 * @param m Mapper
		 * @param g Graph
		 */
		void setMapperAndGraph(Mapper* m, Graph* g) { mMapper = m, mGraph = g; }
		
		/**
		 * @brief Get the sensor's name. The name is used to identify
		 * measurements that have been recorded by this sensor.
		 * @return name of the sensor
		 */
		std::string getName() const { return mName; }
		
		/**
		 * @brief Get the sensor's pose in robot coordinate frame.
		 * @return pose sensor pose in robot coordinates
		 */
		Transform getSensorPose() const { return mSensorPose; }
		
		/**
		 * @brief Calculate the estimated transform between two measurements of this sensor.
		 * @param source measurement of the source node
		 * @param target measurement of the target node
		 * @param odometry estimation of robot movement
		 * @param coarse whether to do a coarse estimate
		 * @throw BadMeasurementType
		 */
		virtual TransformWithCovariance calculateTransform(Measurement::Ptr source,
		                                                   Measurement::Ptr target,
		                                                   TransformWithCovariance odometry,
		                                                   bool coarse = false) const = 0;		
		/**
		 * @brief Creates a virtual measurement at the given pose from a set of vertices.
		 * @param vertices list of vertices that should contain measurements from this sensor
		 * @param pose origin of the virtual measurement
		 * @throw BadMeasurementType
		 */
		virtual Measurement::Ptr createCombinedMeasurement(const VertexObjectList& vertices, Transform pose) const = 0;
		
		/**
		 * @brief Set minimal change in pose between adjacent nodes.
		 * @param t Minimum translation between nodes (in meter).
		 * @param r Minimum rotation between nodes (in rad).
		 */
		void setMinPoseDistance(float t, float r){ mMinTranslation = t; mMinRotation = r; }
		
		/**
		 * @brief Checks the given transformation using the previously set
		 * translation and rotation thresholds.
		 * @param t Relative motion since the last scan was added.
		 * @return Whether a new scan should be added.
		 */
		bool checkMinDistance(const Transform &t)
		{
			ScalarType rot = Eigen::AngleAxis<ScalarType>(t.rotation()).angle();
			ScalarType trans = t.translation().norm();
			if(trans < mMinTranslation && std::abs(rot) < mMinRotation)
				return false;
			else
				return true;
		}

	protected:
		Mapper* mMapper;
		Graph* mGraph;
		Logger* mLogger;

		std::string mName;
		Transform mSensorPose;
		IdType mLastVertex; // This is the last vertex from THIS sensor!
		
		float mMinTranslation;
		float mMinRotation;
	};
	
	typedef std::map<std::string, Sensor*> SensorList;
}

#endif