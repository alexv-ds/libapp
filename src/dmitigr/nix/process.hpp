// -*- C++ -*-
//
// Copyright 2025 Dmitry Igrishin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DMITIGR_NIX_PROCESS_HPP
#define DMITIGR_NIX_PROCESS_HPP

#include <chrono>
#include <csignal>
#include <filesystem>
#include <string>
#include <system_error>
#include <thread>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef __APPLE__
#include <libproc.h>
#endif

namespace dmitigr::nix {

inline void wait_for_exit(const pid_t pid,
  const std::chrono::milliseconds polling_interval)
{
  while (true) {
    if (kill(pid, 0) < 0) {
      if (errno == ESRCH)
        break;
      else
        throw std::system_error{errno, std::generic_category()};
    }
    std::this_thread::sleep_for(polling_interval);
  }
}

inline int wait(const pid_t pid, const int options = {})
{
  int wstatus{};
  if (waitpid(pid, &wstatus, options) < 0)
    throw std::system_error{errno, std::generic_category()};
  return wstatus;
}

#if defined (__linux__) || defined (__APPLE__)
inline std::filesystem::path process_image_path(const pid_t pid)
{
#if defined (__APPLE__)
  std::string result(PROC_PIDPATHINFO_MAXSIZE, 0);
  if (proc_pidpath(pid, result.data(), result.size()) <= 0)
    throw std::system_error{errno, std::system_category()};
  return result;
#elif defined (__linux__)
  return std::filesystem::read_symlink("/proc/"+std::to_string(pid)+"/exe");
#endif
}
#endif

} // dmitigr::nix

#endif  // DMITIGR_NIX_PROCESS_HPP
