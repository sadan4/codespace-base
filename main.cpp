#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;

namespace {
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
  // std::error_code ec;
  // const auto entry = fs::status(path, ec);
  // if (ec) {
  //   return std::format("ERROR: {}", ec.message());
  // }
  // const auto perms = buildPerms(entry.permissions());
  // const auto type = buildType(entry.type());
  // return std::format("{} {} {}", perms, std::string{path}, type);
  static const char *PROG_NAME = "file";
  std::vector<const char *> _cmd = {PROG_NAME, path.c_str(), nullptr};
  auto cmd = const_cast<char *const *>(_cmd.data());
  int stdoutPipe[2] = {-1, -1};
  // int stderrPipe[2] = {-1, -1};
  if (pipe(stdoutPipe)) {
    throw std::runtime_error("pipe failed");
  }

  const int pid = fork();
  if (pid == -1) {
    throw std::runtime_error("vfork failed");
  } else if (pid) {
    const auto stdoutFd = stdoutPipe[0];
    char buffer[512];
    int bytesRead;
    std::stringstream ret;
    while ((bytesRead = read(stdoutFd, buffer, sizeof(buffer)) > 0)) {
      ret.write(buffer, bytesRead);
    }
    close(stdoutFd);
    return ret.str();
  } else {
    // child
    dup2(stdoutPipe[1], STDOUT_FILENO);
    // dup2(stderrPipe[1], STDERR_FILENO);
    execvp(PROG_NAME, cmd);
    throw std::runtime_error("execvp failed");
  }
}
void printOtherOpenFds() {
  for (auto const &openFd : listOtherOpenFds()) {

    // if (openFd.is_regular_file()) {
    //   std::cout << std::format("fd: {} = Unexpected Regular File",
    //                            std::string{openFd.path()})
    //             << std::endl;
    // } else if (openFd.is_directory()) {
    //   std::cout << std::format("fd: {} = Unexpected Dir",
    //                            std::string{openFd.path()})
    //             << std::endl;
    // } else {
    //   std::cout << std::format("fd: {} = {}", std::string{openFd.path()},
    //                            formatPath(openFd.path()))
    //             << std::endl;
    // }
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