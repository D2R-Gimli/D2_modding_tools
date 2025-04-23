#include "one_include_to_rule_them_all.h"

// to avoid all mention of : warning C4055: 'type cast' : from data pointer 'void *' to function pointer '_BMP_BANK_SWITCHER'
#pragma warning(disable:4055)


#define ALLEGRO_NO_MAGIC_MAIN
#include "allegro.h"
#include "winalleg.h"

#include "dccinfo.h"
#include "allegro_animation.h"
#include "dccjob.h"
#include "enums.h"


typedef struct ANIMATION_COMPONENT_S // graphical datas for 1 layer
{
   BITMAP * bmp;
   long   offset_x;
   long   offset_y;
} ANIMATION_COMPONENT;

typedef struct ANIMATION_PRIVATE_S
{
   unsigned char        nb_directions;
   unsigned char        nb_frames_per_direction;
   unsigned char        nb_layers;
   ANIMATION_COMPONENT  comp [DCC_MAX_DIR] [DCC_MAX_FRAME] [16];
   long                 speed;
   long                 animdatad2_speed;
   ANIMATION_LAYER_LIST layer_list;
   unsigned char        * priority; // priority[direction][frame][layer order] = layer code to draw
   int                  current_direction;
   int                  delta_frame;
   int                  force_redraw; // 0 = no, 1 = yes
   unsigned long        layer_color_frequency [16] [256];
   unsigned long        anim_color_frequency [256];
   int                  nb_layer_color_used [16];
   int                  nb_color_used;
   UBYTE                * pl2;
} ANIMATION_PRIVATE;


// global datas
volatile int           timer_25fps_active                              = FALSE;
volatile int           timer_25fps_preview_active                      = FALSE;
volatile unsigned long current_tick_25fps                              = 0;
volatile unsigned long current_tick_25fps_preview                      = 0;
unsigned long          last_tick_25fps_drawned                         = 0;
HWND                   animation_window_handle                         = NULL;
BITMAP                 * animation_buffer                              = NULL;
BITMAP                 * screen_buffer                                 = NULL;
PALETTE                alleg_palette                                   = {0};
APP_PALETTE            application_palette                             = {0};
int                    user_anim_offset_x                              = 0;
int                    user_anim_offset_y                              = 0;
int                    user_pivot_offset_x                             = 0;
int                    user_pivot_offset_y                             = 0;
long                   diff_r [256][256]                               = {0};
long                   diff_g [256][256]                               = {0};
long                   diff_b [256][256]                               = {0};
int                    PL2_offset_for_special_effect [LAYEREFFECT_MAX] = {0};
int                    dlgbox_main_preview_width                       = 0;
int                    dlgbox_main_preview_height                      = 0;
int                    preview_zoom                                    = 1; // from 1 to 4


// functions
static void    _analyse_animation_colors  (ANIMATION_PRIVATE * anim);
static void    callback_25fps             (void);
static void    dc6_decode_RLE_bitmap      (void * src, BITMAP * dst, long size, int x0, int y0);
static void    dc6_destroy                (DC6_S * dc6);
static DC6_S * dc6_mem_load               (void * mem_ptr, long mem_size);
static void    draw_moire                 (BITMAP * bmp);
static int     get_COF_direction_mirrored (ANIMATION * _anim, int COF_direction);
static void    init_diff_rgb_tables       (void);
static void    init_export_box            (EXPORT_BOX * b);


// ===========================================================================
// destroy the memory of a DC6
// ===========================================================================
void dc6_destroy(DC6_S * dc6)
{
   int d = 0;
   int f = 0;


   if (dc6 == NULL)
      return;

   DESTROYME(dc6->file, free)
   for(d = 0; d < 32; d++)
   {
      for(f = 0; f < 256; f++)
         DESTROYME(dc6->frame[d][f].bmp, destroy_bitmap)
   }
   DESTROYME(dc6, free)
}


// ===========================================================================
// copy the file into a new DC6 structure
// return NULL if error
// ===========================================================================
DC6_S * dc6_mem_load (void * mem_ptr, long mem_size)
{
   DC6_S * ptr = NULL;


   if((mem_ptr == NULL) || (mem_size <= 0))
      return NULL;

   ptr = (DC6_S *) calloc(1, sizeof(DC6_S));
   if (ptr == NULL)
      return NULL;

   ptr->file = (UBYTE *) malloc(mem_size);
   if (ptr->file == NULL)
   {
      DESTROYME(ptr, free)
      return NULL;
   }

   memcpy(ptr->file, mem_ptr, mem_size);
   ptr->size = mem_size;
   return ptr;
}


// ===========================================================================
// decode the given RLE frame of a DC6 into a given bitmap
// ===========================================================================
void dc6_decode_RLE_bitmap(void * src, BITMAP * dst, long size, int x0, int y0)
{
   UBYTE * ptr = (UBYTE *) src;
   long  i     = 0;
   int   i2    = 0;
   int   x     = x0;
   int   y     = y0;
   UBYTE c     = 0;
   UBYTE c2    = 0;
   

   if ((src == NULL) || (dst == NULL) || (size <= 0))
      return;

   while (i < size)
   {
      c = ptr[i];
      i++;

      if (c == 0x80)
      {
         x = x0;
         y--;
      }
      else if (c & 0x80)
         x += (c & 0x7F);
      else
      {
         for (i2 = 0; i2 < c; i2++)
         {
            if (i >= size) // just for safety
               return;

            c2 = ptr[i];
            i++;
            putpixel(dst, x, y, c2);
            x++;
         }
      }
   }
}


// ===========================================================================
// decode an entire DC6
// return 0 if success
// ===========================================================================
int dc6_decode(DC6_S * dc6, long dir_bitfield)
{
   int           d          = 0;
   int           f          = 0;
   long          * lptr     = NULL;
   long          * dc6_fptr = 0;
   long          offset     = 0;
   DC6_DIRECTION * ptd      = NULL;
   DC6_FRAME     * ptf      = 0;
#pragma pack(1)
   struct dc6_header_s
   {
      long dc6_ver;
      long dc6_unk1;
      long dc6_unk2;
      long dummy;
      long dc6_dir;
      long dc6_fpd;
   } * dc6_header;
#pragma pack()


   if ((dc6->file == NULL) || (dc6->size < 28))
      return -1;

   // dc6 header datas
   lptr       = (long *) dc6->file;
   dc6_header = (struct dc6_header_s *) lptr;
   dc6_fptr   = & lptr[6];
   if ((dc6_header->dc6_ver != 6) || (dc6_header->dc6_unk1 != 1) || (dc6_header->dc6_unk2 != 0))
      return -1;

   // validate direction
   if ((dc6_header->dc6_dir < 0) || (dc6_header->dc6_dir > 31))
      return -1;

   // validate frames per direction
   if ((dc6_header->dc6_fpd < 0) || (dc6_header->dc6_fpd > 255))
      return -1;

   // decode directions
   for (d = 0; d < dc6_header->dc6_dir; d++)
   {
      if ((dir_bitfield & (1 << d)) == 0)
         continue;

      ptd = & dc6->direction[d];
      ptd->box.xmin   = 0x7FFFFFFF;
      ptd->box.ymin   = 0x7FFFFFFF;
      ptd->box.xmax   = 0x80000000;
      ptd->box.ymax   = 0x80000000;
      ptd->box.width  = 0;
      ptd->box.height = 0;

      for (f = 0; f < dc6_header->dc6_fpd; f++)
      {
         // get pointer to frame header
         offset = dc6_fptr[d * dc6_header->dc6_fpd + f];
         if (offset >= dc6->size)
            return -1;
         lptr = (long *) (dc6->file + offset);

         // frame box
         ptf = & dc6->frame[d][f];
         ptf->box.width  = lptr[1];
         ptf->box.height = lptr[2];
         ptf->box.xmin   = lptr[3];
         ptf->box.ymax   = lptr[4];
         ptf->box.xmax   = ptf->box.xmin + ptf->box.width  - 1;
         ptf->box.ymin   = ptf->box.ymax - ptf->box.height + 1;
         ptf->length     = lptr[7];
         ptf->data       = (unsigned char *) & lptr[8];

         // update direction box
         if (ptf->box.xmin < ptd->box.xmin) ptd->box.xmin = ptf->box.xmin;
         if (ptf->box.ymin < ptd->box.ymin) ptd->box.ymin = ptf->box.ymin;
         if (ptf->box.xmax > ptd->box.xmax) ptd->box.xmax = ptf->box.xmax;
         if (ptf->box.ymax > ptd->box.ymax) ptd->box.ymax = ptf->box.ymax;
      }

      // update direction box
      ptd->box.width  = ptd->box.xmax - ptd->box.xmin + 1;
      ptd->box.height = ptd->box.ymax - ptd->box.ymin + 1;

      if ((ptd->box.width <= 0) || (ptd->box.height <= 0))
         return -1;

      // create frame bitmaps
      for (f = 0; f < dc6_header->dc6_fpd; f++)
      {
         ptf = & dc6->frame[d][f];
         ptf->bmp = create_bitmap(ptd->box.width, ptd->box.height);
         if (ptf->bmp == NULL)
         {
            while (f)
            {
               f--;
               DESTROYME(dc6->frame[d][f].bmp, destroy_bitmap);
            }
            return -1;
         }

         clear(ptf->bmp);
         dc6_decode_RLE_bitmap(ptf->data, ptf->bmp, ptf->length, (ptf->box.xmin - ptd->box.xmin), (ptd->box.height - 1 + ptf->box.ymax - ptd->box.ymax));
      }
   }
   
   return 0;
}


