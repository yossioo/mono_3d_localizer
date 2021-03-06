cmake_minimum_required(VERSION 2.8.3)
project(orb_scan_localizer)

find_package(catkin REQUIRED COMPONENTS
        COMPONENTS
        roscpp
        tf
        sensor_msgs
        image_transport
        cv_bridge
        message_generation
        std_msgs
        pcl_ros
        pcl_conversions
        icp_7dof
)

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)

# Force using C++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")

find_package(Boost REQUIRED COMPONENTS system serialization python)
find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(BLAS REQUIRED)
find_package(LAPACK REQUIRED)
find_package(GLUT REQUIRED)
find_package(OpenMP)

find_package(Eigen3 QUIET)

if (NOT EIGEN3_FOUND)
    # Fallback to cmake_modules
    find_package(cmake_modules REQUIRED)
    find_package(Eigen REQUIRED)
    set(EIGEN3_INCLUDE_DIRS ${EIGEN_INCLUDE_DIRS})
    set(EIGEN3_LIBRARIES ${EIGEN_LIBRARIES})  # Not strictly necessary as Eigen is head only
    # Possibly map additional variables to the EIGEN3_ prefix.
else ()
    set(EIGEN3_INCLUDE_DIRS ${EIGEN3_INCLUDE_DIR})
endif ()

find_package(PCL 1.7 REQUIRED COMPONENTS common octree)
find_package(PythonLibs REQUIRED)
find_package(X11 REQUIRED)

add_message_files(
        FILES
        debug.msg
)

generate_messages(
        DEPENDENCIES
        std_msgs
)

catkin_package(
        INCLUDE_DIRS include ${EIGEN3_INCLUDE_DIRS}
        CATKIN_DEPENDS message_runtime icp_7dof
        DEPENDS OpenCV GLEW PCL
)

# Vocabulary file
set(orb_slam_vocabulary_file ${CATKIN_DEVEL_PREFIX}/share/${PROJECT_NAME}/ORBvoc.txt)
add_definitions(
        -DORB_SLAM_VOCABULARY="${orb_slam_vocabulary_file}"
)

# For building Pangolin
#if (NOT TARGET pangolin)
add_subdirectory(Thirdparty/Pangolin)

# For building DBoW2
add_subdirectory(Thirdparty/DBoW2)

# For building g2o
add_subdirectory(Thirdparty/g2o)
#endif()

include_directories(
        # my own
        include
        ${catkin_INCLUDE_DIRS}

        # Important libraries
        ${OpenCV_INCLUDE_DIRS}
        ${PCL_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${PYTHON_INCLUDE_DIR}
        ${Eigen3_INCLUDE_DIRS}

        # Third parties
        Thirdparty
        Thirdparty/g2o
        Thirdparty/Pangolin/include
)


add_library(
        orig_orb_slam2 STATIC
        src/System.cc
        src/Tracking.cc
        src/LocalMapping.cc
        src/ORBextractor.cc
        src/ORBmatcher.cc
        src/FrameDrawer.cc
        src/Converter.cc
        src/MapPoint.cc
        src/KeyFrame.cc
        src/Map.cc
        src/MapDrawer.cc
        src/Optimizer.cc
        src/PnPsolver.cc
        src/Frame.cc
        src/KeyFrameDatabase.cc
        src/Sim3Solver.cc
        src/Initializer.cc
        src/Viewer.cc
        src/MapPublisher.cc
        src/MapTracking.cc
)

if (OPENMP_FOUND)
    set_target_properties(orig_orb_slam2 PROPERTIES
            COMPILE_FLAGS ${OpenMP_CXX_FLAGS}
            LINK_FLAGS ${OpenMP_CXX_FLAGS}
            )
endif ()

if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif ()


list(APPEND LINK_LIBRARIES
        boost_system
        boost_serialization
        ${catkin_LIBRARIES}
        ${OpenCV_LIBS}
        ${X11_LIBRARIES}
        ${OPENGL_LIBRARIES}
        ${GLEW_LIBRARIES}
        ${PCL_LIBRARIES}
        ${GLUT_LIBRARIES}
        )

list(APPEND ORIG_ORB_BIN_LINKS
        orig_orb_slam2
        DBoW2
        g2o
        pangolin
        )

#add_executable(
#        orig_mono_orb_slam2
#        nodes/mono_orb_slam2/mono_orb_slam2.cpp
#)

# add_dependencies(orig_mono_orb_slam2 ${catkin_EXPORTED_TARGETS})

#target_link_libraries(
#        orig_mono_orb_slam2
#        ${ORIG_ORB_BIN_LINKS}
#        ${LINK_LIBRARIES}
#)

#add_executable(
#        orig_mono_orb_slam2_no
#        nodes/mono_orb_slam2/mono_orb_slam2_no.cpp
#)

# add_dependencies(orig_mono_orb_slam2_no ${catkin_EXPORTED_TARGETS})

#target_link_libraries(
#        orig_mono_orb_slam2_no
#        ${ORIG_ORB_BIN_LINKS}
#        ${LINK_LIBRARIES}
#        ${catkin_INCLUDE_DIRS}
#)

add_executable(
        scan_mono_orb_slam2
        nodes/mono_orb_slam2/scan_mono_orb_slam2.cpp
)

target_link_libraries(
        scan_mono_orb_slam2
        ${ORIG_ORB_BIN_LINKS}
        ${LINK_LIBRARIES}
)

add_executable(
        scan_mono_orb_slam2_no
        nodes/mono_orb_slam2/scan_mono_orb_slam2_no.cpp
)

target_link_libraries(
        scan_mono_orb_slam2_no
        ${ORIG_ORB_BIN_LINKS}
        ${LINK_LIBRARIES}
       ${catkin_INCLUDE_DIRS}
)

add_executable(
        scan_mono_orb_slam2_odom
        nodes/mono_orb_slam2/scan_mono_orb_slam2_odom.cpp
)

target_link_libraries(
        scan_mono_orb_slam2_odom
        ${ORIG_ORB_BIN_LINKS}
        ${LINK_LIBRARIES}
#        ${catkin_INCLUDE_DIRS}
)

#add_dependencies(scan_mono_orb_slam2_odom ${catkin_EXPORTED_TARGETS})

add_executable(
        scan_mono_orb_slam2_odom_no
        nodes/mono_orb_slam2/scan_mono_orb_slam2_odom_no.cpp
)

target_link_libraries(
        scan_mono_orb_slam2_odom_no
        ${ORIG_ORB_BIN_LINKS}
        ${LINK_LIBRARIES}
#        ${catkin_INCLUDE_DIRS}
)


add_executable(
        icp_solver_7dof
        src/icp_solver_7dof.cpp
)

target_link_libraries(
        icp_solver_7dof
        ${ORIG_ORB_BIN_LINKS}
        ${LINK_LIBRARIES}
        ${catkin_INCLUDE_DIRS}
)

add_dependencies(icp_solver_7dof ${catkin_EXPORTED_TARGETS})

