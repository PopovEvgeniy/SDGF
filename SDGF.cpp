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

#include "SDGF.h"

void Show_Error(const char *message)
{
 puts(message);
 exit(EXIT_FAILURE);
}

namespace SDGF
{

Frame::Frame()
{
 frame_width=0;
 frame_height=0;
 pixels=0;
 length=0;
 buffer=NULL;
 shadow=NULL;
}

Frame::~Frame()
{
 if(buffer!=NULL)
 {
  free(buffer);
  buffer=NULL;
 }
 if(shadow!=NULL)
 {
  free(shadow);
  shadow=NULL;
 }

}

unsigned short int Frame::get_bgr565(const unsigned char red,const unsigned char green,const unsigned char blue)
{
 return (blue >> 3) +((green >> 2) << 5)+((red >> 3) << 11); // This code bases on code from SVGALib
}

size_t Frame::get_offset(const unsigned long int x,const unsigned long int y)
{
 return (size_t)x+(size_t)y*(size_t)frame_width;
}

void Frame::set_size(const unsigned long int surface_width,const unsigned long int surface_height)
{
 frame_width=surface_width;
 frame_height=surface_height;
}

unsigned short int *Frame::create_buffer(const char *error)
{
 unsigned short int *target;
 pixels=(size_t)frame_width*(size_t)frame_height;
 target=(unsigned short int*)calloc(pixels,sizeof(unsigned short int));
 length=pixels*sizeof(unsigned short int);
 if(target==NULL)
 {
  Show_Error(error);
 }
 return target;
}

void Frame::create_buffers()
{
 buffer=this->create_buffer("Can't allocate memory for render buffer");
 shadow=this->create_buffer("Can't allocate memory for shadow buffer");
}

unsigned short int *Frame::get_buffer()
{
 return buffer;
}

size_t Frame::get_length()
{
 return length;
}

void Frame::draw_pixel(const unsigned long int x,const unsigned long int y,const unsigned char red,const unsigned char green,const unsigned char blue)
{
 if((x<frame_width)&&(y<frame_height))
 {
  buffer[this->get_offset(x,y)]=this->get_bgr565(red,green,blue);
 }

}

void Frame::clear_screen()
{
 size_t index;
 for (index=0;index<pixels;++index)
 {
  buffer[index]=0;
 }

}

void Frame::save()
{
 size_t index;
 for (index=0;index<pixels;++index)
 {
  shadow[index]=buffer[index];
 }

}

void Frame::restore()
{
 size_t index;
 for (index=0;index<pixels;++index)
 {
  buffer[index]=shadow[index];
 }

}

unsigned long int Frame::get_width()
{
 return frame_width;
}

unsigned long int Frame::get_height()
{
 return frame_height;
}

FPS::FPS()
{
 start=time(NULL);
 current=0;
 fps=0;
}

FPS::~FPS()
{

}

void FPS::update_counter()
{
 time_t stop;
 if(current==0) start=time(NULL);
 ++current;
 stop=time(NULL);
 if(difftime(stop,start)>=1)
 {
  fps=current;
  current=0;
 }

}

unsigned long int FPS::get_fps()
{
 return fps;
}

Render::Render()
{
 device=open("/dev/fb0",O_RDWR);
 if(device==-1)
 {
  Show_Error("Can't get access to frame buffer");
 }
 memset(&setting,0,sizeof(FBIOGET_VSCREENINFO));
 memset(&configuration,0,sizeof(FBIOGET_FSCREENINFO));
}

Render::~Render()
{
 if(device!=-1) close(device);
}

void Render::read_configuration()
{
 if(ioctl(device,FBIOGET_VSCREENINFO,&setting)==-1)
 {
  Show_Error("Can't read framebuffer setting");
 }
 if(ioctl(device,FBIOGET_FSCREENINFO,&configuration)==-1)
 {
  Show_Error("Can't read framebuffer setting");
 }

}

unsigned long int Render::get_start_offset()
{
 return setting.xoffset*(setting.bits_per_pixel/8)+setting.yoffset*configuration.line_length;
}

void Render::refresh()
{
 lseek(device,start,SEEK_SET);
 write(device,this->get_buffer(),this->get_length());
}

void Render::initialize()
{
 this->read_configuration();
 this->set_size(setting.xres,setting.yres);
 this->create_buffers();
 start=this->get_start_offset();
}

Screen::Screen()
{

}

Screen::~Screen()
{

}

void Screen::update()
{
 this->refresh();
 this->update_counter();
}

Screen* Screen::get_handle()
{
 return this;
}

Gamepad::Gamepad()
{
 length=sizeof(input_event);
 memset(&input,0,length);
}

Gamepad::~Gamepad()
{
 if(device!=-1) close(device);
}

void Gamepad::initialize()
{
 device=open("/dev/event0",O_RDONLY|O_NONBLOCK|O_NOCTTY);
 if (device==-1)
 {
  Show_Error("Can't get access to gamepad");
 }
 key.button=0;
 key.state=GAMEPAD_RELEASE;
}

void Gamepad::update()
{
 key.button=0;
 key.state=GAMEPAD_RELEASE;
 while (read(device,&input,length)>0)
 {
  if (input.type==EV_KEY)
  {
   key.button=input.code;
   key.state=input.value;
   if(key.state!=GAMEPAD_HOLDING) break;
  }

 }

}

unsigned short int Gamepad::get_hold()
{
 unsigned short int result;
 result=BUTTON_NONE;
 if(key.state!=GAMEPAD_RELEASE) result=key.button;
 return result;
}

unsigned short int Gamepad::get_press()
{
 unsigned short int result;
 result=BUTTON_NONE;
 if(key.state==GAMEPAD_PRESS) result=key.button;
 return result;
}

unsigned short int Gamepad::get_release()
{
 unsigned short int result;
 result=BUTTON_NONE;
 if(key.state==GAMEPAD_RELEASE) result=key.button;
 return result;
}

unsigned long int Memory::get_total_memory()
{
 unsigned long int memory;
 struct sysinfo information;
 memory=0;
 if (sysinfo(&information)==0) memory=information.totalram*information.mem_unit;
 return memory;
}

unsigned long int Memory::get_free_memory()
{
 unsigned long int memory;
 struct sysinfo information;
 memory=0;
 if (sysinfo(&information)==0) memory=information.freeram*information.mem_unit;
 return memory;
}

System::System()
{
 srand(time(NULL));
}

System::~System()
{

}

unsigned long int System::get_random(const unsigned long int number)
{
 return rand()%number;
}

void System::quit()
{
 exit(EXIT_SUCCESS);
}

void System::run(const char *command)
{
 system(command);
}

char* System::read_environment(const char *variable)
{
 return getenv(variable);
}

void System::enable_logging(const char *name)
{
 if(freopen(name,"wt",stdout)==NULL)
 {
  Show_Error("Can't create log file");
 }

}

Binary_File::Binary_File()
{
 target=NULL;
}

Binary_File::~Binary_File()
{
 if(target!=NULL)
 {
  fclose(target);
  target=NULL;
 }

}

void Binary_File::open(const char *name,const char *mode)
{
 target=fopen(name,mode);
 if(target==NULL)
 {
  Show_Error("Can't open the binary file");
 }

}

void Binary_File::open_read(const char *name)
{
 this->open(name,"rb");
}

void Binary_File::open_write(const char *name)
{
 this->open(name,"w+b");
}

void Binary_File::close()
{
 if(target!=NULL)
 {
  fclose(target);
  target=NULL;
 }

}

void Binary_File::set_position(const long int offset)
{
 fseek(target,offset,SEEK_SET);
}

long int Binary_File::get_position()
{
 return ftell(target);
}

long int Binary_File::get_length()
{
 long int result;
 fseek(target,0,SEEK_END);
 result=ftell(target);
 rewind(target);
 return result;
}

void Binary_File::read(void *buffer,const size_t length)
{
 fread(buffer,sizeof(char),length,target);
}

void Binary_File::write(void *buffer,const size_t length)
{
 fwrite(buffer,sizeof(char),length,target);
}

bool Binary_File::check_error()
{
 bool result;
 result=false;
 if(ferror(target)!=0) result=true;
 return result;
}

Timer::Timer()
{
 interval=0;
 start=time(NULL);
}

Timer::~Timer()
{

}

void Timer::set_timer(const unsigned long int seconds)
{
 interval=seconds;
 start=time(NULL);
}

bool Timer::check_timer()
{
 bool result;
 time_t stop;
 result=false;
 stop=time(NULL);
 if(difftime(stop,start)>=interval)
 {
  result=true;
  start=time(NULL);
 }
 return result;
}

Primitive::Primitive()
{
 surface=NULL;
 color.red=0;
 color.green=0;
 color.blue=0;
}

Primitive::~Primitive()
{
 color.red=0;
 color.green=0;
 color.blue=0;
 surface=NULL;
}

void Primitive::initialize(Screen *Screen)
{
 surface=Screen;
}

void Primitive::set_color(const unsigned char red,const unsigned char green,const unsigned char blue)
{
 color.red=red;
 color.green=green;
 color.blue=blue;
}

void Primitive::draw_line(const unsigned long int x1,const unsigned long int y1,const unsigned long int x2,const unsigned long int y2)
{
 unsigned long int delta_x,delta_y,index,steps;
 float x,y,shift_x,shift_y;
 if (x1>x2)
 {
  delta_x=x1-x2;
 }
 else
 {
  delta_x=x2-x1;
 }
 if (y1>y2)
 {
  delta_y=y1-y2;
 }
 else
 {
  delta_y=y2-y1;
 }
 steps=delta_x;
 if (steps<delta_y) steps=delta_y;
 x=x1;
 y=y1;
 shift_x=(float)delta_x/(float)steps;
 shift_y=(float)delta_y/(float)steps;
 for (index=steps;index>0;--index)
 {
  x+=shift_x;
  y+=shift_y;
  surface->draw_pixel(x,y,color.red,color.green,color.blue);
 }

}

void Primitive::draw_rectangle(const unsigned long int x,const unsigned long int y,const unsigned long int width,const unsigned long int height)
{
 unsigned long int stop_x,stop_y;
 stop_x=x+width;
 stop_y=y+height;
 this->draw_line(x,y,stop_x,y);
 this->draw_line(x,stop_y,stop_x,stop_y);
 this->draw_line(x,y,x,stop_y);
 this->draw_line(stop_x,y,stop_x,stop_y);
}

void Primitive::draw_filled_rectangle(const unsigned long int x,const unsigned long int y,const unsigned long int width,const unsigned long int height)
{
 unsigned long int step_x,step_y,stop_x,stop_y;
 stop_x=x+width;
 stop_y=y+height;
 for(step_x=x;step_x<stop_x;++step_x)
 {
  for(step_y=y;step_y<stop_y;++step_y)
  {
   surface->draw_pixel(step_x,step_y,color.red,color.green,color.blue);
  }

 }

}

Image::Image()
{
 width=0;
 height=0;
 data=NULL;
}

Image::~Image()
{
 if(data!=NULL) free(data);
}

unsigned char *Image::create_buffer(const size_t length)
{
 unsigned char *result;
 result=(unsigned char*)calloc(length,sizeof(unsigned char));
 if(result==NULL)
 {
  Show_Error("Can't allocate memory for image buffer");
 }
 return result;
}

void Image::clear_buffer()
{
 if(data!=NULL)
 {
  free(data);
  data=NULL;
 }

}

FILE *Image::open_image(const char *name)
{
 FILE *target;
 target=fopen(name,"rb");
 if(target==NULL)
 {
  Show_Error("Can't open a image file");
 }
 return target;
}

unsigned long int Image::get_file_size(FILE *target)
{
 unsigned long int length;
 fseek(target,0,SEEK_END);
 length=ftell(target);
 rewind(target);
 return length;
}

void Image::load_tga(const char *name)
{
 FILE *target;
 size_t index,position,amount,compressed_length,uncompressed_length;
 unsigned char *compressed;
 unsigned char *uncompressed;
 TGA_head head;
 TGA_map color_map;
 TGA_image image;
 this->clear_buffer();
 target=this->open_image(name);
 compressed_length=(size_t)this->get_file_size(target)-18;
 fread(&head,3,1,target);
 fread(&color_map,5,1,target);
 fread(&image,10,1,target);
 if((head.color_map!=0)||(image.color!=24))
 {
  Show_Error("Invalid image format");
 }
 if(head.type!=2)
 {
  if(head.type!=10)
  {
   Show_Error("Invalid image format");
  }

 }
 index=0;
 position=0;
 width=image.width;
 height=image.height;
 uncompressed_length=this->get_data_length();
 uncompressed=this->create_buffer(uncompressed_length);
 if(head.type==2)
 {
  fread(uncompressed,uncompressed_length,1,target);
 }
 if(head.type==10)
 {
  compressed=this->create_buffer(compressed_length);
  fread(compressed,compressed_length,1,target);
  while(index<uncompressed_length)
  {
   if(compressed[position]<128)
   {
    amount=compressed[position]+1;
    amount*=3;
    memmove(uncompressed+index,compressed+(position+1),amount);
    index+=amount;
    position+=1+amount;
   }
   else
   {
    for(amount=compressed[position]-127;amount>0;--amount)
    {
     memmove(uncompressed+index,compressed+(position+1),3);
     index+=3;
    }
    position+=4;
   }

  }
  free(compressed);
 }
 fclose(target);
 data=uncompressed;
}

void Image::load_pcx(const char *name)
{
 FILE *target;
 unsigned long int x,y;
 size_t index,position,line,row,length,uncompressed_length;
 unsigned char repeat;
 unsigned char *original;
 unsigned char *uncompressed;
 PCX_head head;
 this->clear_buffer();
 target=this->open_image(name);
 length=(size_t)this->get_file_size(target)-128;
 fread(&head,128,1,target);
 if((head.color*head.planes!=24)&&(head.compress!=1))
 {
  Show_Error("Incorrect image format");
 }
 width=head.max_x-head.min_x+1;
 height=head.max_y-head.min_y+1;
 row=3*(size_t)width;
 line=(size_t)head.planes*(size_t)head.plane_length;
 uncompressed_length=row*height;
 index=0;
 position=0;
 original=this->create_buffer(length);
 uncompressed=this->create_buffer(uncompressed_length);
 fread(original,length,1,target);
 fclose(target);
 while (index<length)
 {
  if (original[index]<192)
  {
   uncompressed[position]=original[index];
   position++;
   index++;
  }
  else
  {
   for (repeat=original[index]-192;repeat>0;--repeat)
   {
    uncompressed[position]=original[index+1];
    position++;
   }
   index+=2;
  }

 }
 free(original);
 original=this->create_buffer(uncompressed_length);
 for(x=0;x<width;++x)
 {
  for(y=0;y<height;++y)
  {
   index=(size_t)x*3+(size_t)y*row;
   position=(size_t)x+(size_t)y*line;
   original[index]=uncompressed[position+2*(size_t)head.plane_length];
   original[index+1]=uncompressed[position+(size_t)head.plane_length];
   original[index+2]=uncompressed[position];
  }

 }
 free(uncompressed);
 data=original;
}

unsigned long int Image::get_width()
{
 return width;
}

unsigned long int Image::get_height()
{
 return height;
}

size_t Image::get_data_length()
{
 return (size_t)width*(size_t)height*3;
}

unsigned char *Image::get_data()
{
 return data;
}

void Image::destroy_image()
{
 width=0;
 height=0;
 this->clear_buffer();
}

Surface::Surface()
{
 width=0;
 height=0;
 image=NULL;
 surface=NULL;
}

Surface::~Surface()
{
 surface=NULL;
 if(image!=NULL) free(image);
}

IMG_Pixel *Surface::create_buffer(const unsigned long int image_width,const unsigned long int image_height)
{
 IMG_Pixel *result;
 size_t length;
 length=(size_t)image_width*(size_t)image_height;
 result=(IMG_Pixel*)calloc(length,3);
 if(result==NULL)
 {
  Show_Error("Can't allocate memory for image buffer");
 }
 return result;
}

void Surface::save()
{
 surface->save();
}

void Surface::restore()
{
 surface->restore();
}

void Surface::clear_buffer()
{
 if(image!=NULL) free(image);
}

void Surface::set_width(const unsigned long int image_width)
{
 width=image_width;
}

void Surface::set_height(const unsigned long int image_height)
{
 height=image_height;
}

size_t Surface::get_offset(const unsigned long int start,const unsigned long int x,const unsigned long int y)
{
 return (size_t)start+(size_t)x+(size_t)y*(size_t)width;
}

void Surface::draw_image_pixel(const size_t offset,const unsigned long int x,const unsigned long int y)
{
 surface->draw_pixel(x,y,image[offset].red,image[offset].green,image[offset].blue);
}

void Surface::initialize(Screen *screen)
{
 surface=screen;
}

size_t Surface::get_length()
{
 return (size_t)width*(size_t)height;
}

IMG_Pixel *Surface::get_image()
{
 return image;
}

Canvas::Canvas()
{
 frames=1;
}

Canvas::~Canvas()
{

}

unsigned long int Canvas::get_image_width()
{
 return width;
}

unsigned long int Canvas::get_image_height()
{
 return height;
}

void Canvas::set_frames(const unsigned long int amount)
{
 if(amount>1) frames=amount;
}

unsigned long int Canvas::get_frames()
{
 return frames;
}

void Canvas::load_image(Image &buffer)
{
 width=buffer.get_width();
 height=buffer.get_height();
 this->clear_buffer();
 image=this->create_buffer(width,height);
 memmove(image,buffer.get_data(),buffer.get_data_length());
 buffer.destroy_image();
}

void Canvas::mirror_image(const MIRROR_TYPE kind)
{
 unsigned long int x,y;
 size_t index,index2;
 IMG_Pixel *mirrored_image;
 mirrored_image=this->create_buffer(width,height);
 if (kind==MIRROR_HORIZONTAL)
 {
  for (x=0;x<width;++x)
  {
   for (y=0;y<height;++y)
   {
    index=this->get_offset(0,x,y);
    index2=this->get_offset(0,(width-x-1),y);
    mirrored_image[index]=image[index2];
   }

  }

 }
 if(kind==MIRROR_VERTICAL)
 {
   for (x=0;x<width;++x)
  {
   for (y=0;y<height;++y)
   {
    index=this->get_offset(0,x,y);
    index2=this->get_offset(0,x,(height-y-1));
    mirrored_image[index]=image[index2];
   }

  }

 }
 free(image);
 image=mirrored_image;
}

void Canvas::resize_image(const unsigned long int new_width,const unsigned long int new_height)
{
 float x_ratio,y_ratio;
 unsigned long int x,y;
 size_t index,index2;
 IMG_Pixel *scaled_image;
 scaled_image=this->create_buffer(new_width,new_height);
 x_ratio=(float)width/(float)new_width;
 y_ratio=(float)height/(float)new_height;
 for (x=0;x<new_width;++x)
 {
  for (y=0;y<new_height;++y)
  {
   index=(size_t)x+(size_t)y*(size_t)new_width;
   index2=(size_t)(x_ratio*(float)x)+(size_t)width*(size_t)(y_ratio*(float)y);
   scaled_image[index]=image[index2];
  }

 }
 free(image);
 image=scaled_image;
 width=new_width;
 height=new_height;
}

Background::Background()
{
 start=0;
 background_width=0;
 background_height=0;
 current=0;
 frame=1;
 current_kind=NORMAL_BACKGROUND;
}

Background::~Background()
{

}

void Background::draw_background_pixel(const unsigned long int x,const unsigned long int y)
{
 size_t offset;
 offset=this->get_offset(start,x,y);
 this->draw_image_pixel(offset,x,y);
}

void Background::slow_draw_background()
{
 unsigned long int x,y;
 for(x=0;x<background_width;++x)
 {
  for(y=0;y<background_height;++y)
  {
   this->draw_background_pixel(x,y);
  }

 }

}

void Background::set_kind(BACKGROUND_TYPE kind)
{
 switch(kind)
 {
  case NORMAL_BACKGROUND:
  background_width=this->get_image_width();
  background_height=this->get_image_height();
  start=0;
  break;
  case HORIZONTAL_BACKGROUND:
  background_width=this->get_image_width()/this->get_frames();
  background_height=this->get_image_height();
  start=(frame-1)*background_width;
  break;
  case VERTICAL_BACKGROUND:
  background_width=this->get_image_width();
  background_height=this->get_image_height()/this->get_frames();
  start=(frame-1)*background_width*background_height;
  break;
 }
 current_kind=kind;
}

void Background::set_target(const unsigned long int target)
{
 if((target>0)&&(target<=this->get_frames()))
 {
  frame=target;
  this->set_kind(current_kind);
 }

}

void Background::draw_background()
{
 if (current!=frame)
 {
  this->slow_draw_background();
  this->save();
  current=frame;
 }
 else
 {
  this->restore();
 }

}

Sprite::Sprite()
{
 transparent=true;
 current_x=0;
 current_y=0;
 sprite_width=0;
 sprite_height=0;
 frame=0;
 start=0;
 current_kind=SINGLE_SPRITE;
}

Sprite::~Sprite()
{

}

bool Sprite::compare_pixels(const IMG_Pixel &first,const IMG_Pixel &second)
{
 bool result;
 result=false;
 if ((first.red!=second.red)||(first.green!=second.green))
 {
  result=true;
 }
 else
 {
  if(first.blue!=second.blue) result=true;
 }
 return result;
}

void Sprite::draw_sprite_pixel(const size_t offset,const unsigned long int x,const unsigned long int y)
{
 if (transparent==true)
 {
  if(this->compare_pixels(image[0],image[offset])==true) this->draw_image_pixel(offset,x,y);
 }
 else
 {
  this->draw_image_pixel(offset,x,y);
 }

}

void Sprite::set_transparent(const bool enabled)
{
 transparent=enabled;
}

bool Sprite::get_transparent()
{
 return transparent;
}

void Sprite::set_x(const unsigned long int x)
{
 current_x=x;
}

void Sprite::set_y(const unsigned long int y)
{
 current_y=y;
}

unsigned long int Sprite::get_x()
{
 return current_x;
}

unsigned long int Sprite::get_y()
{
 return current_y;
}

unsigned long int Sprite::get_width()
{
 return sprite_width;
}

unsigned long int Sprite::get_height()
{
 return sprite_height;
}

Sprite* Sprite::get_handle()
{
 return this;
}

Collision_Box Sprite::get_box()
{
 Collision_Box target;
 target.x=current_x;
 target.y=current_y;
 target.width=sprite_width;
 target.height=sprite_height;
 return target;
}

void Sprite::set_kind(const SPRITE_TYPE kind)
{
 switch(kind)
 {
  case SINGLE_SPRITE:
  sprite_width=this->get_image_width();
  sprite_height=this->get_image_height();
  start=0;
  break;
  case HORIZONTAL_STRIP:
  sprite_width=this->get_image_width()/this->get_frames();
  sprite_height=this->get_image_height();
  start=(frame-1)*sprite_width;
  break;
  case VERTICAL_STRIP:
  sprite_width=this->get_image_width();
  sprite_height=this->get_image_height()/this->get_frames();
  start=(frame-1)*sprite_width*sprite_height;
  break;
 }
 current_kind=kind;
}

SPRITE_TYPE Sprite::get_kind()
{
 return current_kind;
}

void Sprite::set_target(const unsigned long int target)
{
 if((target>0)&&(target<=this->get_frames()))
 {
  frame=target;
  this->set_kind(current_kind);
 }

}

void Sprite::set_position(const unsigned long int x,const unsigned long int y)
{
 current_x=x;
 current_y=y;
}

void Sprite::clone(Sprite &target)
{
 this->set_width(target.get_image_width());
 this->set_height(target.get_image_height());
 this->set_frames(target.get_frames());
 this->set_kind(target.get_kind());
 this->set_transparent(target.get_transparent());
 image=this->create_buffer(target.get_image_width(),target.get_image_width());
 memmove(image,target.get_image(),target.get_length());
}

void Sprite::draw_sprite()
{
 size_t offset;
 unsigned long int sprite_x,sprite_y;
 for(sprite_x=0;sprite_x<sprite_width;++sprite_x)
 {
  for(sprite_y=0;sprite_y<sprite_height;++sprite_y)
  {
   offset=this->get_offset(start,sprite_x,sprite_y);
   this->draw_sprite_pixel(offset,current_x+sprite_x,current_y+sprite_y);
  }

 }

}

Tileset::Tileset()
{
 offset=0;
 rows=0;
 columns=0;
 tile_width=0;
 tile_height=0;
}

Tileset::~Tileset()
{

}

unsigned long int Tileset::get_tile_width()
{
 return tile_width;
}

unsigned long int Tileset::get_tile_height()
{
 return tile_height;
}

unsigned long int Tileset::get_rows()
{
 return rows;
}

unsigned long int Tileset::get_columns()
{
 return columns;
}

void Tileset::select_tile(const unsigned long int row,const unsigned long int column)
{
 unsigned long int x_offset,y_offset;
 x_offset=0;
 y_offset=0;
 if ((row<rows)&&(column<columns))
 {
  x_offset=row*tile_width;
  y_offset=column*tile_height;
  offset=this->get_offset(0,x_offset,y_offset);
 }

}

void Tileset::draw_tile(const unsigned long int x,const unsigned long int y)
{
 size_t tile_offset;
 unsigned long int tile_x,tile_y;
 for(tile_x=0;tile_x<tile_width;++tile_x)
 {
  for(tile_y=0;tile_y<tile_height;++tile_y)
  {
   tile_offset=this->get_offset(offset,tile_x,tile_y);
   this->draw_image_pixel(tile_offset,x+tile_x,y+tile_y);
  }

 }

}

void Tileset::load_tileset(Image &buffer,const unsigned long int row_amount,const unsigned long int column_amount)
{
 width=buffer.get_width();
 height=buffer.get_height();
 rows=row_amount;
 columns=column_amount;
 tile_width=width/rows;
 tile_height=height/columns;
 this->clear_buffer();
 image=this->create_buffer(width,height);
 memmove(image,buffer.get_data(),buffer.get_data_length());
 buffer.destroy_image();
}

Text::Text()
{
 current_x=0;
 current_y=0;
 step_x=0;
 font=NULL;
}

Text::~Text()
{
 font=NULL;
}

void Text::draw_character(const char target)
{
 font->set_position(step_x,current_y);
 font->set_target((unsigned char)target+1);
 font->draw_sprite();
 step_x+=font->get_width();
}

void Text::set_position(const unsigned long int x,const unsigned long int y)
{
 current_x=x;
 current_y=y;
}

void Text::load_font(Sprite *target)
{
 font=target;
 font->set_frames(128);
 font->set_kind(HORIZONTAL_STRIP);
}

void Text::draw_text(const char *text)
{
 size_t index,length;
 length=strlen(text);
 step_x=current_x;
 for (index=0;index<length;++index)
 {
  if (text[index]<32) continue;
  this->draw_character(text[index]);
 }

}

bool Collision::check_horizontal_collision(const Collision_Box &first,const Collision_Box &second)
{
 bool result;
 result=false;
 if((first.x+first.width)>=second.x)
 {
  if(first.x<=(second.x+second.width)) result=true;
 }
 return result;
}

Collision_Box Collision::generate_box(const unsigned long int x,const unsigned long int y,const unsigned long int width,const unsigned long int height)
{
 Collision_Box result;
 result.x=x;
 result.y=y;
 result.width=width;
 result.height=height;
 return result;
}

bool Collision::check_vertical_collision(const Collision_Box &first,const Collision_Box &second)
{
 bool result;
 result=false;
 if((first.y+first.height)>=second.y)
 {
  if(first.y<=(second.y+second.height)) result=true;
 }
 return result;
}

bool Collision::check_collision(const Collision_Box &first,const Collision_Box &second)
{
 return this->check_horizontal_collision(first,second) || this->check_vertical_collision(first,second);
}

}