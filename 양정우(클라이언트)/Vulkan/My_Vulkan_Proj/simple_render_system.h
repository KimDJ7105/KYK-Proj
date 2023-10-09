#pragma once

#include "lve_camera.h"
#include "lve_device.h"
#include "lve_game_object.h"
#include "lve_pipeline.h"

// std
#include <memory>
#include <vector>

namespace lve {
	class SimpleRenderSystem {
	public:
		SimpleRenderSystem(LveDevice& device, VkRenderPass renderPass);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(
			VkCommandBuffer commandBuffer,
			std::vector<LveGameObject>& gameObjects,
			const LveCamera &camera); 

	private:
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);

		LveDevice& lveDevice;

		std::unique_ptr<LvePipeline> lvePipeline;
		VkPipelineLayout pipelineLayout;
	};
}  // namespace lve