/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      Color manipulation routines (blending, format conversion, lighting
 *      table construction, etc).
 *
 *      By Shawn Hargreaves.
 *
 *      Dave Thomson contributed the RGB <-> HSV conversion routines.
 *
 *      Jan Hubicka wrote the super-fast version of create_rgb_table().
 *
 *      Michael Bevin optimised the create_trans_table() function.
 *
 *      Sven Sandberg optimised the create_light_table() function.
 *
 *      See readme.txt for copyright information.
 */


#include <limits.h>
#include <string.h>

#define ALLEGRO_INCLUDE_MATH_H

#include "allegro.h"
#include "allegro/aintern.h"



/* makecol_depth:
 *  Converts R, G, and B values (ranging 0-255) to whatever pixel format
 *  is required by the specified color depth.
 */
int makecol_depth(int color_depth, int r, int g, int b)
{
   switch (color_depth) {

      case 8:
	 return makecol8(r, g, b);

      case 15:
	 return makecol15(r, g, b);

      case 16:
	 return makecol16(r, g, b);

      case 24:
	 return makecol24(r, g, b);

      case 32:
	 return makecol32(r, g, b);
   }

   return 0;
}



/* makeacol_depth:
 *  Converts R, G, B, and A values (ranging 0-255) to whatever pixel format
 *  is required by the specified color depth.
 */
int makeacol_depth(int color_depth, int r, int g, int b, int a)
{
   switch (color_depth) {

      case 8:
	 return makecol8(r, g, b);

      case 15:
	 return makecol15(r, g, b);

      case 16:
	 return makecol16(r, g, b);

      case 24:
	 return makecol24(r, g, b);

      case 32:
	 return makeacol32(r, g, b, a);
   }

   return 0;
}



/* makecol:
 *  Converts R, G, and B values (ranging 0-255) to whatever pixel format
 *  is required by the current video mode.
 */
int makecol(int r, int g, int b)
{
   return makecol_depth(_color_depth, r, g, b);
}



/* makeacol:
 *  Converts R, G, B, and A values (ranging 0-255) to whatever pixel format
 *  is required by the current video mode.
 */
int makeacol(int r, int g, int b, int a)
{
   return makeacol_depth(_color_depth, r, g, b, a);
}



/* getr_depth:
 *  Extracts the red component (ranging 0-255) from a pixel in the format
 *  being used by the specified color depth.
 */
int getr_depth(int color_depth, int c)
{
   switch (color_depth) {

      case 8:
	 return getr8(c);

      case 15:
	 return getr15(c);

      case 16:
	 return getr16(c);

      case 24:
	 return getr24(c);

      case 32:
	 return getr32(c);
   }

   return 0;
}



/* getg_depth:
 *  Extracts the green component (ranging 0-255) from a pixel in the format
 *  being used by the specified color depth.
 */
int getg_depth(int color_depth, int c)
{
   switch (color_depth) {

      case 8:
	 return getg8(c);

      case 15:
	 return getg15(c);

      case 16:
	 return getg16(c);

      case 24:
	 return getg24(c);

      case 32:
	 return getg32(c);
   }

   return 0;
}



/* getb_depth:
 *  Extracts the blue component (ranging 0-255) from a pixel in the format
 *  being used by the specified color depth.
 */
int getb_depth(int color_depth, int c)
{
   switch (color_depth) {

      case 8:
	 return getb8(c);

      case 15:
	 return getb15(c);

      case 16:
	 return getb16(c);

      case 24:
	 return getb24(c);

      case 32:
	 return getb32(c);
   }

   return 0;
}



/* geta_depth:
 *  Extracts the alpha component (ranging 0-255) from a pixel in the format
 *  being used by the specified color depth.
 */
int geta_depth(int color_depth, int c)
{
   if (color_depth == 32)
      return geta32(c);

   return 0;
}



/* getr:
 *  Extracts the red component (ranging 0-255) from a pixel in the format
 *  being used by the current video mode.
 */
int getr(int c)
{
   return getr_depth(_color_depth, c);
}



/* getg:
 *  Extracts the green component (ranging 0-255) from a pixel in the format
 *  being used by the current video mode.
 */
int getg(int c)
{
   return getg_depth(_color_depth, c);
}



