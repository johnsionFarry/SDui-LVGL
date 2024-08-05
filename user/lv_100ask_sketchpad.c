/**
 * @file lv_100ask_sketchpad.c
 *
 */

/*********************
 *      INCLUDES 包含
 *********************/
#include "lvgl/lvgl.h"
#include "user/lv_100ask_sketchpad.h"
/*********************
 *      DEFINES 宏
 *********************/
#define MY_CLASS &lv_100ask_sketchpad_class

#define YOUR_WIDTH 25
#define YOUR_HEIGHT 25
/**********************
 *  STATIC PROTOTYPES 静态函数
 **********************/
// 静态函数，用于构造画板对象
static void lv_100ask_sketchpad_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
// 静态函数，用于销毁画板对象
static void lv_100ask_sketchpad_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
// 静态函数，用于构造工具栏对象
static void lv_100ask_sketchpad_toolbar_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
// 静态函数，用于销毁工具栏对象
static void lv_100ask_sketchpad_toolbar_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
// 静态函数，用于处理画板事件
static void lv_100ask_sketchpad_event(const lv_obj_class_t * class_p, lv_event_t * e);
// 静态函数，用于处理工具栏事件
static void lv_100ask_sketchpad_toolbar_event(const lv_obj_class_t * class_p, lv_event_t * e);
// 静态函数，用于处理工具栏事件
static void sketchpad_toolbar_event_cb(lv_event_t * e);
// 静态函数，用于处理工具栏设置事件
static void toolbar_set_event_cb(lv_event_t * e);

/**********************
 *  STATIC VARIABLES 静态变量
 **********************/
// 定义lv_100ask_sketchpad_class对象类，构造函数为lv_100ask_sketchpad_constructor，
// 销毁函数为lv_100ask_sketchpad_destructor，事件函数为lv_100ask_sketchpad_event，
// 实例大小为sizeof(lv_100ask_sketchpad_t)，基础类为&lv_canvas_class
const lv_obj_class_t lv_100ask_sketchpad_class = {.constructor_cb = lv_100ask_sketchpad_constructor,
                                                  .destructor_cb  = lv_100ask_sketchpad_destructor,
                                                  .event_cb       = lv_100ask_sketchpad_event,
                                                  .instance_size  = sizeof(lv_100ask_sketchpad_t),
                                                  .base_class     = &lv_canvas_class};

// 定义lv_100ask_sketchpad_toolbar_class对象类，构造函数为lv_100ask_sketchpad_toolbar_constructor，
// 销毁函数为lv_100ask_sketchpad_toolbar_destructor，事件函数为lv_100ask_sketchpad_toolbar_event，
// 宽度默认值为LV_SIZE_CONTENT，高度默认值为LV_SIZE_CONTENT，基础类为&lv_obj_class
const lv_obj_class_t lv_100ask_sketchpad_toolbar_class = {.constructor_cb = lv_100ask_sketchpad_toolbar_constructor,
                                                          .destructor_cb  = lv_100ask_sketchpad_toolbar_destructor,
                                                          .event_cb       = lv_100ask_sketchpad_toolbar_event,
                                                          .width_def      = LV_SIZE_CONTENT,
                                                          .height_def     = LV_SIZE_CONTENT,
                                                          .base_class     = &lv_obj_class};

/**********************
 *      MACROS 宏
 **********************/

/**********************
 *   GLOBAL FUNCTIONS 全局函数
 **********************/

lv_obj_t * lv_100ask_sketchpad_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

/*=====================
 * Other functions 其他函数
 *====================*/

/**********************
 *   STATIC FUNCTIONS 静态函数
 **********************/

