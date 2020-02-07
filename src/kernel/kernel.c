#include "../driver/port.h"

extern unsigned char port_byte_in (unsigned short port);
extern void port_byte_out (unsigned short port, unsigned char data);

void main() {
    port_byte_out(0x3d4, 14);
    int pos = port_byte_in(0x3d5);
    pos <<= 8;
    port_byte_out(0x3d4, 15);
    pos += port_byte_in(0x3d5);

    int offset = pos * 2;
    char *vga = (char *)0xb8000;
    vga[offset] = 'X';
    // color
    vga[offset + 1] = 0x0c;

}