/* getb:
 *  Extracts the blue component (ranging 0-255) from a pixel in the format
 *  being used by the current video mode.
 */
int getb(int c)
{
   return getb_depth(_color_depth, c);
}



/* geta:
 *  Extracts the alpha component (ranging 0-255) from a pixel in the format
 *  being used by the current video mode.
 */
int geta(int c)
{
   return geta_depth(_color_depth, c);
}



/* 1.5k lookup table for color matching */
static unsigned int col_diff[3*128]; 



/* bestfit_init:
 *  Color matching is done with weighted squares, which are much faster
 *  if we pregenerate a little lookup table...
 */
static void bestfit_init()
{
   int i;

   for (i=1; i<64; i++) {
      int k = i * i;
      col_diff[0  +i] = col_diff[0  +128-i] = k * (59 * 59);
      col_diff[128+i] = col_diff[128+128-i] = k * (30 * 30);
      col_diff[256+i] = col_diff[256+128-i] = k * (11 * 11);
   }
}



/* bestfit_color:
 *  Searches a palette for the color closest to the requested R, G, B value.
 */
int bestfit_color(AL_CONST PALETTE pal, int r, int g, int b)
{
   int i, coldiff, lowest, bestfit;

   if (col_diff[1] == 0)
      bestfit_init();

   bestfit = 0;
   lowest = INT_MAX;

   if ((r == 63) && (g == 0) && (b == 63))
      i = 0;
   else
      i = 1;

   while (i<PAL_SIZE) {
      AL_CONST RGB *rgb = &pal[i];
      coldiff = (col_diff + 0) [ (rgb->g - g) & 0x7F ];
      if (coldiff < lowest) {
	 coldiff += (col_diff + 128) [ (rgb->r - r) & 0x7F ];
	 if (coldiff < lowest) {
	    coldiff += (col_diff + 256) [ (rgb->b - b) & 0x7F ];
	    if (coldiff < lowest) {
	       bestfit = rgb - pal;    /* faster than `bestfit = i;' */
	       if (coldiff == 0)
		  return bestfit;
	       lowest = coldiff;
	    }
	 }
      }
      i++;
   }

   return bestfit;
}



/* makecol8: 
 *  Converts R, G, and B values (ranging 0-255) to an 8 bit paletted color.
 *  If the global rgb_map table is initialised, it uses that, otherwise
 *  it searches through the current palette to find the best match.
 */
int makecol8(int r, int g, int b)
{
   if (rgb_map)
      return rgb_map->data[r>>3][g>>3][b>>3];
   else
      return bestfit_color(_current_palette, r>>2, g>>2, b>>2);
}



/* hsv_to_rgb:
 *  Converts from HSV colorspace to RGB values.
 */
void hsv_to_rgb(float h, float s, float v, int *r, int *g, int *b)
{
   float f, x, y, z;
   int i;

   v *= 255.0;

   if (s == 0.0) {
      *r = *g = *b = (int)v;
   }
   else {
      while (h < 0)
	 h += 360;
      h = fmod(h, 360) / 60.0;
      i = (int)h;
      f = h - i;
      x = v * (1.0 - s);
      y = v * (1.0 - (s * f));
      z = v * (1.0 - (s * (1.0 - f)));

      switch (i) {
	 case 0: *r = v; *g = z; *b = x; break;
	 case 1: *r = y; *g = v; *b = x; break;
	 case 2: *r = x; *g = v; *b = z; break;
	 case 3: *r = x; *g = y; *b = v; break;
	 case 4: *r = z; *g = x; *b = v; break;
	 case 5: *r = v; *g = x; *b = y; break;
      }
   }
}



/* rgb_to_hsv:
 *  Converts an RGB value into the HSV colorspace.
 */
void rgb_to_hsv(int r, int g, int b, float *h, float *s, float *v)
{
   float min, max, delta, rc, gc, bc;

   rc = (float)r / 255.0;
   gc = (float)g / 255.0;
   bc = (float)b / 255.0;
   max = MAX(rc, MAX(gc, bc));
   min = MIN(rc, MIN(gc, bc));
   delta = max - min;
   *v = max;

   if (max != 0.0)
      *s = delta / max;
   else
      *s = 0.0;

   if (*s == 0.0) {
      *h = 0.0; 
   }
   else {
      if (rc == max)
	 *h = (gc - bc) / delta;
      else if (gc == max)
	 *h = 2 + (bc - rc) / delta;
      else if (bc == max)
	 *h = 4 + (rc - gc) / delta;

      *h *= 60.0;
      if (*h < 0)
	 *h += 360.0;
    }
}



