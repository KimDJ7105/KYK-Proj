#pragma once

#include "lve_model.h"

// lib
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>

namespace lve {

    // ���⼭ ������Ʈ�� �����̴� ������ �����ϰ� ���� �� �ִ�
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
            static id_t currentId = 0;                          // �츮�� ���� ������ ������Ʈ�� ���̵� �ѹ� 0���� ����������̴�.
            return LveGameObject{ currentId++ };
        }


        // ���ӿ�����Ʈ�� ������ �Ǵ°� ���ϱ� ���ؼ�
        // ī�ǿ����ڿ� ������۸� �������ٰ��̴�.
        LveGameObject(const LveGameObject&) = delete;
        LveGameObject& operator=(const LveGameObject&) = delete;

        // ������ �����ӿ����ھ� ������۴� ����Ʈ������ �̿밡���ϰ� �Ұ��̴�.
        LveGameObject(LveGameObject&&) = default;
        LveGameObject& operator=(LveGameObject&&) = default;

        id_t getId() { return id; }

        
        std::shared_ptr<LveModel> model{};
        glm::vec3 color{};
        TransformComponent transform{};

    private:
        // ���ӿ�����Ʈ���� ����ũ�� ���̵�� �����Ǳ� �ϱ� ����
        LveGameObject(id_t objId) : id{ objId } {}

        // ���� ������Ʈ�� ���̵�
        id_t id;
    };
}   // namespace lve
