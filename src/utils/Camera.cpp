//
// Created by petr on 11/8/20.
//

#include "Camera.h"
#include <bits/stl_algo.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <pf_glfw_vulkan/concepts/Window.h>

namespace pf {

Camera::Camera(ui::Resolution resolution, float near, float far, float movementSpeed, float mouseSpeed, const glm::vec3 &position,
               const glm::vec3 &front, const glm::vec3 &up, float fieldOfView, float yaw, float pitch, float roll)
    : screenWidth(resolution.width), screenHeight(resolution.height), nearF(near), farF(far), movementSpeed(movementSpeed),
      mouseSpeed(mouseSpeed), position(position), front(front), up(up), fieldOfView(fieldOfView), yaw(yaw),
      pitch(pitch), roll(roll) {
  update();
}

const glm::vec3 &Camera::move(Direction direction, float deltaTime) {
  const auto velocity = movementSpeed * deltaTime;

  switch (direction) {
    case Direction::Forward: position += front * velocity; break;
    case Direction::Backward: position -= front * velocity; break;
    case Direction::Left: position -= right * velocity; break;
    case Direction::Right: position += right * velocity; break;
  }
  return position;
}

void Camera::mouse(float xDelta, float yDelta, bool contrainPitch) {
  xDelta *= mouseSpeed;
  yDelta *= mouseSpeed;

  if (swapLeftRight) { xDelta = -xDelta; }

  yaw += xDelta;
  pitch += yDelta;

  if (contrainPitch) { pitch = std::clamp(pitch, -89.f, 89.f); }
  update();
}

float Camera::changeFov(float delta) {
  fieldOfView = std::clamp(fieldOfView - delta, 1.f, 90.f);
  return fieldOfView;
}

const glm::vec3 &Camera::getPosition() const { return position; }

void Camera::setPosition(const glm::vec3 &newPosition) { position = newPosition; }

const glm::vec3 &Camera::getUp() const { return up; }

void Camera::setUp(const glm::vec3 &newUp) { up = glm::normalize(newUp); }

float Camera::getFieldOfView() const { return fieldOfView; }

void Camera::setFieldOfView(float newFieldOfView) { fieldOfView = newFieldOfView; }

double Camera::getScreenWidth() const { return screenWidth; }

void Camera::setScreenWidth(double newScreenWidth) { screenWidth = newScreenWidth; }

double Camera::getScreenHeight() const { return screenHeight; }

void Camera::setScreenHeight(double newScreenHeight) { Camera::screenHeight = newScreenHeight; }

float Camera::getMovementSpeed() const { return movementSpeed; }

void Camera::setMovementSpeed(float newMovementSpeed) { movementSpeed = newMovementSpeed; }

float Camera::getMouseSpeed() const { return mouseSpeed; }

void Camera::setMouseSpeed(float newMouseSpeed) { mouseSpeed = newMouseSpeed; }

float Camera::getYaw() const { return yaw; }

void Camera::setYaw(float newYaw) {
  yaw = newYaw;
  update();
}

float Camera::getPitch() const { return pitch; }

void Camera::setPitch(float newPitch) {
  pitch = newPitch;
  update();
}

float Camera::getRoll() const { return roll; }

void Camera::setRoll(float newRoll) {
  roll = newRoll;
  update();
}

const glm::vec3 &Camera::getFront() const { return front; }

void Camera::setFront(const glm::vec3 &newFront) {
  front = newFront;
  update();
}

const glm::vec3 &Camera::getRight() const { return right; }

void Camera::update() {
  front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  front.y = sin(glm::radians(pitch));
  front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  front = glm::normalize(front);

  right = glm::normalize(glm::cross(front, up));
  //up = glm::normalize(glm::cross(right, front));
}
bool Camera::isSwapLeftRight() const { return swapLeftRight; }

void Camera::setSwapLeftRight(bool swap) { swapLeftRight = swap; }

glm::mat4 Camera::getViewMatrix() const {
  return glm::lookAt(position, position + front, up);
}
glm::mat4 Camera::getProjectionMatrix() const {
  return glm::perspective(glm::radians(fieldOfView), static_cast<float>(screenWidth / screenHeight), nearF, farF);
}
float Camera::getNear() const {
  return nearF;
}
float Camera::getFar() const {
  return farF;
}

}// namespace pf