/* create_rgb_table:
 *  Fills an RGB_MAP lookup table with conversion data for the specified
 *  palette. This is the faster version by Jan Hubicka.
 *
 *  Uses alg. similar to floodfill - it adds one seed per every color in
 *  palette to its best position. Then areas around seed are filled by
 *  same color because it is best approximation for them, and then areas
 *  about them etc...
 *
 *  It does just about 80000 tests for distances and this is about 100
 *  times better than normal 256*32000 tests so the caluclation time
 *  is now less than one second at all computers I tested.
 */
void create_rgb_table(RGB_MAP *table, AL_CONST PALETTE pal, void (*callback)(int pos))
{
   #define UNUSED 65535
   #define LAST 65532

   /* macro add adds to single linked list */
   #define add(i)    (next[(i)] == UNUSED ? (next[(i)] = LAST, \
		     (first != LAST ? (next[last] = (i)) : (first = (i))), \
		     (last = (i))) : 0)

   /* same but w/o checking for first element */
   #define add1(i)   (next[(i)] == UNUSED ? (next[(i)] = LAST, \
		     next[last] = (i), \
		     (last = (i))) : 0)

   /* calculates distance between two colors */
   #define dist(a1, a2, a3, b1, b2, b3) \
		     (col_diff[ ((a2) - (b2)) & 0x7F] + \
		     (col_diff + 128)[((a1) - (b1)) & 0x7F] + \
		     (col_diff + 256)[((a3) - (b3)) & 0x7F])

   /* converts r,g,b to position in array and back */
   #define pos(r, g, b) \
		     (((r) / 2) * 32 * 32 + ((g) / 2) * 32 + ((b) / 2))

   #define depos(pal, r, g, b) \
		     ((b) = ((pal) & 31) * 2, \
		      (g) = (((pal) >> 5) & 31) * 2, \
		      (r) = (((pal) >> 10) & 31) * 2)

   /* is current color better than pal1? */
   #define better(r1, g1, b1, pal1) \
		     (((int)dist((r1), (g1), (b1), \
				 (pal1).r, (pal1).g, (pal1).b)) > (int)dist2)

   /* checking of position */
   #define dopos(rp, gp, bp, ts) \
      if ((rp > -1 || r > 0) && (rp < 1 || r < 61) && \
	  (gp > -1 || g > 0) && (gp < 1 || g < 61) && \
	  (bp > -1 || b > 0) && (bp < 1 || b < 61)) { \
	 i = first + rp * 32 * 32 + gp * 32 + bp; \
         if (!data[i]) { \
	    data[i] = val; \
	    add1(i); \
	 } \
	 else if ((ts) && (data[i] != val)) { \
	    dist2 = (rp ? (col_diff+128)[(r+2*rp-pal[val].r) & 0x7F] : r2) + \
		    (gp ? (col_diff    )[(g+2*gp-pal[val].g) & 0x7F] : g2) + \
		    (bp ? (col_diff+256)[(b+2*bp-pal[val].b) & 0x7F] : b2); \
	    if (better((r+2*rp), (g+2*gp), (b+2*bp), pal[data[i]])) { \
	       data[i] = val; \
	       add1(i); \
	    } \
	 } \
      }

   int i, curr, r, g, b, val, r2, g2, b2, dist2;
   unsigned short next[32*32*32];
   unsigned char *data;
   int first = LAST;
   int last = LAST;
   int count = 0;
   int cbcount = 0;

   #define AVERAGE_COUNT   18000

   if (col_diff[1] == 0)
      bestfit_init();

   memset(next, 255, sizeof(next));
   memset(table->data, 0, sizeof(char)*32*32*32);

   data = (unsigned char *)table->data;

   /* add starting seeds for floodfill */
   for (i=1; i<256; i++) { 
      curr = pos(pal[i].r, pal[i].g, pal[i].b);
      if (next[curr] == UNUSED) {
	 data[curr] = i;
	 add(curr);
      }
   }

   /* main floodfill: two versions of loop for faster growing in blue axis */
   while (first != LAST) { 
      depos(first, r, g, b);

      /* calculate distance of current color */
      val = data[first];
      r2 = (col_diff+128)[((pal[val].r)-(r)) & 0x7F];
      g2 = (col_diff    )[((pal[val].g)-(g)) & 0x7F];
      b2 = (col_diff+256)[((pal[val].b)-(b)) & 0x7F];

      /* try to grow to all directions */
      dopos( 0, 0, 1, 1); 
      dopos( 0, 0,-1, 1);
      dopos( 1, 0, 0, 1);
      dopos(-1, 0, 0, 1);
      dopos( 0, 1, 0, 1);
      dopos( 0,-1, 0, 1);

      /* faster growing of blue direction */
      if ((b > 0) && (data[first-1] == val)) { 
	 b -= 2;
	 first--;
	 b2 = (col_diff+256)[((pal[val].b)-(b)) & 0x7F];

	 dopos(-1, 0, 0, 0);
	 dopos( 1, 0, 0, 0);
	 dopos( 0,-1, 0, 0);
	 dopos( 0, 1, 0, 0);

	 first++;
      }

      /* get next from list */
      i = first; 
      first = next[first];
      next[i] = UNUSED;

      /* second version of loop */
      if (first != LAST) { 
	 depos(first, r, g, b);

	 val = data[first];
	 r2 = (col_diff+128)[((pal[val].r)-(r)) & 0x7F];
	 g2 = (col_diff    )[((pal[val].g)-(g)) & 0x7F];
	 b2 = (col_diff+256)[((pal[val].b)-(b)) & 0x7F];

	 dopos( 0, 0, 1, 1);
	 dopos( 0, 0,-1, 1);
	 dopos( 1, 0, 0, 1);
	 dopos(-1, 0, 0, 1);
	 dopos( 0, 1, 0, 1);
	 dopos( 0,-1, 0, 1);

	 if ((b < 61) && (data[first + 1] == val)) {
	    b += 2;
	    first++;
	    b2 = (col_diff+256)[((pal[val].b)-(b)) & 0x7f];

	    dopos(-1, 0, 0, 0);
	    dopos( 1, 0, 0, 0);
	    dopos( 0,-1, 0, 0);
	    dopos( 0, 1, 0, 0);

	    first--;
	 }

	 i = first;
	 first = next[first];
	 next[i] = UNUSED;
      }

      count++;
      if (count == (cbcount+1)*AVERAGE_COUNT/256) {
	 if (cbcount < 256) {
	    if (callback)
	       callback(cbcount);
	    cbcount++;
	 }
      }
   }

   if (callback)
      while (cbcount < 256)
	 callback(cbcount++);
}