static void lv_100ask_sketchpad_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    // 设置构造函数的参数为未使用（压制警告）
    LV_UNUSED(class_p);
    // 在跟踪日志中记录对象的创建
    LV_TRACE_OBJ_CREATE("begin");

    // 将对象转换为特定类型的指针
    lv_100ask_sketchpad_t * sketchpad = (lv_100ask_sketchpad_t *)obj;

    // 初始化图像描述符的头部属性
    sketchpad->dsc.header.always_zero = 0;
    sketchpad->dsc.header.cf          = LV_IMG_CF_TRUE_COLOR;
    sketchpad->dsc.header.h           = 0;
    sketchpad->dsc.header.w           = 0;
    sketchpad->dsc.data_size          = 0;
    sketchpad->dsc.data               = NULL;

    // 初始化线段矩形描述符
    lv_draw_line_dsc_init(&sketchpad->line_rect_dsc);
    sketchpad->line_rect_dsc.width       = 2;
    sketchpad->line_rect_dsc.round_start = true;
    sketchpad->line_rect_dsc.round_end   = true;
    sketchpad->line_rect_dsc.color       = lv_color_black();
    sketchpad->line_rect_dsc.opa         = LV_OPA_COVER;

    // 设置对象的图像源为图像描述符
    lv_img_set_src(obj, &sketchpad->dsc);

    // 将对象标记为可点击
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);

    /* 创建工具栏对象 */
    lv_obj_t * toolbar = lv_obj_class_create_obj(&lv_100ask_sketchpad_toolbar_class, obj);
    lv_obj_class_init_obj(toolbar);

    LV_TRACE_OBJ_CREATE("finished");
}

static void lv_100ask_sketchpad_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    // 设置析构函数的参数为未使用（压制警告）
    LV_UNUSED(class_p);
    // 在跟踪日志中记录对象的创建
    LV_TRACE_OBJ_CREATE("begin");

    // 将对象转换为特定类型的指针
    lv_canvas_t * canvas = (lv_canvas_t *)obj;
    // 使图像缓存的源无效
    lv_img_cache_invalidate_src(&canvas->dsc);
}

static void lv_100ask_sketchpad_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
    // 设置事件处理函数的参数为未使用（压制警告）
    LV_UNUSED(class_p);

    lv_res_t res;

    /* 调用祖先层级的事件处理程序 */
    res = lv_obj_event_base(MY_CLASS, e);
    if(res != LV_RES_OK) return;

    lv_event_code_t code              = lv_event_get_code(e);
    lv_obj_t * obj                    = lv_event_get_target(e);
    lv_100ask_sketchpad_t * sketchpad = (lv_100ask_sketchpad_t *)obj;

    // 定义静态变量来保存上一次的坐标
    static lv_coord_t last_x, last_y = -32768;

    if(code == LV_EVENT_PRESSING) {
        // 获取当前活动的输入设备
        lv_indev_t * indev = lv_indev_get_act();
        if(indev == NULL) return;

        lv_point_t point;
        // 获取输入设备的坐标
        lv_indev_get_point(indev, &point);

        lv_color_t c0;
        c0.full = 10;

        lv_point_t points[2];

        /* 如果是释放或首次使用 */
        if((last_x == -32768) || (last_y == -32768)) {
            last_x = point.x;
            last_y = point.y;
        } else {
            points[0].x = last_x;
            points[0].y = last_y;
            points[1].x = point.x;
            points[1].y = point.y;
            last_x      = point.x;
            last_y      = point.y;

            // 画一条线
            lv_canvas_draw_line(obj, points, 2, &sketchpad->line_rect_dsc);
        }
    }

    /* 放开画笔 */
    else if(code == LV_EVENT_RELEASED) {
        last_x = -32768;
        last_y = -32768;
    }
}

