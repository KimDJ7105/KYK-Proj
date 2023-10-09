// 메인 애플리케이션

#pragma once

#include "lve_device.h"
#include "lve_swap_chain.h"
#include "lve_window.h"


// std
#include <cassert>
#include <memory>		// 파이프라인 오브젝트를 만들기 시작하면서 추가한 메모리, 유니크포인터를 사용할것이다.
#include <vector>

namespace lve {
	class LveRenderer {
		// LveRenderer가 만들어질 때 윈도우가 만들어지고 실행될것이다.
		// LveRenderer가 종료될 때 윈도우는 자동으로 종료될것이다.
		// 포인터와 동적 메모리 할당을 사용하지 않았다.

	public:
		
		LveRenderer(LveWindow &window, LveDevice & device);
		~LveRenderer();

		LveRenderer(const LveRenderer&) = delete;
		LveRenderer& operator=(const LveRenderer&) = delete;

		VkRenderPass getSwapChainRenderPass() const { return lveSwapChain->getRenderPass(); }
		float getAspectRatio() const { return lveSwapChain->extentAspectRatio(); }
		bool isFrameInProgress() const { return isFrameStarted; }
		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
			return commandBuffers[currentFrameIndex];
		}

		int getFrameIndex()const {
			assert(isFrameStarted && "Cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

	private:

		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();
		
		LveWindow& lveWindow;
		LveDevice& lveDevice;
		std::unique_ptr<LveSwapChain> lveSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex{0};
		bool isFrameStarted{false};
	};
}	// namespace lve