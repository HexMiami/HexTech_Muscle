#pragma once

#include <algorithm>
#include <optional>
#include <vector>

namespace HexFF {

template <typename Key, typename Value>
struct Map;

//=======

template <typename TObject, typename TCreator>
class Factory {
 public:
  Factory() = delete;

  static constexpr bool add(std::string_view type_name, TCreator creator) {
    return !s_methods.at(type_name) && s_methods.insert(type_name, creator);
  }

  template <typename... Args>
  static TObject* create(const std::string_view& type_name, Args... args) {
    auto val = s_methods.at(type_name);

    if (!val) {
      return nullptr;
    }

    return (**val)(args...);
  }

 protected:
  static inline constinit Map<std::string_view, TCreator> s_methods;
};

//=======

template <typename Key, typename Value>
struct Map {
  std::vector<std::pair<Key, Value>> data;

  constexpr bool insert(Key key, Value val) {
    data.emplace_back(std::move(key), std::move(val));
    return true;
  }

  [[nodiscard]] constexpr std::optional<const Value*> at(const Key& key) const {
    const auto itr = std::find_if(begin(data), end(data), [&key](const auto& v) { return v.first == key; });

    if (itr == end(data)) {
      return std::nullopt;
    }

    return &(itr->second);
  }
};

}  // namespace HexFF
