#include <iostream>
#include <stdio.h>
#include <stdint.h>

typedef struct {
  uint32_t len: 24; // max: 16777215
  uint32_t seq: 8;  // max: 255
  uint32_t comp_len: 24;
  uint32_t command: 8;
} packet_header_t;

int main(void)
{
  packet_header_t packet = {16777215, 1, 5000, 128};
  printf("packet_header_t size: %d\n", sizeof(packet_header_t));

  printf("len = %d\n", packet.len);
  printf("seq = %d\n", packet.seq);
  printf("comp_len = %d\n", packet.comp_len);
  printf("command = %d\n", packet.command);
}

