/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2017 Amaury Pouly
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
#include <stdint.h>
#include <stddef.h>
#include <getopt.h>
#include "elf.h"
#include "bfin_boot.h"

#define cprintf(col, ...) \
    do { color(col); printf(__VA_ARGS__); }while(0)

const char *g_out_prefix;
bool g_elf_simplify = true;

static void *ptr_add(void *p, ptrdiff_t x)
{
    return (uint8_t *)p + x;
}

uint32_t read32le(void *buf)
{
    uint8_t *p = buf;
    return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
}

void fix_header(struct bfin_boot_header_t *hdr)
{
    hdr->code = read32le(&hdr->code);
    hdr->target = read32le(&hdr->target);
    hdr->count = read32le(&hdr->count);
    hdr->arg = read32le(&hdr->arg);
}

uint8_t hdr_checksum(void *p, size_t sz)
{
    uint8_t *_p = p;
    uint8_t c = 0;
    while(sz-- > 0)
        c ^= *_p++;
    return c;
}

void write_elf(struct elf_params_t *elf, int *counter)
{
    if(!g_out_prefix)
        return;
    if(elf_is_empty(elf))
        return;
    char *filename = malloc(strlen(g_out_prefix) + 32);
    sprintf(filename, "%s%d.elf", g_out_prefix, (*counter)++);
    FILE *fd = fopen(filename, "wb");
    if(fd == NULL)
    {
        cprintf(GREY, "Cannot open '%s' for writing\n", filename);
        free(filename);
        return;
    }
    if(g_debug)
        cprintf(GREY, "Write '%s'\n", filename);
    free(filename);
    if(g_elf_simplify)
        elf_simplify(elf);
    elf_write_file(elf, elf_std_write, generic_std_printf, fd);
    fclose(fd);
    elf_init(elf);
}

void dump_code(void *buf, unsigned long size, unsigned long offset)
{
    struct elf_params_t elf;
    elf_init(&elf);
    int elf_counter = 0;
    unsigned long next_dxe = 0; /* 0 means none */
    uint32_t dxe_start_addr = 0;

    while(offset < size)
    {
        if(offset % 4)
        {
            cprintf(GREY, "Stopping at offset %#lx: not a multiple of 4\n", offset);
            break;
        }
        if(offset + sizeof(struct bfin_boot_header_t) > size)
        {
            cprintf(GREY, "Stopping at offset %#lx: not enough bytes left for a header\n", offset);
            break;
        }
        struct bfin_boot_header_t *hdr = ptr_add(buf, offset);
        fix_header(hdr);
        if(HDRSIGN(hdr->code) != HDRSIGN_BF609)
        {
            cprintf(GREY, "Invalid header sign at offset %#lx: %02x\n", offset,
                HDRSIGN(hdr->code));
            break;
        }
        uint8_t checksum = hdr_checksum(hdr, sizeof(struct bfin_boot_header_t));
        if(checksum != 0)
        {
            cprintf(GREY, "Invalid header checksum at offset %#lx: %02x\n", offset,
                checksum);
            break;
        }
        /* print */
        if(g_debug)
        {
            cprintf(BLUE, "[");
            cprintf(YELLOW, "%#10lx", (unsigned long)offset);
            cprintf(BLUE, "]");
            cprintf(BLUE,  " Mode=");
            cprintf(YELLOW, "%x", BCODE(hdr->code));
            cprintf(BLUE,  " Flags=");
            cprintf(YELLOW, "%#x", BFLAG(hdr->code));
            if(hdr->code & BFLAG_SAVE) cprintf(RED, " SAVE");
            if(hdr->code & BFLAG_AUX) cprintf(RED, " AUX");
            if(hdr->code & BFLAG_FORWARD) cprintf(RED, " FORWARD");
            if(hdr->code & BFLAG_FILL) cprintf(RED, " FILL");
            if(hdr->code & BFLAG_QUICKBOOT) cprintf(RED, " QUICKBOOT");
            if(hdr->code & BFLAG_CALLBACK) cprintf(RED, " CALLBACK");
            if(hdr->code & BFLAG_INIT) cprintf(RED, " INIT");
            if(hdr->code & BFLAG_IGNORE) cprintf(RED, " IGNORE");
            if(hdr->code & BFLAG_INDIRECT) cprintf(RED, " INDIRECT");
            if(hdr->code & BFLAG_FIRST) cprintf(RED, " FIRST");
            if(hdr->code & BFLAG_FINAL) cprintf(RED, " FINAL");
            cprintf(BLUE, " Count=");
            cprintf(YELLOW, "%#lx", (unsigned long)hdr->count);
            cprintf(BLUE, " Target=");
            cprintf(YELLOW, "%#lx", (unsigned long)hdr->target);
            cprintf(BLUE, " Argument=");
            cprintf(YELLOW, "%#lx", (unsigned long)hdr->arg);
            cprintf(OFF, "\n");
        }
        /* special processing: the order is important! */
        if(hdr->code & BFLAG_FIRST)
        {
            /* finish previous elf if any */
            write_elf(&elf, &elf_counter);
            /* info */
            cprintf(RED, "New DXE at ");
            cprintf(YELLOW, "%#lx\n", offset);
            /* record address of next DXE */
            next_dxe = offset + sizeof(*hdr) + hdr->arg;
            dxe_start_addr = hdr->target;
        }
        if(hdr->code & BFLAG_SAVE)
            cprintf(GREY, "Warning: SAVE blocks are not supported\n");
        if(hdr->code & BFLAG_FORWARD)
            cprintf(GREY, "Warning: FORWARD blocks are not supported\n");
        if(hdr->code & BFLAG_AUX)
            cprintf(GREY, "Warning: AUX blocks are not supported\n");
        /* NOTE: INDIRECT and QUICKBOOT flags can be safely ignored, they only
         * matter on the actual chip */
        /* Load/fill data */
        if(hdr->code & BFLAG_FILL)
            elf_add_fill_section(&elf, hdr->target, hdr->count, hdr->arg, ".bss");
        else if(hdr->count > 0)
            elf_add_load_section(&elf, hdr->target, hdr->count, (void *)(hdr + 1), ".text");
        /* Callback are not supported */
        if(hdr->code & BFLAG_CALLBACK)
            cprintf(GREY, "Warning: Ignore CALLBACK on block, data may require further processing\n");
        /* Init */
        if(hdr->code & BFLAG_INIT)
        {
            if(hdr->arg != 0)
            cprintf(GREY, "Warning: INIT block with nonzero argument, cannot represent that in ELF\n");
            /* We assume, as per described in blackfin manual, that init routines
             * load a bit of code and run it immediately, so we stop the current
             * elf here */
            elf_set_start_addr(&elf, hdr->target);
            write_elf(&elf, &elf_counter);
        }
        /* Final */
        if(hdr->code & BFLAG_FINAL)
        {
            elf_set_start_addr(&elf, dxe_start_addr);
            write_elf(&elf, &elf_counter);
        }

        /* On a FINAL block, the boot stream stop, but there migth be another DXE
         * indicated before with a FIRST block */
        if(hdr->code & BFLAG_FINAL)
        {
            if(next_dxe == 0)
            {
                cprintf(GREY, "Warning: FINAL block without FIRST block\n");
                break;
            }
            offset = next_dxe;
            next_dxe = 0;
        }
        else
        {
            /* go to next block */
            offset += sizeof(*hdr);
            /* FILL blocks don't take any space */
            if(!(hdr->code & BFLAG_FILL))
                offset += hdr->count;
        }
    }
    /* finish elf if any */
    write_elf(&elf, &elf_counter);
    elf_release(&elf);
}