/*toolbar*/
static void lv_100ask_sketchpad_toolbar_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    // 设置构造函数的参数为未使用（压制警告）
    LV_UNUSED(class_p);
    // 在跟踪日志中记录对象的创建
    LV_TRACE_OBJ_CREATE("begin");

    // 为对象添加可点击标志
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    // 将对象与父对象对齐，并设置相对偏移
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 20, 20); // 原来是0,0
    // 设置对象的弹性增长属性
    lv_obj_set_flex_grow(obj, 1);
    // 设置对象的弹性布局流动方式为水平排列
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);

    // 初始颜色
    static lv_coord_t sketchpad_toolbar_cw = LV_100ASK_SKETCHPAD_TOOLBAR_OPT_CW;
    // 创建颜色按钮对象
    lv_obj_t * color = lv_label_create(obj);
    // 设置颜色按钮文本为编辑符号
    lv_label_set_text(color, LV_SYMBOL_EDIT);
    lv_obj_set_size(color, YOUR_WIDTH, YOUR_HEIGHT); // 设置颜色按钮的大小（像素）
    // 为颜色按钮添加可点击标志
    lv_obj_add_flag(color, LV_OBJ_FLAG_CLICKABLE);
    // 为颜色按钮对象添加事件回调，监听所有事件，并传递特定值
    lv_obj_add_event_cb(color, sketchpad_toolbar_event_cb, LV_EVENT_ALL, &sketchpad_toolbar_cw);

    // 初始宽度
    static lv_coord_t sketchpad_toolbar_width = LV_100ASK_SKETCHPAD_TOOLBAR_OPT_WIDTH;
    // 创建尺寸按钮对象
    lv_obj_t * size = lv_label_create(obj);
    // 设置尺寸按钮文本为弹出符号
    lv_label_set_text(size, LV_SYMBOL_EJECT);
    lv_obj_set_size(size, YOUR_WIDTH, YOUR_HEIGHT); // 设置颜色按钮的大小（像素）
    // 为尺寸按钮添加可点击标志
    lv_obj_add_flag(size, LV_OBJ_FLAG_CLICKABLE);
    // 为尺寸按钮对象添加事件回调，监听所有事件，并传递特定值
    lv_obj_add_event_cb(size, sketchpad_toolbar_event_cb, LV_EVENT_ALL, &sketchpad_toolbar_width);

    // 在跟踪日志中记录对象创建的结束
    LV_TRACE_OBJ_CREATE("finished");
}

static void lv_100ask_sketchpad_toolbar_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    // 析构函数为空，不执行任何操作
}

static void lv_100ask_sketchpad_toolbar_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
    // 设置事件处理函数的参数为未使用（压制警告）
    LV_UNUSED(class_p);

    lv_res_t res;

    /* 调用祖先层级的事件处理程序 */
    res = lv_obj_event_base(MY_CLASS, e);
    if(res != LV_RES_OK) return;

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj       = lv_event_get_target(e);

    if(code == LV_EVENT_PRESSING) {
        // 获取当前活动的输入设备
        lv_indev_t * indev = lv_indev_get_act();
        if(indev == NULL) return;

        lv_point_t vect;
        // 获取输入设备的向量
        lv_indev_get_vect(indev, &vect);

        lv_coord_t x = lv_obj_get_x(obj) + vect.x;
        lv_coord_t y = lv_obj_get_y(obj) + vect.y;
        // 设置对象的位置为偏移后的位置
        lv_obj_set_pos(obj, x, y);
    }
}

