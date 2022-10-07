#ifndef SCENE_H
#define SCENE_H

#include "./Hittable/Hittable.h"
#include "Camera.h"
#include "Hit.h"
#include "Image.h"
#include "Ray.h"
#include "Vec3.h"

#include <vector>

template <typename T> struct Scene {
  using Hittable = Hittable<T>;
  using Camera = Camera<T>;
  using Hit = Hit<T>;
  using Ray = Ray<T>;
  using Vec3 = Vec3<T>;
  using Color3 = Color3<T>;

  std::vector<Hittable *> hittables;

  Scene() {}

  void pushHittable(Hittable *h) { hittables.push_back(h); }
  void loadHittable(Hittable *h) { hittables = {h}; }
  void loadHittables(std::vector<Hittable *> hs) { hittables = hs; }

  Color3 getPixelColor(const Ray &r) const {
    Hit out, closest;
    bool found = false;
    for (Hittable *h : hittables)
      if (h->rayHit(r, out) && out < closest) {
        closest = out;
        found = true;
      }

    if (found)
      return (Color3(closest.normal) + 1) / 2;

    Vec3 unitDir = r.dir.unit();
    double t = (unitDir.y + 1.0) / 2;

    Color3 color = Color3(1, 1, 1) * (1.0 - t) + Color3(0.5, 0.7, 1.0) * t;
    return color;
  }

  void render(Camera &camera, Image &image) {
    for (int row = 0; row < image.height; row++) {
      for (int col = 0; col < image.width; col++) {
        double x = (double)col / image.width;
        double y = (double)row / image.height;

        Ray r = camera.getRay(x, y);
        Color3 color = getPixelColor(r);
        image.pushPixel(color * 255);
      }
    }

    image.write();
  }
};

#endif