// ===========================================================================
// initialize an Export box
// ===========================================================================
void init_export_box(EXPORT_BOX * b)
{
   if (b == NULL)
      return;

   b->left   = 0x7FFFFFFF;
   b->top    = 0x7FFFFFFF;
   b->right  = 0x80000000;
   b->bottom = 0x80000000;
   b->width  = 0;
   b->height = 0;
}


// ===========================================================================
// get a new Allegro bitmap
// return NULL on error
// ===========================================================================
void * get_allegro_bitmap(int width, int height)
{
   if ((width > 0) && (height > 0))
      return create_bitmap(width, height);
   
   return NULL;
}


// ===========================================================================
// destroy an Allegro bitmap
// ===========================================================================
void destroy_allegro_bitmap(void * _bmp)
{
   BITMAP * bmp = (BITMAP *) _bmp;


   DESTROYME(bmp, destroy_bitmap)
}


// ===========================================================================
// for a given COF direction, return the corresponding mirrored COF direction
// ===========================================================================
int get_COF_direction_mirrored(ANIMATION * _anim, int COF_direction)
{
   ANIMATION_PRIVATE * anim      = (ANIMATION_PRIVATE *) _anim;
   static int        dir4   [4]  = {3, 2, 1, 0};
   static int        dir8   [8]  = {0, 7, 6, 5, 4, 3, 2, 1};
   static int        dir16  [16] = {0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
   static int        dir32  [32] = {0, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};


   if (anim == NULL)
      return 0;

   if ((COF_direction < 0) || (COF_direction >= anim->nb_directions))
      return 0;

   switch (anim->nb_directions)
   {
      case  4 : return dir4 [COF_direction];
      case  8 : return dir8 [COF_direction];
      case 16 : return dir16[COF_direction];
      case 32 : return dir32[COF_direction];
      default : return 0;
   }
}


// ===========================================================================
// build 1 image : merge all layers of 1 given direction of 1 given frame
// using all the color options (shadows / palette shift / drawing mode ...),
// then save it into file(s) if needed
// ===========================================================================
int merge_layers_of_this_frame(EXPORT_PARAMETERS * ex)
{
   ANIMATION_PRIVATE   * anim        = NULL;
   BITMAP              * image       = NULL;
   BITMAP              * image_pivot = NULL;
   ANIMATION_COMPONENT * comp        = NULL;
   int                 c             = 0;
   ANIMATION_DCC       * layer       = NULL;
   int                 x             = 0;
   int                 y             = 0;
   int                 ox            = 0;
   int                 oy            = 0;
   int                 x2            = 0;
   int                 y2            = 0;
   int                 sprite_color  = 0;
   UBYTE               * cmap        = NULL;
   int                 deletion      = FALSE;
   int                 idx           = 0;
   int                 i             = 0;
   int                 d             = 0;
   int                 tmp_color     = 0;
   int                 draw [256]    = {0};
   int                 remap[256]    = {0};
   PALETTE             copypal       = {0};
   BITMAP              * bmp         = NULL;
   int                 offset        = 0;


   if (ex == NULL)
      return -1;

   anim        = (ANIMATION_PRIVATE *) ex->_anim;
   image       = (BITMAP *)            ex->_image;
   image_pivot = (BITMAP *)            ex->_image_pivot;

   if ((image == NULL) || (anim == NULL) || (ex->box == NULL))
      return -1;

   // if needed, fill image with best aproximated desired background color
   if (ex->bg != NULL)
      clear_to_color(image, ex->bg->best_fit_index);

   // get DCC direction given the COF direction
   d = get_COF_to_DCC_direction(anim, ex->COF_dir, ex->mirror_sp);

   // ------------------------------------------------------------------------------------------------------------
   // Part 1 : draw shadows
   // ------------------------------------------------------------------------------------------------------------
   if (ex->shadow_height_percent != 0)
   {
      for (c = 0; c < 16; c++)
      {
         layer = & anim->layer_list.layer[c];
         if ((layer->shadows == 0) || (layer->transparency_type_user != LAYEREFFECT_NONE))
            continue; // if shadows are disabled for this layer, or if layer has some translucent effect : no shadows

         comp = & anim->comp[d][ex->f][c];
         bmp  = comp->bmp;
         if (bmp == NULL)
            continue;

         for (y = 0; y < bmp->h; y++)
         {
            for (x = 0; x < bmp->w; x++)
            {
               if (bmp->line[y][x] == 0) // if (getpixel(bmp, x, y) == 0)
                  continue;

               oy = comp->offset_y + y - user_pivot_offset_y;
               y2 = oy * ex->shadow_height_percent / 100;

               if (ex->mirror_sp == 1)
                  ox = - comp->offset_x - x - user_pivot_offset_x;
               else
                  ox = comp->offset_x + x - user_pivot_offset_x;

               x2 = ox - (y2 * ex->shadow_skew_percent / 100);

               x2 += ex->shadow_offset_x - ex->box->left;
               y2 += ex->shadow_offset_y - ex->box->top;

               if (ex->shadow_cmap != NULL)
               {
                  tmp_color = getpixel(image, x2 , y2);
                  if (tmp_color < 0)
                     tmp_color = 0;
                  tmp_color = ex->shadow_cmap[tmp_color];
                  putpixel(image, x2, y2, tmp_color);
               }
               else
                  putpixel(image, x2, y2, ex->sh->best_fit_index);
            }
         }
      }
   }

   // ------------------------------------------------------------------------------------------------------------
   // Part 2 : draw sprite
   // ------------------------------------------------------------------------------------------------------------

   // get layer priorities for this [eventually mirrored] direction and this frame

   if (ex->mirror_sp == 1)
      idx = get_COF_direction_mirrored(anim, ex->COF_dir) * anim->nb_frames_per_direction * anim->nb_layers;
   else
      idx = ex->COF_dir * anim->nb_frames_per_direction * anim->nb_layers;
      
   idx += (ex->f * anim->nb_layers);

   for (i = 0; i < anim->nb_layers; i++)
   {
      c    = anim->priority[idx + i];
      comp = & anim->comp[d][ex->f][c];
      bmp  = comp->bmp;
      if (bmp == NULL)
         continue;

      // ----------------------------------------------------------------------------------------------------------
      layer = & anim->layer_list.layer[c];

      // prepare the special drawing mode (if any) of this layer
      cmap     = NULL; // pointer to a table of 256 colormaps (total = 256 * 256 bytes)
      deletion = FALSE;
      if (anim->pl2 != NULL)
      {
         offset = 0;
         if ((layer->transparency_type_user >= 0) && (layer->transparency_type_user < LAYEREFFECT_MAX))
            offset  = PL2_offset_for_special_effect[ layer->transparency_type_user ];
         if (offset != 0)
         {
            cmap  = anim->pl2;
            cmap += 4 * 256;
            cmap += offset * 256;
         }
         else
         {
            if (layer->transparency_type_user == LAYEREFFECT_DEL_DARK_PIXELS)
               deletion = TRUE;
         }
      }

      if (deletion == TRUE)
      {
         draw[0] = FALSE;
         for (sprite_color = 1; sprite_color < 256; sprite_color++)
         {
            if (application_palette[sprite_color].luminance >= (1.0 * layer->special_effect_level)) 
               draw[sprite_color] = TRUE;
            else
               draw[sprite_color] = FALSE;
         }
      }
      // ----------------------------------------------------------------------------------------------------------

      // draw the pixels of the layer, using the user palette shift and the layer drawing mode
      for (y = 0; y < bmp->h; y++)
      {
         for (x = 0; x < bmp->w; x++)
         {
            // get the original pixel color
            sprite_color = bmp->line[y][x]; // getpixel(bmp, x, y);

            // if background, always skip this pixel
            if (sprite_color == 0)
               continue;

            // if any, do the palette shift choosen by the user (unique monster color, ...)
            if (layer->user_palshift != NULL)
            {
               sprite_color = layer->user_palshift[sprite_color];
               if (sprite_color == 0)
                  continue;
            }

            // if deletion is active, and the color is "too dark" according to user choice, skip the pixel
            if (deletion == TRUE)
            {
               if (draw[sprite_color] == FALSE)
                  continue;
            }

            // pixel coordinates in the image

            if (ex->mirror_sp == 1)
               ox = - comp->offset_x - x - user_pivot_offset_x;
            else
               ox = comp->offset_x + x - user_pivot_offset_x;

            oy = comp->offset_y + y - user_pivot_offset_y;
            x2 = ox - ex->box->left;
            y2 = oy - ex->box->top;

            // if using a drawing mode, get the new color depending of both the background and the pixel
            if (cmap != NULL)
            {
               tmp_color = getpixel(image, x2, y2);
               if (tmp_color < 0)
                  tmp_color = 0;

               sprite_color = cmap [(sprite_color * 256) + tmp_color];
            }

            if (sprite_color == 0)
               continue;

            if (ex->bg != NULL)
            {
               if (sprite_color == ex->bg->best_fit_index)
                  sprite_color = ex->bg2;
               else if (sprite_color == ex->sh->best_fit_index)
                  sprite_color = ex->sh2;
            }

            // draw the pixel
            putpixel(image, x2, y2, sprite_color);
         }
      }
   }

   // ------------------------------------------------------------------------------------------------------------
   // Part 3 : if the image will be saved, remap background and shadows according to the user options
   // ------------------------------------------------------------------------------------------------------------
   if (ex->file != NULL)
   {
      for (i = 0; i < 256; i++)
         remap[i] = i;

      if (ex->bg->best_fit_index != ex->bg->index)
      {
         remap[ex->bg->best_fit_index] = ex->bg->index;
         remap[ex->bg->index]          = ex->bg->best_fit_index;
      }

      if (ex->sh->best_fit_index != ex->sh->index)
      {
         remap[ex->sh->best_fit_index] = ex->sh->index;
         remap[ex->sh->index]          = ex->sh->best_fit_index;
      }

      for (y = 0; y < image->h; y++)
      {
         for (x = 0; x < image->w; x++)
            putpixel(image, x, y, remap[ image->line[y][x] ]); // putpixel(image, x, y, remap[ getpixel(image, x, y) ]);
      }
   }

   // ------------------------------------------------------------------------------------------------------------
   // Part 4 : if there's a pivot image, add this image into it
   // ------------------------------------------------------------------------------------------------------------
   if (image_pivot != NULL)
   {
      if ((image->w == image_pivot->w) && (image->h == image_pivot->h)) // integrity test
      {
         if (ex->is_first_frame == 1)
            blit(image, image_pivot, 0, 0, 0, 0, image->w, image->h);
         else
         {
            for (y = 0; y < image->h; y++)
            {
               for (x = 0; x < image->w; x++)
               {
                  c = image->line[y][x];
                  if (c != ex->bg->index)
                  {
                     // pixel to add is not the background, so it's either the shadow or the sprite
                     if (c == ex->sh->index)
                     {
                        // the pixel to add is a shadow, ensure we don't owerwrite it over an existing sprite pixel
                        // (on the pivot image, all shadows must be under all the sprites pixels)
                        if (image_pivot->line[y][x] != ex->bg->index)
                           continue;
                     }

                     image_pivot->line[y][x] = c;
                  }
               }
            }
         }

         hline(image_pivot, 0,                 ex->pivot_image_y, image_pivot->w, ex->color_white);
         vline(image_pivot, ex->pivot_image_x, 0,                 image_pivot->h, ex->color_white);
      }
   }

   // ------------------------------------------------------------------------------------------------------------
   // Part 5 : if needed, write the image (and the pivot) in the provided file(s)
   // ------------------------------------------------------------------------------------------------------------
   if (ex->file != NULL)
   {
      char * name       = NULL;
      FILE * alleg_file = NULL;
      long size         = 0;
      void * buffer     = NULL;
      int  ret          = -1;


      name = (char *) calloc(1, 600);
      if (name == NULL)
         return -1;

      // backup palette
      memcpy(copypal, alleg_palette, sizeof(PALETTE));

      alleg_palette[ex->bg->index].r = ex->bg->red   >> 2;
      alleg_palette[ex->bg->index].g = ex->bg->green >> 2;
      alleg_palette[ex->bg->index].b = ex->bg->blue  >> 2;

      alleg_palette[ex->sh->index].r = ex->sh->red   >> 2;
      alleg_palette[ex->sh->index].g = ex->sh->green >> 2;
      alleg_palette[ex->sh->index].b = ex->sh->blue  >> 2;

      // Allegro lost color accuracy in the palette
      alleg_palette[ex->sh->best_fit_index].r = application_palette[ex->sh->index].r >> 2;
      alleg_palette[ex->sh->best_fit_index].g = application_palette[ex->sh->index].g >> 2;
      alleg_palette[ex->sh->best_fit_index].b = application_palette[ex->sh->index].b >> 2;

      // save allegro file
      if (ex->temp_directory != NULL)
         sprintf(name, "%s\\MergeDCCv2_image.tmp", ex->temp_directory);
      else
         sprintf(name, "C:\\MergeDCCv2_image.tmp");

      ret = -1;
      switch (ex->format)
      {
         case EXPORT_BMP : ret = save_bmp(name, image, alleg_palette); break;
         case EXPORT_PCX : ret = save_pcx(name, image, alleg_palette); break;
         case EXPORT_TGA : ret = save_tga(name, image, alleg_palette); break;
         case EXPORT_PNG : break;
         case EXPORT_BAM : break;
      }

      if ((ex->is_last_frame != 1) || (ex->file_pivot == NULL))
         memcpy(alleg_palette, copypal, sizeof(PALETTE));

      if (ret != 0)
      {
         DESTROYME(name, free)
         return -1;
      }

      alleg_file = fopen(name, "rb");
      if (alleg_file == NULL)
      {
         DESTROYME(name, free)
         return -1;
      }

      fseek(alleg_file, 0, SEEK_END);
      size = ftell(alleg_file);
      fseek(alleg_file, 0, SEEK_SET);

      buffer = malloc(size);
      if (buffer == NULL)
      {
         DESTROYME(alleg_file, fclose)
         DESTROYME(name,       free)
         return -1;
      }

      fread(buffer,  size, 1, alleg_file);
      fwrite(buffer, size, 1, ex->file);
      DESTROYME(buffer,     free)
      DESTROYME(alleg_file, fclose)

      // save allegro file for pivot
      if ((ex->is_last_frame == 1) && (ex->file_pivot != NULL))
      {
         if (ex->temp_directory != NULL)
            sprintf(name, "%s\\MergeDCCv2_image.tmp", ex->temp_directory);
         else
            sprintf(name, "C:\\MergeDCCv2_image.tmp");

         ret = -1;
         switch (ex->format)
         {
            case EXPORT_BMP : ret = save_bmp(name, image_pivot, alleg_palette); break;
            case EXPORT_PCX : ret = save_pcx(name, image_pivot, alleg_palette); break;
            case EXPORT_TGA : ret = save_tga(name, image_pivot, alleg_palette); break;
            case EXPORT_PNG : break;
            case EXPORT_BAM : break;
         }

         memcpy(alleg_palette, copypal, sizeof(PALETTE));

         if (ret != 0)
         {
            DESTROYME(name, free)
            return -1;
         }

         alleg_file = fopen(name, "rb");
         if (alleg_file == NULL)
         {
            DESTROYME(name, free)
            return -1;
         }

         fseek(alleg_file, 0, SEEK_END);
         size = ftell(alleg_file);
         fseek(alleg_file, 0, SEEK_SET);

         buffer = malloc(size);
         if (buffer == NULL)
         {
            DESTROYME(alleg_file, fclose)
            DESTROYME(name,       free)
            return -1;
         }

         fread(buffer,  size, 1, alleg_file);
         fwrite(buffer, size, 1, ex->file_pivot);
         DESTROYME(buffer,     free)
         DESTROYME(alleg_file, fclose)
      }

      DESTROYME(name, free)
   }

   return 0;
}


// ===========================================================================
// search the box datas, given the exported frames and layers, and shadow parameters
// return 0 on success
// ===========================================================================
int search_exported_animation_box(
   ANIMATION            * _anim,
   int                  * direction_order,
   int                  * frame_order,
   EXPORT_BOX           * box,
   EXPORT_PALETTE_ENTRY * bg,
   int                  * color_frequency,
   int                  mirror_sp,
   int                  shadow_height_percent,
   int                  shadow_skew_percent,
   int                  shadow_offset_x,
   int                  shadow_offset_y
)
{
   ANIMATION_PRIVATE   * anim       = (ANIMATION_PRIVATE *) _anim;
   ANIMATION_COMPONENT * comp       = NULL;
   int                 idx_d        = 0;
   int                 idx_f        = 0;
   int                 d            = 0;
   int                 f            = 0;
   int                 c            = 0;
   ANIMATION_DCC       * layer      = NULL;
   int                 x            = 0;
   int                 y            = 0;
   int                 ox           = 0;
   int                 oy           = 0;
   int                 x2           = 0;
   int                 y2           = 0;
   int                 have_shadow  = FALSE;
   int                 sprite_color = 0;
   UBYTE               * cmap       = NULL;
   int                 deletion     = FALSE;
   int                 tmp_color    = 0;
   int                 draw [256]   = {0};
   EXPORT_BOX          shadbox      = {0};
   BITMAP              * bmp        = NULL;
   int                 offset       = 0;


   if ((anim == NULL) || (direction_order == NULL) || (frame_order == NULL) || (box == NULL) || (bg == NULL) || (color_frequency == NULL))
      return -1;

   memset(color_frequency, 0, 256 * sizeof(int));

   init_export_box(box);
   init_export_box( & shadbox);

   for (idx_d = 0; (idx_d < 32) && (direction_order[idx_d] != -1); idx_d++)
   {
      d = direction_order[idx_d];
      d = get_COF_to_DCC_direction((ANIMATION *) anim, d, mirror_sp);
      for (idx_f = 0; (idx_f < 256) && (frame_order[idx_f] != -1); idx_f++)
      {
         f = frame_order[idx_f];
         for (c = 0; c < 16; c++)
         {
            comp = & anim->comp[d][f][c];
            bmp  = comp->bmp;
            if (bmp == NULL)
               continue;

            layer = & anim->layer_list.layer[c];

            // check if there's a shadow projection
            have_shadow = TRUE;
            if ((layer->shadows == 0) || (layer->transparency_type_user != LAYEREFFECT_NONE))
               have_shadow = FALSE;

            // handle drawing modes
            cmap     = NULL;
            deletion = FALSE;
            if (anim->pl2 != NULL)
            {
               offset = 0;
               if ((layer->transparency_type_user >= 0) && (layer->transparency_type_user < LAYEREFFECT_MAX))
                  offset  = PL2_offset_for_special_effect[ layer->transparency_type_user ];
               if (offset != 0)
               {
                  cmap  = anim->pl2;
                  cmap += 4 * 256;
                  cmap += offset * 256;
               }
               else
               {
                  if (layer->transparency_type_user == LAYEREFFECT_DEL_DARK_PIXELS)
                     deletion = TRUE;
               }
            }

            if (deletion == TRUE)
            {
               draw[0] = FALSE;
               for (sprite_color = 1; sprite_color < 256; sprite_color++)
               {
                  if (application_palette[sprite_color].luminance >= (1.0 * layer->special_effect_level)) 
                     draw[sprite_color] = TRUE;
                  else
                     draw[sprite_color] = FALSE;
               }
            }

            // update the export box limits
            for (y = 0; y < bmp->h; y++)
            {
               for (x = 0; x < bmp->w; x++)
               {
                  sprite_color = bmp->line[y][x]; //getpixel(bmp, x, y);

                  // if background, always skip this pixel
                  if (sprite_color == 0)
                     continue;

                  // handle here the palette shift choosen by the user (unique monster color, ...)
                  if (layer->user_palshift != NULL)
                  {
                     sprite_color = layer->user_palshift[sprite_color];
                     if (sprite_color == 0)
                        continue;
                  }

                  // if deletion is active, and the color is "too dark" according to user choice, skip the pixel
                  if (deletion == TRUE)
                  {
                     if (draw[sprite_color] == FALSE)
                        continue;
                  }

                  // drawing modes
                  if (cmap != NULL)
                  {
                     tmp_color = 0;
                     sprite_color = cmap [(sprite_color * 256) + tmp_color];
                  }
                  else if (deletion == TRUE)
                  {
                     if (draw[sprite_color] == FALSE)
                        sprite_color = 0;
                  }

                  if (sprite_color == 0)
                     continue;

                  color_frequency[sprite_color]++;

                  // update the export box

                  if (mirror_sp == 1)
                     ox = - comp->offset_x - x - user_pivot_offset_x;
                  else
                     ox = comp->offset_x + x - user_pivot_offset_x;

                  oy = comp->offset_y + y - user_pivot_offset_y;

                  if (box->left   > ox) box->left   = ox;
                  if (box->top    > oy) box->top    = oy;
                  if (box->right  < ox) box->right  = ox;
                  if (box->bottom < oy) box->bottom = oy;

                  // shadow projection
                  if (have_shadow == TRUE)
                  {
                     y2 = oy * shadow_height_percent / 100;
                     x2 = ox - (y2 * shadow_skew_percent / 100);

                     x2 += shadow_offset_x;
                     y2 += shadow_offset_y;

                     if (shadbox.left   > x2) shadbox.left   = x2;
                     if (shadbox.top    > y2) shadbox.top    = y2;
                     if (shadbox.right  < x2) shadbox.right  = x2;
                     if (shadbox.bottom < y2) shadbox.bottom = y2;
                  }
               }
            }
         }
      }
   }

   // update export box with shadow projection box
   if (box->left   > shadbox.left)   box->left   = shadbox.left;
   if (box->top    > shadbox.top)    box->top    = shadbox.top;
   if (box->right  < shadbox.right)  box->right  = shadbox.right;
   if (box->bottom < shadbox.bottom) box->bottom = shadbox.bottom;

   // update export box with the sprite pivot (must always be inside the box)
   if (box->left   > 0) box->left   = 0;
   if (box->top    > 0) box->top    = 0;
   if (box->right  < 0) box->right  = 0;
   if (box->bottom < 0) box->bottom = 0;

   // deduce width and height of the box
   box->width  = box->right  + 1 - box->left;
   box->height = box->bottom + 1 - box->top;

   return 0;
}


// ===========================================================================
// for a given COF direction, return the corresponding DCC direction
// ===========================================================================
int get_COF_to_DCC_direction(ANIMATION * _anim, int COF_direction, int mirror)
{
   static int        dir4   [4]  = {0, 1, 2, 3};
   static int        dir8   [8]  = {4, 0, 5, 1, 6, 2, 7, 3};
   static int        dir16  [16] = {4, 8, 0, 9, 5, 10, 1, 11, 6, 12, 2, 13, 7, 14, 3, 15};
   static int        dir32  [32] = {4, 16, 8, 17, 0, 18, 9, 19, 5, 20, 10, 21, 1, 22, 11, 23, 6, 24, 12, 25, 2, 26, 13, 27, 7, 28, 14, 29, 3, 30, 15, 31};

   static int        mdir4  [4]  = {3, 2, 1, 0};
   static int        mdir8  [8]  = {4, 3, 7, 2, 6, 1, 5, 0};
   static int        mdir16 [16] = {4, 15, 3, 14, 7, 13, 2, 12, 6, 11, 1, 10, 5, 9, 0, 8};
   static int        mdir32 [32] = {4, 31, 15, 30, 3, 29, 14, 28, 7, 27, 13, 26, 2, 25, 12, 24, 6, 23, 11, 22, 1, 21, 10, 20, 5, 19, 9, 18, 0, 17, 8, 16};

   ANIMATION_PRIVATE * anim      = (ANIMATION_PRIVATE *) _anim;


   if (anim == NULL)
      return 0;

   if ((COF_direction < 0) || (COF_direction >= anim->nb_directions))
      return 0;

   if (mirror == 0)
   {
      switch (anim->nb_directions)
      {
         case  4 : return dir4 [COF_direction];
         case  8 : return dir8 [COF_direction];
         case 16 : return dir16[COF_direction];
         case 32 : return dir32[COF_direction];
         default : return 0;
      }
   }
   else
   {
      switch (anim->nb_directions)
      {
         case  4 : return mdir4 [COF_direction];
         case  8 : return mdir8 [COF_direction];
         case 16 : return mdir16[COF_direction];
         case 32 : return mdir32[COF_direction];
         default : return 0;
      }
   }
}


// ===========================================================================
// just to be sure there's no problem with 'dcctypes.h'
// return 0 on success
// ===========================================================================
int test_dcc_library_integrity(void)
{
   int size = 0;


   size = sizeof(UBYTE);  if (size != 1) return 1;
   size = sizeof(WORD);   if (size != 2) return 1;
   size = sizeof(UWORD);  if (size != 2) return 1;
   size = sizeof(UDWORD); if (size != 4) return 1;

   return 0;
}


// ===========================================================================
// [ask to] analyse colors used for each layer, and for the entire animation
// ===========================================================================
void analyse_animation_colors(ANIMATION * _anim)
{
   _analyse_animation_colors((ANIMATION_PRIVATE *) _anim);
}


// ===========================================================================
// analyse colors used for each layer, and for the entire animation
// ===========================================================================
void _analyse_animation_colors(ANIMATION_PRIVATE * anim)
{
   ANIMATION_COMPONENT * comp                          = NULL;
   int                 x                               = 0;
   int                 y                               = 0;
   int                 color                           = 0;
   int                 d                               = 0;
   int                 f                               = 0;
   int                 c                               = 0;
   int                 i                               = 0;
   int                 layer_color_is_used [16]  [256] = {0};   // 0 = no, 1 = yes
   int                 anim_color_is_used  [256]       = {0};   // 0 = no, 1 = yes
   BITMAP              * bmp                           = NULL;
   ANIMATION_DCC       * layer                         = NULL;
   UBYTE               * cmap                          = NULL;  // in fact we don't really need it
   int                 deletion                        = FALSE;
   int                 offset                          = 0;     // in fact we don't really need it
   int                 draw [256]                      = {0};


   if (anim == NULL)
      return;

   memset(anim->layer_color_frequency, 0, sizeof(anim->layer_color_frequency));
   memset(anim->anim_color_frequency,  0, sizeof(anim->anim_color_frequency));

   memset(layer_color_is_used, 0, sizeof(layer_color_is_used));
   memset(anim_color_is_used,  0, sizeof(anim_color_is_used));

   // FIXME : there's a problem in this function
   //         pixels of lower layers hidden by higher layer pixels should NOT be take into account
   //         don't count invisilbe pixels !
   //         else it could lead to have 16 colors while the animation would be only 1 pixel (the same pixel position in 16 layers, but different colors)
   //         ... ==> should merge all layers first, only then should it analyse pixels

   for (d = 0; d < anim->nb_directions; d++)
   {
      for (f = 0; f < anim->nb_frames_per_direction; f++)
      {
         for (c = 0; c < 16; c++)
         {
            comp = & anim->comp[d][f][c];
            bmp  = comp->bmp;
            if (bmp == NULL)
               continue;

            // handle the drawing modes / [delete dark pixels] mode
            // ----------------------------------------------------------------------------------------------------------
            layer = & anim->layer_list.layer[c];

            // prepare the special drawing mode (if any) of this layer
            cmap     = NULL; // pointer to a table of 256 colormaps (total = 256 * 256 bytes)
            deletion = FALSE;
            if (anim->pl2 != NULL)
            {
               offset = 0;
               if ((layer->transparency_type_user >= 0) && (layer->transparency_type_user < LAYEREFFECT_MAX))
                  offset  = PL2_offset_for_special_effect[ layer->transparency_type_user ];
               if (offset != 0)
               {
                  cmap  = anim->pl2;
                  cmap += 4 * 256;
                  cmap += offset * 256;
               }
               else
               {
                  if (layer->transparency_type_user == LAYEREFFECT_DEL_DARK_PIXELS)
                     deletion = TRUE;
               }
            }

            if (deletion == TRUE)
            {
               draw[0] = FALSE;
               for (color = 1; color < 256; color++)
               {
                  if (application_palette[color].luminance >= (1.0 * layer->special_effect_level)) 
                     draw[color] = TRUE;
                  else
                     draw[color] = FALSE;
               }
            }
            // ----------------------------------------------------------------------------------------------------------

            for (y = 0; y < bmp->h; y++)
            {
               for (x = 0; x < bmp->w; x++)
               {
                  // get the original pixel color
                  color = bmp->line[y][x]; //getpixel(bmp, x, y);

                  // if background, always skip this pixel
                  if (color != 0)
                  {
                     // if any, do the palette shift choosen by the user (unique monster color, ...)
                     if (layer->user_palshift != NULL)
                        color = layer->user_palshift[color];

                     if (color != 0)
                     {
                        // if deletion is active, and the color is "too dark" according to user choice, skip the pixel
                        if (deletion == TRUE)
                        {
                           if (draw[color] == FALSE)
                              color = 0;
                        }
                     }
                  }

                  anim->layer_color_frequency[c][color]++;
                  anim->anim_color_frequency[color]++;
                  layer_color_is_used[c][color] = 1;
                  anim_color_is_used[color] = 1;
               }
            }
         }
      }
   }

   memset(anim->nb_layer_color_used, 0, sizeof(anim->nb_layer_color_used));
   for (c = 0; c < 16; c++)
   {
      for (i = 0; i < 256; i++)
      {
         if (layer_color_is_used[c][i] == 1)
            anim->nb_layer_color_used[c]++;
      }
   }

   anim->nb_color_used = 0;
   for (i = 0; i < 256; i++)
   {
      if (anim_color_is_used[i] == 1)
         anim->nb_color_used++;
   }
}


// ===========================================================================
// set a new user transparency type for a layer
// ===========================================================================
void set_layer_transparency_type_user(ANIMATION * _anim, int layer_idx, ENUM_LAYER_EFFECT_TYPE new_transparency_type_user)
{
   ANIMATION_PRIVATE * anim       = (ANIMATION_PRIVATE *) _anim;
   ANIMATION_DCC     * anim_layer = NULL;


   if (anim == NULL)
      return;

   if ((layer_idx < 0) || (layer_idx > 15))
         return;

   anim_layer = & anim->layer_list.layer[ layer_idx ];
   anim_layer->transparency_type_user = new_transparency_type_user;
}


// ===========================================================================
// set a new special effect level for a layer
// ===========================================================================
void set_layer_special_effect_level(ANIMATION * _anim, int layer_idx, int new_special_effect_level)
{
   ANIMATION_PRIVATE * anim       = (ANIMATION_PRIVATE *) _anim;
   ANIMATION_DCC     * anim_layer = NULL;


   if (anim == NULL)
      return;

   if ((layer_idx < 0) || (layer_idx > 15))
         return;

   anim_layer = & anim->layer_list.layer[ layer_idx ];
   anim_layer->special_effect_level = new_special_effect_level;
}


// ===========================================================================
// set a new colormap (palshift) for this layer
// ===========================================================================
void set_layer_colormap(ANIMATION * _anim, int layer_idx, UBYTE * palshift)
{
   ANIMATION_PRIVATE * anim       = (ANIMATION_PRIVATE *) _anim;
   ANIMATION_DCC     * anim_layer = NULL;


   if (anim == NULL)
      return;

   if ((layer_idx < 0) || (layer_idx > 15))
         return;

   anim_layer = & anim->layer_list.layer[ layer_idx ];
   anim_layer->user_palshift = palshift;
}


// ===========================================================================
// add (or replace) a layer in the animation, part 1
// ===========================================================================
void add_layer_to_animation_part1(ANIMATION * _anim, ANIMATION_DCC * new_layer, long job_ID)
{
   ANIMATION_PRIVATE   * anim       = (ANIMATION_PRIVATE *) _anim;
   int                 d            = 0;
   int                 f            = 0;
   ANIMATION_COMPONENT * comp       = NULL;
   ANIMATION_DCC       * anim_layer = NULL;
   TASK                * task_table = NULL;


   if ((anim == NULL) || (new_layer == NULL))
      return;

   if ((new_layer->layer_idx < 0) || (new_layer->layer_idx > 15))
      return;

   anim_layer = & anim->layer_list.layer[ new_layer->layer_idx ];

   anim_layer->file              = NULL;
   anim_layer->length            = 0;
   anim_layer->shadows           = 0;
   anim_layer->transparency      = 0;
   anim_layer->transparency_type = (ENUM_LAYER_EFFECT_TYPE) LAYEREFFECT_NONE;

   // free the precedent DCC / DC6 variant, if any
   DESTROYME(anim_layer->dcc, dcc_destroy)
   DESTROYME(anim_layer->dc6, dc6_destroy)

   // reinitialise the bitmaps, for the case when we won't be able to decode the DCC, or if no DCC
   for (d = 0; d < anim->nb_directions; d++)
   {
      for (f = 0; f < anim->nb_frames_per_direction; f++)
      {
         comp = & anim->comp[d][f][ new_layer->layer_idx ];
         comp->bmp      = NULL;
         comp->offset_x = 0;
         comp->offset_y = 0;
      }
   }

   // if no DCC, end
   if ((new_layer->file == NULL) || (new_layer->length <= 0))
      return;

   // load this DCC
   if (new_layer->is_dc6 == 1)
      anim_layer->dc6 = dc6_mem_load(new_layer->file, new_layer->length);
   else
      anim_layer->dcc = dcc_mem_load(new_layer->file, new_layer->length);

   if ((anim_layer->dcc == NULL) && (anim_layer->dc6 == NULL))
      return;

   // add all directions into tasks for dcc decoding threads

   task_table = dccjob_get_new_task_table(anim->nb_directions);
   if (task_table == NULL)
      return;

   for (d = 0; d < anim->nb_directions; d++)
   {
      task_table[d].dir_bitfield = 1 << d;
      if (new_layer->is_dc6 == 1)
         task_table[d].dc6 = anim_layer->dc6;
      else
         task_table[d].dcc = anim_layer->dcc;
   }

   dccjob_add_task_table(job_ID, task_table, anim->nb_directions);
}


// ===========================================================================
// add (or replace) a layer in the animation, part 2
// ===========================================================================
void add_layer_to_animation_part2(ANIMATION * _anim, ANIMATION_DCC * new_layer)
{
   ANIMATION_PRIVATE   * anim       = (ANIMATION_PRIVATE *) _anim;
   int                 d            = 0;
   int                 f            = 0;
   ANIMATION_COMPONENT * comp       = NULL;
   ANIMATION_DCC       * anim_layer = NULL;


   if ((anim == NULL) || (new_layer == NULL))
      return;

   if ((new_layer->layer_idx < 0) || (new_layer->layer_idx > 15))
         return;

   anim_layer = & anim->layer_list.layer[ new_layer->layer_idx ];

   // if no DCC, end
   if ((new_layer->file == NULL) || (new_layer->length <= 0))
      return;

   // set up animation frames for this layer
   for (d = 0; d < anim->nb_directions; d++)
   {
      for (f = 0; f < anim->nb_frames_per_direction; f++)
      {
         comp = & anim->comp[d][f][ new_layer->layer_idx ];
         if (new_layer->is_dc6 == 1)
         {
            comp->bmp      = anim_layer->dc6->frame[d][f].bmp;
            comp->offset_x = anim_layer->dc6->direction[d].box.xmin;
            comp->offset_y = anim_layer->dc6->direction[d].box.ymin;
         }
         else
         {
            comp->bmp      = anim_layer->dcc->frame[d][f].bmp;
            comp->offset_x = anim_layer->dcc->direction[d].box.xmin;
            comp->offset_y = anim_layer->dcc->direction[d].box.ymin;
         }
      }
   }

   anim_layer->is_dc6                 = new_layer->is_dc6;
   anim_layer->file                   = new_layer->file;
   anim_layer->length                 = new_layer->length;
   anim_layer->shadows                = new_layer->shadows;
   anim_layer->transparency           = new_layer->transparency;
   anim_layer->transparency_type      = new_layer->transparency_type;
   anim_layer->transparency_type_user = new_layer->transparency_type_user;
}


// ===========================================================================
// create a new ANIMATION, with decoded images
// return NULL on error
// ===========================================================================
ANIMATION * create_animation(CREATE_ANIMATION_PARAMS * params)
{
   ANIMATION_PRIVATE * anim  = NULL;
   int               i       = 0;
   ANIMATION_DCC     * layer = NULL;
   clock_t           start   = 0;
   long              job_ID  = 0;


   if (params == NULL)
      return NULL;

   if (params->layer_list == NULL)
      return NULL;

   anim = (ANIMATION_PRIVATE *) calloc(1, sizeof(ANIMATION_PRIVATE));
   if (anim == NULL)
      return NULL;

   anim->nb_directions           = params->nb_directions;
   anim->nb_frames_per_direction = params->nb_frames_per_direction;
   anim->nb_layers               = params->nb_layers;
   anim->speed                   = params->speed;
   anim->animdatad2_speed        = params->animdatad2_speed;
   anim->priority                = params->priority;
   anim->pl2                     = params->pl2;
   memcpy( & anim->layer_list, params->layer_list, sizeof(ANIMATION_LAYER_LIST));

   start  = clock();
   job_ID = get_new_job_ID();

   // for all layers, decode the first part of the images
   for (i = 0; i < 16; i++)
   {
      layer = & params->layer_list->layer[i];
      if (layer->file != NULL)
         add_layer_to_animation_part1(anim, layer, job_ID);
   }

   // wait for all the tasks of this job to be done
   dccjob_wait_for_tasks(job_ID);

   // for all layers
   for (i = 0; i < 16; i++)
   {
      layer = & params->layer_list->layer[i];
      if (layer->file != NULL)
         add_layer_to_animation_part2(anim, layer);
   }

   // update color usage statistics
   _analyse_animation_colors(anim);

   // update DCC decoding timing statistics
   params->dcc_decode_nb_clocks = clock() - start;

   return (ANIMATION *) anim;
}


// ===========================================================================
// free memory used by the ANIMATION structure
// ===========================================================================
void destroy_animation(ANIMATION * _anim)
{
   ANIMATION_PRIVATE * anim = (ANIMATION_PRIVATE *) _anim;
   int               i      = 0;


   if (anim == NULL)
      return;

   for (i = 0; i < 16; i++)
   {
      DESTROYME(anim->layer_list.layer[i].dcc, dcc_destroy)
      DESTROYME(anim->layer_list.layer[i].dc6, dc6_destroy)
   }

   DESTROYME(anim, free)
}


// ===========================================================================
// free Allegro stuff, called by atexit()
// ===========================================================================
void my_allegro_exit(void)
{
   DESTROYME(animation_buffer, destroy_bitmap)
   DESTROYME(screen_buffer,    destroy_bitmap)
      
   allegro_exit();
}


// ===========================================================================
// called 25 times per second
// ===========================================================================
void callback_25fps(void)
{
   if (timer_25fps_active == TRUE)
      current_tick_25fps++;

   if (timer_25fps_preview_active == TRUE)
      current_tick_25fps_preview++;
}
END_OF_FUNCTION(callback_25fps)


// ===========================================================================
// initialise the Allegro library, the way it is needed for this application
// return 0 on SUCCESS
// ===========================================================================
int my_allegro_init(HWND hWindow, int width, int height)
{
   if (hWindow == NULL)
      return 1;

   win_set_window(hWindow);
   if (install_allegro(SYSTEM_AUTODETECT, & errno, NULL) != 0)
      return 2;

   set_gdi_color_format(); // not sure it'll be very usefull, since there's the set_color_depth(8) right after
   set_color_depth(8);
   if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, width, height, 0, 0) != 0)
      return 3;

   if (install_timer() != 0)
      return 4;

   LOCK_VARIABLE(counter_25_ticks_per_second);
   LOCK_VARIABLE(counter_25_ticks_per_second_preview);
   LOCK_VARIABLE(timer_25fps_active);
   LOCK_VARIABLE(timer_25fps_preview_active);
   LOCK_FUNCTION(callback_25fps);
   if (install_int_ex(callback_25fps, BPS_TO_TIMER(25)) != 0)
      return 5;

   animation_window_handle = hWindow;

   animation_buffer = create_bitmap_ex(8, width, height);
   if (animation_buffer == NULL)
      return 6;

   screen_buffer = create_bitmap_ex(8, width, height);
   if (screen_buffer == NULL)
      return 7;

   dlgbox_main_preview_width  = width;
   dlgbox_main_preview_height = height;

   clear(animation_buffer);
   clear(screen_buffer);
   clear(screen);

   init_diff_rgb_tables(); // helper tables

   memset(PL2_offset_for_special_effect, 0, sizeof(PL2_offset_for_special_effect));
   PL2_offset_for_special_effect [LAYEREFFECT_25_TRANS]         =  561;
   PL2_offset_for_special_effect [LAYEREFFECT_50_TRANS]         =  305;
   PL2_offset_for_special_effect [LAYEREFFECT_75_TRANS]         =   49;
   PL2_offset_for_special_effect [LAYEREFFECT_SCREEN]           =  817;
   PL2_offset_for_special_effect [LAYEREFFECT_LUMINANCE]        = 1073;
   PL2_offset_for_special_effect [LAYEREFFECT_SPIDER_WEB_TRANS] = 1457;

   return 0;
}


