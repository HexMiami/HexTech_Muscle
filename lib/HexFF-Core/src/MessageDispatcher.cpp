#include "HexFF/MessageDispatcher.h"

#include <Arduino.h>

#include <etl/queue_spsc_atomic.h>
#include <etl/unordered_map.h>

#include <SafeStringReader.h>

namespace HexFF {

  class MessageDispatcherImpl : public MessageDispatcher {
  public:
    static const int DEFAULT_QUEUE_MAXSIZE = 20;
    static const int WAITING_QUEUE_MAXSIZE = 20;
    static const int SOS_QUEUE_MAXSIZE = 2;
    static const char PIPE_CHAR = '|';

    /// TODO: Create the queues considering the time when message was pushed
    /// The message time can be used for debugging/logging
    using DefaultQueue = etl::queue_spsc_atomic<Message, DEFAULT_QUEUE_MAXSIZE>;
    using WaitingQueue = etl::queue_spsc_atomic<Message, WAITING_QUEUE_MAXSIZE>;
    using SOSQueue = etl::queue_spsc_atomic<Message, SOS_QUEUE_MAXSIZE>;

    DefaultQueue defaultQueue;
    WaitingQueue waitingQueue;
    SOSQueue sosQueue;
    bool parsePipeAndPush(char *cmdList);
    template <typename Q> bool push(Q &queue, const Message &msg) { return queue.push(msg); };
    bool push(const Message &) override;
    bool push(const String &messageIn) override;
    bool pop(Message &) override;
    bool popPriority(Message &) override;
    void clearAll();
  };

  bool MessageDispatcherImpl::parsePipeAndPush(char *cmdList) {
    createSafeStringFromCharPtr(inputLine, cmdList);  // enough space for the input text
    createSafeString(cmd,
                     257);  // for the field strings. Should have capacity > largest field length
    // firstToken/nextToken consumes input so this only runs once
    bool haveToken = inputLine.firstToken(cmd, PIPE_CHAR);
    while (haveToken) {
      MessageDispatcher::Message hmMsg;
      unsigned int length = cmd.length();
      hmMsg[length] = 0;
      strncpy(hmMsg.data(), cmd.c_str(), length);
      push(waitingQueue, hmMsg);
      haveToken
          = inputLine.nextToken(cmd, PIPE_CHAR);  // skips empty fields by default,  use
                                                  // optional arg (true) to return empty fields
    }
    return true;
  }

  bool MessageDispatcherImpl::push(const Message &messageIn) {
    /// If message start with P_. It is a priority message all queue are clreared and message is
    /// pushed to the default queue
    if (const char *data = messageIn.data(); data[0] == 'P' && data[1] == '_') {
      clearAll();
      MessageDispatcher::Message hmMsg;
      strcpy(hmMsg.data(), data + 2);
      return push(defaultQueue, hmMsg);
    }
    MessageDispatcher::Message hmMsg = messageIn;
    return parsePipeAndPush(hmMsg.data());
  }

  bool MessageDispatcherImpl::push(const String &messageIn) {
    if (!messageIn.isEmpty()) {
      MessageDispatcher::Message hmMsg;
      unsigned int length = messageIn.length();
      hmMsg[length] = 0;
      strncpy(hmMsg.data(), messageIn.c_str(), length);
      return push(hmMsg);
    } else {
      return false;
    }
  }

  bool MessageDispatcherImpl::popPriority(Message &messageOut) {
    return defaultQueue.pop(messageOut);
  }
  bool MessageDispatcherImpl::pop(Message &messageOut) {
    if (!waitingQueue.empty() && defaultQueue.empty()) {
      return waitingQueue.pop(messageOut);
    };
    return popPriority(messageOut);
  }

  void MessageDispatcherImpl::clearAll() {
    waitingQueue.clear();
    defaultQueue.clear();
    sosQueue.clear();
  }

  MessageDispatcher &MessageDispatcher::getInstance() {
    static MessageDispatcherImpl instance;
    return instance;
  }

}  // namespace HexFF
