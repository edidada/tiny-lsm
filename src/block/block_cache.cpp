#include "../../include/block/block_cache.h"
#include "../../include/block/block.h"
#include <chrono>
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>

BlockCache::BlockCache(size_t capacity, size_t k)
    : capacity_(capacity), k_(k) {}

BlockCache::~BlockCache() = default;

std::shared_ptr<Block> BlockCache::get(int sst_id, int block_id) {
  // TODO: 按照实现lru-k的逻辑实现get方法
  // 注意：需要维护total_requests_和hit_requests_
  return {};
}

void BlockCache::put(int sst_id, int block_id, std::shared_ptr<Block> block) {
  // TODO: 按照实现lru-k的逻辑实现get方法
}

double BlockCache::hit_rate() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return total_requests_ == 0
             ? 0.0
             : static_cast<double>(hit_requests_) / total_requests_;
}