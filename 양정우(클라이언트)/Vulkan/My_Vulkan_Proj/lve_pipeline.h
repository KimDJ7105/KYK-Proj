#pragma once

#include "lve_device.h"

// std
#include <string>
#include <vector>

namespace lve {

	struct PipelineConfigInfo {
		PipelineConfigInfo() = default;
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;
		
		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class LvePipeline {
	public:
		LvePipeline(
			LveDevice &device, 
			const std::string& vertFilepath, 
			const std::string& fragFilepath, 
			const PipelineConfigInfo& configInfo);
		~LvePipeline();

		LvePipeline(const LvePipeline&) = delete;
		LvePipeline& operator=(const LvePipeline&) = delete;


		void bind(VkCommandBuffer commandBuffer);

		// 디폴트로 작동할 파이프라인 배열
		static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

	private:
		static std::vector<char> readFile(const std::string& filepath);

		void createGraphicsPipeline(
			const std::string& vertFilepath,
			const std::string& fragFilepath,
			const PipelineConfigInfo& configInfo
		);

		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
		// VkShaderModule이 포인터 형식이기에 포인터에서 포인터로 넘어가는 형식이다.
		// 모듈을 만들고 변수를 초기화 해준다.

		LveDevice& lveDevice;
		/*위 코드는 잠재적으로 메모리가 안전하지가 않다.
		만약 파이프라인 이전에 디바이스가 메모리로부터 해제되었을때
		프로그램에서 "dangling pointer"를 역참조(데이터에 접근)하려고 할 수 있습니다.
		이와 같은 방식으로 참조 유형의 멤버 변수를 사용하는 경우는,
		멤버 변수(이 경우에는 장치)가 이를 의존하는 포함 클래스의 모든 인스턴스보다 오래 살아남을 것으로 암시되는 경우입니다.
		이것은 파이프라인이 존재하려면 기본적으로 장치가 필요하기 때문에 이해할 만한 것입니다.*/

		VkPipeline graphicsPipeline;
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;
	};
}