// ===========================================================================
// return 1 if there's a new frame to draw
// ===========================================================================
int animation_must_be_updated(void)
{
   if (current_tick_25fps != last_tick_25fps_drawned)
      return 1;

   return 0;
}


// ===========================================================================
// start the animation
// ===========================================================================
void start_tick_25fps(int anim, int preview)
{
   if (anim != 0)
      timer_25fps_active = TRUE;

   if (preview != 0)
      timer_25fps_preview_active = TRUE;
}


// ===========================================================================
// stop the animation
// ===========================================================================
void stop_tick_25fps(int anim, int preview)
{
   if (anim != 0)
      timer_25fps_active = FALSE;

   if (preview != 0)
      timer_25fps_preview_active = FALSE;
}


// ===========================================================================
// restart the animation
// ===========================================================================
void reset_tick_25fps(void)
{
   current_tick_25fps      = 0;
   last_tick_25fps_drawned = 0;
}


// ===========================================================================
// draw the background, here squares of 2 colors
// ===========================================================================
void draw_moire(BITMAP * bmp)
{
   int n  = 60;
   int x0 = 0; // current_tick_25fps % n;
   int y0 = 0; // (current_tick_25fps / 2) % n;
   int x  = 0;
   int y  = 0;
   int c1 = 0;
   int c2 = 0;
   

   // FIXME : colors should be user choice
   c1 = makecol( 85,  85,  85); // dark gray
   c2 = makecol(170, 170, 170); // light_gray

   for (x = -x0; x < bmp->w; x += n)
   {
      for (y = -y0; y < bmp->h; y += n)
      {
         rectfill(bmp, x,     y,     x+n,   y+n,   c1);
         rectfill(bmp, x+n/2, y,     x+n,   y+n/2, c2);
         rectfill(bmp, x,     y+n/2, x+n/2, y+n,   c2);
      }
   }
}


