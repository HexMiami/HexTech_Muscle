#include "HexFF/utils.h"

#include <memory>

#include <Arduino.h>
#include <LittleFS.h>
#include <WString.h>

namespace HexFF {

  int split(const String &source, String *destiny, int destinySize, char const *delimiter) {
    int length = source.length();
    char sourceStr[length + 1];
    sourceStr[length] = '\0';
    source.toCharArray(sourceStr, length + 1);

    /* First call to strtok should be done with string and delimiter as first and second parameter*/
    const char *nextToken = strtok(sourceStr, delimiter);

    /* Consecutive calls to the strtok should be with first parameter as NULL and second parameter
     * as delimiter
     * * return value of the strtok will be the split string based on delimiter*/
    int parsedItems = 0;
    while (nextToken != NULL && parsedItems < destinySize) {
      destiny[parsedItems] = nextToken;
      ++parsedItems;
      nextToken = strtok(NULL, delimiter);
    }
    return parsedItems;
  }


  std::string_view getToken(std::string_view &source, const std::string_view &delimiters) {
    auto endPos = source.find_first_of(delimiters);
    auto token = source.substr(0, endPos);

    source.remove_prefix(std::min(token.size() + 1, source.size()));

    return token;
  }


  bool str2mac(const char *mac, uint8_t *values) {
    if (values != nullptr && mac != nullptr
        && 6
               == sscanf(mac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &values[0], &values[1], &values[2],
                         &values[3], &values[4], &values[5])) {
      return true;
    } else {
      return false;
    }
  }


  void reboot(const char *msg) {
    log_e("%s. Rebooting ...", msg);
    delay(5000);
    esp_restart();
  };


  bool isFilesystemInitialized() {
    static bool result = LittleFS.begin();
    return result;
  }


  void merge(JsonVariant dst, JsonVariantConst src) {
    if (src.is<JsonObjectConst>()) {
      for (JsonPairConst kvp : src.as<JsonObjectConst>()) {
        if (dst[kvp.key()])
          merge(dst[kvp.key()], kvp.value());
        else
          dst[kvp.key()] = kvp.value();
      }
    }
    else {
      dst.set(src);
    }
  }

}  // namespace HexFF
