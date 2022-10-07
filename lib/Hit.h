#ifndef HIT_H
#define HIT_H

#include "Vec3.h"

#include "float.h"

template <typename T> struct Hit {
  using Vec3 = Vec3<T>;
  using Point3 = Point3<T>;

  Point3 loc;
  Vec3 normal;
  double t;

  Hit() : t(DBL_MAX) {}
  Hit(Point3 &loc, Vec3 &normal, double t) : loc(loc), normal(normal), t(t) {}

  bool operator<(const Hit &o) const { return t < o.t; }
};

#endif