// ===========================================================================
// draw the current frame of the animation
// called by the dlgbox_main dialog : that's the preview like in vanilla Diablo II
// ===========================================================================
void draw_current_animation_frame(ANIMATION * _anim)
{
   ANIMATION_PRIVATE   * anim       = (ANIMATION_PRIVATE *) _anim;
   unsigned long       tick         = current_tick_25fps;
   int                 pivot_x      = (dlgbox_main_preview_width / preview_zoom) / 2;
   int                 pivot_y      = (dlgbox_main_preview_height / preview_zoom) - (50 / preview_zoom);
   long                tmp_speed    = 0;
   unsigned char       frame_index  = 0;
   int                 color_black  = 0;
   int                 color_white  = 0;
   int                 color_red    = 0;
   int                 d2speedcol   = 0;
   EXPORT_PARAMETERS   ex           = {0};
   EXPORT_BOX          box          = {0};
   int                 w            = animation_buffer->w;
   int                 h            = animation_buffer->h;


   if ((animation_window_handle == NULL) || (anim == NULL))
      return;

   if (anim->force_redraw == 0)
   {
      if (tick == last_tick_25fps_drawned)
         return;
   }

   anim->force_redraw = 0;

   memset( & ex,  0, sizeof(ex));
   memset( & box, 0, sizeof(box));

   pivot_x += user_anim_offset_x + user_pivot_offset_x;
   pivot_y += user_anim_offset_y + user_pivot_offset_y;

   box.left = - pivot_x;
   box.top  = - pivot_y;

   last_tick_25fps_drawned = tick;

   tmp_speed = anim->speed;
   if (tmp_speed == 0)
      tmp_speed = 256; // 25 fps instead of no animation

   frame_index = ((tick * tmp_speed / 256) + anim->delta_frame) % anim->nb_frames_per_direction;

   color_black  = makecol(  0,   0,   0);
   color_white  = makecol(255, 255, 255);
   color_red    = makecol(255,  44,   0);

   draw_moire(animation_buffer);
   hline(animation_buffer, 0,       pivot_y, animation_buffer->w, color_white);
   vline(animation_buffer, pivot_x, 0,       animation_buffer->h, color_white);

   // prepare the call to merge_layers_of_this_frame()
   ex.file                  = NULL;
   ex.format                = EXPORT_NULL;
   ex._image                = animation_buffer;
   ex._anim                 = _anim;
   ex.COF_dir               = anim->current_direction;
   ex.f                     = frame_index;
   ex.box                   = & box;
   ex.bg                    = NULL;
   ex.sh                    = NULL;
   ex.temp_directory        = NULL;
   ex.bg2                   = 0;
   ex.sh2                   = 0;
   ex.mirror_sp             = 0;
   ex._image_pivot          = NULL;
   ex.is_first_frame        = 0;
   ex.is_last_frame         = 0;
   ex.color_white           = 0;
   ex.pivot_image_x         = 0;
   ex.pivot_image_y         = 0;
   ex.file_pivot            = NULL;
   ex.shadow_height_percent = 50;
   ex.shadow_skew_percent   = -100;
   ex.shadow_offset_x       = 0;
   ex.shadow_offset_y       = 0;
   ex.shadow_cmap           = anim->pl2 + (4 * 256) + (15 * 256);

   // draw the frame in the sprite buffer
   merge_layers_of_this_frame( & ex);

   // draw the animation at the correct zoom into the screen buffer
   stretch_blit(animation_buffer, screen_buffer, 0, 0, w, h, 0, 0, w * preview_zoom, h * preview_zoom);

   // draw texts with no zoom
   d2speedcol = color_white;

   textprintf(screen_buffer, font, 10,   5, color_white, "COF:%0.2f fps. AnimData.d2:%0.2f fps", anim->speed / 256.0 * 25.0, anim->animdatad2_speed / 256.0 * 25.0);
   textprintf(screen_buffer, font, 10,  15, color_white, "Dir:%2d [%2d]          Frm:%3d [%3d]", anim->nb_directions, anim->current_direction, anim->nb_frames_per_direction, frame_index);
   textprintf(screen_buffer, font, 10,  25, color_white, "user_pivot_offset : %d, %d", user_pivot_offset_x, user_pivot_offset_y);
/*
   textprintf(screen_buffer, font, 10,   5, color_white, "nb_directions           = %d", anim->nb_directions);
   textprintf(screen_buffer, font, 10,  15, color_white, "nb_frames_per_direction = %d", anim->nb_frames_per_direction);
   textprintf(screen_buffer, font, 10,  25, color_white, "nb_layers               = %d", anim->nb_layers);
   textprintf(screen_buffer, font, 10,  35, color_white, "COF speed               = %d", anim->speed);
   textprintf(screen_buffer, font, 10,  55, d2speedcol,  "AnimData.d2 speed       = %d", anim->animdatad2_speed);
   textprintf(screen_buffer, font, 10,  75, (frame_index == 0) ? color_red : color_white, "frame_index             = %d", frame_index);
*/

   // black border around the control
   rect(screen_buffer, 0, 0, screen_buffer->w - 1, screen_buffer->h - 1, color_black);

   // no vsync() for performance issue... blitting to screen is maybe already using an internal vsync() ?
   blit(screen_buffer, screen, 0, 0, 0, 0, screen_buffer->w, screen_buffer->h);
}


