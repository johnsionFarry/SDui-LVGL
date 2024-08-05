#include "tip.h"

extern char like;           // 记录下拉列表设置
extern char like_d2;        // 记录二级下拉菜单的设置
extern char t_or_i;         // 0:t 1:i
extern char CorS;           // 0:canny硬边缘 1:scribble_pidinet涂鸦
extern char opinion;        // 记录复选框的设置，实时
extern char opinion_d2[12]; // 记录所有二级复选框的设置，发送用 randoms[0] cscenery[1-5] figure[6-11]
extern char * send_str;     // 初始json项目为空字符串

extern void appendStringWithComma(char ** str_ptr, const char * content);

const char randoms[][10] = {"wather", "space", "ice", "dack", "wind", "flare"};                      // 魔法
const char cscenery[][10] = {"mountain",  "cliff",   "floating",  "canyon",    "plain",   "glacier", // 避免强制换行
                             "beach",     "river",   "waterfall", "lake",      "geyser",  "bottom", //
                             "Cyberpunk", "lights",  "fences",    "cityscape", "park",    "ground", //
                             "day",       "night",   "rain",      "star",      "dusk",    "cloudy", //
                             "25xxxxx",   "26xxxxx", "27xxxxx",   "28xxxxx",   "29xxxxx", "30xxxxx"};
const char figure[][10] = {
    "long",     "short",    "twintails", "ahoge",    "hair bun", "side ponytail", //
    "coat",     "dresst",   "angel",     "sailo",    "pant",     "checkered",     //
    "13xxxxxx", "14xxxxxx", "15xxxxxx",  "16xxxxxx", "17xxxxxx", "18xxxxxx",      //
    "19xxxxxx", "20xxxxxx", "21xxxxxx",  "22xxxxxx", "23xxxxxx", "24xxxxxx",      //
    "25xxxxxx", "26xxxxxx", "27xxxxxx",  "28xxxxxx", "29xxxxxx", "30xxxxxx",      //
    "31xxxxxx", "32xxxxxx", "33xxxxxx",  "34xxxxxx", "35xxxxxx", "36xxxxxx",
};
//------------------------------------------------random------------------------------------------------
const char nenegative_prompt[] = {
    "(worst quality:2), (low quality:2), (normal quality:2), lowres, normal quality, ((monochrome)), ((grayscale)), "
    "skin spots, acnes, skin blemishes, age spot, (ugly:1.331), (duplicate:1.331), (morbid:1.21), (mutilated:1.21), "
    "(tranny:1.331), mutated hands, (poorly drawn hands:1.5), blurry, (bad anatomy:1.21), (bad proportions:1.331), "
    "extra limbs, (disfigured:1.331), (missing arms:1.331), (extra legs:1.331), (fused fingers:1.61051), (too many "
    "fingers:1.61051), (unclear eyes:1.331), lowers, bad hands, missing fingers, extra digit,bad hands, missing "
    "fingers, (((extra arms and legs)))"};
const char water[] = {
    "(masterpiece:1.21), (best quality:1.331), (ultra-detailed:1.21), (illustration:1.21), (disheveled "
    "hair:1.21), (frills:1.21), (1girl:1.1), (solo:1.1), dynamic angle, big top sleeves, floating, "
    "beautiful detailed sky, beautiful detailed water, beautiful detailed eyes, overexposure, (fist:1.1), "
    "expressionless, side blunt bangs, hair between eyes, ribbons, bowties, buttons, bare shoulders, "
    "(small breast:1.331), detailed wet clothes, blank stare, pleated skirt, flowers"};
const char space[] = {
    "illustration, floating hair, chromatic aberration, caustics, lens flare, dynamic angle, portrait, 1girl, solo, "
    "cute face, hidden hands, asymmetrical bangs, beautiful detailed eyes, eye shadow, huge clocks, glass strips, "
    "floating glass fragments, colorful refraction, beautiful detailed sky, dark intense shadows, cinematic lighting, "
    "overexposure, expressionless, blank stare, big top sleeves, frills, hair ornament, ribbon, bowtie, buttons, small "
    "breasts, pleated skirt, sharp focus, masterpiece, best quality, extremely detailed, colorful, hdr"};
const char ice[] = {
    "masterpiece, best quality, illustration, beautiful detailed girl, beautiful detailed glow, detailed "
    "ice, beautiful detailed water, beautiful detailed eyes, expressionless, floating palaces, azure hair, "
    "disheveled hair, long bangs, hairs between eyes, skyblue dress, black ribbon, white bowties, midriff, "
    "half closed eyes, big forhead, blank stare, flower, large top sleeves"};
const char dack[] = {
    "masterpiece, best quality, illustration, nuclear explosion behide, flames of war, beautiful detailed "
    "girl, beautiful detailed glow, rain, detailed lighting, detailed water, beautiful detailed eyes, "
    "expressionless, palace, azure hair, disheveled hair, long bangs, hair between eyes, whitegrey dress, "
    "black ribbon, white bowties, midriff, big forhead, blank stare, flower, long sleeves"};
