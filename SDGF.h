/*
Simple dingux game framework license

Copyright (C) 2015-2019 Popov Evgeniy Alekseyevich

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

Third�party code license

Pixel packing algorithm bases on code from SVGALib. SVGALib is public domain.
SVGALib homepage: http://www.svgalib.org/
*/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <linux/fb.h>

#define BUTTON_NONE 0
#define BUTTON_UP KEY_UP
#define BUTTON_DOWN KEY_DOWN
#define BUTTON_LEFT KEY_LEFT
#define BUTTON_RIGHT KEY_RIGHT
#define BUTTON_A KEY_LEFTCTRL
#define BUTTON_B KEY_LEFTALT
#define BUTTON_X KEY_SPACE
#define BUTTON_Y KEY_LEFTSHIFT
#define BUTTON_L KEY_TAB
#define BUTTON_R KEY_BACKSPACE
#define BUTTON_START KEY_ENTER
#define BUTTON_SELECT KEY_ESC
#define BUTTON_POWER KEY_POWER
#define BUTTON_HOLD KEY_HOLD
#define GAMEPAD_HOLDING 2
#define GAMEPAD_PRESS 1
#define GAMEPAD_RELEASE 0

enum MIRROR_TYPE {MIRROR_HORIZONTAL=0,MIRROR_VERTICAL=1};
enum BACKGROUND_TYPE {NORMAL_BACKGROUND=0,HORIZONTAL_BACKGROUND=1,VERTICAL_BACKGROUND=2};
enum SPRITE_TYPE {SINGLE_SPRITE=0,HORIZONTAL_STRIP=1,VERTICAL_STRIP=2};

struct IMG_Pixel
{
 unsigned char blue:8;
 unsigned char green:8;
 unsigned char red:8;
};

struct Key_State
{
 unsigned short int button;
 unsigned char state;
};

struct TGA_head
{
 unsigned char id:8;
 unsigned char color_map:8;
 unsigned char type:8;
};

struct TGA_map
{
 unsigned short int index:16;
 unsigned short int length:16;
 unsigned char map_size:8;
};

struct TGA_image
{
 unsigned short int x:16;
 unsigned short int y:16;
 unsigned short int width:16;
 unsigned short int height:16;
 unsigned char color:8;
 unsigned char alpha:3;
 unsigned char direction:5;
};

struct PCX_head
{
 unsigned char vendor:8;
 unsigned char version:8;
 unsigned char compress:8;
 unsigned char color:8;
 unsigned short int min_x:16;
 unsigned short int min_y:16;
 unsigned short int max_x:16;
 unsigned short int max_y:16;
 unsigned short int vertical_dpi:16;
 unsigned short int horizontal_dpi:16;
 unsigned char palette[48];
 unsigned char reversed:8;
 unsigned char planes:8;
 unsigned short int plane_length:16;
 unsigned short int palette_type:16;
 unsigned short int screen_width:16;
 unsigned short int screen_height:16;
 unsigned char filled[54];
};

struct Collision_Box
{
 unsigned long int x:32;
 unsigned long int y:32;
 unsigned long int width:32;
 unsigned long int height:32;
};

void Show_Error(const char *message);

