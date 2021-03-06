/* Copyright (c) 2019, Arm Limited and Contributors
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "afbc.h"

#include "platform/platform.h"

#include "gui.h"
#include "stats.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#	include "platform/android/android_platform.h"
#endif

#include "gltf_loader.h"

AFBCSample::AFBCSample()
{
	get_configuration().insert<vkb::BoolSetting>(0, afbc_enabled, false);

	get_configuration().insert<vkb::BoolSetting>(1, afbc_enabled, true);
}

bool AFBCSample::prepare(vkb::Platform &platform)
{
	if (!VulkanSample::prepare(platform))
	{
		return false;
	}

	std::vector<const char *> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	device = std::make_unique<vkb::Device>(get_gpu(0), get_surface(), extensions);

	std::unique_ptr<vkb::Swapchain> swapchain = std::make_unique<vkb::Swapchain>(*device,
	                                                                             get_surface(),
	                                                                             VkExtent2D({}),
	                                                                             3,
	                                                                             VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
	                                                                             VK_PRESENT_MODE_FIFO_KHR,
	                                                                             /* We want AFBC disabled by default, hence we create swapchain with VK_IMAGE_USAGE_STORAGE_BIT. */
	                                                                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);

	stats = std::make_unique<vkb::Stats>(platform.get_profiler(),
	                                     std::set<vkb::StatIndex>{vkb::StatIndex::l2_ext_write_beats});

	render_context = std::make_unique<vkb::RenderContext>(*device, std::move(swapchain));
	render_context->prepare();

	pipeline_layout = &create_pipeline_layout(*device, "shaders/base.vert", "shaders/base.frag");

	load_scene("scenes/sponza/Sponza01.gltf");

	auto& camera_node = add_free_camera("main_camera");

	camera = &camera_node.get_component<vkb::sg::Camera>();

	gui = std::make_unique<vkb::Gui>(*render_context, platform.get_dpi_factor());

	fs_push_constant.light_pos   = glm::vec4(500.0f, 1550.0f, 0.0f, 1.0);
	fs_push_constant.light_color = glm::vec4(1.0, 1.0, 1.0, 1.0);

	return true;
}

void AFBCSample::update(float delta_time)
{
	if (afbc_enabled != afbc_enabled_last_value)
	{
		std::set<VkImageUsageFlagBits> image_usage_flags = {VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT};

		if (!afbc_enabled)
		{
			image_usage_flags.insert(VK_IMAGE_USAGE_STORAGE_BIT);
		}

		render_context->get_device().wait_idle();

		auto new_swapchain = std::make_unique<vkb::Swapchain>(
		    render_context->get_swapchain(),
		    image_usage_flags);

		render_context->update_swapchain(std::move(new_swapchain));

		afbc_enabled_last_value = afbc_enabled;
	}

	VulkanSample::update(delta_time);
}

void AFBCSample::draw_gui()
{
	gui->show_options_window(
	    /* body = */ [this]() {
		    ImGui::Checkbox("AFBC", &afbc_enabled);
	    },
	    /* lines = */ 1);
}

void AFBCSample::draw_scene(vkb::CommandBuffer &cmd_buf)
{
	vs_push_constant.camera_view_proj = vkb::vulkan_style_projection(camera->get_projection()) * camera->get_view();

	cmd_buf.bind_pipeline_layout(*pipeline_layout);

	cmd_buf.push_constants(0, vs_push_constant);
	cmd_buf.push_constants(sizeof(vkb::VertPushConstant), fs_push_constant);

	draw_scene_meshes(cmd_buf, *pipeline_layout, scene);
}

std::unique_ptr<vkb::VulkanSample> create_afbc()
{
	return std::make_unique<AFBCSample>();
}