// ===========================================================================
// remove animation, display only the background
// ===========================================================================
void clear_animation(void)
{
   int color_black = makecol(0, 0, 0);


   draw_moire(animation_buffer);
   rect(animation_buffer, 0, 0, animation_buffer->w - 1, animation_buffer->h - 1, color_black);
   blit(animation_buffer, screen, 0, 0, 0, 0, animation_buffer->w, animation_buffer->h);
}


// ===========================================================================
// change the Allegro palette for displaying the animation
// ===========================================================================
void set_animation_palette(APP_PALETTE pal)
{
   int i = 0;


   for (i = 0; i < 256; i++)
   {
      alleg_palette[i].r = pal[i].r >> 2;
      alleg_palette[i].g = pal[i].g >> 2;
      alleg_palette[i].b = pal[i].b >> 2;
      if ((alleg_palette[i].r != 63) && (pal[i].r & 2)) alleg_palette[i].r++;
      if ((alleg_palette[i].g != 63) && (pal[i].g & 2)) alleg_palette[i].g++;
      if ((alleg_palette[i].b != 63) && (pal[i].b & 2)) alleg_palette[i].b++;
   }

   set_palette(alleg_palette);
   memcpy(application_palette, pal, sizeof(APP_PALETTE));
}


// ===========================================================================
// set the current animation direction
// ===========================================================================
void set_animation_direction(ANIMATION * _anim, int direction)
{
   ANIMATION_PRIVATE * anim = (ANIMATION_PRIVATE *) _anim;


   if (anim == NULL)
      return;

   if ((direction < 0) || (direction >= anim->nb_directions))
      return;

   anim->current_direction = direction;
}