namespace SDGF
{

class Frame
{
 private:
 size_t pixels;
 size_t length;
 unsigned long int frame_width;
 unsigned long int frame_height;
 unsigned short int *buffer;
 unsigned short int *shadow;
 unsigned short int get_bgr565(const unsigned char red,const unsigned char green,const unsigned char blue);
 size_t get_offset(const unsigned long int x,const unsigned long int y);
 protected:
 void set_size(const unsigned long int surface_width,const unsigned long int surface_height);
 unsigned short int *create_buffer(const char *error);
 void create_buffers();
 unsigned short int *get_buffer();
 size_t get_length();
 public:
 Frame();
 ~Frame();
 void draw_pixel(const unsigned long int x,const unsigned long int y,const unsigned char red,const unsigned char green,const unsigned char blue);
 void clear_screen();
 void save();
 void restore();
 unsigned long int get_width();
 unsigned long int get_height();
};

class FPS
{
 private:
 time_t start;
 unsigned long int current;
 unsigned long int fps;
 protected:
 void update_counter();
 public:
 FPS();
 ~FPS();
 unsigned long int get_fps();
};

class Render:public Frame
{
 private:
 int device;
 unsigned long int start;
 fb_fix_screeninfo configuration;
 fb_var_screeninfo setting;
 void read_configuration();
 unsigned long int get_start_offset();
 protected:
 void refresh();
 public:
 Render();
 ~Render();
 void initialize();
};

class Screen:public Render,public FPS
{
 public:
 Screen();
 ~Screen();
 void update();
 Screen* get_handle();
};

class Gamepad
{
 private:
 int device;
 size_t length;
 input_event input;
 Key_State key;
 public:
 Gamepad();
 ~Gamepad();
 void initialize();
 void update();
 unsigned short int get_hold();
 unsigned short int get_press();
 unsigned short int get_release();
};

class Memory
{
 public:
 unsigned long int get_total_memory();
 unsigned long int get_free_memory();
};

class System
{
 public:
 System();
 ~System();
 unsigned long int get_random(const unsigned long int number);
 void quit();
 void run(const char *command);
 char* read_environment(const char *variable);
 void enable_logging(const char *name);
};

class Binary_File
{
 private:
 FILE *target;
 void open(const char *name,const char *mode);
 public:
 Binary_File();
 ~Binary_File();
 void open_read(const char *name);
 void open_write(const char *name);
 void close();
 void set_position(const long int offset);
 long int get_position();
 long int get_length();
 void read(void *buffer,const size_t length);
 void write(void *buffer,const size_t length);
 bool check_error();
};

class Timer
{
 private:
 unsigned long int interval;
 time_t start;
 public:
 Timer();
 ~Timer();
 void set_timer(const unsigned long int seconds);
 bool check_timer();
};

class Primitive
{
 private:
 IMG_Pixel color;
 Screen *surface;
 public:
 Primitive();
 ~Primitive();
 void initialize(Screen *Screen);
 void set_color(const unsigned char red,const unsigned char green,const unsigned char blue);
 void draw_line(const unsigned long int x1,const unsigned long int y1,const unsigned long int x2,const unsigned long int y2);
 void draw_rectangle(const unsigned long int x,const unsigned long int y,const unsigned long int width,const unsigned long int height);
 void draw_filled_rectangle(const unsigned long int x,const unsigned long int y,const unsigned long int width,const unsigned long int height);
};

class Image
{
 private:
 unsigned long int width;
 unsigned long int height;
 unsigned char *data;
 unsigned char *create_buffer(const size_t length);
 void clear_buffer();
 public:
 Image();
 ~Image();
 void load_tga(const char *name);
 void load_pcx(const char *name);
 unsigned long int get_width();
 unsigned long int get_height();
 size_t get_data_length();
 unsigned char *get_data();
 void destroy_image();
};

class Surface
{
 private:
 Screen *surface;
 protected:
 unsigned long int width;
 unsigned long int height;
 IMG_Pixel *image;
 void save();
 void restore();
 void clear_buffer();
 IMG_Pixel *create_buffer(const unsigned long int image_width,const unsigned long int image_height);
 void set_width(const unsigned long int image_width);
 void set_height(const unsigned long int image_height);
 size_t get_offset(const unsigned long int start,const unsigned long int x,const unsigned long int y);
 void draw_image_pixel(const size_t offset,const unsigned long int x,const unsigned long int y);
 public:
 Surface();
 ~Surface();
 void initialize(Screen *screen);
 size_t get_length();
 IMG_Pixel *get_image();
};

class Canvas:public Surface
{
 private:
 unsigned long int frames;
 public:
 Canvas();
 ~Canvas();
 unsigned long int get_image_width();
 unsigned long int get_image_height();
 void set_frames(const unsigned long int amount);
 unsigned long int get_frames();
 void load_image(Image &buffer);
 void mirror_image(const MIRROR_TYPE kind);
 void resize_image(const unsigned long int new_width,const unsigned long int new_height);
};

class Background:public Canvas
{
 private:
 unsigned long int start;
 unsigned long int background_width;
 unsigned long int background_height;
 unsigned long int frame;
 unsigned long int current;
 BACKGROUND_TYPE current_kind;
 void draw_background_pixel(const unsigned long int x,const unsigned long int y);
 void slow_draw_background();
 public:
 Background();
 ~Background();
 void set_kind(BACKGROUND_TYPE kind);
 void set_target(const unsigned long int target);
 void draw_background();
};

class Sprite:public Canvas
{
 private:
 bool transparent;
 unsigned long int current_x;
 unsigned long int current_y;
 unsigned long int sprite_width;
 unsigned long int sprite_height;
 unsigned long int frame;
 unsigned long int start;
 SPRITE_TYPE current_kind;
 bool compare_pixels(const IMG_Pixel &first,const IMG_Pixel &second);
 void draw_sprite_pixel(const size_t offset,const unsigned long int x,const unsigned long int y);
 public:
 Sprite();
 ~Sprite();
 void set_transparent(const bool enabled);
 bool get_transparent();
 void set_x(const unsigned long int x);
 void set_y(const unsigned long int y);
 unsigned long int get_x();
 unsigned long int get_y();
 unsigned long int get_width();
 unsigned long int get_height();
 Sprite* get_handle();
 Collision_Box get_box();
 void set_kind(const SPRITE_TYPE kind);
 SPRITE_TYPE get_kind();
 void set_target(const unsigned long int target);
 void set_position(const unsigned long int x,const unsigned long int y);
 void clone(Sprite &target);
 void draw_sprite();
 void draw_sprite(const unsigned long int x,const unsigned long int y);
};

class Tileset:public Surface
{
 private:
 unsigned long int offset;
 unsigned long int tile_width;
 unsigned long int tile_height;
 unsigned long int rows;
 unsigned long int columns;
 public:
 Tileset();
 ~Tileset();
 unsigned long int get_tile_width();
 unsigned long int get_tile_height();
 unsigned long int get_rows();
 unsigned long int get_columns();
 void select_tile(const unsigned long int row,const unsigned long int column);
 void draw_tile(const unsigned long int x,const unsigned long int y);
 void load_tileset(Image &buffer,const unsigned long int row_amount,const unsigned long int column_amount);
};

class Text
{
 private:
 unsigned long int current_x;
 unsigned long int current_y;
 unsigned long int step_x;
 Sprite *font;
 void draw_character(const char target);
 public:
 Text();
 ~Text();
 void set_position(const unsigned long int x,const unsigned long int y);
 void load_font(Sprite *target);
 void draw_text(const char *text);
};

class Collision
{
 public:
 Collision_Box generate_box(const unsigned long int x,const unsigned long int y,const unsigned long int width,const unsigned long int height);
 bool check_horizontal_collision(const Collision_Box &first,const Collision_Box &second);
 bool check_vertical_collision(const Collision_Box &first,const Collision_Box &second);
 bool check_collision(const Collision_Box &first,const Collision_Box &second);
};

}