// 顶部工具栏的回调函数
static void sketchpad_toolbar_event_cb(lv_event_t * e)
{
    // 获取用户数据指针
    lv_coord_t * toolbar_opt = lv_event_get_user_data(e);
    // 获取事件代码
    lv_event_code_t code = lv_event_get_code(e);
    // 获取事件目标对象
    lv_obj_t * obj = lv_event_get_target(e);
    // 获取工具栏对象
    lv_obj_t * toolbar = lv_obj_get_parent(obj);
    // 获取涂鸦板对象
    lv_obj_t * sketchpad = lv_obj_get_parent(toolbar);
    // 获取涂鸦板结构体指针
    lv_100ask_sketchpad_t * sketchpad_t = (lv_100ask_sketchpad_t *)sketchpad;

    // 判断事件为 LV_EVENT_CLICKED
    if(code == LV_EVENT_CLICKED) {
        // 判断选择的工具为 LV_100ASK_SKETCHPAD_TOOLBAR_OPT_CW
        if((*toolbar_opt) == LV_100ASK_SKETCHPAD_TOOLBAR_OPT_CW) {
            // 定义静态变量并创建颜色选择器对象
            static lv_coord_t sketchpad_toolbar_cw = LV_100ASK_SKETCHPAD_TOOLBAR_OPT_CW;
            lv_obj_t * cw                          = lv_colorwheel_create(sketchpad, true);

            // 后续加入：取消色环模式切换
            lv_colorwheel_set_mode_fixed(cw, true);
            // 后续加入：设置主体宽度
            lv_obj_set_style_arc_width(cw, 30, LV_PART_MAIN);
            // 后续加入：设置主体半径
            lv_obj_set_style_size(cw, 200, 200, LV_PART_MAIN);

            // 将颜色选择器与工具栏按钮对齐并注册事件回调
            lv_obj_align_to(cw, obj, LV_ALIGN_OUT_BOTTOM_MID, 0, 50); // 色轮位置
            lv_obj_add_event_cb(cw, toolbar_set_event_cb, LV_EVENT_RELEASED, &sketchpad_toolbar_cw);
        } else if((*toolbar_opt) == LV_100ASK_SKETCHPAD_TOOLBAR_OPT_WIDTH) {
            // 定义静态变量并创建滑条对象
            static lv_coord_t sketchpad_toolbar_width = LV_100ASK_SKETCHPAD_TOOLBAR_OPT_WIDTH;
            lv_obj_t * slider                         = lv_slider_create(sketchpad);
            // 设置滑条初始化数值并将其与工具栏按钮对齐，然后注册事件回调
            lv_slider_set_value(slider, (int32_t)(sketchpad_t->line_rect_dsc.width), LV_ANIM_OFF);
            lv_obj_align_to(slider, obj, LV_ALIGN_OUT_BOTTOM_MID, 0, 10); // 线条位置
            lv_obj_add_event_cb(slider, toolbar_set_event_cb, LV_EVENT_ALL, &sketchpad_toolbar_width);
        }
    }
}
// 处理工具栏设置事件
static void toolbar_set_event_cb(lv_event_t * e)
{
    // 获取用户数据指针
    lv_coord_t * toolbar_opt = lv_event_get_user_data(e);
    // 获取事件代码
    lv_event_code_t code = lv_event_get_code(e);
    // 获取事件目标对象
    lv_obj_t * obj = lv_event_get_target(e);
    // 获取涂鸦板结构体指针
    lv_100ask_sketchpad_t * sketchpad = (lv_100ask_sketchpad_t *)lv_obj_get_parent(obj);

    // 判断事件为 LV_EVENT_RELEASED
    if(code == LV_EVENT_RELEASED) {
        // 判断选择的工具为 LV_100ASK_SKETCHPAD_TOOLBAR_OPT_CW
        if((*toolbar_opt) == LV_100ASK_SKETCHPAD_TOOLBAR_OPT_CW) {
            // 更新线条颜色并删除颜色选择器对象
            sketchpad->line_rect_dsc.color = lv_colorwheel_get_rgb(obj);
            lv_obj_del(obj);
        } else if(*(toolbar_opt) == LV_100ASK_SKETCHPAD_TOOLBAR_OPT_WIDTH) {
            // 删除滑条对象
            lv_obj_del(obj);
        }
    } else if(code == LV_EVENT_VALUE_CHANGED) {
        if((*toolbar_opt) == LV_100ASK_SKETCHPAD_TOOLBAR_OPT_WIDTH) {
            // 更新线条宽度
            sketchpad->line_rect_dsc.width = (lv_coord_t)lv_slider_get_value(obj);
        }
    }
}
