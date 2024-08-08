#pragma once
#include <WString.h>

#include <array>

namespace HexFF {
  class MessageDispatcher {
  protected:
    MessageDispatcher() = default;
    virtual ~MessageDispatcher() = default;
    MessageDispatcher(MessageDispatcher &&) = delete;
    MessageDispatcher &operator=(MessageDispatcher &&) = delete;
    MessageDispatcher(const MessageDispatcher &) = delete;
    MessageDispatcher &operator=(const MessageDispatcher &) = delete;

  public:
    static const int MEESAGE_LENGTH = 256;
    using Message = std::array<char, MEESAGE_LENGTH + 1>;

    static MessageDispatcher &getInstance();
    virtual bool push(const Message &) = 0;
    virtual bool push(const String &) = 0;
    virtual bool pop(Message &) = 0;
    virtual bool popPriority(Message &) = 0;
  };
}  // namespace HexFF