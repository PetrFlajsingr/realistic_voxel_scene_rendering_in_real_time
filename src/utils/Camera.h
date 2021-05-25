//
// Created by petr on 11/8/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_UTILS_CAMERA_H
#define REALISTIC_VOXEL_RENDERING_SRC_UTILS_CAMERA_H

#include "common_enums.h"
#include <glm/glm.hpp>
#include <pf_glfw_vulkan/concepts/Window.h>

namespace pf {
class Camera {
 public:
  explicit Camera(ui::Resolution resolution, float near, float far, float movementSpeed = 2.5, float mouseSpeed = 2.5,
                  const glm::vec3 &position = {0, 0, 0}, const glm::vec3 &front = {0, 0, -1},
                  const glm::vec3 &up = {0, 1, 0}, float fieldOfView = 45, float yaw = -90, float pitch = 0,
                  float roll = 0);

  void registerControls(ui::Window auto &window) {
    const auto interactionPredicate = [&window] {
      return window.getMouseButtonsDown().contains(events::MouseButton::Right);
    };
    window.addMouseListener(events::MouseEventType::Wheel,
                            [this, interactionPredicate](const events::MouseEvent &event) {
                              if (interactionPredicate()) {
                                changeFov(event.delta.second);
                                return true;
                              }
                              return false;
                            });
    window.addMouseListener(events::MouseEventType::Move,
                            [this, interactionPredicate](const events::MouseEvent &event) {
                              if (interactionPredicate()) {
                                mouse(event.delta.first, event.delta.second);
                                return true;
                              }
                              return false;
                            });
    const auto keyMove = [this, interactionPredicate](const events::KeyEvent &event) {
      auto interacted = true;
      switch (std::tolower(event.key)) {
        case 'w': move(Direction::Forward, 0.167); break;
        case 'a': move(Direction::Left, 0.167); break;
        case 's': move(Direction::Backward, 0.167); break;
        case 'd': move(Direction::Right, 0.167); break;
        default: interacted = false;
      }
      return interacted;
    };
    window.addKeyListener(events::KeyEventType::Repeat, keyMove);
    window.addKeyListener(events::KeyEventType::Pressed, keyMove);
  }

  const glm::vec3 &move(Direction direction, float deltaTime);
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

  bool swapLeftRight;

  float fieldOfView;

  float yaw;
  float pitch;
  float roll;
};
}// namespace pf

#endif//REALISTIC_VOXEL_RENDERING_SRC_UTILS_CAMERA_H
