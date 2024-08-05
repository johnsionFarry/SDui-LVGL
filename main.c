#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>

#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "user/cJSON.h"
#include "user/tip.h"
//---------提示词相关//---------

// #include "user/curl.h"
//  #include <png.h>
//-------------连接服务器-------------
#define MAX_BUFFER_SIZE 10240000
#define SERVER_PORT 2300
#define SERVER_IP "192.168.10.104"
#define BASE_NAME "1.base64"
#define SERVER_BASE_FILE_NAME "server.base"
#define SERVER_PNG_FILE_NAME "server.png"
#define SERVER_BMP_FILE_NAME "server.bmp"
// #define SERVER_BIN_FILE_NAME "server.bin"

#define BMP_PIXEL_OFFSET 138
//-------------------JSON----------------------
// char show_txt[][18] = {"1xxxxx", "2xxxxx", "3xxxxx", "4xxxxx", "5xxxxx", "6xxxxx", "water", "beach", "street",
//                        "day",    "night",  "flower", "sfw",    "nsfw",   "one",    "two",   "girl",  "boy"};
//-------------------JSON----------------------
// 定义PNG文件头的标识符
const uint8_t png_signature[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
char * request;         // 发送缓存
char * response_buffer; // 接收缓存
char * base64_decode;   // 解码指令缓存
char bad_ = 0;          // 记录跳转

lv_obj_t * clear_button;  // 清除按钮
lv_obj_t * checkboxes[6]; // 声明一个静态数组来存储复选框对象句柄
lv_obj_t * dropdown;      // 下拉列表句柄
lv_obj_t * d2;            // 二级下拉菜单
lv_obj_t * select_button; // 生成方式句柄
lv_obj_t * CorS_button;   // 线稿切换句柄
lv_obj_t * bar;           // 进度条
char opinion;             // 记录复选框的设置，实时
char opinion_d2[12];      // 记录所有二级复选框的设置，发送用 randoms[0] cscenery[1-5] figure[6-11]
char like;                // 记录下拉列表设置
char like_d2;             // 记录二级下拉菜单的设置
char t_or_i     = 1;      // 0:t 1:i
char * send_str = NULL;   // 初始json项目为空字符串
char send_end_1 = 0;      // 发送结束标志位
char CorS       = 0;      // 0:canny硬边缘 1:scribble_pidinet涂鸦

int pipefd[2]; // 接收线程和主线程的简单异步通讯

// 定义t_or_i按钮的状态
typedef enum { BUTTON_STATE_NORMAL, BUTTON_STATE_SELECTED } button_state_t;
// 全局变量用于保存按钮的状态
button_state_t t_or_i_button_state = BUTTON_STATE_NORMAL; // t_or_i
button_state_t CorS_button_state   = BUTTON_STATE_NORMAL; // CorS
//-------------连接服务器-------------

//-------------画板-------------
#include "user/lv_100ask_sketchpad.h"

#define LV_CANVAS_BUF_SIZE_TRUE_COLOR(w, h) LV_IMG_BUF_SIZE_TRUE_COLOR(w, h)
#define LV_CANVAS_BUF_SIZE_TRUE_COLOR_CHROMA_KEYED(w, h) LV_IMG_BUF_SIZE_TRUE_COLOR_CHROMA_KEYED(w, h)
#define LV_CANVAS_BUF_SIZE_TRUE_COLOR_ALPHA(w, h) LV_IMG_BUF_SIZE_TRUE_COLOR_ALPHA(w, h)

#define SKETCHPAD_DEFAULT_WIDTH 800  /*LV_HOR_RES*/
#define SKETCHPAD_DEFAULT_HEIGHT 480 /*LV_VER_RES*/

#define BIN_NAME "canvas_snapshot.bin"
#define BMP_NAME "canvas_snapshot.bmp"
#define BASE_NAME "canvas_snapshot.BASE"

void lv_100ask_sketchpad_simple_test(void);
void CorS_button_NORMAL(lv_obj_t * label_d2);
void CorS_button_SELECTED(lv_obj_t * label_d2);
void btn_handler(lv_event_t * e);
void store_result_to_bit(char * byte, int position, int result);
bool read_bit_char(char * byte, int bit_position);
void print_binary_char_padding(char * byte);
void clear_button_callback(lv_obj_t * button, lv_event_t event);
void appendStringWithComma(char ** str_ptr, const char * content);
lv_obj_t * create_clear_button(lv_obj_t * sketchpad);
lv_obj_t * create_send_button(lv_obj_t * sketchpad);
void create_select_button(lv_obj_t * sketchpad);
void create_CorS_button(lv_obj_t * sketchpad);
void create_dropdown(lv_obj_t * clear_button);
void create_progress(lv_obj_t * parent); // 进度条
void convert_bin_to_bmp(const char * binFilename, const char * bmpFilename, uint32_t width, uint32_t height);

int read_png_header(FILE * file);
void read_png_pixels(FILE * file, uint8_t * pixels, size_t width, size_t height);
void lv_png_test(void);
void png_check(void);

void decodeBase64(const char * input, const char * output);
void decodePng(const char * input, const char * output);
void * send_request(void * sockfd);
void * receive_response(void * sockfd);
void read_bmp_pixels(const char * filename, size_t cbuf_size);
void * send_to_server(void);

lv_obj_t * sketchpad; // 画布
//-------------画板-------------

//-------------截图-------------
// BMP 文件头结构
#pragma pack(1)
typedef struct
{
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} BMP_Header; // 定义位图文件头

typedef struct
{
    uint32_t headerSize;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    int32_t xPixelsPerMeter;
    int32_t yPixelsPerMeter;
    uint32_t colorsUsed;
    uint32_t colorsImportant;
} BMP_InfoHeader; // 定义位图信息头
#pragma pack()
//-------------截图-------------

#define DISP_BUF_SIZE (800 * 480) // 之前是480 * 1024

// 绘画板缓存
lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(SKETCHPAD_DEFAULT_WIDTH, SKETCHPAD_DEFAULT_HEIGHT)];
// 解码器缓存
lv_color_t server_cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(SKETCHPAD_DEFAULT_WIDTH, SKETCHPAD_DEFAULT_HEIGHT)];

