#ifndef PIXEL_H
#define PIXEL_H

#define F_DEBUG Serial.printf("call from: %s\n",__PRETTY_FUNCTION__ );

#include "Arduino.h"
#include <FastLED.h>

#define PX_COUNT 8
#define PIN__PX  2

#define PX_BLACK  0x000000
#define PX_GREEN  0x00FF00
#define PX_BLUE   0x0000FF
#define PX_RED    0xFF0000
#define PX_PURPLE 0xFF00FF
#define PX_YELLOW 0xFFFF00
#define PX_ORANGE 0xFF7000
//#define PX_YELLOW 0xFF00FF
#define PX_WHITE  0xFFFFFF


// converting

#define RGB_565(r,g,b) ( ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3) )
#define RGB_565(r,g,b) ( ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3) )
#define RGB_24(r,g,b)  ( (r << 16) | (g << 8) | b )
#define GBR_24(g,b,r)  ( (g << 16) | (b << 8) | r )


//template <uint16_t DATA_PIN, uint8_t PX_COUNT>
class Pixel{

  private:
    
    //uint8_t Brightness = 255;
    bool Change = false;
  
  public:

    // ===============================================================
    // Single Pixels
    // ===============================================================

    enum mode_t{
      px_mode_static = 0,
      px_mode_clone,
      px_mode_blink,
      px_mode_fade,
      px_mode_function
    };

    typedef struct{
      public:
        // mode
          mode_t   mode    = px_mode_static;
        // state
          uint16_t state   = 0;
          uint16_t len     = 0;
        // periode
          uint16_t periode = 0;
          uint32_t timeout = 0;
        // color
          CRGB  color    = PX_BLACK;
          CRGB  color_bg = PX_BLACK;
    }single_pixel_t;


    // ===============================================================
    // Pixel array
    // ===============================================================
    
    enum array_mode_t{
      px_array_mode_static = 0,
      px_array_mode_function,
      px_array_mode_shift_up,
      px_array_mode_shift_down,
      px_array_mode_ring_shift_up,
      px_array_mode_ring_shift_down
    };

    typedef struct{
      public:
        // mode
          array_mode_t mode = px_array_mode_static;
        // state
          uint16_t state   = 0;
          uint16_t len     = 0;
        // periode
          uint16_t periode = 0;
          uint32_t timeout = 0;
        // color
          CRGB  color    = PX_BLACK;
          CRGB  color_bg = PX_BLACK;
          void (*callback)( Pixel* ) = nullptr;
    }pixel_array_t;

    // ===============================================================
    // variables
    // ===============================================================
    CRGB           leds[PX_COUNT];
    single_pixel_t spx[PX_COUNT];
    pixel_array_t  global;

    // ===============================================================
    // Basic functions
    // ===============================================================
    
    void begin(){
      //FastLED.addLeds<SK6812, DATA_PIN, GRB>(leds, PX_COUNT);  // GRB ordering is typical
      //FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, PX_COUNT);  // GRB ordering is typical
      FastLED.addLeds<WS2812, PIN__PX, GRB>(leds, PX_COUNT);  // GRB ordering is typical
    }

    void setBrightness(uint8_t b){
      FastLED.setBrightness(b);
    }

    void write(){
      FastLED.show();
    };

    // ===============================================================
    // Fill
    // ===============================================================

    void fill( uint32_t color ){ push(0,PX_COUNT,color); }
    void fill_HSV( int H, int S, int v ){ push(0,PX_COUNT,HSV2RGB(H,S,v)); }
    void fill_blink( uint32_t color, uint32_t color_bg, uint16_t periode ){ push_blink( 0, PX_COUNT, color, color_bg, periode ); }

    // ===============================================================
    // Shifting
    // ===============================================================

    void shift_up( uint8_t px0, uint8_t count, bool ring = false, bool default_color = PX_BLACK ){
      // check range
      if( px0 >= PX_COUNT ) return;

      single_pixel_t spx_temp;
      spx_temp.color = default_color;
      
      // ring
      if( ring ){
        spx_temp = spx[(px0+count-1)%PX_COUNT];
      }

      for(int i=count-1;i>0;i--){
        spx[(i+px0)%PX_COUNT] = spx[(i-1+px0)%PX_COUNT];
      }
      
      spx[px0] = spx_temp;
    }

