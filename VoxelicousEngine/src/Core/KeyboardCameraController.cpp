#include "vepch.h"
#include "KeyboardCameraController.h"

namespace VoxelicousEngine
{
    void KeyboardCameraController::MoveInPlaneXZ(GLFWwindow* window, const float dt, GameObject& gameObject)
    {
        glm::vec3 rotate{0};
        if (glfwGetKey(window, Keys.LookRight) == GLFW_PRESS) rotate.y += 1.f;
        if (glfwGetKey(window, Keys.LookLeft) == GLFW_PRESS) rotate.y -= 1.f;
        if (glfwGetKey(window, Keys.LookUp) == GLFW_PRESS) rotate.x -= 1.f;
        if (glfwGetKey(window, Keys.LookDown) == GLFW_PRESS) rotate.x += 1.f;

        if (dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
        {
            gameObject.Transform.Rotation += LookSpeed * dt * normalize(rotate);
        }

        gameObject.Transform.Rotation.x = glm::clamp(gameObject.Transform.Rotation.x, -1.5f, 1.5f);
        gameObject.Transform.Rotation.y = glm::mod(gameObject.Transform.Rotation.y, glm::two_pi<float>());

        const float yaw = gameObject.Transform.Rotation.y;
        const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
        const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
        const glm::vec3 upDir{0.f, 1.f, 0.f};

        MoveDir = glm::vec3(0.f, 0.f, 0.f);
        if (glfwGetKey(window, Keys.MoveForward) == GLFW_PRESS) MoveDir += forwardDir;
        if (glfwGetKey(window, Keys.MoveBackward) == GLFW_PRESS) MoveDir -= forwardDir;
        if (glfwGetKey(window, Keys.MoveRight) == GLFW_PRESS) MoveDir += rightDir;
        if (glfwGetKey(window, Keys.MoveLeft) == GLFW_PRESS) MoveDir -= rightDir;
        if (glfwGetKey(window, Keys.MoveUp) == GLFW_PRESS) MoveDir += upDir;
        if (glfwGetKey(window, Keys.MoveDown) == GLFW_PRESS) MoveDir -= upDir;

        if (dot(MoveDir, MoveDir) > std::numeric_limits<float>::epsilon())
        {
            gameObject.Transform.Translation += MoveSpeed * dt * normalize(MoveDir);
        }
    }
}