/* create_light_table:
 *  Constructs a lighting color table for the specified palette. At light
 *  intensity 255 the table will produce the palette colors directly, and
 *  at level 0 it will produce the specified R, G, B value for all colors
 *  (this is specified in 0-63 VGA format). If the callback function is 
 *  not NULL, it will be called 256 times during the calculation, allowing
 *  you to display a progress indicator.
 */
void create_light_table(COLOR_MAP *table, AL_CONST PALETTE pal, int r, int g, int b, void (*callback)(int pos))
{
   int r1, g1, b1, x, y;
   unsigned int tmp;
   RGB c;

   for (x=0; x<PAL_SIZE; x++) {
      tmp = (255 - x) * 65793;

      r1 = r * tmp + (1 << 23);
      g1 = g * tmp + (1 << 23);
      b1 = b * tmp + (1 << 23);

      tmp = (1 << 24) - tmp;

      for (y=0; y<PAL_SIZE; y++) {
	 c.r = (r1 + (unsigned int)pal[y].r * tmp) >> 24;
	 c.g = (g1 + (unsigned int)pal[y].g * tmp) >> 24;
	 c.b = (b1 + (unsigned int)pal[y].b * tmp) >> 24;

	 if (rgb_map)
	    table->data[x][y] = rgb_map->data[c.r>>1][c.g>>1][c.b>>1];
	 else
	    table->data[x][y] = bestfit_color(pal, c.r, c.g, c.b);
      }

      if (callback)
	 (*callback)(x);
   }
}



