#pragma once
#include "common/page.h"

namespace mini {

class BufferPool;

class PageGuard {
public:
  PageGuard(BufferPool *bpm, page_id_t pid, Page *page);

  PageGuard(const PageGuard &) = delete;
  PageGuard &operator=(const PageGuard &) = delete;

  PageGuard(PageGuard &&other) noexcept;

  ~PageGuard();

  Page *GetPage() { return page_; }

  void SetDirty() { dirty_ = true; }

private:
  BufferPool *bpm_;
  page_id_t pid_;
  Page *page_;
  bool dirty_;
};

} // namespace mini
