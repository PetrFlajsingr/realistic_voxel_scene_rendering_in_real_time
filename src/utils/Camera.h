/**
 * @file Camera.h
 * @brief 3D camera.
 * @author Petr Flajšingr
 * @date 8.11.20
 */

#ifndef REALISTIC_VOXEL_RENDERING_SRC_UTILS_CAMERA_H
#define REALISTIC_VOXEL_RENDERING_SRC_UTILS_CAMERA_H

#include "common_enums.h"
#include <glm/glm.hpp>
#include <logging/loggers.h>
#include <pf_glfw_vulkan/ui/Window.h>
#include <vector>

namespace pf {
/**
 * @brief A camera, which can register it's movement callbacks within a Window.
 */
class Camera {
 public:
  /**
   * Construct Camera.
   * @param resolution resolution of the screen.
   * @param near near plane distance
   * @param far far plane distance
   * @param movementSpeed
   * @param mouseSpeed
   * @param position
   * @param front
   * @param up
   * @param fieldOfView
   * @param yaw
   * @param pitch
   * @param roll
   */
  explicit Camera(ui::Resolution resolution, float near, float far, float movementSpeed = 2.5, float mouseSpeed = 2.5,
                  const glm::vec3 &position = {0, 0, 0}, const glm::vec3 &front = {0, 0, -1},
                  const glm::vec3 &up = {0, 1, 0}, float fieldOfView = 45, float yaw = -90, float pitch = 0,
                  float roll = 0);
  Camera(Camera &&other) noexcept;
  Camera &operator=(Camera &&other) noexcept;
  ~Camera();

  void registerControls(ui::Window &window);

  const glm::vec3 &move(Direction direction, float deltaTime, float multiplier = 1.f);
  void mouse(float xDelta, float yDelta, bool contrainPitch = true);
  float changeFov(float delta);

  [[nodiscard]] const glm::vec3 &getPosition() const;
  void setPosition(const glm::vec3 &position);
  [[nodiscard]] const glm::vec3 &getUp() const;
  void setUp(const glm::vec3 &up);
  [[nodiscard]] const glm::vec3 &getFront() const;
  void setFront(const glm::vec3 &front);
  [[nodiscard]] float getFieldOfView() const;
  void setFieldOfView(float fieldOfView);
  [[nodiscard]] double getScreenWidth() const;
  void setScreenWidth(double screenWidth);
  [[nodiscard]] double getScreenHeight() const;
  void setScreenHeight(double screenHeight);
  [[nodiscard]] float getMovementSpeed() const;
  void setMovementSpeed(float movementSpeed);
  [[nodiscard]] float getMouseSpeed() const;
  void setMouseSpeed(float mouseSpeed);
  [[nodiscard]] float getYaw() const;
  void setYaw(float yaw);
  [[nodiscard]] float getPitch() const;
  void setPitch(float pitch);
  [[nodiscard]] float getRoll() const;
  void setRoll(float roll);
  [[nodiscard]] const glm::vec3 &getRight() const;

  [[nodiscard]] bool isSwapLeftRight() const;
  void setSwapLeftRight(bool swap);

  [[nodiscard]] glm::mat4 getViewMatrix() const;
  [[nodiscard]] glm::mat4 getProjectionMatrix() const;

  [[nodiscard]] float getNear() const;
  [[nodiscard]] float getFar() const;

 private:
  void update();

  double screenWidth;
  double screenHeight;

  float nearF;
  float farF;

  float movementSpeed;
  float mouseSpeed;

  glm::vec3 position;
  glm::vec3 front;
  glm::vec3 up;
  glm::vec3 right;

  bool swapLeftRight = false;

  float fieldOfView;

  float yaw;
  float pitch;
  float roll;
  std::vector<Subscription> subscriptions;
};
}// namespace pf

#endif//REALISTIC_VOXEL_RENDERING_SRC_UTILS_CAMERA_H
