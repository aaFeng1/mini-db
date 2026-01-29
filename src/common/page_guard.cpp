#include "common/page_guard.h"
#include "storage/buffer_pool.h"

namespace mini {

PageGuard::PageGuard(BufferPool *bpm, page_id_t pid, Page *page)
    : bpm_(bpm), pid_(pid), page_(page) {}

PageGuard::PageGuard(PageGuard &&other) noexcept
    : bpm_(other.bpm_), pid_(other.pid_), page_(other.page_),
      dirty_(other.dirty_) {
  other.page_ = nullptr;
}

PageGuard::~PageGuard() {
  if (page_ != nullptr) {
    bpm_->UnpinPage(pid_, dirty_);
  }
}

} // namespace mini
