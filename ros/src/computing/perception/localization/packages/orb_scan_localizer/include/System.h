/**
* This file is part of ORB-SLAM2.
*
* Copyright (C) 2014-2016 Raúl Mur-Artal <raulmur at unizar dot es> (University of Zaragoza)
* For more information see <https://github.com/raulmur/ORB_SLAM2>
*
* ORB-SLAM2 is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ORB-SLAM2 is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ORB-SLAM2. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <thread>
#include <mutex>
#include <opencv2/core/core.hpp>

#include "Tracking.h"
#include "MapTracking.h"
#include "FrameDrawer.h"
#include "MapDrawer.h"
#include "Map.h"
#include "LocalMapping.h"
#include "KeyFrameDatabase.h"
#include "ORBVocabulary.h"
#include "Viewer.h"

#include <ros/ros.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl_ros/point_cloud.h>
#include <pcl/point_types.h>
#include <sensor_msgs/PointCloud2.h>
#include <icp_7dof/voxel_grid.h>
#include <icp_7dof/icp_7dof_correspondence_estimation.h>

using std::string;


namespace ORB_SLAM2
{

class Viewer;
class MapPublisher;
class FrameDrawer;
class Map;
class Tracking;
class MapTracking;
class LocalMapping;

class System
{
public:
    // Input sensor
    enum eSensor{
        MONOCULAR=0,
        STEREO=1,
        RGBD=2
    };

    enum operationMode {
    	MAPPING=0,
		LOCALIZATION=1
    };

public:

    // Initialize the SLAM system. It launches the Local Mapping, Loop Closing and Viewer threads.
    System (
    	const string &strVocFile,
  		const string &strSettingsFile,
  		const eSensor sensor,
  		const bool bUseViewer = true,
  		const string &mapFileName=string(),
  		const operationMode mode=System::MAPPING
    );

    // It launches the Local Mapping and Viewer threads.
    // Orb scan matching
    System (
    	const string &strVocFile,
  		const string &strSettingsFile,
  		const eSensor sensor,
      const ros::NodeHandle &node,
      const bool bUseMapPublisher,
      const bool is_publish,
  		const bool bUseViewer,
      const bool is_visualize,
  		const string &mapFileName=string(),
  		const operationMode mode=System::MAPPING
    );

    // Proccess the given monocular frame
    // Input images: RGB (CV_8UC3) or grayscale (CV_8U). RGB is converted to grayscale.
    // Returns the camera pose (empty if tracking fails).
    cv::Mat TrackMonocular(const cv::Mat &im, const double &timestamp);
    cv::Mat TrackMonocular(const cv::Mat &im, const double &timestamp, const bool is_publish);

    // This stops local mapping thread (map building) and performs only camera tracking.
    void ActivateLocalizationMode();
    // This resumes local mapping thread and performs SLAM again.
    void DeactivateLocalizationMode();

    // Reset the system (clear map)
    void Reset();

    // All threads will be requested to finish.
    // It waits until all threads have finished.
    // This function must be called before saving the trajectory.
    void Shutdown();

    // Save camera trajectory in the TUM RGB-D dataset format.
    // Call first Shutdown()
    // See format details at: http://vision.in.tum.de/data/datasets/rgbd-dataset
    void SaveTrajectoryTUM(const string &filename);

    // Save keyframe poses in the TUM RGB-D dataset format.
    // Use this function in the monocular case.
    // Call first Shutdown()
    // See format details at: http://vision.in.tum.de/data/datasets/rgbd-dataset
    void SaveKeyFrameTrajectoryTUM(const string &filename);

    // Save camera trajectory in the KITTI dataset format.
    // Call first Shutdown()
    // See format details at: http://www.cvlibs.net/datasets/kitti/eval_odometry.php
    void SaveTrajectoryKITTI(const string &filename);

    // TODO: Save/Load functions
    // LoadMap () should be called before grabbing image
    void SaveMap(const string &filename);
    void LoadMap(const string &filename);

    // Move retrievable setting here
    cv::FileStorage fsSettings;

    ros::NodeHandle monoNode;

    // 'get' resources
    Tracking* getTracker() { return mpTracker; }
    Map* getMap() { return mpMap; }
    LocalMapping* getLocalMapper() { return mpLocalMapper; }
    FrameDrawer* getFrameDrawer() { return mpFrameDrawer; }

    const operationMode opMode;

    float fps;
    bool isUseMapPublisher;

    icp_7dof::VoxelGrid voxel_grid_;
    pcl::IterativeClosestPoint7dof icp_;
    void SetSourceMap(pcl::PointCloud<pcl::PointXYZ>::Ptr priorMap);

private:

    // Input sensor
    eSensor mSensor;

    // ORB vocabulary used for place recognition and feature matching.
    ORBVocabulary* mpVocabulary;

    // KeyFrame database for place recognition (relocalization and loop detection).
    KeyFrameDatabase* mpKeyFrameDatabase;

    // Map structure that stores the pointers to all KeyFrames and MapPoints.
    Map* mpMap;
    const string &mapFileName;

    // Tracker. It receives a frame and computes the associated camera pose.
    // It also decides when to insert a new keyframe, create some new MapPoints and
    // performs relocalization if tracking fails.
    Tracking* mpTracker;
    MapTracking* mpMapTracker;

    // Local Mapper. It manages the local map and performs local bundle adjustment.
    LocalMapping* mpLocalMapper;

    // The viewer draws the map and the current camera pose. It uses Pangolin.
    Viewer* mpViewer;
    MapPublisher* mpMapPublisher;

    FrameDrawer* mpFrameDrawer;
    MapDrawer* mpMapDrawer;

    bool isUseViewer;

    // System threads: Local Mapping, Loop Closing, Viewer.
    // The Tracking thread "lives" in the main execution thread that creates the System object.
    std::thread* mptLocalMapping;
    std::thread* mptViewer;
    std::thread* mptMapPublisher;

    // Reset flag
    std::mutex mMutexReset;
    bool mbReset;

    // Change mode flags
    std::mutex mMutexMode;
    bool mbActivateLocalizationMode;
    bool mbDeactivateLocalizationMode;

    // Tracking state
    int mTrackingState;
    std::vector<MapPoint*> mTrackedMapPoints;
    std::vector<cv::KeyPoint> mTrackedKeyPointsUn;
    std::mutex mMutexState;
};

}// namespace ORB_SLAM

#endif // SYSTEM_H