int main(void)
{
    struct rlimit rl;
    rl.rlim_cur = RLIM_INFINITY; // 设置为无限制
    rl.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_STACK, &rl);

    // printf("\033[?25l"); // 隐藏光标闪烁

    lv_init();                                                  // lvgl初始化
    fbdev_init();                                               // 输出设备初始化及注册
    static lv_color_t buf[DISP_BUF_SIZE];                       // 显存大小
    static lv_disp_draw_buf_t disp_buf;                         // 显存设置结构体
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE); // 将显存空间传递到结构体中
    /*注册屏幕驱动与相关参数*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = fbdev_flush;
    disp_drv.hor_res  = 800;
    disp_drv.ver_res  = 480;
    lv_disp_drv_register(&disp_drv);

    // 输入设备初始化及注册
    evdev_init();
    static lv_indev_drv_t indev_drv_1;
    lv_indev_drv_init(&indev_drv_1); /*Basic initialization*/
    indev_drv_1.type = LV_INDEV_TYPE_POINTER;
    /*This function will be called periodically (by the library) to get the mouse position and state*/
    indev_drv_1.read_cb      = evdev_read;
    lv_indev_t * mouse_indev = lv_indev_drv_register(&indev_drv_1);

    // 官方demo---可以换为自己的demo
    lv_100ask_sketchpad_simple_test(); // 画板
    // lv_demo_widgets();

    /*事物处理及告知lvgl节拍数*/
    while(1) {
        lv_timer_handler(); // 事务处理
        lv_tick_inc(5);     // 节拍累计
        usleep(5000);
    }

    return 0;
}

