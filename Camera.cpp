// Camera.cpp
#include "Camera.h"

Camera::Camera(float aspectRatio) {
    m_viewMatrix = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 3.0f), // Camera position
        glm::vec3(0.0f, 0.0f, 0.0f), // Target
        glm::vec3(0.0f, 1.0f, 0.0f)  // Up vector
    );

    m_projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
}

const glm::mat4& Camera::getViewMatrix() const {
    return m_viewMatrix;
}

const glm::mat4& Camera::getProjectionMatrix() const {
    return m_projectionMatrix;
}