/* create_trans_table:
 *  Constructs a translucency color table for the specified palette. The
 *  r, g, and b parameters specifiy the solidity of each color component,
 *  ranging from 0 (totally transparent) to 255 (totally solid). Source
 *  color #0 is a special case, and is set to leave the destination 
 *  unchanged, so that masked sprites will draw correctly. If the callback 
 *  function is not NULL, it will be called 256 times during the calculation, 
 *  allowing you to display a progress indicator.
 */
void create_trans_table(COLOR_MAP *table, AL_CONST PALETTE pal, int r, int g, int b, void (*callback)(int pos))
{
   int tmp[768], *q;
   int x, y, i, j, k;
   unsigned char *p;
   RGB c;

   for (x=0; x<256; x++) {
      tmp[x*3]   = pal[x].r * (255-r) / 255;
      tmp[x*3+1] = pal[x].g * (255-g) / 255;
      tmp[x*3+2] = pal[x].b * (255-b) / 255;
   }

   for (y=0; y<PAL_SIZE; y++)
      table->data[0][y] = y;

   if (callback)
      (*callback)(0);

   for (x=1; x<256; x++) {
      i = pal[x].r * r / 255;
      j = pal[x].g * g / 255;
      k = pal[x].b * b / 255;

      p = table->data[x];
      q = tmp;

      if (rgb_map) {
	 for (y=0; y<256; y++) {
	    c.r = i + *(q++);
	    c.g = j + *(q++);
	    c.b = k + *(q++);
	    p[y] = rgb_map->data[c.r>>1][c.g>>1][c.b>>1];
	 }
      }
      else {
	 for (y=0; y<256; y++) {
	    c.r = i + *(q++); 
	    c.g = j + *(q++); 
	    c.b = k + *(q++);
	    p[y] = bestfit_color(pal, c.r, c.g, c.b);
	 }
      }

      if (callback)
	 (*callback)(x);
   }
}



/* create_color_table:
 *  Creates a color mapping table, using a user-supplied callback to blend
 *  each pair of colors. Your blend routine will be passed a pointer to the
 *  palette and the two colors to be blended (x is the source color, y is
 *  the destination), and should return the desired output RGB for this
 *  combination. If the callback function is not NULL, it will be called 
 *  256 times during the calculation, allowing you to display a progress 
 *  indicator.
 */
void create_color_table(COLOR_MAP *table, AL_CONST PALETTE pal, void (*blend)(AL_CONST PALETTE pal, int x, int y, RGB *rgb), void (*callback)(int pos))
{
   int x, y;
   RGB c;

   for (x=0; x<PAL_SIZE; x++) {
      for (y=0; y<PAL_SIZE; y++) {
	 blend(pal, x, y, &c);

	 if (rgb_map)
	    table->data[x][y] = rgb_map->data[c.r>>1][c.g>>1][c.b>>1];
	 else
	    table->data[x][y] = bestfit_color(pal, c.r, c.g, c.b);
      }

      if (callback)
	 (*callback)(x);
   }
}



/* create_blender_table:
 *  Fills the specified color mapping table with lookup data for doing a 
 *  paletted equivalent of whatever truecolor blender mode is currently 
 *  selected.
 */
void create_blender_table(COLOR_MAP *table, AL_CONST PALETTE pal, void (*callback)(int pos))
{
   int x, y, c;
   int r, g, b;
   int r1, g1, b1;
   int r2, g2, b2;

   ASSERT(_blender_func24);

   for (x=0; x<PAL_SIZE; x++) {
      for (y=0; y<PAL_SIZE; y++) {
	 r1 = pal[x].r * 255 / 63;
	 g1 = pal[x].g * 255 / 63;
	 b1 = pal[x].b * 255 / 63;

	 r2 = pal[y].r * 255 / 63;
	 g2 = pal[y].g * 255 / 63;
	 b2 = pal[y].b * 255 / 63;

	 c = _blender_func24(makecol24(r1, g1, b1), makecol24(r2, g2, b2), _blender_alpha);

	 r = getr24(c);
	 g = getg24(c);
	 b = getb24(c);

	 if (rgb_map)
	    table->data[x][y] = rgb_map->data[r>>3][g>>3][b>>3];
	 else
	    table->data[x][y] = bestfit_color(pal, r>>2, g>>2, b>>2);
      }

      if (callback)
	 (*callback)(x);
   }
}

