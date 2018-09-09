// g2o - General Graph Optimization
// Copyright (C) 2011 R. Kuemmerle, G. Grisetti, W. Burgard
// All rights reserved.
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

#include "robot_laser.h"

#include "g2o/stuff/macros.h"

#include <iomanip>
using namespace std;

namespace g2o {

  RobotLaser::RobotLaser() :
    RawLaser(),
    _laserTv(0.), _laserRv(0.), _forwardSafetyDist(0.), _sideSaftyDist(0.), _turnAxis(0.)
  {
  }

  RobotLaser::~RobotLaser()
  {
  }

  bool RobotLaser::read(std::istream& is)
  {
    int type;
    double angle, fov, res, maxrange, acc;
    int remission_mode;
    is >> type >> angle >> fov >> res >> maxrange >> acc >> remission_mode;

    int beams;
    is >> beams;
    _laserParams = LaserParameters(type, beams, angle, res, maxrange, acc, remission_mode);
    _ranges.resize(beams);
    for (int i=0; i<beams; i++)
      is >> _ranges[i];

    is >> beams;
    _remissions.resize(beams);
    for (int i = 0; i < beams; i++)
      is >> _remissions[i];

    // special robot laser stuff
    double x,y,theta;
    is >> x >> y >> theta;
    SE2 lp(x,y,theta);
    //cerr << "x: " << x << " y:" << y << " th:" << theta << " ";
    is >> x >> y >> theta;
    //cerr << "x: " << x << " y:" << y << " th:" << theta;
    _odomPose = SE2(x,y,theta);
    _laserParams.laserPose = _odomPose.inverse()*lp;
    is >> _laserTv >>  _laserRv >>  _forwardSafetyDist >> _sideSaftyDist >> _turnAxis;

    // timestamp + host
    is >> _timestamp;
    is >> _hostname;
    is >> _loggerTimestamp;
    return true;
  }

  bool RobotLaser::write(std::ostream& os) const
  {
    os << _laserParams.type << " " << _laserParams.firstBeamAngle << " " << _laserParams.fov << " "
      << _laserParams.angularStep << " " << _laserParams.maxRange << " " << _laserParams.accuracy << " "
      << _laserParams.remissionMode << " ";
    os << ranges().size();
    for (size_t i = 0; i < ranges().size(); ++i)
      os << " " << ranges()[i];
    os << " " << _remissions.size();
    for (size_t i = 0; i < _remissions.size(); ++i)
      os << " " << _remissions[i];

    // odometry pose
    Vector3D p = (_odomPose * _laserParams.laserPose).toVector();
    os << " " << p.x() << " " << p.y() << " " << p.z();
    p = _odomPose.toVector();
    os << " " << p.x() << " " << p.y() << " " << p.z();

    // crap values
    os << FIXED(" " <<  _laserTv << " " <<  _laserRv << " " << _forwardSafetyDist << " "
        << _sideSaftyDist << " " << _turnAxis);
    os << FIXED(" " << timestamp() << " " << hostname() << " " << loggerTimestamp());

    return os.good();
  }

  void RobotLaser::setOdomPose(const SE2& odomPose)
  {
    _odomPose = odomPose;
  }
}
