#pragma once

#include "core/device/src3_device.h"

namespace src3 {

class SrcBuffer {
 public:
  SrcBuffer(
      SrcDevice& device,
      VkDeviceSize bufferSize,
      VkBufferUsageFlags usageFlags,
      VkMemoryPropertyFlags memoryPropertyFlags);
  ~SrcBuffer();

  SrcBuffer(const SrcBuffer&) = delete;
  SrcBuffer& operator=(const SrcBuffer&) = delete;

  VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  void unmap();

  void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

  VkBuffer getBuffer() const { return buffer; }
  void* getMappedMemory() const { return mapped; }
  VkBufferUsageFlags getUsageFlags() const { return usageFlags; }
  VkMemoryPropertyFlags getMemoryPropertyFlags() const { return memoryPropertyFlags; }
  VkDeviceSize getBufferSize() const { return bufferSize; }

 private:
  SrcDevice& srcDevice;
  void* mapped = nullptr;
  VkBuffer buffer = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;

  VkDeviceSize bufferSize;
  VkBufferUsageFlags usageFlags;
  VkMemoryPropertyFlags memoryPropertyFlags;
};

}  // namespace lve