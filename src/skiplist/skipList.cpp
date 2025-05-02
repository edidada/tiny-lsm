#include "../../include/skiplist/skiplist.h"
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <utility>

// ************************ SkipListIterator ************************

BaseIterator &SkipListIterator::operator++() {
  // TODO: 实现迭代器++操作符
  return *this;
}

bool SkipListIterator::operator==(const BaseIterator &other) const {
  if (other.get_type() != IteratorType::SkipListIterator)
    return false;
  auto other2 = dynamic_cast<const SkipListIterator &>(other);
  return current == other2.current;
}

bool SkipListIterator::operator!=(const BaseIterator &other) const {
  return !(*this == other);
}

SkipListIterator::value_type SkipListIterator::operator*() const {
  if (!current)
    throw std::runtime_error("Dereferencing invalid iterator");
  return {current->key_, current->value_};
}

IteratorType SkipListIterator::get_type() const {
  return IteratorType::SkipListIterator;
}

bool SkipListIterator::is_valid() const {
  return current && !current->key_.empty();
}
bool SkipListIterator::is_end() const { return current == nullptr; }

std::string SkipListIterator::get_key() const { return current->key_; }
std::string SkipListIterator::get_value() const { return current->value_; }
uint64_t SkipListIterator::get_tranc_id() const { return current->tranc_id_; }

// ************************ SkipList ************************
// 构造函数
SkipList::SkipList(int max_lvl) : max_level(max_lvl), current_level(1) {
  head = std::make_shared<SkipListNode>("", "", 0);
}

int SkipList::random_level() {
  // TODO: 实现随机生成level函数
  // 通过"抛硬币"的方式随机生成层数：
  // - 每次有50%的概率增加一层
  // - 确保层数分布为：第1层100%，第2层50%，第3层25%，以此类推
  // - 层数范围限制在[1, max_level]之间，避免浪费内存
  return -1;
}

// 插入或更新键值对
void SkipList::put(const std::string &key, const std::string &value,
                   uint64_t tranc_id) {
  // TODO: 实现在跳表中删除键为key的函数
  // 注意：需要维护size_bytes
}

// 查找键值对
SkipListIterator SkipList::get(const std::string &key, uint64_t tranc_id) {
  // TODO: 实现跳表的查找函数
  // 注意：在lab1中无需注意tranc_id
  return SkipListIterator{};
}

// 删除键值对
// ! 这里的 remove 是跳表本身真实的 remove,  lsm 应该使用 put 空值表示删除,
// ! 这里只是为了实现完整的 SkipList 不会真正被上层调用
void SkipList::remove(const std::string &key) {
  // TODO: 实现在跳表中删除键为key的函数
  // 注意：需要维护size_bytes
}

// 刷盘时可以直接遍历最底层链表
std::vector<std::tuple<std::string, std::string, uint64_t>> SkipList::flush() {
  // TODO: 实现跳表的导出函数
  // tuple格式为：key, value, tranc_id
  return {};
}

size_t SkipList::get_size() {
  // std::shared_lock<std::shared_mutex> slock(rw_mutex);
  return size_bytes;
}

// 清空跳表，释放内存
void SkipList::clear() {
  // std::unique_lock<std::shared_mutex> lock(rw_mutex);
  head = std::make_shared<SkipListNode>("", "", 0);
  size_bytes = 0;
}

SkipListIterator SkipList::begin() {
  // TODO: 返回跳表的第一个节点
  return SkipListIterator();
}

SkipListIterator SkipList::end() {
  return SkipListIterator(); // 使用空构造函数
}

// 找到前缀的起始位置
// 返回第一个前缀匹配或者大于前缀的迭代器
SkipListIterator SkipList::begin_preffix(const std::string &preffix) {
  // TODO: 找到前缀为prefix第一个节点
  return SkipListIterator();
}

// 找到前缀的终结位置
SkipListIterator SkipList::end_preffix(const std::string &prefix) {
  // TODO: 找到前缀为prefix最后一个节点
  return SkipListIterator();
}

// 返回第一个满足谓词的位置和最后一个满足谓词的迭代器
// 如果不存在, 范围nullptr
// 谓词作用于key, 且保证满足谓词的结果只在一段连续的区间内, 例如前缀匹配的谓词
// predicate返回值:
//   0: 谓词
//   >0: 不满足谓词, 需要向右移动
//   <0: 不满足谓词, 需要向左移动
// ! Skiplist 中的谓词查询不会进行事务id的判断, 需要上层自己进行判断
std::optional<std::pair<SkipListIterator, SkipListIterator>>
SkipList::iters_monotony_predicate(
    std::function<int(const std::string &)> predicate) {
  // TODO: 返回第一个满足谓词的位置和最后一个满足谓词的迭代器
  return {};
}

void SkipList::print_skiplist() {
  // TODO: 实现打印跳表方法，用于debug
}