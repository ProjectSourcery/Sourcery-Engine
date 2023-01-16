#pragma once

#include "core/window/src3_window.h"
#include "core/device/src3_device.h"
#include "game/gameobject/src3_game_object.h"
#include "game/ecs/src3_ecs.h"
#include "render/renderer/src3_renderer.h"
#include "core/buffer/uniform/src3_descriptors.h"

#include <memory>
#include <vector>

namespace src3 {
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp&) = delete;
		FirstApp& operator=(const FirstApp&) = delete;

		void run();
	private:
		void loadGameObjects();

		SrcWindow srcWindow{ WIDTH,HEIGHT,"Testing Vulkan" };
		SrcDevice srcDevice{ srcWindow };
		SrcRenderer srcRenderer{srcWindow, srcDevice};

		std::unique_ptr<SrcDescriptorPool> globalPool{};
		std::vector<std::unique_ptr<SrcDescriptorPool>> framePools{};
		EntManager ecs{};
	};
}