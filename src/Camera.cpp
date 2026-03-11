// Camera.cpp
#include "Camera.h"
#include <algorithm>
#include <cmath>

Camera::Camera(float aspectRatio) : m_aspectRatio(aspectRatio) {
    recalculate();
}

void Camera::setAspectRatio(float ar) {
    if (ar != m_aspectRatio) {
        m_aspectRatio = ar;
        m_dirty = true;
    }
}

void Camera::rotate(float dTheta, float dPhi) {
    theta += dTheta;
    phi += dPhi;
    phi = glm::clamp(phi, 0.1f, 3.05f);
    m_dirty = true;
}

void Camera::zoom(float dDistance) {
    distance += dDistance;
    distance = glm::clamp(distance, 0.5f, 100.0f);
    m_dirty = true;
}

const glm::mat4& Camera::getViewMatrix() const {
    if (m_dirty) recalculate();
    return m_viewMatrix;
}

const glm::mat4& Camera::getProjectionMatrix() const {
    if (m_dirty) recalculate();
    return m_projectionMatrix;
}

void Camera::recalculate() const {
    float sinPhi = std::sin(phi);
    float x = distance * sinPhi * std::cos(theta);
    float y = distance * std::cos(phi);
    float z = distance * sinPhi * std::sin(theta);

    glm::vec3 eye = target + glm::vec3(x, y, z);

    m_viewMatrix = glm::lookAt(eye, target, glm::vec3(0.0f, 1.0f, 0.0f));
    m_projectionMatrix = glm::perspective(
        glm::radians(m_fov), m_aspectRatio, 0.1f, 200.0f
    );
    m_dirty = false;
}
