#pragma once

#include "lve_model.h"

// lib
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>

namespace lve {

    // 여기서 오브젝트를 움직이는 역할을 수행하게 만들 수 있다
    struct TransformComponent {
        glm::vec3 translation{};  // (position offset)
        glm::vec3 scale{ 1.f, 1.f, 1.f };
        glm::vec3 rotation{};

        
        glm::mat4 mat4() {
            auto transform = glm::translate(glm::mat4{ 1.f }, translation);
            
            transform = glm::rotate(transform, rotation.y, { 0.f, 1.f, 0.f });
            transform = glm::rotate(transform, rotation.x, { 1.f, 0.f, 0.f });
            transform = glm::rotate(transform, rotation.z, { 0.f, 0.f, 1.f });

            transform = glm::scale(transform, scale);
            return transform;

        }
    };

    class LveGameObject {
    public:
        using id_t = unsigned int;

        static LveGameObject createGameObject() {
            static id_t currentId = 0;                          // 우리가 만들 게임의 오브젝트는 아이디 넘버 0부터 만들어질것이다.
            return LveGameObject{ currentId++ };
        }


        // 게임오브젝트가 복제가 되는걸 피하기 위해서
        // 카피연산자와 수행오퍼를 삭제해줄것이다.
        LveGameObject(const LveGameObject&) = delete;
        LveGameObject& operator=(const LveGameObject&) = delete;

        // 하지만 움직임연산자아 수행오퍼는 디폴트값으로 이용가능하게 할것이다.
        LveGameObject(LveGameObject&&) = default;
        LveGameObject& operator=(LveGameObject&&) = default;

        id_t getId() { return id; }

        
        std::shared_ptr<LveModel> model{};
        glm::vec3 color{};
        TransformComponent transform{};

    private:
        // 게임오브젝트들이 유니크한 아이디로 생성되기 하기 위함
        LveGameObject(id_t objId) : id{ objId } {}

        // 게임 오브젝트의 아이디
        id_t id;
    };
}   // namespace lve
