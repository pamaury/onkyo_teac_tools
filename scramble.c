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
#include <assert.h>
#include <arpa/inet.h>

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

int main(int argc, char **argv)
{
     if(argc <= 1) {
	  fprintf(stderr, "usage: %s <file1> <file2> ... > <output file>\n", argv[0]);
	  exit(1);
     }

    size_t allocated = sizeof(struct teac_header) + 4 /* trailing checksum */;
    unsigned char *buf = malloc(allocated);
    if (!buf) {
	 perror("malloc failed");
	 exit(1);
    }

    /* Read input file(s). */
    for (int i=1; i<argc; i++) {
	 FILE *f = fopen(argv[i], "rb");
	 if(f == NULL) {
	      perror("failed to open input file");
	      exit(1);
	 }
	 fseek(f, 0, SEEK_END);
	 long size = ftell(f);
	 fseek(f, 0, SEEK_SET);

	 buf = realloc(buf, allocated + size);
	 if (!buf) {
	      perror("realloc failed");
	      exit(1);
	 }
	 allocated += size;
	 size_t bytes_read = fread(buf + allocated - 4 - size, 1, size, f);
	 if (bytes_read != size) {
	      perror("short read");
	      exit(1);
	 }
	 if ( 0xad != buf[allocated - 4 - size + 3]) {
	      fprintf(stderr, "Input file '%s' does not look like a bfin boot stream.\n", argv[i]);
	      fprintf(stderr, "Giving up as the image would probably brick the unit.\n");
	      exit(1);
	 }
	 fclose(f);
    }

    /* Invert descrambling table */
    unsigned char scramble_inverse[256];
    for (int i=0; i<256; i++)
	 scramble_inverse[scramble[i]] = i;

    /* Compute checksum and scramble */
    uint32_t plain_sum = 0;
    for(unsigned char *payload = buf + sizeof(struct teac_header);
	payload < (buf + allocated - 4);
	payload++)
    {
	 plain_sum += *payload;
	*payload = scramble_inverse[*payload];
    }
    uint32_t be_sum = htonl(plain_sum);
    memcpy(buf + allocated - 4, &be_sum, 4);

    /* Scramble trailing checksum */
    for(unsigned char *checksum = buf + allocated - 4;
	checksum < buf + allocated;
	checksum++)
    {
	*checksum = scramble_inverse[*checksum];
    }

    /* Generate header */
    {
	 struct teac_header *head = (void *)buf;
	 memcpy(head, "HA-P90\000\000", 8);
	 head->version = htons(130);
	 head->build = htons(108);
	 head->year = htons(2016);
	 head->month = 7;
	 head->day = 12;
	 head->checksum = be_sum;
	 head->file_size = htonl(allocated);
	 head->zero0 = 0;
	 head->zero1 = 0;
    }

    size_t bytes_written = fwrite(buf, 1, allocated, stdout);
    if (bytes_written != allocated) {
	 perror("short write");
	 exit(1);
    }

    return 0;
}