    void shift_down( uint8_t px0, uint8_t count, bool ring = false, bool default_color = PX_BLACK ){
      // check range
      if( px0 >= PX_COUNT ) return;

      single_pixel_t spx_temp;
      spx_temp.color = default_color;

      uint8_t pxf = px0-count+1;
      
      // ring
      if( ring ){
        spx_temp = spx[pxf%PX_COUNT];
      }

      for(int i=0;i<(count-1);i++){
        spx[(pxf+i)%PX_COUNT] = spx[(pxf+i+1)%PX_COUNT];
      }
      
      spx[px0] = spx_temp;
    }

    /*
      px0      = 2
      PX_count = 5
      count    = 4
      i=0 -> [ 0+2 | 5 ] <- [ 0+1+4 | 5 ] -> ( 4 <- 0 )
      i=0 -> [ 0+4 | 5 ] <- [ 0+1+4 | 5 ] -> ( 4 <- 0 )
      i=1 -> [ 1+4 | 5 ] <- [ 1+1+4 | 5 ] -> ( 0 <- 1 )
      i=2 -> [ 2+4 | 5 ] <- [ 2+1+4 | 5 ] -> ( 1 <- 2 )
    */ 

    // ===============================================================
    // Push
    // ===============================================================

    void push( uint8_t px0, uint32_t count, uint32_t color ){
      push( px0, count, (CRGB) color );
    }

    void push( uint8_t px0, uint32_t count, CRGB color ){
      if( px0 >= PX_COUNT ) return;
      count = ( PX_COUNT-px0 >= count ? count : PX_COUNT-px0 );
      for(int i=0; i<count; i++){
        leds[px0+i]      = color;
        spx[px0+i].color = color;
        spx[px0+i].mode  = px_mode_static;
      }
    }

    void push( uint8_t px0, uint32_t count, uint32_t *colors ){
      if( px0 >= PX_COUNT ) return;
      count = ( PX_COUNT-px0 >= count ? count : PX_COUNT-px0 );
      for(int i=0; i<count; i++){
        leds[px0+i]      = colors[i];
        spx[px0+i].color = colors[i];
        spx[px0+i].mode  = px_mode_static;
      }
    }

    void push( uint8_t px0, uint32_t count, CRGB *colors ){
      if( px0 >= PX_COUNT ) return;
      count = ( PX_COUNT-px0 >= count ? count : PX_COUNT-px0 );
      for(int i=0; i<count; i++){
        leds[px0+i]      = colors[i];
        spx[px0+i].color = colors[i];
        spx[px0+i].mode  = px_mode_static;
      }
    }

    // =========================================================================
    // push blink
    // =========================================================================
    void push_blink( uint8_t px0, uint32_t count, uint32_t color, uint32_t color_bg, uint16_t periode ){
      if( px0 >= PX_COUNT ) return;
      Change = true;
      count = ( PX_COUNT-px0 >= count ? count : PX_COUNT-px0 );
      spx[px0].color    = color;
      spx[px0].color_bg = color_bg;
      spx[px0].timeout  = 0;
      spx[px0].periode  = periode/2;
      spx[px0].state    = 0;
      spx[px0].mode     = px_mode_blink;
      for(int i=1; i<count; i++){
        spx[px0+i].mode  = px_mode_clone;
        spx[px0+i].state = px0;
      }
    }

    // =========================================================================
    // push fade
    // =========================================================================
    void push_fade( uint8_t px0, uint32_t count, uint32_t color, uint32_t color_bg, uint16_t periode ){
      if( px0 >= PX_COUNT ) return;
      Change = true;
      count = ( PX_COUNT-px0 >= count ? count : PX_COUNT-px0 );
      spx[px0].color    = color;
      spx[px0].color_bg = color_bg;
      spx[px0].state    = 0;
      spx[px0].len      = 512;
      spx[px0].periode  = periode/512;
      spx[px0].mode     = px_mode_fade;
      for(int i=1; i<count; i++){
        spx[px0+i].mode  = px_mode_clone;
        spx[px0+i].state = px0;
      }
    }

    CRGB fade( CRGB color, CRGB color_bg, uint8_t scale ){
      return color%( scale ) + color_bg%( 255 - scale);
    }

