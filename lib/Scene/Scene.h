#ifndef SCENE_H
#define SCENE_H

#include "../Camera.h"
#include "../Common.h"
#include "../Hittable/Bounding/BVH.h"
#include "../Hittable/Hit.h"
#include "../Hittable/HittableList.h"
#include "../Image.h"
#include "../ProgressIndicator.h"
#include "../Ray.h"
#include "../Vec3.h"

#include "unistd.h"

#include <cfloat>
#include <atomic>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <utility>
#include <vector>

struct Scene {
  HittableList objects;
  BVH bvh;

  Scene() = default;
  explicit Scene(const std::vector<std::shared_ptr<Hittable>> &hittables)
      : objects(hittables), bvh(objects) {}

  void pushHittable(const std::shared_ptr<Hittable>& hittable) {
    objects.pushHittable(hittable);
    bvh = BVH(objects);
  }
  void loadHittable(const std::shared_ptr<Hittable>& hittable) {
    objects.loadHittable(hittable);
    bvh = BVH(objects);
  }
  void loadHittables(const std::vector<std::shared_ptr<Hittable>>& hittables) {
    objects.loadHittables(hittables);
    bvh = BVH(objects);
  }

  // Recursively scatter the ray, depth limited by bouncesLeft
  [[nodiscard]] Color3 getPixelColor(const Ray &ray, const int bouncesLeft) const {
    if (bouncesLeft <= 0)
      return {0, 0, 0};

    Hit hit;
    if (bvh.hit(ray, hit, 0.001, DBL_MAX)) {
      Color3 attenuation;
      Ray out;
      hit.material->scatter(ray, hit, attenuation, out);
      return getPixelColor(out, bouncesLeft - 1) * attenuation;
    }

    Vec3 unitDirection = ray.direction.unit();
    double t = (unitDirection.y + 1.0) / 2;

    Color3 color = Color3(1, 1, 1) * (1.0 - t) + Color3(0.5, 0.7, 1.0) * t;
    return color;
  }

  void render(Camera &camera, Image &image, const int threads = 19) const {
    std::cout << "Beginning render\n";
    std::vector<std::vector<std::pair<int, int>>> locations(threads);
    int curThread = 0;
    for (int row = 0; row < image.height; row++)
      for (int col = 0; col < image.width; col++) {
        locations[curThread].emplace_back(row, col);
        curThread = (curThread + 1) % threads;
      }

    std::atomic<int> progress = 0;
    int entries = image.width * image.height;
    ProgressIndicator progressIndicator(entries);

    auto getJob = [&](int threadIdx) {
      for (auto [row, col] : locations[threadIdx]) {
        Color3 color(0, 0, 0);
        for (int sample = 0; sample < image.samples; sample++) {
          double dx = common::randomDouble();
          double dy = common::randomDouble();
          double x = (col + dx) / image.width;
          double y = (row + dy) / image.height;
          Ray ray = camera.getRay(x, y);
          color += getPixelColor(ray, image.bounces);
        }

        image.setPixel(row, col, color);
        progress++;
      }
    };

    std::vector<std::thread> jobs;
    for (int thread = 0; thread < threads; thread++)
      jobs.emplace_back(getJob, thread);

    while (progress < entries) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      progressIndicator.indicate(progress);
    }
    progressIndicator.done();

    for (auto &job : jobs)
      job.join();

    image.write();
  }
};

#endif
