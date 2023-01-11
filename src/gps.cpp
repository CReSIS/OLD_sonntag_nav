#include "gps.h"

#include <QTextStream>

GPS::GPS() {}

GPS::~GPS() {}

bool GPS::ingest(uint8_t byte) {
  switch (state)
  {
  case Idle:
    message_id = 0;
    if (byte == 0xAA) state = Byte_AA;
    break;
  case Byte_AA:
    if (byte == 0x44) state = Byte_44;
    else state = Idle;
    break;
  case Byte_44:
    if (byte == 0x12) {
        state = Parsing_Message;
        byte_index = 3;
    } else state = Idle;
    break;
  case Parsing_Message:
    buf[byte_index] = byte;
    switch (byte_index)
    {
    case 3:
      // printf("%s:%d: new message\n", __FILE__, __LINE__);
      header_len = byte;
      break;
    case 5:
      message_id = *((uint16_t*)(buf+4));
      break;
    case 9:
      message_len = *((uint16_t*)(buf+8));
      break;
    default:
      break;
    }
    if (byte_index >= header_len + message_len) {
      state = Idle;
      return true;
    }
    byte_index++;
    break;
  default:
    break;
  }
  return false;
}

Header GPS::parse_header() {
  Header msg = *((Header*)(buf));
  return msg;
}

BestPos GPS::parse_bestpos() {
  BestPos msg = *((BestPos*)(buf+header_len));
  return msg;
}

BestVel GPS::parse_bestvel() {
  BestVel msg = *((BestVel*)(buf+header_len));
  return msg;
}

BestTime GPS::parse_time() {
  BestTime msg = *((BestTime*)(buf+header_len));
  return msg;
}

Heading GPS::parse_heading() {
  Heading msg = *((Heading*)(buf+header_len));
  return msg;
}

InsAtt GPS::parse_insatt() {
  InsAtt msg = *((InsAtt*)(buf+header_len));
  return msg;
}
