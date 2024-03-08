#pragma once

#include "Model.h"

#include <unordered_map>
#include <memory>

#include <gtc/matrix_transform.hpp>

namespace VoxelicousEngine
{
    struct TransformComponent
    {
        glm::vec3 Translation{0.f, 0.f, 0.f};
        glm::vec3 Scale{1.f, 1.f, 1.f};
        glm::vec3 Rotation;

        glm::mat4 Mat4() const
        {
            const float c3 = glm::cos(Rotation.z);
            const float s3 = glm::sin(Rotation.z);
            const float c2 = glm::cos(Rotation.x);
            const float s2 = glm::sin(Rotation.x);
            const float c1 = glm::cos(Rotation.y);
            const float s1 = glm::sin(Rotation.y);
            return glm::mat4
            {
                {
                    Scale.x * (c1 * c3 + s1 * s2 * s3),
                    Scale.x * (c2 * s3),
                    Scale.x * (c1 * s2 * s3 - c3 * s1),
                    0.0f,
                },
                {
                    Scale.y * (c3 * s1 * s2 - c1 * s3),
                    Scale.y * (c2 * c3),
                    Scale.y * (c1 * c3 * s2 + s1 * s3),
                    0.0f,
                },
                {
                    Scale.z * (c2 * s1),
                    Scale.z * -s2,
                    Scale.z * (c1 * c2),
                    0.0f,
                },
                {Translation.x * 2, Translation.y * 2, Translation.z * 2, 2}
            };
        }
    };

    class GameObject
    {
    public:
        using IdT = unsigned int;
        using Map = std::unordered_map<IdT, GameObject>;

        static GameObject CreateGameObject()
        {
            static IdT currentId = 0;
            return GameObject(currentId++);
        }

        GameObject(const GameObject&) = delete;
        GameObject& operator=(const GameObject&) = delete;
        GameObject(GameObject&&) = default;
        GameObject& operator=(GameObject&&) = default;

        IdT GetId() const { return m_Id; }

        std::shared_ptr<Model> Model{};
        glm::vec3 Color{};
        TransformComponent Transform{};

    private:
        explicit GameObject(const IdT objId) : m_Id(objId)
        {
        }

        IdT m_Id;
    };
}