const char wind[] = {
    "masterpiece, best quality, beautiful detailed sky, illustration, 1girl, solo, small breasts, "
    "ultra-detailed, an extremely delicate and beautiful little girl, beautiful detailed eyes, side blunt "
    "bangs, hair between eyes, ribbon, bowtie, buttons, bare shoulders, blank stare, pleated skirt, close "
    "to viewer, breeze, flying splashes, flying petals, wind"};
const char flare[] = {
    "1girl, masterpiece, best quality, illustration, watercolor, sleepy, sunset, looking at viewer, upper "
    "body, bare shoulders, cinematic lighting, surrounded by heavy floating sand flow and floating sharp "
    "stones, detailed beautiful desert with cactus, extremely detailed eyes and face, ink, depth of field, "
    "extremely detailed, anime face, dramatic angle, medium breasts, 8k wallpaper, bright eyes, an "
    "detailed organdie dress, very close to viewers, messy long hair, veil, focus on face, golden "
    "bracelet, long yarn, lens flare, light leaks, medium wind, detailed beautiful sky"};
//------------------------------------------------soil------------------------------------------------
const char mountain[] = {"mountain"};
const char cliff[]    = {"cliff"};
const char floating[] = {"floating island"};
const char canyon[]   = {"canyon"};
const char plain[]    = {"plain"};
const char glacier[]  = {"geyser"};
//------------------------------------------------water------------------------------------------------
const char beach[]     = {"beach"};
const char river[]     = {"river"};
const char waterfall[] = {"waterfall"};
const char lake[]      = {"lake"};
const char geyser[]    = {"geyser"};
const char bottom[]    = {"ocean bottom"};
//------------------------------------------------city------------------------------------------------
const char Cyberpunk[]  = {"Cyberpunk"};
const char lights[]     = {"neon lights"};
const char fences[]     = {"fences"};
const char skyscraper[] = {"skyscraper"};
const char park[]       = {"park"};
const char ground[]     = {"play ground"};
//------------------------------------------------sky------------------------------------------------
const char day[]    = {"day"};
const char night[]  = {"night"};
const char rain[]   = {"rain"};
const char star[]   = {"shooting star"};
const char dusk[]   = {"dusk"};
const char cloudy[] = {"cloudy"};
//------------------------------------------------hair------------------------------------------------
const char longhair[]  = {"long hair"};
const char shorthair[] = {"short hair"};
const char twintails[] = {"twintails"};
const char ahoge[]     = {"ahoge"};
const char hairbun[]   = {"hair bun"};
const char sidepony[]  = {"side ponytail"};
//------------------------------------------------body------------------------------------------------
const char coat[]      = {"coat"};
const char dress[]     = {"dress shirt"};
const char angel[]     = {"angel wing"};
const char sailor[]    = {"sailor shirt "};
const char pant[]      = {"pant suit "};
const char checkered[] = {"checkered shirt"};

