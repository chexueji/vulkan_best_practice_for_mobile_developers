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

#pragma once

#include "core/command_buffer.h"
#include "core/command_pool.h"
#include "core/descriptor_set.h"
#include "core/descriptor_set_layout.h"
#include "core/framebuffer.h"
#include "core/pipeline.h"
#include "core/pipeline_layout.h"
#include "core/queue.h"
#include "core/render_pass.h"
#include "core/shader_module.h"
#include "core/swapchain.h"

#include "cache_resource.h"
#include "graphics_pipeline_state.h"
#include "render_frame.h"
#include "render_target.h"

namespace vkb
{
class RenderContext : public NonCopyable
{
  public:
	RenderContext(Device &device, std::unique_ptr<Swapchain> &&swapchain);

	virtual ~RenderContext() = default;

	void prepare(RenderFrame::CreateFunc render_frame_create_func = RenderFrame::DEFAULT_CREATE_FUNC);

	VkSemaphore begin_frame();

	VkSemaphore submit(const Queue &queue, const CommandBuffer &command_buffer, VkSemaphore wait_semaphore, VkPipelineStageFlags wait_pipeline_stage);

	/**
	 * @brief Submits a command buffer related to a frame to a queue
	 */
	void submit(const Queue &queue, const CommandBuffer &command_buffer);

	/**
	 * @brief Waits a frame to finish its rendering
	 */
	void wait_frame();

	void end_frame(VkSemaphore semaphore);

	/**
	 * @return The current active frame
	 * An error should be raised if the frame is not active
	 * A frame is active after @ref begin_frame has been called
	 */
	RenderFrame &get_active_frame();

	/**
	 * @brief Requests a command buffer not related to a frame
	 * @return A command buffer
	 */
	CommandBuffer &request_command_buffer();

	/**
	 * @brief Requests a command buffer to the command pool of the active frame
	 * A frame should be active at the moment of requesting it
	 * @return A command buffer related to the current active frame
	 */
	CommandBuffer &request_frame_command_buffer(const Queue &queue);

	VkSemaphore request_semaphore();

	Device &get_device();

	void update_swapchain(std::unique_ptr<Swapchain> &&new_swapchain);

	Swapchain &get_swapchain();

	VkExtent2D get_surface_extent();

  protected:
	VkExtent2D surface_extent;

	virtual void handle_surface_changes();

  private:
	Device &device;

	std::unique_ptr<Swapchain> swapchain;

	/// Current active frame index
	uint32_t active_frame_index{0};

	/// Whether a frame is active or not
	bool frame_active{false};

	std::vector<std::unique_ptr<RenderFrame>> frames;

	/// Queue to submit commands for rendering our frames
	const Queue &present_queue;
};

}        // namespace vkb
