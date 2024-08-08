#pragma once

#include <string_view>

#include <ArduinoJson.h>
#include <WString.h>
#include <etl/hash.h>

namespace HexFF {

  /// @brief Hash and equal function to be able to use containers on type const char *
  struct cstring_hash {
    size_t operator()(const char *v) const {
      int length = strlen(v);
      const uint8_t *p = reinterpret_cast<const uint8_t *>(v);
      return etl::private_hash::generic_hash<size_t>(p, p + length);
    }
  };

  struct cstring_equal_to {
    bool operator()(const char *a, const char *b) const { return strcmp(a, b) == 0; }
  };

  /// @brief Split a character array and store each token in array of String
  /// @param source the secquence to parse
  /// @param destiny where the result will be stored
  /// @param destinySize size of the destiny array
  /// @param delimiter string that identify the delimiter
  /// @return number of the parsed items.
  int split(const String &source, String *destiny, int destinySize, char const *delimiter);

  //TODO Create tokenizer
  std::string_view getToken(std::string_view &source, const std::string_view &delimiters);

  // Convert a mac address string to an array of uint8_t
  bool str2mac(const char *mac, uint8_t *values);

  void reboot(const char *msg);

  bool isFilesystemInitialized();

  /// @brief: Deep Merge arduino JSONs
  void merge(JsonVariant dst, JsonVariantConst src);

}  // namespace HexFF