    // ===============================================================
    // Update
    // ===============================================================
    void log(){
      for(int i=0;i<PX_COUNT;i++){
        Serial.printf( "PX[%d]\t%02X%02X%02X\tmode:%d\n", i, spx[i].color.r, spx[i].color.g, spx[i].color.b, spx[i].mode );
      }
    }

    // ===============================================================
    // Update
    // ===============================================================
    boolean update(){
      
      bool change = false;

      for(int i=0;i<PX_COUNT;i++){
        switch( spx[i].mode ){
          case px_mode_static:{
            if( leds[i] != spx[i].color ){
              leds[i] = spx[i].color;
              change = true;
            }
          }break;
          case px_mode_blink:{
            if( spx[i].timeout <= millis() ){
              spx[i].timeout = millis() + spx[i].periode;
              spx[i].state = (spx[i].state?0:1);
              leds[i] = ( spx[i].state ? spx[i].color : spx[i].color_bg );
              change = true;
            }
          }break;
          case px_mode_fade:{
            if( spx[i].timeout <= millis() ){
              spx[i].timeout = millis() + spx[i].periode;
              leds[i] = fade( spx[i].color, spx[i].color_bg, spx[i].state > 255 ? 511 - spx[i].state : spx[i].state );
              spx[i].state = ( spx[i].state >= 511 ? 0 : spx[i].state+1 );
              change = true;
            }
          }break;
        }
      }

      // clone
      for(int i=0;i<PX_COUNT;i++){
        if( spx[i].mode == px_mode_clone ){
          CRGB color = leds[spx[i].state];
          if( leds[i] != color ){
            leds[i] = color;
            change = true;
          }
        }
      }

      if( global.mode == px_array_mode_function ){
        if( global.callback != nullptr ){
          if( global.timeout <= millis() ){
            global.callback( this );
          }
        }
      }else if( global.mode == px_array_mode_shift_up || global.mode == px_array_mode_ring_shift_up ){
        if( global.timeout <= millis() ){
          Serial.println( ">" );
          global.timeout = millis() + global.periode;
          shift_up( 0, PX_COUNT, global.mode == px_array_mode_ring_shift_up );
        }
      }else if( global.mode == px_array_mode_shift_down || global.mode == px_array_mode_ring_shift_down ){
        if( global.timeout <= millis() ){
          global.timeout = millis() + global.periode;
          shift_down( PX_COUNT-1, PX_COUNT, global.mode == px_array_mode_ring_shift_down );
        }
      }

      if(change){
        write();
        return true;
      }

      return false;
    }

    // colors
    uint32_t colors[ 7 ] = {
      0xff0000,
      0x00ff00,
      0x0000ff,
      0xffff00,
      0x00ffff,
      0xff00ff,
      0xffffff
    };

    uint16_t color_H11[11] ={ // HUE de 11 cores distinguiveis
      0, // 
      15,
      40,
      90,
      120,
      135,
      180,
      240,
      256,
      300,
      350
    };

    uint16_t color_H7[7] ={ // distinguivel com brilho baixo
      0,
      40,
      90,
      120,
      180,
      240,
      300
    };

    uint32_t HSV2RGB(int H_in, int S_in, int V_in){
      // H (0,360)  S(0,100)  V(0,100)
      double H = H_in;
      double S = S_in/100.0;
      double V = 255.0*(V_in/100.0);
      int r = V, g = V, b = V;
      if ( S != 0 ){
         int i;
         double f, p, q, t;
         H = ( H == 360 ? 0 : H / 60 );
         i = (int)trunc(H);
         f = H - i;
         p = (1.0 -  S);
         q = (1.0 - (S * f));
         t = (1.0 - (S * (1.0 - f)));
         switch (i) {
           case  0: r *= 1; g *= t; b *= p; break;
           case  1: r *= q; g *= 1; b *= p; break;
           case  2: r *= p; g *= 1; b *= t; break;
           case  3: r *= p; g *= q; b *= 1; break;
           case  4: r *= t; g *= p; b *= 1; break;
           default: r *= 1; g *= p; b *= q; break;
         }
      }
      //Serial.printf("Colors: RGB( %d, %d, %d )\n",r,g,b);
      return RGB_24(r,g,b);
    }
};

Pixel PX;

#endif