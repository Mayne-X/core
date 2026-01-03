#pragma once

#include <string>
#include <vector>

struct KVProperty {
  std::string name;
  std::string value;
};
struct KVProperties {
  std::string title;
  std::vector<KVProperty> entries;
};
