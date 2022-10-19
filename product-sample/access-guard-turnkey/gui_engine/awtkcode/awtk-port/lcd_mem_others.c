/**
 * File:   lcd_mem_others.c
 * Author: AWTK Develop Team
 * Brief:  support other special format linux framebuffers
 *
 * Copyright (c) 2018 - 2020  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2019-06-17 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "lcd_mem_others.h"
#include "base/system_info.h"
#include "base/pixel.h"
#include "blend/image_g2d.h"
#include "fb_info.h"
#define pixel_dst_t pixel_bgr565_t
#define pixel_t pixel_bgr565_t

static ret_t lcd_bgra5551_flush(lcd_t* lcd) {
  xy_t dx = 0;
  xy_t dy = 0;
  uint32_t i = 0;
  uint32_t k = 0;
  uint32_t w = 0;
  uint32_t h = 0;
  uint32_t bpp = bitmap_get_bpp_of_format(BITMAP_FMT_BGR565);
  uint16_t* src_p = NULL;
  uint16_t* dst_p = NULL;
  

  rect_t* dr = &(lcd->dirty_rect);
  lcd_mem_special_t* special = (lcd_mem_special_t*)lcd;
  fb_info_t* info = (fb_info_t*)(special->ctx);

  uint32_t dst_line_length = (info->fix.line_length)/bpp;
  uint32_t src_line_length = special->lcd_mem->line_length/bpp;
  lcd_orientation_t o = system_info()->lcd_orientation;
  if (o == LCD_ORIENTATION_0) {
    if (dr->w > 0 && dr->h > 0) {
      int32_t x = 0;
      int32_t y = 0;
      int src_line_length = lcd->w;
      int dst_line_length = info->fix.line_length / 2;
      uint16_t* dst = (uint16_t*)(info->fbmem0);
      uint16_t* src = (uint16_t*)(special->lcd_mem->offline_fb);

      src += dr->y * src_line_length;
      dst += dr->y * dst_line_length;

      for (y = 0; y < dr->h; y++) {
        for (x = 0; x < dr->w; x++) {
          uint16_t s = src[x];
          uint8_t b = 0x1f & s;
          uint8_t g = (0x3e0 & s) >> 5;
          uint8_t r = (0xf800 & s) >> 11;
          uint8_t a = (0x8000 & s) >> 15;
          dst[x] = (a << 15) | (r << 10) | (g << 5) | b;
        }

        src += src_line_length;
        dst += dst_line_length;
      }
    }
    return RET_OK;
  }
  switch (o) {
    case LCD_ORIENTATION_90: {
      dx = dr->y;
      dy = lcd->w - dr->x - dr->w;
      break;
    }
    case LCD_ORIENTATION_180: {
      dy = lcd->h - dr->y - 1;
      dx = lcd->w - dr->x - 1;
      break;
    }
    case LCD_ORIENTATION_270: {
      dx = lcd->h - dr->y - 1;
      dy = dr->x;
      break;
    }
    default:
      break;
  }
  uint16_t* dst = (uint16_t*)(info->fbmem0);
  uint16_t* src = (uint16_t*)(special->lcd_mem->offline_fb);
  dst_p = (uint16_t*)(dst + dy * dst_line_length + dx);
  src_p = (uint16_t*)(src + dr->y * src_line_length + dr->x);

  w = dr->w;
  h = dr->h;

  switch (o) {
    case LCD_ORIENTATION_90: {
      for (i = 0; i < h; i++) {
        uint16_t* s = src_p + w - 1;
        uint16_t* d = dst_p;

        for (k = 0; k < w; k++) {
          uint16_t t = s[0];
          uint8_t b = 0x1f & t;
          uint8_t g = (0x3e0 & t) >> 5;
          uint8_t r = (0xf800 & t) >> 11;
          uint8_t a = (0x8000 & t) >> 15;
          d[0] = (a << 15) | (r << 10) | (g << 5) | b;
          d = (uint16_t*)(d + dst_line_length);
          s--;
        }

        dst_p++;
        src_p = (uint16_t*)(src_p + src_line_length);
      }
      break;
    }
    case LCD_ORIENTATION_180: {
      for (i = 0; i < h; i++) {
        uint16_t* s = src_p;
        uint16_t* d = dst_p;

        for (k = 0; k < w; k++) {
          uint16_t t = s[k];
          uint8_t b = 0x1f & t;
          uint8_t g = (0x3e0 & t) >> 5;
          uint8_t r = (0xf800 & t) >> 11;
          uint8_t a = (0x8000 & t) >> 15;
          d[0] = (a << 15) | (r << 10) | (g << 5) | b;
          d--;
        }
        dst_p = (uint16_t*)(dst_p - dst_line_length);
        src_p = (uint16_t*)(src_p + src_line_length);
      }
      break;
    }
    case LCD_ORIENTATION_270: {
      for (i = 0; i < h; i++) {
        uint16_t* s = src_p;
        uint16_t* d = dst_p;

        for (k = 0; k < w; k++) {
          uint16_t t = s[k];
          uint8_t b = 0x1f & t;
          uint8_t g = (0x3e0 & t) >> 5;
          uint8_t r = (0xf800 & t) >> 11;
          uint8_t a = (0x8000 & t) >> 15;
          d[0] = (a << 15) | (r << 10) | (g << 5) | b;
          d = (uint16_t*)(d + dst_line_length);
        }
        dst_p--;
        src_p = (uint16_t*)(src_p + src_line_length);
      }
      break;
    }
    default:
      break;
  }

  return RET_OK;
}

lcd_t* lcd_mem_bgra5551_create(fb_info_t* info) {
  wh_t w = fb_width(info);
  wh_t h = fb_height(info);

  return lcd_mem_special_create(w, h, BITMAP_FMT_BGR565, lcd_bgra5551_flush, NULL, NULL, info);
}
