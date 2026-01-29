#include "storage/disk_manager.h"

#include <cerrno>
#include <cstring>
#include <stdexcept>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace mini {

static std::runtime_error SysErr(const std::string &what) {
  return std::runtime_error(what + " (errno=" + std::to_string(errno) + ", " +
                            std::strerror(errno) + ")");
}

long long DiskManager::OffsetOf(page_id_t page_id) {
  return static_cast<long long>(page_id) * static_cast<long long>(PAGE_SIZE);
}

DiskManager::DiskManager(const std::string &file_path) : file_path_(file_path) {
  fd_ = ::open(file_path_.c_str(), O_RDWR | O_CREAT, 0644);
  if (fd_ < 0)
    throw SysErr("open failed: " + file_path_);
}

DiskManager::~DiskManager() {
  if (fd_ >= 0)
    ::close(fd_);
}

void DiskManager::WritePage(page_id_t page_id, const Page &page) {
  const auto off = OffsetOf(page_id);
  if (::lseek(fd_, off, SEEK_SET) < 0)
    throw SysErr("lseek failed (write)");
  ssize_t n = ::write(fd_, page.GetConstData(), PAGE_SIZE);
  if (n < 0)
    throw SysErr("write failed");
  if (n != static_cast<ssize_t>(PAGE_SIZE))
    throw std::runtime_error("partial write");
  ::fsync(fd_); // 小项目先“稳”，别先追性能
}

void DiskManager::ReadPage(page_id_t page_id, Page &page) {
  const auto off = OffsetOf(page_id);
  if (::lseek(fd_, off, SEEK_SET) < 0)
    throw SysErr("lseek failed (read)");
  ssize_t n = ::read(fd_, page.GetData(), PAGE_SIZE);
  if (n < 0)
    throw SysErr("read failed");

  // 文件不够长时，read 可能返回 0 或不足 PAGE_SIZE ——剩余部分补 0
  if (n < static_cast<ssize_t>(PAGE_SIZE)) {
    std::memset(page.GetData() + n, 0, PAGE_SIZE - static_cast<size_t>(n));
  }
}

} // namespace mini
