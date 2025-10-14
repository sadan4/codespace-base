#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <iterator>
#include <string>
#include <unistd.h>
#include <vector>

using namespace std::chrono_literals;
namespace fs = std::filesystem;

namespace {
bool isWhitespace(char c) {
  switch (c) {
  case ' ':
  case '\n':
  case '\t':
    return true;
  default:
    return false;
  }
}
constexpr void triml(std::string &str) {
  while (!str.empty() && isWhitespace(str.back())) {
    str.erase(str.end() - 1, str.end());
  }
}
constexpr void trimr(std::string &str) {
  while (!str.empty() && isWhitespace(str.front())) {
    str.erase(str.begin(), str.begin() + 1);
  }
}
constexpr void trim(std::string &str) {
  triml(str);
  trimr(str);
}
std::string getStdinNonInteractive() {

  if (isatty(STDIN_FILENO)) {
    return "<TTY>";
  } else {
    return {std::istreambuf_iterator{std::cin},
            std::istreambuf_iterator<char>{}};
  }
}
std::vector<fs::directory_entry> listOtherOpenFds() {
  static const fs::path openFdPath{"/proc/self/fd"};
  return {fs::directory_iterator{openFdPath}, fs::directory_iterator{}};
}
std::string buildPerms(const fs::perms &perms) {
  std::stringstream sb{};
  using p = fs::perms;
  auto add = [&](char c, p cond) {
    if (p::none != (perms & cond)) {
      sb.put(c);
    } else {
      sb.put('-');
    }
  };
  add('r', p::owner_read);
  add('w', p::owner_write);
  add('x', p::owner_exec);
  add('r', p::group_read);
  add('w', p::group_write);
  add('x', p::group_exec);
  add('r', p::others_read);
  add('w', p::others_write);
  add('x', p::others_exec);
  return sb.str();
}

std::string buildType(const fs::file_type &type) {
  switch (type) {
  case fs::file_type::none:
    return "has `not-evaluated-yet` type";
  case fs::file_type::not_found:
    return "does not exist";
  case fs::file_type::regular:
    return "is a regular file";
  case fs::file_type::directory:
    return "is a directory";
  case fs::file_type::symlink:
    return "is a symlink";
  case fs::file_type::block:
    return "is a block device";
  case fs::file_type::character:
    return "is a character device";
  case fs::file_type::fifo:
    return "is a named IPC pipe";
  case fs::file_type::socket:
    return "is a named IPC socket";
  case fs::file_type::unknown:
    return "has `unknown` type";
  default:
    return "has `implementation-defined` type";
  }
}
std::string formatPath(const fs::path &path) {
  std::string cmd = std::format("file {}", std::string{path});
  FILE *fd = popen(cmd.c_str(), "r");
  char buffer[512];
  auto bytesRead = 0uz;
  std::stringstream str;
  while ((bytesRead = std::fread(buffer, sizeof(*buffer), sizeof(buffer), fd)) >
         0) {
    str.write(buffer, bytesRead);
  }
  if (bytesRead == -1) {
    std::perror("read");
    std::abort();
  }
  std::fclose(fd);
  auto ret = str.str();
  trim(ret);
  return ret;
}
void printOtherOpenFds() {
  std::cout << "Open fds" << '\n';
  for (auto const &openFd : listOtherOpenFds()) {
    if (fs::exists(openFd)) {
      std::cout << formatPath(openFd.path()) << '\n';
    }
  }
}
} // namespace

int main(int argc, char **argv) {
  std::cout << "stdin: " << getStdinNonInteractive() << std::endl;
  std::cout << "argc = " << argc << std::endl;
  for (int i = 0; i < argc; i++) {
    std::cout << std::format("arg[{}] = {}", i, argv[i]) << std::endl;
  }
  printOtherOpenFds();
}