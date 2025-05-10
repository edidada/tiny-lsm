#include "../../include/lsm/engine.h"
#include "../../include/config/config.h"
#include "../../include/consts.h"
#include "../../include/logger/logger.h"
#include "../../include/lsm/level_iterator.h"
#include "../../include/sst/concact_iterator.h"
#include "../../include/sst/sst.h"
#include "../../include/sst/sst_iterator.h"
#include "spdlog/spdlog.h"
#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

namespace toni_lsm {

// *********************** LSMEngine ***********************
LSMEngine::LSMEngine(std::string path) : data_dir(path) {
  // 初始化日志
  init_spdlog_file();

  // TODO: Lab 4.2 引擎初始化
}

LSMEngine::~LSMEngine() = default;

std::optional<std::pair<std::string, uint64_t>>
LSMEngine::get(const std::string &key, uint64_t tranc_id) {
  // TODO: Lab 4.2 查询

  return std::nullopt;
}

std::vector<
    std::pair<std::string, std::optional<std::pair<std::string, uint64_t>>>>
LSMEngine::get_batch(const std::vector<std::string> &keys, uint64_t tranc_id) {
  // TODO: Lab 4.2 批量查询

  return {};
}

std::optional<std::pair<std::string, uint64_t>>
LSMEngine::sst_get_(const std::string &key, uint64_t tranc_id) {
  // TODO: Lab 4.2 sst 内部查询
  return std::nullopt;
}

uint64_t LSMEngine::put(const std::string &key, const std::string &value,
                        uint64_t tranc_id) {
  // TODO: Lab 4.1 插入
  // ? 由于 put 操作可能触发 flush
  // ? 如果触发了 flush 则返回新刷盘的 sst 的 id
  // ? 在没有实现  flush 的情况下，你返回 0即可
  return 0;
}

uint64_t LSMEngine::put_batch(
    const std::vector<std::pair<std::string, std::string>> &kvs,
    uint64_t tranc_id) {
  // TODO: Lab 4.1 批量插入
  // ? 由于 put 操作可能触发 flush
  // ? 如果触发了 flush 则返回新刷盘的 sst 的 id
  // ? 在没有实现  flush 的情况下，你返回 0即可
  return 0;
}
uint64_t LSMEngine::remove(const std::string &key, uint64_t tranc_id) {
  // TODO: Lab 4.1 删除
  // ? 在 LSM 中，删除实际上是插入一个空值
  // ? 由于 put 操作可能触发 flush
  // ? 如果触发了 flush 则返回新刷盘的 sst 的 id
  // ? 在没有实现  flush 的情况下，你返回 0即可
  return 0;
}

uint64_t LSMEngine::remove_batch(const std::vector<std::string> &keys,
                                 uint64_t tranc_id) {
  // TODO: Lab 4.1 批量删除
  // ? 在 LSM 中，删除实际上是插入一个空值
  // ? 由于 put 操作可能触发 flush
  // ? 如果触发了 flush 则返回新刷盘的 sst 的 id
  // ? 在没有实现  flush 的情况下，你返回 0即可
  return 0;
}

void LSMEngine::clear() {
  memtable.clear();
  level_sst_ids.clear();
  ssts.clear();
  // 清空当前文件夹的所有内容
  try {
    for (const auto &entry : std::filesystem::directory_iterator(data_dir)) {
      if (!entry.is_regular_file()) {
        continue;
      }
      std::filesystem::remove(entry.path());

      spdlog::info("LSMEngine--"
                   "clear file {} successfully.",
                   entry.path().string());
    }
  } catch (const std::filesystem::filesystem_error &e) {
    // 处理文件系统错误
    spdlog::error("Error clearing directory: {}", e.what());
  }
}

uint64_t LSMEngine::flush() {
  // TODO: Lab 4.1 刷盘形成sst文件
}

std::string LSMEngine::get_sst_path(size_t sst_id, size_t target_level) {
  // sst的文件路径格式为: data_dir/sst_<sst_id>，sst_id格式化为32位数字
  std::stringstream ss;
  ss << data_dir << "/sst_" << std::setfill('0') << std::setw(32) << sst_id
     << '.' << target_level;
  return ss.str();
}

std::optional<std::pair<TwoMergeIterator, TwoMergeIterator>>
LSMEngine::lsm_iters_monotony_predicate(
    uint64_t tranc_id, std::function<int(const std::string &)> predicate) {
  // TODO: Lab 4.x 谓词查询
}

Level_Iterator LSMEngine::begin(uint64_t tranc_id) {
  return Level_Iterator(shared_from_this(), tranc_id);
}

Level_Iterator LSMEngine::end() { return Level_Iterator{}; }

void LSMEngine::full_compact(size_t src_level) {
  // 将 src_level 的 sst 全体压缩到 src_level + 1

  // 递归地判断下一级 level 是否需要 full compact
  if (level_sst_ids[src_level + 1].size() >=
      TomlConfig::getInstance().getLsmSstLevelRatio()) {
    full_compact(src_level + 1);
  }

  spdlog::debug("LSMEngine--"
                "Compaction: Starting full compaction from level{} to level{}",
                src_level, src_level + 1);

  // 获取源level和目标level的 sst_id
  auto old_level_id_x = level_sst_ids[src_level];
  auto old_level_id_y = level_sst_ids[src_level + 1];
  std::vector<std::shared_ptr<SST>> new_ssts;
  std::vector<size_t> lx_ids(old_level_id_x.begin(), old_level_id_x.end());
  std::vector<size_t> ly_ids(old_level_id_y.begin(), old_level_id_y.end());
  if (src_level == 0) {
    // l0这一层不同sst的key有重叠, 需要额外处理
    new_ssts = full_l0_l1_compact(lx_ids, ly_ids);
  } else {
    new_ssts = full_common_compact(lx_ids, ly_ids, src_level + 1);
  }
  // 完成 compact 后移除旧的sst记录
  for (auto &old_sst_id : old_level_id_x) {
    ssts[old_sst_id]->del_sst();
    ssts.erase(old_sst_id);
  }
  for (auto &old_sst_id : old_level_id_y) {
    ssts[old_sst_id]->del_sst();
    ssts.erase(old_sst_id);
  }
  level_sst_ids[src_level].clear();
  level_sst_ids[src_level + 1].clear();

  cur_max_level = std::max(cur_max_level, src_level + 1);

  // 添加新的sst
  for (auto &new_sst : new_ssts) {
    level_sst_ids[src_level + 1].push_back(new_sst->get_sst_id());
    ssts[new_sst->get_sst_id()] = new_sst;
  }
  // 此处没必要reverse了
  std::sort(level_sst_ids[src_level + 1].begin(),
            level_sst_ids[src_level + 1].end());

  spdlog::debug("LSMEngine--"
                "Compaction: Finished compaction. New SSTs added at level{}",
                src_level + 1);
}

std::vector<std::shared_ptr<SST>>
LSMEngine::full_l0_l1_compact(std::vector<size_t> &l0_ids,
                              std::vector<size_t> &l1_ids) {
  // TODO: 这里需要补全的是对已经完成事务的删除
  std::vector<SstIterator> l0_iters;
  std::vector<std::shared_ptr<SST>> l1_ssts;

  for (auto id : l0_ids) {
    auto sst_it = ssts[id]->begin(0);
    l0_iters.push_back(sst_it);
  }
  for (auto id : l1_ids) {
    l1_ssts.push_back(ssts[id]);
  }
  // l0 的sst之间的key有重叠, 需要合并
  auto [l0_begin, l0_end] = SstIterator::merge_sst_iterator(l0_iters, 0);

  std::shared_ptr<HeapIterator> l0_begin_ptr = std::make_shared<HeapIterator>();
  *l0_begin_ptr = l0_begin;

  std::shared_ptr<ConcactIterator> old_l1_begin_ptr =
      std::make_shared<ConcactIterator>(l1_ssts, 0);

  TwoMergeIterator l0_l1_begin(l0_begin_ptr, old_l1_begin_ptr, 0);

  return gen_sst_from_iter(l0_l1_begin,
                           TomlConfig::getInstance().getLsmPerMemSizeLimit() *
                               TomlConfig::getInstance().getLsmSstLevelRatio(),
                           1);
}

std::vector<std::shared_ptr<SST>>
LSMEngine::full_common_compact(std::vector<size_t> &lx_ids,
                               std::vector<size_t> &ly_ids, size_t level_y) {
  // TODO 需要补全已完成事务的滤除
  std::vector<std::shared_ptr<SST>> lx_iters;
  std::vector<std::shared_ptr<SST>> ly_iters;

  for (auto id : lx_ids) {
    lx_iters.push_back(ssts[id]);
  }
  for (auto id : ly_ids) {
    ly_iters.push_back(ssts[id]);
  }

  std::shared_ptr<ConcactIterator> old_lx_begin_ptr =
      std::make_shared<ConcactIterator>(lx_iters, 0);

  std::shared_ptr<ConcactIterator> old_ly_begin_ptr =
      std::make_shared<ConcactIterator>(ly_iters, 0);

  TwoMergeIterator lx_ly_begin(old_lx_begin_ptr, old_ly_begin_ptr, 0);

  // TODO:如果目标 level 的下一级 level+1 不存在, 则为底层的level,
  // 可以清理掉删除标记

  return gen_sst_from_iter(lx_ly_begin, LSMEngine::get_sst_size(level_y),
                           level_y);
}

std::vector<std::shared_ptr<SST>>
LSMEngine::gen_sst_from_iter(BaseIterator &iter, size_t target_sst_size,
                             size_t target_level) {
  // TODO: 这里需要补全的是对已经完成事务的删除

  std::vector<std::shared_ptr<SST>> new_ssts;
  auto new_sst_builder =
      SSTBuilder(TomlConfig::getInstance().getLsmBlockSize(), true);
  while (iter.is_valid() && !iter.is_end()) {

    new_sst_builder.add((*iter).first, (*iter).second, 0);
    ++iter;

    if (new_sst_builder.estimated_size() >= target_sst_size) {
      size_t sst_id = next_sst_id++; // TODO: 后续优化并发性
      std::string sst_path = get_sst_path(sst_id, target_level);
      auto new_sst = new_sst_builder.build(sst_id, sst_path, this->block_cache);
      new_ssts.push_back(new_sst);

      spdlog::debug("LSMEngine--"
                    "Compaction: Generated new SST file with sst_id={}"
                    "at level{}",
                    sst_id, target_level);

      new_sst_builder = SSTBuilder(TomlConfig::getInstance().getLsmBlockSize(),
                                   true); // 重置builder
    }
  }
  if (new_sst_builder.estimated_size() > 0) {
    size_t sst_id = next_sst_id++; // TODO: 后续优化并发性
    std::string sst_path = get_sst_path(sst_id, target_level);
    auto new_sst = new_sst_builder.build(sst_id, sst_path, this->block_cache);
    new_ssts.push_back(new_sst);

    spdlog::debug("LSMEngine--"
                  "Compaction: Generated new SST file with sst_id={} "
                  "at level{}",
                  sst_id, target_level);
  }

  return new_ssts;
}

size_t LSMEngine::get_sst_size(size_t level) {
  if (level == 0) {
    return TomlConfig::getInstance().getLsmPerMemSizeLimit();
  } else {
    return TomlConfig::getInstance().getLsmPerMemSizeLimit() *
           static_cast<size_t>(std::pow(
               TomlConfig::getInstance().getLsmSstLevelRatio(), level));
  }
}

// *********************** LSM ***********************
LSM::LSM(std::string path)
    : engine(std::make_shared<LSMEngine>(path)),
      tran_manager_(std::make_shared<TranManager>(path)) {
  tran_manager_->set_engine(engine);
  auto check_recover_res = tran_manager_->check_recover();
  for (auto &[tranc_id, records] : check_recover_res) {
    tran_manager_->update_max_finished_tranc_id(tranc_id);
    for (auto &record : records) {
      if (record.getOperationType() == OperationType::PUT) {
        engine->put(record.getKey(), record.getValue(), tranc_id);
      } else if (record.getOperationType() == OperationType::DELETE) {
        engine->remove(record.getKey(), tranc_id);
      }
    }
    spdlog::debug("LSMEngine--"
                  "Recover: Recovered transaction with tranc_id={}",
                  tranc_id);
  }
  tran_manager_->init_new_wal();
}

LSM::~LSM() {
  flush_all();
  tran_manager_->write_tranc_id_file();
}

std::optional<std::string> LSM::get(const std::string &key) {
  auto tranc_id = tran_manager_->getNextTransactionId();
  auto res = engine->get(key, tranc_id);

  if (res.has_value()) {
    return res.value().first;
  }
  return std::nullopt;
}

std::vector<std::pair<std::string, std::optional<std::string>>>
LSM::get_batch(const std::vector<std::string> &keys) {
  // 1. 获取事务ID
  auto tranc_id = tran_manager_->getNextTransactionId();

  // 2. 调用 engine 的批量查询接口
  auto batch_results = engine->get_batch(keys, tranc_id);

  // 3. 构造最终结果
  std::vector<std::pair<std::string, std::optional<std::string>>> results;
  for (const auto &[key, value] : batch_results) {
    if (value.has_value()) {
      results.emplace_back(key, value->first); // 提取值部分
    } else {
      results.emplace_back(key, std::nullopt); // 键不存在
    }
  }

  return results;
}

void LSM::put(const std::string &key, const std::string &value) {
  auto tranc_id = tran_manager_->getNextTransactionId();
  engine->put(key, value, tranc_id);
}

void LSM::put_batch(
    const std::vector<std::pair<std::string, std::string>> &kvs) {
  auto tranc_id = tran_manager_->getNextTransactionId();
  engine->put_batch(kvs, tranc_id);
}
void LSM::remove(const std::string &key) {
  auto tranc_id = tran_manager_->getNextTransactionId();
  engine->remove(key, tranc_id);
}

void LSM::remove_batch(const std::vector<std::string> &keys) {
  auto tranc_id = tran_manager_->getNextTransactionId();
  engine->remove_batch(keys, tranc_id);
}

void LSM::clear() { engine->clear(); }

void LSM::flush() { auto max_tranc_id = engine->flush(); }

void LSM::flush_all() {
  while (engine->memtable.get_total_size() > 0) {
    auto max_tranc_id = engine->flush();
    tran_manager_->update_max_flushed_tranc_id(max_tranc_id);
  }
}

LSM::LSMIterator LSM::begin(uint64_t tranc_id) {
  return engine->begin(tranc_id);
}

LSM::LSMIterator LSM::end() { return engine->end(); }

std::optional<std::pair<TwoMergeIterator, TwoMergeIterator>>
LSM::lsm_iters_monotony_predicate(
    uint64_t tranc_id, std::function<int(const std::string &)> predicate) {
  return engine->lsm_iters_monotony_predicate(tranc_id, predicate);
}

// 开启一个事务
std::shared_ptr<TranContext>
LSM::begin_tran(const IsolationLevel &isolation_level) {
  auto tranc_context = tran_manager_->new_tranc(isolation_level);

  spdlog::info("LSM--"
               "lsm_iters_monotony_predicate: Starting query for tranc_id={}",
               tranc_context->tranc_id_);

  return tranc_context;
}

void LSM::set_log_level(const std::string &level) { reset_log_level(level); }
} // namespace toni_lsm