/*用户节拍获取*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
//-------------画板-------------
void lv_100ask_sketchpad_simple_test(void)
{
    // 创建一个画布，大小为SKETCHPAD_DEFAULT_WIDTH x SKETCHPAD_DEFAULT_HEIGHT，颜色缓冲区为cbuf
    sketchpad = lv_100ask_sketchpad_create(lv_scr_act());
    // 设置画布的缓冲区，大小为SKETCHPAD_DEFAULT_WIDTH x SKETCHPAD_DEFAULT_HEIGHT，颜色格式为LV_IMG_CF_TRUE_COLOR
    lv_canvas_set_buffer(sketchpad, cbuf, SKETCHPAD_DEFAULT_WIDTH, SKETCHPAD_DEFAULT_HEIGHT, LV_IMG_CF_TRUE_COLOR);
    // 将画布居中
    lv_obj_center(sketchpad);
    // 画布背景色为LV_PALETTE_GREY，不透明度为LV_OPA_COVER
    lv_canvas_fill_bg(sketchpad, lv_palette_lighten(LV_PALETTE_GREY, 3), LV_OPA_COVER);

    create_clear_button(sketchpad);  // 创建清空按钮
    create_select_button(sketchpad); // 创建切换生成方式按钮
    create_dropdown(sketchpad);      // 创建风格选择列表
    create_d2(sketchpad);            // 创建二级下拉菜单
    create_CorS_button(sketchpad);   // 创建线稿切换按钮

    create_send_button(sketchpad);   // 创建发送按钮
    create_checkboxes(lv_scr_act()); // 复选框
    create_progress(lv_scr_act());   // 进度条
}
void CorS_button_NORMAL(lv_obj_t * label_d2)
{
    // if(CorS_button_state == BUTTON_STATE_NORMAL) {
    if(t_or_i == 1) {
        lv_label_set_text(label_d2, "CtrlNet OFF");
    } else if(t_or_i == 0) {
        lv_label_set_text(label_d2, "CtrlNet OFF");
    }
    // }
}
void CorS_button_SELECTED(lv_obj_t * label_d2)
{
    // if(CorS_button_state == BUTTON_STATE_SELECTED) {
    if(t_or_i == 1) {
        lv_label_set_text(label_d2, "Scribble");
    } else if(t_or_i == 0) {
        lv_label_set_text(label_d2, "Canny");
    }
    // }
}
void btn_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);               // 事件
    lv_obj_t * label_d1  = lv_obj_get_child(select_button, 0); // 通过子对象的索引获取子对象
    lv_obj_t * label_d2  = lv_obj_get_child(CorS_button, 0);   // 通过子对象的索引获取子对象
    lv_obj_t * obj       = lv_event_get_target(e);             // 对象
    //----------------------------CLEAN----------------------------
    if(obj == clear_button) {
        printf("BTN-CLEAN:\tPRESSED\n");
        lv_canvas_fill_bg(sketchpad, lv_palette_lighten(LV_PALETTE_GREY, 3), LV_OPA_COVER);
        send_end_1 = 1; // 设立清空标志位

        lv_obj_clear_state(checkboxes[0], LV_STATE_CHECKED | LV_STATE_DISABLED); // 清除选中和状态
        lv_obj_clear_state(checkboxes[1], LV_STATE_CHECKED | LV_STATE_DISABLED);
        lv_obj_clear_state(checkboxes[2], LV_STATE_CHECKED | LV_STATE_DISABLED);
        lv_obj_clear_state(checkboxes[3], LV_STATE_CHECKED | LV_STATE_DISABLED);
        lv_obj_clear_state(checkboxes[4], LV_STATE_CHECKED | LV_STATE_DISABLED);
        lv_obj_clear_state(checkboxes[5], LV_STATE_CHECKED | LV_STATE_DISABLED);
        opinion = 0; // 清除选择
        for(int a = 0; a <= 11; a++) {
            opinion_d2[a] = 0; // 清空所有永久配置
        }

        DIR * dir;
        struct dirent * entry;
        dir = opendir(".");
        if(dir == NULL) {
            perror("Unable to open directory");
            exit(EXIT_FAILURE);
        }
        while((entry = readdir(dir)) != NULL) {
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            if(strcmp(entry->d_name, "demo") == 0) continue;
            if(remove(entry->d_name) != 0) {
                perror("Error deleting file");
            } else {
                printf("Deleted file: %s\n", entry->d_name);
            }
        }
        closedir(dir);
        lv_bar_set_value(bar, 0, LV_ANIM_OFF); // 清空进度条
        bad_ = 0;                              // 清空跳转
    }
    //----------------------------一级按钮----------------------------
    if(obj == select_button) {
        if(code == LV_EVENT_PRESSED) {
            // 根据按钮的状态进行切换 && obj == select_button
            if(t_or_i_button_state == BUTTON_STATE_NORMAL) {
                // 按下按钮后变为红色
                lv_obj_set_style_bg_color(select_button, lv_color_hex(0xff0000), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_label_set_text(label_d1, "txt 2 img");
                t_or_i              = 0;
                t_or_i_button_state = BUTTON_STATE_SELECTED;
                CorS_button_NORMAL(label_d2);
                CorS_button_SELECTED(label_d2); // 强制刷新
            } else if(t_or_i_button_state == BUTTON_STATE_SELECTED) {
                // 再按一下恢复为黄色
                lv_obj_set_style_bg_color(select_button, lv_color_hex(0xFFA500), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_label_set_text(label_d1, "img 2 img");
                t_or_i              = 1;
                t_or_i_button_state = BUTTON_STATE_NORMAL;
                CorS_button_NORMAL(label_d2);
                CorS_button_SELECTED(label_d2); // 强制刷新
            }
            printf("Btn_1_state: %d\tt_or_i:%d\n", t_or_i_button_state, t_or_i); // 要在标志位刷新完后打印
        }
    }
    //----------------------------二级按钮----------------------------
    if(obj == CorS_button) {
        if(code == LV_EVENT_PRESSED) {
            if(CorS_button_state == BUTTON_STATE_NORMAL) {
                // 按下按钮后变为红色
                lv_obj_set_style_bg_color(CorS_button, lv_color_hex(0xff0000), LV_PART_MAIN | LV_STATE_DEFAULT);
                CorS              = 0;
                CorS_button_state = BUTTON_STATE_SELECTED;
                CorS_button_NORMAL(label_d2);
            } else if(CorS_button_state == BUTTON_STATE_SELECTED) {
                // 再按一下恢复为黄色
                lv_obj_set_style_bg_color(CorS_button, lv_color_hex(0xFFA500), LV_PART_MAIN | LV_STATE_DEFAULT);
                CorS              = 1;
                CorS_button_state = BUTTON_STATE_NORMAL;
                CorS_button_SELECTED(label_d2);
            }
            printf("Btn_2_state: %d\tCorS:%d\n", CorS_button_state, CorS);
        }
    }
}
void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);                     // 事件
    lv_obj_t * obj       = lv_event_get_target(e);                   // 对象
    like                 = (char)lv_dropdown_get_selected(dropdown); // 获取下拉菜单选项
    like_d2              = (char)lv_dropdown_get_selected(d2);       // 获取二级下拉菜单选项
    char temp;                                                       // 存储需要写入所有选项的下标
    if(like == 0) {
        temp = 0; // 只有一个选项
    } else if(like == 1) {
        temp = like_d2 + 1; // 五个选项
    } else if(like == 2) {
        temp = like_d2 + 6; // 六个选项
    }

    bool state_checkBox;
    //------------------------------复选框----------------------------
    if(code == LV_EVENT_VALUE_CHANGED) { // 切换状态时触发

        for(int a = 0; a <= 5; a++) {
            if(obj == checkboxes[a]) {
                state_checkBox = lv_obj_has_state(checkboxes[a], LV_STATE_CHECKED);
                store_result_to_bit(&opinion, a, (int)state_checkBox);          // 写入实时选项
                store_result_to_bit(&opinion_d2[temp], a, (int)state_checkBox); // 保存所有选项
                /*if(like == 2) {// 旧的业务逻辑 互斥逻辑可以参考
                    if(read_bit_char(&opinion, a) == 1) {
                        if(a % 2 == 0) { // 构建人物模式下按钮两两互斥逻辑
                            lv_obj_clear_state(checkboxes[a + 1], LV_STATE_CHECKED); // 清除选中状态
                            store_result_to_bit(&opinion, a + 1, (int)~state_checkBox);
                        } else {
                            lv_obj_clear_state(checkboxes[a - 1], LV_STATE_CHECKED);
                            store_result_to_bit(&opinion, a - 1, (int)~state_checkBox);
                        }
                    }
                } else {
                }*/
                printf("CheckBox %d:\tPUSSED\tOpinion:\t", a);
                print_binary_char_padding(&opinion); // 输出实时配置
                printf("final opinion: \t");
                for(int a = 0; a <= 11; a++) {
                    printf("%d ", opinion_d2[a]); // 按数字打印所有配置
                }
                printf("\n");
            }
        }
        //--------------------复选框----------------下拉列表----------------
        if(obj == dropdown) {
            printf("Dropdown:\t%d\n", like);
            opinion = 0; // 重置提示词选择
            // int b   = like * 6; // 计算选项索引
            for(char a = 0; a <= 5; a++) {
                lv_obj_clear_state(checkboxes[a], LV_STATE_DISABLED); // 清除不可用状态
                lv_obj_clear_state(checkboxes[a], LV_STATE_CHECKED);  // 移除选中
                // 读取之前的选择
                if(read_bit_char(&opinion_d2[temp], a)) {
                    lv_obj_add_state(checkboxes[a], LV_STATE_CHECKED); // 添加选中
                }
                if(like == 0) {
                    lv_checkbox_set_text(checkboxes[a], randoms[a]);
                } else if(like == 1) {
                    lv_checkbox_set_text(checkboxes[a], cscenery[a]);
                } else if(like == 2) {
                    lv_checkbox_set_text(checkboxes[a], figure[a]);
                }
            }
            if(like == 0) {
                lv_dropdown_set_options(d2, "None");
            } else if(like == 1) {
                lv_dropdown_set_options(d2, "soil\nwater\ncity\nsky\nexample");
            } else if(like == 2) {
                lv_dropdown_set_options(d2, "hair\nbody\nlegs\nshoes\nposturen\nexample");
            }
        }
        //--------------------------二级下拉菜单----------------------------
        if(obj == d2) {
            printf("Dropdown_d2:\t%d\n", like_d2);
            opinion = 0; // 切换二级菜单时清空复选框内容
            for(char a = 0; a <= 5; a++) {
                lv_obj_clear_state(checkboxes[a], LV_STATE_DISABLED); // 清除不可用状态
                lv_obj_clear_state(checkboxes[a], LV_STATE_CHECKED);  // 移除选中
                // 读取之前的选择
                if(read_bit_char(&opinion_d2[temp], a)) {
                    lv_obj_add_state(checkboxes[a], LV_STATE_CHECKED); // 添加选中
                }
                if(like == 1) {
                    lv_checkbox_set_text(checkboxes[a], cscenery[a + 6 * like_d2]);
                } else if(like == 2) {
                    lv_checkbox_set_text(checkboxes[a], figure[a + 6 * like_d2]);
                }
            }
        }
    }
}
void store_result_to_bit(char * byte, int position, int result) // 将函数返回值存储到 char 变量的指定位上
{
    if(result == 1) {
        *byte |= (1 << position); // 将返回值设置到指定位上
    } else {
        *byte &= ~(1 << position); // 将返回值设置到指定位上
    }
}
bool read_bit_char(char * byte, int bit_position) // 读取 char 变量的特定位，并返回该位的值
{
    if(bit_position < 0 || bit_position > 7) // 确保 bit_position 在有效范围内（0到7）
    {
        printf("Error: Invalid bit position.\n");
        return -1; // 返回错误代码表示无效的位位置
    }
    int bit_value = (*byte & (1 << bit_position)) ? 1 : 0; // 使用位运算获取特定位的值（0或1）
    return bit_value;
}
void print_binary_char_padding(char * byte) // 输出 char 变量的二进制形式（高位补0）
{
    // 从最高位开始检查并输出每一位
    for(int i = 7; i >= 0; --i) {
        // 使用位运算获取每一位的值，并输出到控制台
        printf("%d", (*byte & (1 << i)) ? 1 : 0);
    }
    printf("\n"); // 换行
}
lv_obj_t * create_clear_button(lv_obj_t * sketchpad) // 清除画板
{
    // 创建按钮
    clear_button = lv_btn_create(lv_scr_act());
    // 设置按钮位置和对齐方式
    // lv_obj_set_pos(clear_button, 0, 0);
    lv_obj_set_align(clear_button, LV_ALIGN_TOP_LEFT);
    // 设置按钮为蓝色
    lv_obj_set_style_bg_color(clear_button, lv_color_hex(0x007aff), LV_PART_MAIN | LV_STATE_DEFAULT);
    // 设置按钮大小
    lv_obj_set_size(clear_button, 100, 40);

    // 创建按钮文本
    lv_obj_t * label = lv_label_create(clear_button);
    lv_label_set_text(label, "CLEAN");
    lv_obj_set_align(label, LV_ALIGN_CENTER);
    lv_obj_set_style_text_color(label, lv_color_white(), 0); // 设置文本颜色为白色

    // 设置按钮回按下时触发，并且传递sketchpad
    // lv_obj_add_event_cb(clear_button, clear_button_callback, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(clear_button, btn_handler, LV_EVENT_PRESSED, NULL);

    return clear_button;
}
void create_select_button(lv_obj_t * sketchpad)
{
    // 创建按钮
    select_button = lv_btn_create(lv_scr_act());
    // 设置按钮位置和对齐方式
    lv_obj_set_pos(select_button, 120, 0);
    lv_obj_set_align(select_button, LV_ALIGN_TOP_LEFT);
    // 设置按钮为蓝色
    lv_obj_set_style_bg_color(select_button, lv_color_hex(0xFFA500), LV_PART_MAIN | LV_STATE_DEFAULT);
    // 设置按钮大小
    lv_obj_set_size(select_button, 100, 40);

    // 创建按钮文本
    lv_obj_t * label = lv_label_create(select_button);
    lv_label_set_text(label, "img 2 img");
    lv_obj_set_align(label, LV_ALIGN_CENTER);
    lv_obj_set_style_text_color(label, lv_color_white(), 0); // 设置文本颜色为白色

    // 设置按钮回按下时触发，并且传递sketchpad
    // lv_obj_add_event_cb(select_button, select_button_callback, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(select_button, btn_handler, LV_EVENT_PRESSED, NULL);
}
void create_CorS_button(lv_obj_t * sketchpad) // 切换图生线还是线生图
{
    // 创建按钮
    CorS_button = lv_btn_create(lv_scr_act());
    // 设置按钮位置和对齐方式
    lv_obj_set_pos(CorS_button, 240, 0);
    lv_obj_set_align(CorS_button, LV_ALIGN_TOP_LEFT);
    // 设置按钮为蓝色
    lv_obj_set_style_bg_color(CorS_button, lv_color_hex(0xFFA500), LV_PART_MAIN | LV_STATE_DEFAULT);
    // 设置按钮大小
    lv_obj_set_size(CorS_button, 100, 40);

    // 创建按钮文本
    lv_obj_t * label = lv_label_create(CorS_button);
    lv_label_set_text(label, "Scribble");
    lv_obj_set_align(label, LV_ALIGN_CENTER);
    lv_obj_set_style_text_color(label, lv_color_white(), 0); // 设置文本颜色为白色

    // 设置按钮回按下时触发，并且传递sketchpad
    // lv_obj_add_event_cb(CorS_button, CorS_button_callback, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(CorS_button, btn_handler, LV_EVENT_PRESSED, NULL);
}
void create_dropdown(lv_obj_t * parent) // 选择风格
{
    dropdown = lv_dropdown_create(parent);            // 创建下拉菜单
    lv_obj_align(dropdown, LV_ALIGN_TOP_LEFT, 0, 50); // 设置下拉菜单的位置
    // lv_obj_set_pos(dropdown, 50, 50);
    lv_dropdown_set_options(dropdown, "random\ncscenery\nfigure");    // 设置下拉菜单的选项
    lv_dropdown_set_selected(dropdown, 0);                            // 设置下拉菜单的初始值
    lv_obj_add_event_cb(dropdown, event_handler, LV_EVENT_ALL, NULL); // 添加回调函数
}
void create_d2(lv_obj_t * parent) // 风格二级菜单
{
    d2 = lv_dropdown_create(parent);             // 创建下拉菜单
    lv_obj_align(d2, LV_ALIGN_TOP_LEFT, 0, 100); // 设置下拉菜单的位置
    // lv_obj_set_pos(dropdown, 50, 50);
    lv_dropdown_set_options(d2, "None");
    // 不设置下拉菜单的选项，而是使用lv_dropdown_add_option(dropdown, "New option", pos)来按条件添加索引
    lv_dropdown_set_selected(d2, 0);                            // 设置下拉菜单的初始值
    lv_obj_add_event_cb(d2, event_handler, LV_EVENT_ALL, NULL); // 添加回调函数
}
void create_progress(lv_obj_t * parent) // 进度条
{
    bar = lv_bar_create(parent);
    lv_obj_set_size(bar, 200, 20);                   // 设置进度条大小
    lv_obj_align(bar, LV_ALIGN_TOP_RIGHT, -120, 10); // 对齐位置
    lv_obj_set_style_anim_time(bar, 3000, LV_STATE_DEFAULT); // 设置动画时间，默认动画是很快的 需要在设置值之前
    lv_bar_set_range(bar, 0, 100);                           // 设置进度条范围
    lv_bar_set_value(bar, 0, LV_ANIM_ON); // 其实默认就是0，如果想更改初始默认值，需要修改模式
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x50FF50), LV_PART_INDICATOR);
    // printf("create_progress\n");
}
void send_button_callback(lv_event_t * e)
{
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);
    lv_bar_set_value(bar, 90, LV_ANIM_ON);
    // lv_obj_invalidate(bar); // 标记无效对象
    // lv_task_handler();      // 调用任务句柄

    lv_obj_t * btn   = lv_event_get_target(e);   // 获取对象
    lv_obj_t * label = lv_obj_get_child(btn, 0); // 通过子对象的索引获取子对象
    // lv_event_code_t code = lv_event_get_code(e);      // 获取事件的事件代码
    // lv_obj_t * (对应的变量)     = lv_event_get_user_data(e); // 获取lv_label_set_text携带数据

    printf("BTN-SEND:\tPRESSED\n");
    // 更改按钮文本
    // lv_label_set_text(label, "RUNING");
    // lv_task_handler();

    // cbuf转bin
    save_canvas_to_bin(BIN_NAME, cbuf, SKETCHPAD_DEFAULT_WIDTH, SKETCHPAD_DEFAULT_HEIGHT);
    // bin转bmp
    convert_bin_to_bmp(BIN_NAME, BMP_NAME, SKETCHPAD_DEFAULT_WIDTH, SKETCHPAD_DEFAULT_HEIGHT);
    // bmp转base64
    base64_encode_file(BMP_NAME, BASE_NAME);
    // 发送到服务器 这里会产生阻塞等待 如果不想等待需要再开一个线程

    // 创建分离属性的子线程
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_t thread;
    pthread_create(&thread, &attr, send_to_server, NULL);
    pthread_attr_destroy(&attr);

    // send_to_server();

    // 执行完毕后恢复文本
    // lv_label_set_text(label, "SEND");
}
lv_obj_t * create_send_button(lv_obj_t * sketchpad) // 发送图片
{
    // 创建按钮
    lv_obj_t * send_button = lv_btn_create(lv_scr_act());
    // 设置按钮位置和对齐方式
    // lv_obj_set_pos(clear_button, 0, 0);
    lv_obj_set_align(send_button, LV_ALIGN_TOP_RIGHT);
    // 设置按钮为蓝色
    lv_obj_set_style_bg_color(send_button, lv_color_hex(0x007aff), LV_PART_MAIN | LV_STATE_DEFAULT);
    // 设置按钮大小
    lv_obj_set_size(send_button, 100, 40);
    // 设置按钮文本
    lv_obj_t * label = lv_label_create(send_button);
    lv_label_set_text(label, "SEND");
    lv_obj_set_align(label, LV_ALIGN_CENTER);
    lv_obj_set_style_text_color(label, lv_color_white(), 0); // 设置文本颜色为白色
    // 设置按钮回按下时触发，并且传递参数
    lv_obj_add_event_cb(send_button, send_button_callback, LV_EVENT_PRESSED, NULL);

    return send_button;
}
void create_checkboxes(lv_obj_t * parent)
{
    lv_coord_t y1 = 65;  // 垂直起始值
    lv_coord_t y  = 30;  // 垂直偏移值
    lv_coord_t x1 = -10; // 水平位置
    lv_coord_t f  = 10;  // 复选框间隔
    lv_coord_t w  = 120; // 限制复选框宽度

    for(int a = 0; a <= 5; a++) {
        checkboxes[a] = lv_checkbox_create(parent);
        lv_obj_set_width(checkboxes[a], w);
        lv_checkbox_set_text(checkboxes[a], randoms[a]);
        lv_obj_set_style_pad_column(checkboxes[a], f, LV_STATE_DEFAULT);
        lv_obj_align(checkboxes[a], LV_ALIGN_TOP_RIGHT, x1, (y1 + y * a)); // 设置位置
        lv_obj_add_event_cb(checkboxes[a], event_handler, LV_EVENT_ALL, NULL);
    }
    // 创建更多的复选框...
}
void save_canvas_to_bin(const char * filename, lv_color_t * cbuf, uint32_t width,
                        uint32_t height) // 画布缓存转bin文件
{
    // 判断filename文件是否存在，如果存在则替换，不存在则创建
    FILE * file = fopen(filename, "wb");
    if(file == NULL) {
        printf("Error:\tCould not open file for writing.\n");
        return;
    }
    // 写入颜色缓存数据
    fwrite(cbuf, sizeof(lv_color_t), width * height, file);
    printf("CANCAS TO BIN:\tOK\n");
    fclose(file);
}
void convert_bin_to_bmp(const char * binFilename, const char * bmpFilename, uint32_t width,
                        uint32_t height) // 二进制转BMP
{
    FILE * binFile = fopen(binFilename, "rb");
    FILE * bmpFile = fopen(bmpFilename, "wb");

    BMP_Header header         = {0x4D42, 0, 0, 0, 54};
    BMP_InfoHeader infoHeader = {40, width, -height, 1, 24, 0, 0, 0, 0, 0, 0};

    // 计算 BMP 文件大小
    header.size = 54 + width * height * 3;

    // 写入 BMP 文件头和信息头
    fwrite(&header, sizeof(BMP_Header), 1, bmpFile);
    fwrite(&infoHeader, sizeof(BMP_InfoHeader), 1, bmpFile);

    // 读取颜色缓存数据并写入 BMP 文件
    for(uint32_t i = 0; i < width * height; i++) {
        lv_color_t color;
        fread(&color, sizeof(lv_color_t), 1, binFile);

        // 写入 B-G-R 顺序的颜色数据
        fputc(color.ch.blue, bmpFile);
        fputc(color.ch.green, bmpFile);
        fputc(color.ch.red, bmpFile);
    }

    printf("BIN TO BMP:\tOK\n");

    fclose(binFile);
    fclose(bmpFile);
}
int read_png_header(FILE * file) // 读取PNG文件头
{
    uint8_t header[8];
    fread(header, 1, 8, file);
    return memcmp(header, png_signature, 8) == 0 ? 1 : 0;
}
void read_png_pixels(FILE * file, uint8_t * pixels, size_t width, size_t height) // 读取PNG文件的像素数据
{}
void png_check() // BMP数据导入 lv_obj_t * sketchpad 画布
{
    const char * filename = SERVER_PNG_FILE_NAME;
    FILE * file           = fopen(filename, "rb");
    if(file == NULL) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return 1;
    }

    // 检查文件头 0-7
    if(!read_png_header(file)) {
        fprintf(stderr, "Error: %s is not a valid PNG file\n", filename);
        fclose(file);
        return 1;
    }
    // 读取后续长度
    uint32_t chunkLength;
    fseek(file, 8, SEEK_SET);
    fread(&chunkLength, sizeof(uint32_t), 1, file);
    printf("数据长度为:\t%d\n", chunkLength = ntohl(chunkLength));

    // 定位到IHDR块（图像头块）
    fseek(file, 16, SEEK_SET);
    // 读取图像尺寸
    uint32_t width, height;
    fread(&width, sizeof(uint32_t), 1, file); // 读取宽度
    // fseek(file, 2, SEEK_CUR);
    fread(&height, sizeof(uint32_t), 1, file); // 读取高度
    // 输出图像尺寸
    printf("宽度:\t\t%u\n高度:\t\t%u\n", width = ntohl(width), height = ntohl(height));

    // 色彩深度
    uint8_t bit_depth;
    fread(&bit_depth, sizeof(uint8_t), 1, file);
    printf("色彩深度:\t%u\n", bit_depth);

    fclose(file);
}
void lv_png_test()
{
    lv_obj_t * png_img = lv_img_create(lv_scr_act());
    lv_img_set_src(png_img, "S:/path/to/your/image.png");
}
void base64_encode_file(const char * bmp_file, const char * base_name)
{
    char command[100000];

    // 构建 base64 命令
    sprintf(command, "base64 -w 0 %s > %s", bmp_file, base_name);

    // 调用 system 函数执行命令
    int result = system(command);

    // 检查命令执行结果
    if(result == -1) {
        printf("BMP TO BASE:\tERROR\n");
    } else {
        printf("BMP TO BASE:\tOK\n");
    }
}
void decodeBase64(const char * input, const char * output) // BASE64 数据解码
{
    // 构建解码命令
    base64_decode = (char *)malloc(MAX_BUFFER_SIZE * sizeof(char));
    if(base64_decode == NULL) {
        printf("内存分配失败\n");
    }
    sprintf(base64_decode, "base64 -d %s > %s", input, output);

    // 执行解码命令
    system(base64_decode);
}
void decodePng(const char * input, const char * output)
{
    char command[128];
    sprintf(command, "convert %s %s", input, output);

    // 调用 system 函数执行命令
    int result = system(command);

    // 检查命令执行结果
    if(result == 0) {
        printf("\n图像转换成功\n");
    } else {
        printf("\n图像转换失败\n");
    }
}
void appendStringWithComma(char ** str_ptr, const char * content) // 合成键值
{
    static int first_run = 1; // 静态变量，用于跟踪是否是第一次运行
    if(send_end_1 == 1) {
        first_run = 1; // 如果执行完发送操作，那么重置静态变量状态
        printf("Send_End !!!\n");
        memset(*str_ptr, 0, strlen(*str_ptr));
        send_end_1 = 0;
    }
    // 获取传入字符串的长度
    size_t current_length = (*str_ptr) ? strlen(*str_ptr) : 0;
    size_t content_length = strlen(content);
    // 计算新字符串的长度，包括逗号和空字符
    size_t new_length = current_length + content_length + (first_run ? 0 : 1) + 1;
    // 重新分配内存以容纳新的字符串
    *str_ptr = (char *)realloc(*str_ptr, new_length);
    // 添加逗号（如果不是第一次运行）
    if(!first_run) {
        strcat(*str_ptr, ",");
    } else {
        first_run = 0; // 标记第一次运行已经完成
    }
    // 将内容追加到字符串末尾
    strcat(*str_ptr, content);
    // printf("本次追加内容%s\n", *str_ptr);
}
void * send_request(void * sockfd) // 发送线程
{
    cJSON * json;
    int sockfd_value = *((int *)sockfd);

    FILE * file = fopen(BASE_NAME, "r");
    if(file == NULL) {
        printf("BASE64:\tOPEN FIAL 1");
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    char * buffer = (char *)malloc(file_size + 1);
    if(buffer == NULL) {
        printf("BASE64:\tOPEN FIAL 2");
    } else {
        // printf("BASE64:\t\tSEND OK\n");
    }

    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';
    fclose(file);
    //--------------------------文件读取--------------------------
    // 创建一个 JSON 对象
    json = cJSON_CreateObject();
    appendStringWithComma(&send_str, "masterpiece,best quality"); // 添加前置提示词
    // 提示词添加逻辑
    prompt(); // 新的添加逻辑已经移到tip.c

    if(opinion == 0) {
        cJSON_AddItemToObject(json, "prompt", cJSON_CreateString("")); // 如果所有选项都没有勾选，则填充空
    } else {                                                           // 否则填入处理过的字段
        cJSON_AddItemToObject(json, "prompt", cJSON_CreateString(send_str));
    }
    cJSON_AddItemToObject(json, "negative_prompt",
                          cJSON_CreateString(nenegative_prompt)); // 固定的反向提示词

    printf("正向:\n%s\n反向:\n%s", send_str, nenegative_prompt);
    // printf("提示词为: %s", cJSON_PrintUnformatted(json));
    //-----------------------------提示词逻辑----------------------
    cJSON_AddItemToObject(json, "seed", cJSON_CreateNumber(-1));                         // 种子
    cJSON_AddItemToObject(json, "batch_size", cJSON_CreateNumber(1));                    // 并行队列
    cJSON_AddItemToObject(json, "n_iter", cJSON_CreateNumber(1));                        // 批量队列
    cJSON_AddItemToObject(json, "steps", cJSON_CreateNumber(20));                        // 迭代步数
    cJSON_AddItemToObject(json, "cfg_scale", cJSON_CreateNumber(7));                     // 提示词相关性
    cJSON_AddItemToObject(json, "width", cJSON_CreateNumber(SKETCHPAD_DEFAULT_WIDTH));   // 宽
    cJSON_AddItemToObject(json, "height", cJSON_CreateNumber(SKETCHPAD_DEFAULT_HEIGHT)); // 高
    cJSON_AddItemToObject(json, "restore_faces", cJSON_CreateTrue());                    // 面部修复
    cJSON_AddItemToObject(json, "tiling", cJSON_CreateFalse()); // 是否使用平铺/重复生成图像
    cJSON_AddItemToObject(json, "eta", cJSON_CreateNumber(0));
    cJSON_AddItemToObject(json, "script_args", cJSON_CreateArray()); // 传递给生成图像的脚本的参数
    cJSON_AddItemToObject(json, "sampler_index", cJSON_CreateString("DPM++ SDE Karras")); // 图像采样器的索引
    //-----------------------------公用部分--------------------------
    if(t_or_i == 1) { // 图生图
        if(CorS == 1) {
            t_or_i = 0;
            bad_   = 1;
            goto bad;
        }
        cJSON * override_settings = cJSON_AddObjectToObject(json, "override_settings");
        cJSON_AddItemToObject(
            override_settings, "sd_model_checkpoint",
            cJSON_CreateString(
                "cetusMix_Whalefall2.safetensors [876b4c7ba5]")); // anything-v5-PrtRE.safetensors [7f96a1a9ca]
        cJSON_AddItemToObject(override_settings, "sd_vae", cJSON_CreateString("Automatic"));
        // 图生图控制网关基础图像是在外面的
        cJSON * initImagesArray = cJSON_AddArrayToObject(json, "init_images"); // 图生图的基础图片
        cJSON_AddItemToArray(initImagesArray, cJSON_CreateString(buffer));
    }
    if(t_or_i == 0) { // 单文生图 ：忽略画板内容，仅输入提示词
        cJSON * override_settings = cJSON_AddObjectToObject(json, "override_settings");
        cJSON_AddItemToObject(
            override_settings, "sd_model_checkpoint",
            cJSON_CreateString(
                "cetusMix_Whalefall2.safetensors [876b4c7ba5]")); // anything-v5-PrtRE.safetensors [7f96a1a9ca]
        cJSON_AddItemToObject(override_settings, "sd_vae", cJSON_CreateString("Automatic"));
        // 以下为启动控制网所独有
        if(CorS == 1) { // 文生图 + 控制网 ：画板内容作为轮廓
        bad:
            // 添加 "alwayson_scripts" 对象
            cJSON * alwaysonScriptsObject = cJSON_AddObjectToObject(json, "alwayson_scripts");       // 脚本参数
            cJSON * controlNetObject = cJSON_AddObjectToObject(alwaysonScriptsObject, "ControlNet"); // 控制网参数
            cJSON * argsArray        = cJSON_AddArrayToObject(controlNetObject, "args");
            cJSON * argsObject       = cJSON_CreateObject();
            cJSON_AddItemToArray(argsArray, argsObject);

            // 向 "args" 对象中添加键值对
            cJSON_AddItemToObject(argsObject, "enabled", cJSON_CreateTrue());         // 是否启用
            cJSON_AddItemToObject(argsObject, "control_mode", cJSON_CreateNumber(0)); // 对应ControlMode使用012
            if(bad_ == 0) {
                cJSON_AddItemToObject(argsObject, "model",
                                      cJSON_CreateString("control_v11p_sd15_canny_fp16 [b18e0966]")); // 控制网名字
                cJSON_AddItemToObject(argsObject, "module",
                                      cJSON_CreateString("canny")); // 预处理器名字
            } else {
                cJSON_AddItemToObject(argsObject, "model",
                                      cJSON_CreateString("control_v11p_sd15_scribble_fp16 [4e6af23e]")); // 控制网名字
                cJSON_AddItemToObject(argsObject, "module",
                                      cJSON_CreateString("t2ia_sketch_pidi")); // 预处理器名字
                bad_ = 0;                                                      // 结束跳转
            }
            // if(CorS == 0) {
            //     cJSON_AddItemToObject(argsObject, "module",
            //                           cJSON_CreateString("canny")); // 生成线稿
            // } else if(CorS == 1) {
            //     cJSON_AddItemToObject(argsObject, "module",
            //                           cJSON_CreateString("invert (from white bg & black line)")); // 线稿生图
            // }
            cJSON_AddItemToObject(argsObject, "weight", cJSON_CreateNumber(1));                      // 权重
            cJSON_AddItemToObject(argsObject, "resize_mode", cJSON_CreateString("Crop and Resize")); // 裁剪模式
            // cJSON_AddItemToObject(argsObject, "mask", cJSON_CreateNull());                           // 蒙版
            cJSON_AddItemToObject(argsObject, "invert_image", cJSON_CreateFalse()); // 是否翻转

            cJSON_AddItemToObject(argsObject, "rgbbgr_mode", cJSON_CreateFalse());       // 色彩通道
            cJSON_AddItemToObject(argsObject, "lowvram", cJSON_CreateFalse());           // 是否为低显存
            cJSON_AddItemToObject(argsObject, "processor_res", cJSON_CreateNumber(512)); // 处理器分辨率
            cJSON_AddItemToObject(argsObject, "threshold_a", cJSON_CreateNumber(100));   // 阈值a
            cJSON_AddItemToObject(argsObject, "threshold_b", cJSON_CreateNumber(200));   // 阈值b
            cJSON_AddItemToObject(argsObject, "guidance_start", cJSON_CreateNumber(0));  // 什么时候介入
            cJSON_AddItemToObject(argsObject, "guidance_end", cJSON_CreateNumber(1));    // 什么时候退出
            cJSON_AddItemToObject(argsObject, "pixel_perfect", cJSON_CreateTrue());      // 完美像素
            // cJSON * initImagesArray = cJSON_AddArrayToObject(argsObject, "input_image"); // 向控制网输入图片
            // cJSON_AddItemToArray(initImagesArray, cJSON_CreateString(buffer));  这样会增加中括号 弃用
            cJSON_AddItemToObject(argsObject, "input_image", cJSON_CreateString(buffer)); // 向控制网输入图片
        }
    }

    char * data = cJSON_PrintUnformatted(json);

    cJSON_Delete(json);
    free(buffer); // 释放缓冲区内存

    request = (char *)malloc(MAX_BUFFER_SIZE * sizeof(char));
    if(request == NULL) {
        printf("内存分配失败\n");
    }
    if(t_or_i == 0) {
        sprintf(request, "POST /sdapi/v1/txt2img HTTP/1.1\r\n");
        printf("\ntxt2img\n");
    } else {
        sprintf(request, "POST /sdapi/v1/img2img HTTP/1.1\r\n");
        printf("\nimg2img\n");
    }
    sprintf(request + strlen(request),
            "Host: %s\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %zu\r\n\r\n"
            "%s\r\n",
            SERVER_IP, strlen(data), data);

    // printf("\n发送的数据\n%s\n", request);

    if(send(sockfd_value, request, strlen(request), 0) < 0) {
        printf("Send:\t\tFailed\n");
        exit(EXIT_FAILURE);
    } else {
        printf("\nSend:\t\tOK\n");
    }

    // 打开文件以写入（如果文件不存在则创建，如果文件存在则覆盖）  保存JSON
    FILE * jsonfile = fopen("Send.json", "w");
    if(jsonfile == NULL) {
        fprintf(stderr, "无法创建/覆盖文件 Send.json\n");
        return 1;
    }
    // 将数据写入文件
    fprintf(jsonfile, "%s", data);
    fclose(jsonfile);
    printf("数据已写入 Send.json 文件\n");

    free(data);
    send_end_1 = 1; // 开启发送结束标志位，会在JSON融合中执行初始化
    // free(send_str);
    // send_str = NULL;
    // send_str   = (char *)realloc(send_str, 0); // 清空内容
    //  free(send_str); // 字符串组合 下次还要用 需要指回NULL
    //  send_str = NULL;
    //  重新分配内存
    //  send_str = (char *)realloc(send_str, 0);

    printf("send_request:\tClose\n");
    pthread_exit(NULL);
}
void * receive_response(void * sockfd) // 接收线程
{
    int sockfd_value = *((int *)sockfd);
    response_buffer  = (char *)malloc(MAX_BUFFER_SIZE * sizeof(char));
    if(response_buffer == NULL) {
        printf("内存分配失败\n");
    }
    // 接收服务器返回的数据
    memset(response_buffer, 0, MAX_BUFFER_SIZE);
    int total_bytes_received = 0;
    int bytes_received;
    while((bytes_received = recv(sockfd_value, response_buffer + total_bytes_received,
                                 MAX_BUFFER_SIZE - total_bytes_received - 1, 0)) > 0) {
        total_bytes_received += bytes_received;
    }
    // printf("%s\n", response_buffer);
    // 提取"images"
    char * key       = "\"images\"";
    char * value     = NULL;
    char * key_start = strstr(response_buffer, key);
    if(key_start != NULL) {
        // 找到 "images" 键后，查找开始
        char * quote_start = strstr(key_start, "[\"");
        if(quote_start != NULL) {
            // 查找结尾
            value            = quote_start + 2;
            char * quote_end = strstr(value, "\"]");
            if(quote_end != NULL) {
                // 如果找到了双引号的结束，截取值并打印
                *quote_end = '\0';
                // printf("\n提取到的数据\n\"Images\": %s\n\n\n\n", value);
            }
        }
    }
    // 将BASE64数据保存到文件
    FILE * tempFile = fopen(SERVER_BASE_FILE_NAME, "w");
    if(tempFile != NULL) {
        fputs(value, tempFile);
        fclose(tempFile);
        // 解码BASE64数据
        decodeBase64(SERVER_BASE_FILE_NAME, SERVER_PNG_FILE_NAME);
        // 删除临时文件
        remove(SERVER_BASE_FILE_NAME);
    } else {
        printf("Error opening temporary file.\n");
    }
    // close(sockfd);
    printf("pipe write\n");
    int data = 123;
    write(pipefd[1], &data, sizeof(int)); // 只是说一声结束了
    printf("receive_response:\tClose\n");
    pthread_exit(NULL);
}
void read_bmp_pixels(const char * filename, size_t cbuf_size) // 读取bmp到画布
{
    FILE * bmpFile = fopen(filename, "rb");
    if(bmpFile == NULL) {
        perror("Failed to open BMP file");
        exit(EXIT_FAILURE);
    }

    // 跳过 BMP 文件头和信息头
    fseek(bmpFile, 138, SEEK_SET);

    // 从底部开始逐行读取颜色数据并写入颜色缓冲区
    for(int32_t y = SKETCHPAD_DEFAULT_HEIGHT - 1; y >= 0; y--) {
        for(uint32_t x = 0; x < SKETCHPAD_DEFAULT_WIDTH; x++) {
            lv_color_t color;
            color.ch.blue                         = fgetc(bmpFile);
            color.ch.green                        = fgetc(bmpFile);
            color.ch.red                          = fgetc(bmpFile);
            color.ch.alpha                        = LV_OPA_COVER;
            cbuf[y * SKETCHPAD_DEFAULT_WIDTH + x] = color;
        }
        // BMP 文件中每一行的字节数必须是4的倍数，因此需要跳过可能存在的填充字节
        fseek(bmpFile, (4 - (SKETCHPAD_DEFAULT_WIDTH * 3) % 4) % 4, SEEK_CUR);
    }

    printf("BMP TO COLOR BUFFER:\tOK\n");

    fclose(bmpFile);
}
void * send_to_server(void)
{
    // 创建管道
    if(pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    // 设置管道读取端为非阻塞
    int flags = fcntl(pipefd[0], F_GETFL, 0);
    fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);

    // 创建套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        printf("Socket creation failed\n");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址信息
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(SERVER_PORT);

    if(inet_pton(AF_INET, SERVER_IP, &(server_addr.sin_addr)) <= 0) {
        printf("Invalid address/ Address not supported\n");
        exit(EXIT_FAILURE);
    }
    // 连接服务器
    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connection failed\n");
        exit(EXIT_FAILURE);
    }

    pthread_t send_thread, receive_thread;
    // 创建发送请求线程
    if(pthread_create(&send_thread, NULL, send_request, (void *)&sockfd) < 0) {
        printf("Could not create send thread\n");
        exit(EXIT_FAILURE);
    }

    // 创建接收响应线程
    if(pthread_create(&receive_thread, NULL, receive_response, (void *)&sockfd) < 0) {
        printf("Could not create receive thread\n");
        exit(EXIT_FAILURE);
    }
    // create_progress(lv_scr_act());   // 进度条
    //  等待两个线程执行完毕
    // if(pthread_join(send_thread, NULL) < 0) {
    //     printf("Send thread join failed\n");
    //     exit(EXIT_FAILURE);
    // }
    // if(pthread_join(receive_thread, NULL) < 0) {
    //     printf("Receive thread join failed\n");
    //     exit(EXIT_FAILURE);
    // }
    pthread_join(send_thread, NULL); // 阻塞等待发送端结束
    // create_progress(lv_scr_act());   // 进度条
    // 主线程从管道读取数据
    int received_data;
    while(read(pipefd[0], &received_data, sizeof(int)) <= 0)
        ;
    printf("rec_EXIT\n");
    // lv_bar_set_value(bar, 100, LV_ANIM_ON);

    // 等待子线程结束
    pthread_join(receive_thread, NULL);

    png_check();
    decodePng(SERVER_PNG_FILE_NAME, SERVER_BMP_FILE_NAME); // png转bmp
    read_bmp_pixels(SERVER_BMP_FILE_NAME,
                    LV_CANVAS_BUF_SIZE_TRUE_COLOR(SKETCHPAD_DEFAULT_WIDTH, SKETCHPAD_DEFAULT_HEIGHT));

    // 刷新画布
    lv_obj_invalidate(sketchpad);
    lv_task_handler();

    free(request);
    free(response_buffer);
    close(sockfd);

    close(pipefd[0]);                                       // 关闭管道的读端
    close(pipefd[1]);                                       // 关闭管道的写端
    lv_obj_set_style_anim_time(bar, 500, LV_STATE_DEFAULT); // 最后一段动快点
    lv_bar_set_value(bar, 100, LV_ANIM_ON);
    lv_obj_set_style_anim_time(bar, 3000, LV_STATE_DEFAULT);
    // lv_bar_set_value(bar, 0, LV_ANIM_OFF);
}
