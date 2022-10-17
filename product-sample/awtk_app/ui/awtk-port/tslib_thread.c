/**
 * File:   tslib_thread.c
 * Author: AWTK Develop Team
 * Brief:  thread to handle touch screen events
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
 * 2018-09-07 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "tkc/mem.h"
#include "base/keys.h"
#include "tkc/thread.h"
#include "tslib_thread.h"
#include "tkc/utils.h"
#include "cvi_hal_touchpad.h"

typedef struct _run_info_t {
  int32_t max_x;
  int32_t max_y;
  int32_t fd;
  void* dispatch_ctx;
  char* filename;
  input_dispatch_t dispatch;

  event_queue_req_t req;
} run_info_t;

static ret_t tslib_dispatch(run_info_t* info) {
  ret_t ret = info->dispatch(info->dispatch_ctx, &(info->req), "tslib");
  info->req.event.type = EVT_NONE;

  return ret;
}

static ret_t tslib_dispatch_one_event(run_info_t* info) {

  CVI_HAL_TOUCHPAD_INPUTINFO_S e = {0};
  int ret = -1;
  static int x = 0;
  static int y = 0;
  if (info->fd != -1) {
    CVI_HAL_TOUCHPAD_ReadInputEvent(&e);
  }

  event_queue_req_t* req = &(info->req);

  req->event.type = EVT_NONE;
  req->pointer_event.x = e.s32X;
  req->pointer_event.y = e.s32Y;

  log_debug("%s%d: e.pru32Pressuressure=%d x=%d y=%d ret=%d\n", __func__, __LINE__, e.u32Pressure, e.s32X, e.s32Y,
            ret);

  if (e.u32Pressure > 0) {
    if (req->pointer_event.pressed) {
      if(req->pointer_event.x != x || req->pointer_event.y != y) {
        req->event.type = EVT_POINTER_MOVE;
      } 
      x = req->pointer_event.x;
      y = req->pointer_event.y;    
    } else {
      req->event.type = EVT_POINTER_DOWN;
      req->pointer_event.pressed = TRUE;
      x = req->pointer_event.x;
      y = req->pointer_event.y;
    }
  } else {
    if (req->pointer_event.pressed) {
      req->event.type = EVT_POINTER_UP;
    }
    req->pointer_event.pressed = FALSE;
  }

  return tslib_dispatch(info);
}

void* tslib_run(void* ctx) {
  run_info_t info = *(run_info_t*)ctx;
  if (info.fd == -1) {
    log_debug("%s:%d: open tslib failed, filename=%s\n", __func__, __LINE__, info.filename);
  } else {
    log_debug("%s:%d: open tslib successful\n", __func__, __LINE__);
  }

  TKMEM_FREE(ctx);
  while (tslib_dispatch_one_event(&info) == RET_OK)
    ;
  CVI_HAL_TOUCHPAD_Stop();
  TKMEM_FREE(info.filename);

  return NULL;
}

static run_info_t* info_dup(run_info_t* info) {
  run_info_t* new_info = TKMEM_ZALLOC(run_info_t);

  *new_info = *info;

  return new_info;
}

tk_thread_t* tslib_thread_run(const char* filename, input_dispatch_t dispatch, void* ctx,
                              int32_t max_x, int32_t max_y) {
  run_info_t info;
  tk_thread_t* thread = NULL;
  return_value_if_fail(filename != NULL && dispatch != NULL, NULL);

  memset(&info, 0x00, sizeof(info));
  int fd;
  CVI_HAL_TOUCHPAD_Init();
  CVI_HAL_TOUCHPAD_Start(&fd);

  info.max_x = max_x;
  info.max_y = max_y;
  info.dispatch_ctx = ctx;
  info.dispatch = dispatch;
  info.fd = fd;
  //info.filename = tk_strdup(filename);

  if(fd != -1) {
    thread = tk_thread_create(tslib_run, info_dup(&info));
    if (thread != NULL) {
      tk_thread_start(thread);
    } else {
      CVI_HAL_TOUCHPAD_Stop();
      TKMEM_FREE(info.filename);
    }
  } else {
      CVI_HAL_TOUCHPAD_Stop();
      TKMEM_FREE(info.filename);
  }
  return thread;
}
