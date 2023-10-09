#pragma once

#include "lve_device.h"

// libs
#define GLM_FORCE_RADIANS				// 파이나 도가 아니라 라디안을 강제적으로 사용할것임을 명시적으로 알림
#define GLM_FORCE_DEPTH_ZERO_TO_ONE		// depth버퍼의 값이 0과 1을 범위로 잡겟다(openGl은 -1에서 1이여서 정의해줄 필요가 있다.)
#include <glm/glm.hpp>

// std
#include <vector>

namespace lve {
	class LveModel {
	public:

		struct Vertex {
			glm::vec3 position;
			glm::vec3 color;

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		LveModel(LveDevice& device, const std::vector<Vertex>& vertices);
		~LveModel();

		LveModel(const LveModel&) = delete;
		LveModel& operator=(const LveModel&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);

		LveDevice& lveDevice;
		VkBuffer vertexBuffer;					// 이 두개가
		VkDeviceMemory vertexBufferMemory;		// 메모리 관리의 핵심이다
		uint32_t vertexCount;
	};
}	// namespace lve