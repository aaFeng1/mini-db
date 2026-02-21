#pragma once
#include "common/page.h"
#include "common/page_guard.h"
#include "storage/disk_manager.h"
#include <cstddef>
#include <deque>
#include <unordered_map>
#include <vector>

namespace mini {

using frame_id_t = int32_t;

class BufferPool {
public:
  explicit BufferPool(std::size_t pool_size, DiskManager *disk);
  ~BufferPool();

  Page *FetchPage(page_id_t pid);
  Page *NewPage(page_id_t *pid);
  bool UnpinPage(page_id_t pid, bool is_dirty);
  bool FlushPage(page_id_t pid);
  void FlushAllPages();

  PageGuard FetchPageGuarded(page_id_t pid);
  PageGuard NewPageGuarded(page_id_t *pid);

private:
  struct FrameMeta {
    page_id_t page_id = -1;
    int pin_count = 0;
    bool is_dirty = false;
  };

  std::vector<Page> pages_;     // 所有的槽位
  std::vector<FrameMeta> meta_; // 帧信息，包括pin数，是否为脏

  std::unordered_map<page_id_t, frame_id_t>
      page_table_;            // 保存page_id到槽位下表的映射
  std::deque<int> free_list_; // 空闲页的下标链表

  DiskManager *disk_; // 用于写内存和写文件

  int hand_; // 为实现基本替换策略，用循环枚举的方式，hand_为寻找的起点
  std::size_t pool_size_;

  int FindVictimFrame();
};

} // namespace mini
