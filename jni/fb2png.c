/**
 * fb2png  Save screenshot into .png.
 *
 * Copyright (C) 2012  Kyan <kyan.ql.he@gmail.com>
 * Copyright (C) 2014, philz-cwm6 <phytowardt@gmail.com>
 * Copyright (C) 2014, Mikael Berthe "McKael" <mikael@lilotux.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <errno.h>

#include "log.h"
#include "fb2png.h"
#include "fb.h"

// multi buffering support
// -1: will be auto detect (default)
// 0 for single buffering, 1 for double, 2 for triple, 3 for 4x buffering
int user_set_buffers_num = -1;

/**
 * Get the {@code struct fb} from device's framebuffer.
 * Return
 *      0 for success.
 */
int get_device_fb(const char* path, struct fb *fb)
{
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    unsigned char *raw;
    unsigned int bytespp;
    unsigned int raw_size;
    unsigned int raw_line_length;
    ssize_t read_size;
    int fd;

    fd = open(path, O_RDONLY);
    if (fd < 0) {
    	E("open path failed, %d", fd);
    	return -1;
    }

    if(ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        E("ioctl failed, %s", strerror(errno));
        close(fd);
        return -1;
    }

    if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo) < 0) {
        E("ioctl fixed failed, %s", strerror(errno));
        close(fd);
        return -1;
    }

    bytespp = vinfo.bits_per_pixel / 8;
    raw_line_length = finfo.line_length; // (xres + padding_offset) * bytespp
    raw_size = vinfo.yres * raw_line_length;

    // output data handler struct
    fb->bpp = vinfo.bits_per_pixel;
    fb->size = vinfo.xres * vinfo.yres * bytespp;
    fb->width = vinfo.xres;
    fb->height = vinfo.yres;
    fb->red_offset = vinfo.red.offset;
    fb->red_length = vinfo.red.length;
    fb->green_offset = vinfo.green.offset;
    fb->green_length = vinfo.green.length;
    fb->blue_offset = vinfo.blue.offset;
    fb->blue_length = vinfo.blue.length;
    fb->alpha_offset = vinfo.transp.offset;
    fb->alpha_length = vinfo.transp.length;

    // container for raw bits from the active frame buffer
    raw = malloc(raw_size);
    if (!raw) {
        E("raw: memory error");
        close(fd);
        return -1;
    }

    // capture active buffer: n is 0 for first buffer, 1 for second
    // graphics.c -> set_active_framebuffer() -> vi.yoffset = n * vi.yres;
    unsigned int active_buffer_offset = 0;
    int num_buffers = user_set_buffers_num;
    if (num_buffers < 0) {
        // default: auto detect
        num_buffers = (int)(vinfo.yoffset / vinfo.yres);
        if (num_buffers > MAX_ALLOWED_FB_BUFFERS)
            num_buffers = 0;
    }

    if (finfo.smem_len >= (raw_size * (num_buffers + 1))) {
        active_buffer_offset = raw_size * num_buffers;
    }

    // display debug fb info
    fb_dump(fb);
    I("%13s : %u", "bytespp", bytespp);
    I("%13s : %u", "raw size", raw_size);
    I("%13s : %u", "yoffset", vinfo.yoffset);
    I("%13s : %u", "pad offset", (raw_line_length / bytespp) - fb->width);
    I("%13s : %u", "buffer offset", active_buffer_offset);

    // copy the active frame buffer bits into the raw container
    lseek(fd, active_buffer_offset, SEEK_SET);
    read_size = read(fd, raw, raw_size);
    if (read_size < 0 || (unsigned)read_size != raw_size) {
        E("read buffer error");
        goto oops;
    }

	/*
		Image padding (needed on some RGBX_8888 formats, maybe others?)
		we have padding_offset in bytes and bytespp = bits_per_pixel / 8
		raw_line_length = (width + padding_offset) * bytespp

		This gives: padding_offset = (raw_line_length / bytespp) - width
	*/
    /*
     * 我是这么理解下面这段的
     * Framebuffer 里保存的每一行的数据并不一定刚好就是我们的屏幕分辨率大小，它的横向像素值有可能比屏幕的横向像素值多！
     * 因此在这个地方进行判断
     * bytespp是什么？就是位数除以8，比如我的手机位数是32，则除以8就是4，我理解是一个像素有四个字节组成，比如（ARGB）
     * 那因此，(raw_line_length / bytespp) - fb->width 正好就是Framebuffer中所存数据与实际数据之差
     * 若不等于零，则读数据时，必须要过滤掉
     */
    unsigned int padding_offset = (raw_line_length / bytespp) - fb->width;
    if (padding_offset) {
        unsigned char *data;
        unsigned char *pdata;
        unsigned char *praw;
        const unsigned char *data_buffer_end;
        const unsigned char *raw_buffer_end;

        // container for final aligned image data
        data = malloc(fb->size);
        if (!data) {
            E("data: memory error");
            goto oops;
        }

        pdata = data;
        praw = raw;
        data_buffer_end = data + fb->size;
        raw_buffer_end = raw + raw_size;

        // Add a margin to prevent buffer overflow during copy
        data_buffer_end -= bytespp * fb->width;
        raw_buffer_end -= raw_line_length;
        /*
         * 这个地方进行读取数据，xoffset一般是0，从最左边读取数据，读取的宽度就是xres，但实际宽度是raw_line_length
         */
        while (praw < raw_buffer_end && pdata < data_buffer_end) {
            memcpy(pdata, praw, bytespp * fb->width);
            pdata += bytespp * fb->width;
            praw += raw_line_length;
        }
        I("Padding done.");

        fb->data = data;
        free(raw);
    } else {
        fb->data = raw;
    }

    close(fd);
    return 0;

oops:
    free(raw);
    close(fd);
    return -1;
}

int fb2png(const char *path)
{
    struct fb fb;
    int ret;

    ret = get_device_fb("/dev/graphics/fb0", &fb);

    if (ret) {
        I("Failed to read framebuffer.");
        return -1;
    }

    return fb_save_png(&fb, path);
}
