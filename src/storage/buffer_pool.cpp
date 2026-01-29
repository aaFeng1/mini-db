#include "storage/buffer_pool.h"
#include <numeric>

namespace mini {
BufferPool::BufferPool(std::size_t pool_size, DiskManager *disk)
    : pages_(pool_size), meta_(pool_size), free_list_(pool_size), disk_(disk),
      hand_(0), pool_size_(pool_size) {
  std::iota(free_list_.begin(), free_list_.end(), 0);
}

BufferPool::~BufferPool() {}

Page *BufferPool::FetchPage(page_id_t pid) {
  // 需要选择从磁盘加载到内存，然后返回内存地址
  auto it = page_table_.find(pid);
  if (it != page_table_.end()) {
    frame_id_t fid = it->second;
    meta_[fid].pin_count++;
    return &pages_[fid];
  }

  frame_id_t fid = FindVictimFrame();
  if (fid == -1)
    return nullptr;

  page_id_t old = meta_[fid].page_id;
  if (old != -1) {
    page_table_.erase(old);
  }

  if (meta_[fid].is_dirty) {
    disk_->WritePage(meta_[fid].page_id, pages_[fid]);
    meta_[fid].is_dirty = false;
  }
  disk_->ReadPage(pid, pages_[fid]);
  page_table_[pid] = fid;

  meta_[fid].page_id = pid;
  meta_[fid].pin_count = 1;
  meta_[fid].is_dirty = false;

  return &pages_[fid];
}

bool BufferPool::UnpinPage(page_id_t pid, bool is_dirty) {
  // 由上层调用，表明要取消pin并且告诉我们是否被写过
  auto it = page_table_.find(pid);
  if (it != page_table_.end()) {
    frame_id_t fid = it->second;
    if (meta_[fid].pin_count == 0)
      return false;
    meta_[fid].pin_count--;
    meta_[fid].is_dirty |= is_dirty;

    return true;
  }

  return false;
}

bool BufferPool::FlushPage(page_id_t pid) {
  // 如果是脏页，将某页写回磁盘，并且删掉脏页标记
  auto it = page_table_.find(pid);
  if (it != page_table_.end()) {
    frame_id_t fid = it->second;
    if (meta_[fid].page_id != -1 && meta_[fid].is_dirty) {
      disk_->WritePage(meta_[fid].page_id, pages_[fid]);
      meta_[fid].is_dirty = false;
    }
    return true;
  }

  return false;
}

void BufferPool::FlushAllPages() {
  // 写回所有页
  for (std::size_t i = 0; i < pool_size_; ++i) {
    if (meta_[i].page_id != -1 && meta_[i].is_dirty) {
      disk_->WritePage(meta_[i].page_id, pages_[i]);
      meta_[i].is_dirty = false;
    }
  }
  return;
}

PageGuard BufferPool::FetchPageGuarded(page_id_t pid) {
  Page *page = FetchPage(pid);
  return PageGuard(this, pid, page);
}

int BufferPool::FindVictimFrame() {
  // 返回可用槽位，可能会失败
  if (!free_list_.empty()) {
    int res = free_list_.front();
    free_list_.pop_front();
    return res;
  }
  // free_list_ is empty
  for (int i = 0; i < (int)pool_size_; i++) {
    int fid = hand_;
    hand_ = (hand_ + 1) % pool_size_;
    if (meta_[fid].pin_count == 0)
      return fid;
  }

  return -1;
}

} // namespace mini
