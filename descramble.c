/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2011 Amaury Pouly
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

const unsigned char scramble[256] =
{
    0x00, 0xcc, 0xc1, 0xc2, 0xc3, 0xcd, 0xce, 0xcf, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb,
    0x20, 0x2a, 0x2b, 0x2c, 0x2d, 0x24, 0x2e, 0x2f, 0x21, 0x22, 0x23, 0x25, 0x26, 0x27, 0x28, 0x29,
    0xd0, 0xd8, 0xd9, 0xda, 0xdb, 0xdd, 0xde, 0xdf, 0xd1, 0xdc, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
    0x40, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x41, 0x42, 0x43, 0x44,
    0xf0, 0xfb, 0xfc, 0xfd, 0xf2, 0xf3, 0xfe, 0x14, 0xf1, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa,
    0x70, 0x7b, 0x7c, 0x7d, 0x7e, 0x71, 0x72, 0x7f, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,
    0xe0, 0xeb, 0xec, 0xed, 0xee, 0xe2, 0xe3, 0xef, 0xe1, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
    0xc0, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x60, 0x66, 0x67, 0x68, 0x69, 0x6d, 0x6e, 0x6f, 0x6a, 0x6b, 0x6c, 0x61, 0x62, 0x63, 0x64, 0x65,
    0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x81, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x30, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3e, 0x3f, 0x31, 0x32, 0x33, 0x3b, 0x3c, 0x3d, 0x34, 0x35,
    0x50, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5f, 0x51, 0x5c, 0x5d, 0x5e, 0x52, 0x53, 0x54, 0x55,
    0xa0, 0xac, 0xad, 0xae, 0xa2, 0xa3, 0xa4, 0xaf, 0xa1, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab,
    0x90, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9f, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x91, 0x92, 0x93,
    0xb0, 0xbc, 0xbd, 0xbe, 0xb1, 0xb2, 0xb3, 0xbf, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb,
    0x10, 0x15, 0x16, 0x17, 0x1c, 0x1d, 0x1e, 0x1f, 0x11, 0x12, 0x13, 0x18, 0x19, 0x1a, 0x1b, 0xff
};

struct teac_header {
   char product[8];
   uint16_t version;
   uint16_t build;
   uint16_t year;
   uint8_t month;
   uint8_t day;
   uint32_t checksum;
   uint32_t file_size;
   uint32_t zero0;
   uint32_t zero1;
} __attribute__((packed));

uint32_t read32le(void *buf)
{
    uint8_t *p = buf;
    return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
}

uint32_t read32be(void *buf)
{
    uint8_t *p = buf;
    return p[3] | p[2] << 8 | p[1] << 16 | p[0] << 24;
}

uint32_t read16be(void *buf)
{
    uint8_t *p = buf;
    return p[1] | p[0] << 8;
}

void teac_header_print(struct teac_header *head)
{
     char buf[9];
     strncpy(buf, head->product, 8);
     buf[8] = '\0';
     printf("header info: prod=%s, ", buf);
     printf("ver=%d, build=%d, ", read16be(&head->version),
	    read16be(&head->build));
     printf("stamp=%04d/%02d/%02d, ", read16be(&head->year),
	    head->month, head->day);
     printf("size=%d\n", read32be(&head->file_size));
     printf("\t checksum=0x%x zero0=0x%x zero1=0x%x\n", read32be(&head->checksum),
	    read32be(&head->zero0), read32be(&head->zero1));
     return;
}

int main(int argc, char **argv)
{
    if(argc <= 2)
        return printf("usage: %s <file> <output>\n", argv[0]);
    FILE *f = fopen(argv[1], "rb");
    if(f == NULL)
        return printf("cannot open %s: %m\n", argv[1]);
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);

    /* Read header */
    struct teac_header head;
    assert(sizeof(head) == 32);
    int bytes_read = fread(&head, 1, sizeof(head), f);
    if (bytes_read != sizeof(head))
	 perror("short read");
    teac_header_print(&head);
    if (read32be(&head.file_size) != size)
	 fprintf(stderr, "WARNING: unexpected file size, continuing anyway.\n");

    /* Read scrambled rest */
    unsigned char *buf = malloc(size);
    fread(buf, 1, size - sizeof(head), f);
    fclose(f);

    int plain_sum = 0;

    for(int i = 0; i < size - sizeof(head); i++) {
	 buf[i] = scramble[buf[i]];
	 if (i < size - sizeof(head) - 4)
	      plain_sum += buf[i];
    }

    if (plain_sum != read32be(&head.checksum)) {
	 fprintf(stderr, "WARNING: Header checksum mismatch: 0x%08x != 0x%08x (computed)\n",
		read32be(&head.checksum),
		plain_sum);
    }

    int checksum_at_eof = read32be(buf + size - sizeof(head) - 4);

    if (plain_sum != checksum_at_eof) {
	 fprintf(stderr, "WARNING: EOF checksum mismatch: 0x%08x != 0x%08x (computed)\n",
		checksum_at_eof,
		plain_sum);
    }

    f = fopen(argv[2], "wb");
    if(f == NULL)
        return printf("cannot open %s: %m\n", argv[2]);
    fwrite(buf, 1, size - sizeof(head) - 4, f);
    fclose(f);

    return 0;
}
