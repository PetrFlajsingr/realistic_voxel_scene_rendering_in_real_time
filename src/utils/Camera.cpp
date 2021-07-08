/**
 * @file Camera.cpp
 * @brief 3D camera.
 * @author Petr Flajšingr
 * @date 8.11.20
 */
#include "Camera.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

namespace pf {

Camera::Camera(ui::Resolution resolution, float near, float far, float movementSpeed, float mouseSpeed,
               const glm::vec3 &position, const glm::vec3 &front, const glm::vec3 &up, float fieldOfView, float yaw,
               float pitch, float roll)
    : screenWidth(resolution.width), screenHeight(resolution.height), nearF(near), farF(far),
      movementSpeed(movementSpeed), mouseSpeed(mouseSpeed), position(position), front(front), up(up),
      fieldOfView(fieldOfView), yaw(yaw), pitch(pitch), roll(roll) {
  update();
}

void Camera::registerControls(ui::Window &window) {
  const auto interactionPredicate = [&window] { return window.getMouseButtonsDown().is(events::MouseButton::Right); };
  subscriptions.push_back(window.addMouseListener(events::MouseEventType::Wheel,
                                                  [this, interactionPredicate](const events::MouseEvent &event) {
                                                    if (interactionPredicate()) {
                                                      changeFov(event.delta.second);
                                                      return true;
                                                    }
                                                    return false;
                                                  }));
  subscriptions.push_back(window.addMouseListener(
      events::MouseEventType::Move, [this, interactionPredicate, &window](const events::MouseEvent &event) {
        if (interactionPredicate()) {
          mouse(event.delta.first, event.delta.second);
          return true;
        }
        return false;
      }));
  subscriptions.push_back(
      window.addMouseListener(events::MouseEventType::Down, [&window](const events::MouseEvent &event) {
        if (event.button == events::MouseButton::Right) { window.setCursorHiddenAndCaptured(true); }
        return false;
      }));
  subscriptions.push_back(
      window.addMouseListener(events::MouseEventType::Up, [&window](const events::MouseEvent &event) {
        if (event.button == events::MouseButton::Right) { window.setCursorHiddenAndCaptured(false); }
        return false;
      }));
  const auto keyMove = [this, interactionPredicate](const events::KeyEvent &event) {
    auto interacted = true;
    const auto multiplier = event.modifiersKeys.is(events::ModifierKey::Shift) ? 2.0 : 1.0;
    switch (std::tolower(event.key)) {
      case 'w': move(Direction::Forward, 0.167, multiplier); break;
      case 'a': move(Direction::Left, 0.167, multiplier); break;
      case 's': move(Direction::Backward, 0.167, multiplier); break;
      case 'd': move(Direction::Right, 0.167, multiplier); break;
      case 'q': move(Direction::Up, 0.167, multiplier); break;
      case 'e': move(Direction::Down, 0.167, multiplier); break;
      default: interacted = false;
    }
    return interacted;
  };
  window.addKeyboardListener(events::KeyEventType::Repeat, keyMove);
  window.addKeyboardListener(events::KeyEventType::Pressed, keyMove);
}

const glm::vec3 &Camera::move(Direction direction, float deltaTime, float multiplier) {
  const auto velocity = movementSpeed * deltaTime;

  switch (direction) {
    case Direction::Forward: position += front * velocity * multiplier; break;
    case Direction::Backward: position -= front * velocity * multiplier; break;
    case Direction::Left: position -= right * velocity * multiplier; break;
    case Direction::Right: position += right * velocity * multiplier; break;
    case Direction::Up: position -= up * velocity * multiplier; break;
    case Direction::Down: position += up * velocity * multiplier; break;
  }
  return position;
}

void Camera::mouse(float xDelta, float yDelta, bool contrainPitch) {
  xDelta *= mouseSpeed;
  yDelta *= mouseSpeed;

  if (!swapLeftRight) { xDelta = -xDelta; }

  yaw += xDelta;
  pitch -= yDelta;

  if (contrainPitch) { pitch = std::clamp(pitch, -89.f, 89.f); }
  update();
}

float Camera::changeFov(float delta) {
  fieldOfView = std::clamp(fieldOfView - delta, 1.f, 180.f);
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
glm::mat4 Camera::getViewMatrix() const { return glm::lookAt(position, position + front, up); }
glm::mat4 Camera::getProjectionMatrix() const {
  return glm::perspective(glm::radians(fieldOfView), static_cast<float>(screenHeight / screenWidth), nearF, farF);
}
float Camera::getNear() const { return nearF; }
float Camera::getFar() const { return farF; }
Camera::~Camera() {
  std::ranges::for_each(subscriptions, [](auto &subscription) { subscription.unsubscribe(); });
}
Camera::Camera(Camera &&other) noexcept {
  screenWidth = other.screenWidth;
  screenHeight = other.screenHeight;
  nearF = other.nearF;
  farF = other.farF;
  movementSpeed = other.movementSpeed;
  mouseSpeed = other.mouseSpeed;
  position = other.position;
  front = other.front;
  up = other.up;
  right = other.right;
  swapLeftRight = other.swapLeftRight;
  fieldOfView = other.fieldOfView;
  yaw = other.yaw;
  pitch = other.pitch;
  roll = other.roll;
  subscriptions = std::move(other.subscriptions);
}
Camera &Camera::operator=(Camera &&other) noexcept {
  screenWidth = other.screenWidth;
  screenHeight = other.screenHeight;
  nearF = other.nearF;
  farF = other.farF;
  movementSpeed = other.movementSpeed;
  mouseSpeed = other.mouseSpeed;
  position = other.position;
  front = other.front;
  up = other.up;
  right = other.right;
  swapLeftRight = other.swapLeftRight;
  fieldOfView = other.fieldOfView;
  yaw = other.yaw;
  pitch = other.pitch;
  roll = other.roll;
  subscriptions = std::move(other.subscriptions);
  return *this;
}

}// namespace pf