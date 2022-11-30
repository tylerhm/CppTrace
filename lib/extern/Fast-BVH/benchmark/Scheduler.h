#pragma once

#include <fstream>
#include <thread>
#include <vector>

namespace FastBVH {

namespace Benchmark {

//! This is the scheduler used for benchmarking the ray traversal.
//! \tparam Float The floating point type to generate rays with.
template <typename Float>
class Scheduler final {
  //! The image pixel data.
  std::vector<unsigned char> pixels;

  //! The width of the image to produce.
  std::size_t img_x_res;

  //! The height of the image to produce.
  std::size_t img_y_res;

  //! The position of the camera in world space.
  Vector3<Float> camera_position{1.6, 1.3, 1.6};

  //! Where the camera is looking at.
  Vector3<Float> camera_focus{0, 0, 0};

  //! The direction of 'up' in camera space.
  Vector3<Float> camera_up{0, 1, 0};

 public:
  //! Constructs a new scheduler.
  //! \param img_w The width of the image to produce.
  //! \param img_h The height of the image to produce.
  Scheduler(std::size_t img_w, std::size_t img_h) {
    pixels.resize(img_w * img_h * 3);
    img_x_res = img_w;
    img_y_res = img_h;
  }

  //! Moves the camera to a different location.
  //! \param pos The position, in world space, to move the camera to.
  void moveCamera(const Vector3<Float>& pos) { camera_position = pos; }

  //! Assigns the position that the camera is 'looking at'.
  //! \param pos The position to assign the camera to look at.
  void lookAt(const Vector3<Float>& pos) { camera_focus = pos; }

  //! Schedules rays to be traced.
  //! \tparam Tracer The type of the tracer kernel.
  //! \param tracer The tracer kernel that maps rays to colors.
  //! \param observer The observer instance to notify the progress with.
  template <typename Tracer, typename Observer>
  void schedule(Tracer tracer, Observer observer) {

    auto max_threads = std::thread::hardware_concurrency();

    for (std::size_t y = 0; y < img_y_res; y += max_threads) {

      observer(y, img_y_res);

      std::vector<std::thread> threads;

      for (std::size_t i = 0; i < max_threads; i++) {

        std::thread line_thread(&Scheduler<Float>::traceLine<Tracer>, this, tracer, y + i);

        threads.emplace_back(std::move(line_thread));
      }

      for (auto& thread : threads) {
        thread.join();
      }
    }

    observer(img_y_res, img_y_res);
  }

  //! Saves the result of the render operation to a PPM image file.
  //! \param filename The name of the file to save the results to.
  //! \return True on success, false on failure.
  bool saveResults(const char* filename) {
    std::ofstream file(filename);
    if (!file.good()) {
      return false;
    }

    file << "P6" << std::endl;
    file << img_x_res << std::endl;
    file << img_y_res << std::endl;
    file << "255" << std::endl;

    file.write((const char*)pixels.data(), pixels.size());

    return true;
  }

  //! Gets a copy of the pixel data.
  //! This is useful for comparing two images.
  //! \return A copy of the pixel data generated by the tracer.
  std::vector<unsigned char> copyPixelData() const {
    return pixels;
  }

protected:
  //! Traces a single line.
  //! \param tracer The tracer kernel to pass the rays to.
  //! \param y The index of the line to schedule rays for.
  //! A value of zero indicates the line in the top of the image.
  template <typename Tracer>
  void traceLine(Tracer tracer, std::size_t y) {

    // Camera tangent space
    Vector3<Float> camera_dir = normalize(camera_focus - camera_position);
    Vector3<Float> camera_u = normalize(cross(camera_dir, camera_up));
    Vector3<Float> camera_v = normalize(cross(camera_u, camera_dir));

    auto fov = Float(0.5f) / std::tan(70.f * 3.14159265 * .5f / 180.f);

    auto* pixel = pixels.data() + (y * img_x_res * 3);

    for (std::size_t x = 0; x < img_x_res; x++) {

      auto u = (x + 0.5f) / (Float)(img_x_res - 1) - 0.5f;
      auto v = (img_y_res - 1 - y + Float(0.5f)) / (Float)(img_y_res - 1) - 0.5f;

      // This is only valid for square aspect ratio images
      Ray<float> ray(camera_position, normalize(camera_u * u + camera_v * v + camera_dir * fov));

      auto color = tracer(ray);

      pixel[0] = color.x * 255;
      pixel[1] = color.y * 255;
      pixel[2] = color.z * 255;

      pixel += 3;
    }
  }
};

} // namespace Benchmark

}  // namespace FastBVH
