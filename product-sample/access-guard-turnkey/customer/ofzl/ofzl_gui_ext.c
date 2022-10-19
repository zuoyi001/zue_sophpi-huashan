#include "gui_ext.h"
#include "cvi_hal_face_api.h"
#include "draw_helper.h"

#define min(x,y) (((x)<=(y))?(x):(y))
#define max(x,y) (((x)>=(y))?(x):(y))

#define DEFAULT_RECT_COLOR_R          (53. / 255.)
#define DEFAULT_RECT_COLOR_G          (208. / 255.)
#define DEFAULT_RECT_COLOR_B          (217. / 255.)
#define DEFAULT_LIVENESS_THRESHOLD    0.6
#define DEFAULT_RECT_THINKNESS        4
#define DEFAULT_ALPHABET_IMAGES_PATH  "./labels"


typedef struct {
    float r;
    float g;
    float b;
} color_rgb;

void clear_face_rect(void)
{
    #ifdef SUPPORT_LVGL
    gui_clear_face_rect();
    #endif
}

void draw_face_meta(VIDEO_FRAME_INFO_S *draw_frame, hal_face_t face_meta, int rect_color)
{
    draw_frame->stVFrame.u32Width = 1080;
    draw_frame->stVFrame.u32Height = 1920;
    clear_face_rect();
    int face_num = HAL_FACE_GetNum(face_meta);
    if (0 == face_num) {
        // printf("No face detected.\n");
        return;
    }

    //color_rgb rgb_color;
    //rgb_color.r = DEFAULT_RECT_COLOR_R;
    //rgb_color.g = DEFAULT_RECT_COLOR_G;
    //rgb_color.b = DEFAULT_RECT_COLOR_B;

  for (int i = 0; i < face_num; ++i) {
    hal_face_bbox_t bbox;
    HAL_FACE_RectRescale(draw_frame->stVFrame.u32Width, draw_frame->stVFrame.u32Height, face_meta, i, true, &bbox);
    // int draw_x1 = bbox.x1;
    // int draw_x2 = bbox.x2;
    // int draw_y1 = bbox.y1;
    // int draw_y2 = bbox.y2;

    int draw_x1 = (bbox.y1 / 4);
    int draw_x2 = (bbox.y2 / 4);
    int draw_y1 = ((draw_frame->stVFrame.u32Width - bbox.x2) / 4);
    int draw_y2 = ((draw_frame->stVFrame.u32Width - bbox.x1) / 4);

    // printf("support SUPPORT_LVGL");
    gui_draw_face_rect(draw_x1,draw_y1,(draw_x2-draw_x1),(draw_y2-draw_y1),i*4,rect_color);

#if 0
    if (face_meta->info[i].name[0] != '\0') {
        //launcher_draw_name(draw_x1,draw_y1-30,i,face_meta->info[i].name);
    }
    #endif

  }
}

#if 0
void draw_obj_meta(VIDEO_FRAME_INFO_S *draw_frame, cvi_object_meta_t *meta)
{
    if (0 == meta->size) {
        #ifdef SUPPORT_LVGL
        //launcher_hide_obj_rect();
        //launcher_hide_name();
        #endif
        return;
    }
    float width = draw_frame->stVFrame.u32Width;
    float height = draw_frame->stVFrame.u32Height;
    float ratio_x,ratio_y,bbox_y_height,bbox_x_height,bbox_padding_top,bbox_padding_left;
    if(width >=height)
    {
        ratio_x = width / meta->width;
        bbox_y_height = meta->height * height / width;
        ratio_y = height / bbox_y_height;
        bbox_padding_top = (meta->height - bbox_y_height) / 2;
    }
    else{
        ratio_y = height / meta->height;
        bbox_x_height = meta->width * width / height;
        ratio_x = width / bbox_x_height;
        bbox_padding_left = (meta->width - bbox_x_height) / 2;
    }
    color_rgb rgb_color;
    rgb_color.r = DEFAULT_RECT_COLOR_R;
    rgb_color.g = DEFAULT_RECT_COLOR_G;
    rgb_color.b = DEFAULT_RECT_COLOR_B;

  for (int i = 0; i < meta->size; ++i) {
    cvai_bbox_t bbox = meta->objects[i].bbox;
    float x1,x2,y1,y2;
    if(width >=height)
    {
        x1 = bbox.x1 * ratio_x;
        x2 = bbox.x2 * ratio_x;
        y1 = (bbox.y1 - bbox_padding_top) * ratio_y;
        y2 = (bbox.y2 - bbox_padding_top) * ratio_y;
    }
    else{
        x1 = (bbox.x1 - bbox_padding_left) * ratio_x;
        x2 = (bbox.x2 - bbox_padding_left) * ratio_x;
        y1 = bbox.y1 * ratio_y;
        y2 = bbox.y2 * ratio_y;
    }

    //draw_rect(draw_frame, x1, x2, y1, y2, rgb_color, DEFAULT_RECT_THINKNESS);
    int draw_x1 = x1;
    int draw_x2 = x2;
    int draw_y1 = y1;
    int draw_y2 = y2;
    #ifdef SUPPORT_LVGL
    launcher_draw_obj_rect(draw_x1,draw_y1,(draw_x2-draw_x1),(draw_y2-draw_y1),i);

    if (meta->objects[i].name[0] != '\0') {
        launcher_draw_name(draw_x1,draw_y1-30,i,meta->objects[i].name);
    }
    #endif
    
  }
}
#endif
