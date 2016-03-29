#pragma once

#include "mbed-drivers/mbed.h"

#define SOURCE_ADDRESS 0x80 // The initial GP2Y0E03 address (0x80 for writing, 0x81 for reading)
#define VPP_PIN D7

void e_fuse_run(Serial* pc, uint8_t new_address);

uint16_t gp2y0e03_read_distance(uint8_t address);
