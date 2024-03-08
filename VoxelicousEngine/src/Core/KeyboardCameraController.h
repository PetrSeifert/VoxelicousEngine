#pragma once

#include "GameObject.h"
#include "Window.h"

namespace VoxelicousEngine
{
    class KeyboardCameraController
    {
    public:
        struct KeyMappings
        {
            int MoveLeft = GLFW_KEY_A;
            int MoveRight = GLFW_KEY_D;
            int MoveForward = GLFW_KEY_W;
            int MoveBackward = GLFW_KEY_S;
            int MoveUp = GLFW_KEY_E;
            int MoveDown = GLFW_KEY_Q;
            int LookLeft = GLFW_KEY_LEFT;
            int LookRight = GLFW_KEY_RIGHT;
            int LookUp = GLFW_KEY_UP;
            int LookDown = GLFW_KEY_DOWN;
        };

        void MoveInPlaneXZ(GLFWwindow* window, float dt, GameObject& gameObject);
        glm::vec3 GetMoveDirection(GLFWwindow* window);

        KeyMappings Keys{};
        float MoveSpeed{3.f};
        float LookSpeed{1.5f};
        glm::vec3 MoveDir{0.f};
    };
}
