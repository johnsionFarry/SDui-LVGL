/**
 * @file lv_100ask_sketchpad.h
 *
 */

#ifndef LV_100ASK_SKETCHPAD_H
#define LV_100ASK_SKETCHPAD_H

// 定义枚举类型lv_100ask_sketchpad_toolbar_t，用于表示工具栏选项
typedef enum {
    // 全部选项
    LV_100ASK_SKETCHPAD_TOOLBAR_OPT_ALL = 0,
    // 画布
    LV_100ASK_SKETCHPAD_TOOLBAR_OPT_CW,
    // 宽度
    LV_100ASK_SKETCHPAD_TOOLBAR_OPT_WIDTH,
    // 最后一个选项
    LV_100ASK_SKETCHPAD_TOOLBAR_OPT_LAST
} lv_100ask_sketchpad_toolbar_t;

/*Data of canvas*/
// 定义一个lv_100ask_sketchpad_t类型的结构体
typedef struct
{
    // 定义一个lv_img_t类型的变量img
    lv_img_t img;
    // 定义一个lv_img_dsc_t类型的变量dsc
    lv_img_dsc_t dsc;
    // 定义一个lv_draw_line_dsc_t类型的变量line_rect_dsc
    lv_draw_line_dsc_t line_rect_dsc;
} lv_100ask_sketchpad_t;

lv_obj_t * lv_100ask_sketchpad_create(lv_obj_t * parent);

#endif /*LV_SKETCHPAD_H*/