void prompt()
{
    if(read_bit_char(&opinion_d2[0], 0)) {
        appendStringWithComma(&send_str, water);
    }
    if(read_bit_char(&opinion_d2[0], 1)) {
        appendStringWithComma(&send_str, space);
    }
    if(read_bit_char(&opinion_d2[0], 2)) {
        appendStringWithComma(&send_str, ice);
    }
    if(read_bit_char(&opinion_d2[0], 3)) {
        appendStringWithComma(&send_str, dack);
    }
    if(read_bit_char(&opinion_d2[0], 4)) {
        appendStringWithComma(&send_str, wind);
    }
    if(read_bit_char(&opinion_d2[0], 5)) {
        appendStringWithComma(&send_str, flare);
    }
    if(read_bit_char(&opinion_d2[1], 0)) {
        appendStringWithComma(&send_str, mountain);
    }
    if(read_bit_char(&opinion_d2[1], 1)) {
        appendStringWithComma(&send_str, cliff);
    }
    if(read_bit_char(&opinion_d2[1], 2)) {
        appendStringWithComma(&send_str, floating);
    }
    if(read_bit_char(&opinion_d2[1], 3)) {
        appendStringWithComma(&send_str, canyon);
    }
    if(read_bit_char(&opinion_d2[1], 4)) {
        appendStringWithComma(&send_str, plain);
    }
    if(read_bit_char(&opinion_d2[1], 5)) {
        appendStringWithComma(&send_str, glacier);
    }
    if(read_bit_char(&opinion_d2[2], 0)) {
        appendStringWithComma(&send_str, beach);
    }
    if(read_bit_char(&opinion_d2[2], 1)) {
        appendStringWithComma(&send_str, river);
    }
    if(read_bit_char(&opinion_d2[2], 2)) {
        appendStringWithComma(&send_str, waterfall);
    }
    if(read_bit_char(&opinion_d2[2], 3)) {
        appendStringWithComma(&send_str, lake);
    }
    if(read_bit_char(&opinion_d2[2], 4)) {
        appendStringWithComma(&send_str, geyser);
    }
    if(read_bit_char(&opinion_d2[2], 5)) {
        appendStringWithComma(&send_str, bottom);
    }
    if(read_bit_char(&opinion_d2[3], 0)) {
        appendStringWithComma(&send_str, Cyberpunk);
    }
    if(read_bit_char(&opinion_d2[3], 1)) {
        appendStringWithComma(&send_str, lights);
    }
    if(read_bit_char(&opinion_d2[3], 2)) {
        appendStringWithComma(&send_str, fences);
    }
    if(read_bit_char(&opinion_d2[3], 3)) {
        appendStringWithComma(&send_str, skyscraper);
    }
    if(read_bit_char(&opinion_d2[3], 4)) {
        appendStringWithComma(&send_str, park);
    }
    if(read_bit_char(&opinion_d2[3], 5)) {
        appendStringWithComma(&send_str, ground);
    }
    if(read_bit_char(&opinion_d2[4], 0)) {
        appendStringWithComma(&send_str, day);
    }
    if(read_bit_char(&opinion_d2[4], 1)) {
        appendStringWithComma(&send_str, night);
    }
    if(read_bit_char(&opinion_d2[4], 2)) {
        appendStringWithComma(&send_str, rain);
    }
    if(read_bit_char(&opinion_d2[4], 3)) {
        appendStringWithComma(&send_str, star);
    }
    if(read_bit_char(&opinion_d2[4], 4)) {
        appendStringWithComma(&send_str, dusk);
    }
    if(read_bit_char(&opinion_d2[4], 5)) {
        appendStringWithComma(&send_str, cloudy);
    }
    //----------------------例子没有写
    if(read_bit_char(&opinion_d2[6], 0)) {
        appendStringWithComma(&send_str, longhair);
    }
    if(read_bit_char(&opinion_d2[6], 1)) {
        appendStringWithComma(&send_str, shorthair);
    }
    if(read_bit_char(&opinion_d2[6], 2)) {
        appendStringWithComma(&send_str, twintails);
    }
    if(read_bit_char(&opinion_d2[6], 3)) {
        appendStringWithComma(&send_str, ahoge);
    }
    if(read_bit_char(&opinion_d2[6], 4)) {
        appendStringWithComma(&send_str, hairbun);
    }
    if(read_bit_char(&opinion_d2[6], 5)) {
        appendStringWithComma(&send_str, sidepony);
    }
    if(read_bit_char(&opinion_d2[7], 0)) {
        appendStringWithComma(&send_str, coat);
    }
    if(read_bit_char(&opinion_d2[7], 1)) {
        appendStringWithComma(&send_str, dress);
    }
    if(read_bit_char(&opinion_d2[7], 2)) {
        appendStringWithComma(&send_str, angel);
    }
    if(read_bit_char(&opinion_d2[7], 3)) {
        appendStringWithComma(&send_str, sailor);
    }
    if(read_bit_char(&opinion_d2[7], 4)) {
        appendStringWithComma(&send_str, pant);
    }
    if(read_bit_char(&opinion_d2[7], 5)) {
        appendStringWithComma(&send_str, checkered);
    }
}

// 以下为早期版本
/*else if(like == 2) {
        if(read_bit_char(&opinion, 3) && read_bit_char(&opinion, 4)) {
            appendStringWithComma(&send_str, "2girls");
        } else if(read_bit_char(&opinion, 2) && read_bit_char(&opinion, 4)) {
            appendStringWithComma(&send_str, "1girl");
        } else if(read_bit_char(&opinion, 3) && read_bit_char(&opinion, 5)) {
            appendStringWithComma(&send_str, "2boys");
        } else if(read_bit_char(&opinion, 2) && read_bit_char(&opinion, 5)) {
            appendStringWithComma(&send_str, "1boy");
        }
        //--------------------以上两两互斥-----------------
        if(read_bit_char(&opinion, 1) && read_bit_char(&opinion, 4)) {
            appendStringWithComma(&send_str, "open clothes, breasts, pussy juice"); // 瑟瑟打开 且为女
        } else if(read_bit_char(&opinion, 0) && read_bit_char(&opinion, 4)) {
            appendStringWithComma(&send_str,
                                  "yellow hair,green eyes, white dress, day, beach, smile"); // 瑟瑟关闭 且为女
        }
    }*/
// if(opinion == 0) {
//     cJSON_AddItemToObject(json, "prompt", cJSON_CreateString("")); // 如果所有选项都没有勾选，则填充空
// } else {                                                           // 否则填入处理过的字段
//     cJSON_AddItemToObject(json, "prompt", cJSON_CreateString(send_str));
// }