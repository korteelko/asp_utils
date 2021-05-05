/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "asp_utils/Common.h"
#include "asp_utils/ErrorWrap.h"

#include <cassert>
#include <fstream>
#include <map>

namespace asp_utils {
static std::map<io_loglvl, std::string> level_names = {{no_log, "no logs"},
                                                       {err_logs, "err"},
                                                       {warn_logs, "warn"},
                                                       {info_logs, "info"},
                                                       {debug_logs, "debug"}};

std::string io_loglvl_str(io_loglvl ll) {
  auto l = level_names.find(ll);
  return (l != level_names.end()) ? l->second : "undefiend io_loglvl";
}

std::string trim_str(const std::string& str) {
  if (str.empty())
    return "";
  auto wsfront = std::find_if_not(str.begin(), str.end(),
                                  [](int c) { return std::isspace(c); });
  return std::string(
      wsfront, std::find_if_not(str.rbegin(),
                                std::string::const_reverse_iterator(wsfront),
                                [](int c) { return std::isspace(c); })
                   .base());
}

bool is_exists(const fs::path& path) {
  return fs::exists(path);
}

bool is_exists(const std::string& path) {
  return fs::exists(path);
}

std::string dir_by_path(const std::string& path) {
  return std::filesystem::path(path).parent_path().string();
}

bool is_equal(double a, double b, double accur) {
  return (std::abs(a - b) < accur);
}

bool is_equal(std::complex<double> a, std::complex<double> b, double accur) {
  return is_equal(a.real(), b.real(), accur) &&
         is_equal(a.imag(), b.imag(), accur);
}
std::stringstream read_file(const fs::path& path, ErrorWrap& error) {
  std::stringstream wss;
  if (fs::exists(path)) {
    std::ifstream wif(path);
    wss << wif.rdbuf();
  } else {
    error.SetError(ERROR_FILE_EXISTS_ST,
                   "File open error for: " + path.string());
  }
  return wss;
}
}  // namespace asp_utils