// ===========================================================================
// animation direction --
// ===========================================================================
void animation_direction_minus(ANIMATION * _anim)
{
   ANIMATION_PRIVATE * anim = (ANIMATION_PRIVATE *) _anim;


   if (anim == NULL)
      return;

   anim->current_direction--;
   if (anim->current_direction < 0)
      anim->current_direction = anim->nb_directions - 1;
}


// ===========================================================================
// animation direction ++
// ===========================================================================
void animation_direction_plus(ANIMATION * _anim)
{
   ANIMATION_PRIVATE * anim = (ANIMATION_PRIVATE *) _anim;


   if (anim == NULL)
      return;

   anim->current_direction++;
   if (anim->current_direction >= anim->nb_directions)
      anim->current_direction = 0;
}


// ===========================================================================
// get animation number of directions and current direction
// ===========================================================================
void get_anim_nb_and_current_directions(ANIMATION * _anim, int * nb_directions, int * current_direction)
{
   ANIMATION_PRIVATE * anim = (ANIMATION_PRIVATE *) _anim;


   if (anim == NULL)
      return;

   if (nb_directions != NULL)
      (* nb_directions) = anim->nb_directions;

   if (current_direction != NULL)
      (* current_direction) = anim->current_direction;
}


// ===========================================================================
// return the number of distinct color used in the entire animation (0 to 256)
// ===========================================================================
int get_anim_nb_colors(ANIMATION * _anim)
{
   ANIMATION_PRIVATE * anim = (ANIMATION_PRIVATE *) _anim;


   if (anim == NULL)
      return 0;

   return anim->nb_color_used;
}


