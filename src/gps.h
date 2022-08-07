#pragma once

#include <cstdint>
#include "messages.h"

class GPS
{
private:
  enum State {
    Idle,
    Byte_AA,
    Byte_44,
    Parsing_Message
  };

  State state = Idle;

  uint16_t byte_index;
  uint8_t header_len = 28;
  uint8_t message_len = 200;
  uint16_t message_id;

  uint8_t buf[256];
public:
  GPS();
  ~GPS();

  bool ingest(uint8_t byte);
  uint16_t get_message_id() { return message_id; }
  Header parse_header();
  BestPos parse_bestpos();
  BestVel parse_bestvel();
  Time parse_time();
};