/* Copyright © 2023 Georgy E. All rights reserved. */

#include "modbus_rtu_base.h"

#include <stdint.h>


uint16_t modbus_crc16(const uint8_t* data, uint16_t len)
{
  uint16_t crc = 0xFFFF;

  for (unsigned i = 0; i < len; i++) {
    crc ^= (uint16_t)data[i];

    for (int i = 8; i != 0; i--) {
      if ((crc & 0x0001) != 0) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }

  return crc;
}
