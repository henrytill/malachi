#pragma once

#include <map>
#include <string>

namespace test {

class MockEnvironment {
public:
  void set(const std::string &key, const std::string &value) {
    env_[key] = value;
  }

  auto get(const char *name) -> char * {
    if (auto iter = env_.find(name); iter != env_.end()) {
      return iter->second.data();
    }
    return nullptr;
  }

private:
  std::map<std::string, std::string> env_;
};

} // namespace test
