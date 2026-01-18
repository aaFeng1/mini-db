#include <cstring>
#include <iostream>

#include "storage/disk_manager.h"

int main() {
  mini::DiskManager dm("mini.db");

  mini::Page out;
  const char *msg = "page-0";
  std::memcpy(out.data.data(), msg, std::strlen(msg));
  dm.WritePage(0, out);

  msg = "page-1";
  std::memcpy(out.data.data(), msg, std::strlen(msg));
  dm.WritePage(100, out);

  mini::Page in;
  dm.ReadPage(0, in);
  std::cout << "read: " << reinterpret_cast<const char *>(in.data.data())
            << "\n";
  dm.ReadPage(100, in);
  std::cout << "read: " << reinterpret_cast<const char *>(in.data.data())
            << "\n";
  return 0;
}