static void usage(void)
{
    printf("Usage: bfin_boot [options] file\n");
    printf("Options:\n");
    printf("  -h/--help             Display this message\n");
    printf("  -o <prefix>           Enable output and set prefix\n");
    printf("  -d/--debug            Enable debug output*\n");
    printf("  -n/--no-color         Disable output colors\n");
    printf("  -s/--seek <offset>    Start at a specific offset\n");
    exit(1);
}

int main(int argc, char **argv)
{
    unsigned long offset = 0;
    while(1)
    {
        static struct option long_options[] =
        {
            {"help", no_argument, 0, 'h'},
            {"debug", no_argument, 0, 'd'},
            {"no-color", no_argument, 0, 'n'},
            {"seek", required_argument, 0, 's'},
            {0, 0, 0, 0}
        };

        int c = getopt_long(argc, argv, "hdno:s:", long_options, NULL);
        if(c == -1)
            break;
        switch(c)
        {
            case -1:
                break;
            case 'n':
                enable_color(false);
                break;
            case 'd':
                g_debug = true;
                break;
            case 'h':
                usage();
                break;
            case 'o':
                g_out_prefix = optarg;
                break;
            case 's':
            {
                char *end;
                offset = strtoul(optarg, &end, 0);
                if(*end)
                {
                    printf("Invalid offset '%s'\n", optarg);
                    return 1;
                }
                break;
            }
            default:
                printf("Internal error: unknown option '%c'\n", c);
                abort();
                break;
        }
    }

    if(argc - optind != 1)
    {
        usage();
        return 1;
    }

    const char *filename = argv[optind];
    FILE *f = fopen(filename, "rb");
    if(f == NULL)
        return printf("cannot open %s: %m\n", filename);
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char *buf = malloc(size);
    fread(buf, 1, size, f);
    fclose(f);

    if(offset >= size)
    {
        free(buf);
        printf("Offset 0x%lx is past the end of the file\n", offset);
        return 1;
    }

    dump_code(buf, size, offset);
    free(buf);

    return 0;
}
