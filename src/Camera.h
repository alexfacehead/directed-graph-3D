// Camera.h
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    Camera(float aspectRatio);

    float distance = 5.0f;
    float theta = 0.0f;
    float phi = 1.2f;
    glm::vec3 target = glm::vec3(0.0f);

    void setAspectRatio(float ar);
    void rotate(float dTheta, float dPhi);
    void zoom(float dDistance);

    const glm::mat4& getViewMatrix() const;
    const glm::mat4& getProjectionMatrix() const;

private:
    float m_aspectRatio;
    float m_fov = 45.0f;
    mutable glm::mat4 m_viewMatrix;
    mutable glm::mat4 m_projectionMatrix;
    mutable bool m_dirty = true;

    void recalculate() const;
};