// ===========================================================================
// fix the Diablo II PL2 colormap incompatibility with the way allegro handles source pixel 0 (transparency) in colormaps
// you can test with Monster Token E9, Weapon class HTH, Mode NU
// ===========================================================================
void fix_PL2_colormaps(UBYTE * pl2)
{
   UBYTE * cmap = NULL;
   int   n      = 0;
   int   i      = 0;
   int   offset = 0;


   if (pl2 == NULL)
      return;

   for (n = 0; n < LAYEREFFECT_MAX; n++)
   {
      offset = 0;
      if ((n >= 0) && (n < LAYEREFFECT_MAX))
         offset = PL2_offset_for_special_effect[n];
      if (offset != 0)
      {
         cmap  = pl2;
         cmap += 4 * 256;
         cmap += offset * 256;
         for (i = 0; i < 256; i++)
            cmap[i] = i;
      }
   }
}


// ===========================================================================
// set the offsets by which drawing the animation or animation pivot (which impact the shadows)
// moving_type : MOVING_ANIMATION or MOVING_PIVOT
// ===========================================================================
void set_anim_user_offsets(ANIMATION * _anim, int offset_x, int offset_y, MOVING_TYPE moving_type)
{
   ANIMATION_PRIVATE * anim = (ANIMATION_PRIVATE *) _anim;


   if (anim == NULL)
      return;

   if (moving_type == MOVING_ANIMATION)
   {
      user_anim_offset_x = offset_x;
      user_anim_offset_y = offset_y;
   }
   else if (moving_type == MOVING_PIVOT)
   {
      user_pivot_offset_x = offset_x;
      user_pivot_offset_y = offset_y;
   }
}


