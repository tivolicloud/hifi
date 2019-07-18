// "WaveVR SDK
// © 2017 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#ifndef WVR_HELLOVR_RAY_H
#define WVR_HELLOVR_RAY_H

#include <iostream>
#include <cmath>
#include <limits>
#include <Vectors.h>
#include <log.h>

struct Vector3D {
  float x,y,z;

  Vector3D(float x, float y, float z):x(x),y(y),z(z){
  }

  float dot(Vector3D const &v) const {
    return x*v.x+y*v.y+z*v.z;
  }
};

struct Point3D {
  float x,y,z;

  Point3D(float x, float y, float z):x(x),y(y),z(z){
  }
};

Vector3D operator-(Point3D const &p1, Point3D const &p2){
  return Vector3D(p1.x-p2.x,p1.y-p2.y,p1.z-p2.z);
}

struct Ray3D {
  Point3D p;
  Vector3D v;
  Vector3 collision;
  Ray3D(Point3D const &p, Vector3D const &v, Vector3 const collision):p(p),v(v),collision(collision){
  }
};

struct Sphere3D {
  Point3D center;
  float radius;

  Sphere3D(Point3D const &center, float radius):center(center),radius(radius){
  }
};

bool intersection(Ray3D &r, Sphere3D const &s){
  Vector3D w = r.p - s.center;
  float A = r.v.dot(r.v);
  float B = 2*w.dot(r.v);
  float C = w.dot(w) - s.radius*s.radius;
  double t, epsilon = 1e-4;

  float D = (B*B-4.0f*A*C);
  if (D < 0){
  return false;
  }else{
    Vector3 ori(r.p.x,r.p.y,r.p.z);
    Vector3 dir(r.v.x,r.v.y,r.v.z);
    t=(t = (-B - sqrtf(D)) * 0.5 / A) > epsilon ? t : ((t = (-B + sqrtf(D)) * 0.5 / A) > epsilon ? t : 0.0);
    Vector3 t_first=(ori+(dir*t));
//    LOGD("RayInter Start");
//    LOGD("RayInter Intersection First intersection: %f , %f ,%f ",t_first.x,t_first.y,t_first.z);
    r.collision.set(t_first.x,t_first.y,t_first.z);
    float t1_v=((-B+sqrtf(D))*0.5f)/A;
    float t2_v=((-B-sqrtf(D))*0.5f)/A;
    Vector3 t1(ori+(dir*t1_v));
    Vector3 t2(ori+(dir*t2_v));
//    LOGD("RayInter Intersection Point 1: %f , %f ,%f ",t1.x,t1.y,t1.z);
//    LOGD("RayInter Intersection Point 2: %f , %f ,%f ",t2.x,t2.y,t2.z);
//    LOGD("RayInter End");
    if(t_first==t1 || t_first==t2) // make sure the ray's direction forward to sphere
      return true;
    else
      return false;
  }
}

#endif //WVR_HELLOVR_RAY_H