#include "vepch.h"
#include "Camera.h"

void Camera::SetOrthographicProjection(
    const float left, const float right, const float top, const float bottom, const float nearClip, const float farClip)
{
    m_ProjectionMatrix = glm::mat4{1.0F};
    m_ProjectionMatrix[0][0] = 2.F / (right - left);
    m_ProjectionMatrix[1][1] = 2.F / (bottom - top);
    m_ProjectionMatrix[2][2] = 1.F / (farClip - nearClip);
    m_ProjectionMatrix[3][0] = -(right + left) / (right - left);
    m_ProjectionMatrix[3][1] = -(bottom + top) / (bottom - top);
    m_ProjectionMatrix[3][2] = -nearClip / (farClip - nearClip);
}

void Camera::SetPerspectiveProjection(const float fovY, const float aspect, const float nearClip, const float farClip)
{
    assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
    const float tanHalfFovY = tan(fovY / 2.F);
    m_ProjectionMatrix = glm::mat4{0.0F};
    m_ProjectionMatrix[0][0] = 1.F / (aspect * tanHalfFovY);
    m_ProjectionMatrix[1][1] = 1.F / tanHalfFovY;
    m_ProjectionMatrix[2][2] = farClip / (farClip - nearClip);
    m_ProjectionMatrix[2][3] = 1.F;
    m_ProjectionMatrix[3][2] = -(farClip * nearClip) / (farClip - nearClip);
}

void Camera::SetViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
{
    const glm::vec3 w{normalize(direction)};
    const glm::vec3 u{normalize(cross(w, up))};
    const glm::vec3 v{cross(w, u)};

    m_ViewMatrix = glm::mat4{1.F};
    m_ViewMatrix[0][0] = u.x;
    m_ViewMatrix[1][0] = u.y;
    m_ViewMatrix[2][0] = u.z;
    m_ViewMatrix[0][1] = v.x;
    m_ViewMatrix[1][1] = v.y;
    m_ViewMatrix[2][1] = v.z;
    m_ViewMatrix[0][2] = w.x;
    m_ViewMatrix[1][2] = w.y;
    m_ViewMatrix[2][2] = w.z;
    m_ViewMatrix[3][0] = -dot(u, position);
    m_ViewMatrix[3][1] = -dot(v, position);
    m_ViewMatrix[3][2] = -dot(w, position);
    
    m_Position = position; // Store position
}

void Camera::SetViewTarget(const glm::vec3 position, const glm::vec3 target, const glm::vec3 up)
{
    SetViewDirection(position, target - position, up);
    // m_Position is set implicitly by SetViewDirection call above
    // m_Position = position; // No need to set again
}

void Camera::SetViewYXZ(const glm::vec3 position, const glm::vec3 rotation)
{
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
    const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
    const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};
    m_ViewMatrix = glm::mat4{1.F};
    m_ViewMatrix[0][0] = u.x;
    m_ViewMatrix[1][0] = u.y;
    m_ViewMatrix[2][0] = u.z;
    m_ViewMatrix[0][1] = v.x;
    m_ViewMatrix[1][1] = v.y;
    m_ViewMatrix[2][1] = v.z;
    m_ViewMatrix[0][2] = w.x;
    m_ViewMatrix[1][2] = w.y;
    m_ViewMatrix[2][2] = w.z;
    m_ViewMatrix[3][0] = -dot(u, position);
    m_ViewMatrix[3][1] = -dot(v, position);
    m_ViewMatrix[3][2] = -dot(w, position);

    m_Position = position; // Store position
}