// ===========================================================================
// get the offsets by which drawing the animation or animation pivot
// ===========================================================================
int get_anim_user_offsets(ANIMATION * _anim, int * offset_x, int * offset_y, MOVING_TYPE moving_type)
{
   ANIMATION_PRIVATE * anim = (ANIMATION_PRIVATE *) _anim;


   if ((anim == NULL) || (offset_x == NULL) || (offset_y == NULL))
      return -1;

   if (moving_type == MOVING_ANIMATION)
   {
      (* offset_x) = user_anim_offset_x;
      (* offset_y) = user_anim_offset_y;
   }
   else if (moving_type == MOVING_PIVOT)
   {
      (* offset_x) = user_pivot_offset_x;
      (* offset_y) = user_pivot_offset_y;
   }


   return 0;
}


// ===========================================================================
// build helper tables for searching the best fit color
// ===========================================================================
void init_diff_rgb_tables(void)
{
   int  i = 0;
   int  k = 0;
   long d = 0;


   for (i = 0; i < 256; i++)
   {
      for (k = 0; k < 256; k++)
      {
         d = i - k;
         d *= d;
         diff_r [i][k] = d * 30; // FIXME : should be user choice
         diff_g [i][k] = d * 59;
         diff_b [i][k] = d * 11;
      }
   }
}


// ===========================================================================
// return the color index in the palette that is the best approximation of the given RGB color.
// variable 'exceptions' is a pointer to a table of 256 int, each indicates if a color is reserved (can't be returned)
// ===========================================================================
int animation_best_fit_color_with_exceptions(int r, int g, int b, int * exceptions)
{
   int  i         = 0;
   long best_diff = 0x7FFFFFFF;
   int  best_i    = 0;
   int  ir        = 0;
   int  ig        = 0;
   int  ib        = 0;
   long idiff     = 0;


   if (exceptions == NULL)
      return 0;

   for (i = 0; i < 256; i++)
   {
      if (exceptions[i] != 0)
         continue;

      ir = application_palette[i].r;
      ig = application_palette[i].g;
      ib = application_palette[i].b;

      idiff = diff_r[r][ir] + diff_g[g][ig] + diff_b[b][ib];
      if (idiff < best_diff)
      {
         best_diff = idiff;
         best_i    = i;
      }
   }

   return best_i;
}


// ===========================================================================
// return the value in the current_tick_25fps variable
// ===========================================================================
unsigned long get_current_tick_25fps(void)
{
   return current_tick_25fps;
}


// ===========================================================================
// return the value in the current_tick_25fps_preview variable
// ===========================================================================
unsigned long get_current_tick_25fps_preview(void)
{
   return current_tick_25fps_preview;
}


// ===========================================================================
// copy an image to the application device context
// ===========================================================================
void allegro_blit_to_hdc(void * bmp, HDC dc, int sx, int sy, int dx, int dy, int w, int h)
{
   blit_to_hdc((BITMAP *) bmp, dc, sx, sy, dx, dy, w, h);
}


// ===========================================================================
// return 0 if nothing to do, or non-zero if the preview image has been updated
// ===========================================================================
int update_preview_bitmap(EXPORT_PARAMETERS * ex)
{
   static volatile int now_working   = FALSE;
   BITMAP              * bmp         = (BITMAP *) ex->_image;
   BITMAP              * bmp_control = (BITMAP *) ex->_image_control;
   char                str_dir [50]  = "";
   char                str_frm [50]  = "";


   if (now_working == TRUE)
      return 0;

   if (bmp == NULL)
      return 0;

   now_working = TRUE;

   merge_layers_of_this_frame(ex);

   sprintf(str_dir, "D %2d", ex->COF_dir);
   sprintf(str_frm, "F %3d", ex->f);

   draw_moire(bmp_control);
   blit(bmp, bmp_control, 0, 0, 1, 1, bmp->w, bmp->h);

   textprintf(bmp_control, font, 5,                   bmp_control->h - 15, ex->color_white, str_dir);
   textprintf(bmp_control, font, bmp_control->w - 45, bmp_control->h - 15, ex->color_white, str_frm);

   rect(bmp_control, 0, 0, bmp_control->w - 1, bmp_control->h - 1, 0);

   now_working = FALSE;
   return 1;
}


// ===========================================================================
// fill the image with the color 0
// ===========================================================================
void allegro_clear_bitmap(void * bmp)
{
   if (bmp != NULL)
      clear((BITMAP *) bmp);
}


// ===========================================================================
// set new zoom
// ===========================================================================
void set_new_preview_zoom(int zoom)
{
   if (zoom < 1)
      zoom = 1;
   else if (zoom > 4)
      zoom = 4;

   preview_zoom = zoom;
}


// ===========================================================================
// animation frame --
// ===========================================================================
void animation_frame_minus(ANIMATION * _anim)
{
   ANIMATION_PRIVATE * anim = (ANIMATION_PRIVATE *) _anim;


   if (anim == NULL)
      return;

   anim->delta_frame--;
}


// ===========================================================================
// animation frame ++
// ===========================================================================
void animation_frame_plus(ANIMATION * _anim)
{
   ANIMATION_PRIVATE * anim = (ANIMATION_PRIVATE *) _anim;


   if (anim == NULL)
      return;

   anim->delta_frame++;
}


// ===========================================================================
// force redraw
// ===========================================================================
void animation_force_redraw(ANIMATION * _anim)
{
   ANIMATION_PRIVATE * anim = (ANIMATION_PRIVATE *) _anim;


   if (anim == NULL)
      return;

   anim->force_redraw = 1;
}
