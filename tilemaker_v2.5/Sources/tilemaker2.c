/* =====================================
   TileMaker v 2.5, Freeware
   By Paul SIRAMY
   19 August 2004
   quick but hopefuly not-so-dirty style
   ===================================== */

#include <stdio.h>

#define USE_CONSOLE
#include "allegro.h"

//#define MAX_SUB_INDEX 64
#define MAX_SUB_INDEX 256


// defines
#define UBYTE  unsigned char
#define WORD   short int
#define UWORD  unsigned short int
#define UDWORD unsigned long

#define FALSE 0
#define TRUE -1

// structs
#pragma pack(1)
typedef struct 
{
   UWORD  x;
   UWORD  y;
   UWORD  dummy_1;
   UBYTE  grid_x;
   UBYTE  grid_y;
   UWORD  format;
   UDWORD encoding_length;
   UWORD  dummy_2;
   long   pixels_file_pointer;
} BLOCK_DATA_S;
#pragma pack()

typedef struct TILE_DATA_S
{
   int                ds1_x;
   int                ds1_y;
   int                main_index;
   int                sub_index;
   int                nb_block;
   int                blocks_size;
   BLOCK_DATA_S       * block[25];         // table of pointer to BLOCK_DATA_S
   UBYTE              * encoding_data[25]; // table of pointers to buffers
   long               block_file_pointer;
   struct TILE_DATA_S * prec,
                      * next;
} TILE_DATA_S;

// global datas
TILE_DATA_S * first_tile = NULL,
            * last_tile  = NULL;
BITMAP      * bmp_area   = NULL,
            * bmp_tiles  = NULL;
PALETTE     pal_temp;
char        str_tmp[512];
char        dt1_filename[512];
int         starting_main_index, ds1_width = 0, act, floor_layer;
long        tile_count = 0;
int         xjump[15] = {14, 12, 10, 8, 6, 4, 2, 0, 2, 4, 6, 8, 10, 12, 14},
            nbpix[15] = {4, 8, 12, 16, 20, 24, 28, 32, 28, 24, 20, 16, 12, 8, 4};
struct
{
   int x;
   int y;
} sub_tile_coord[25];


// ==================================================================================
void make_sub_coord(void)
{
   int xs, ys, a, b, i, x, y;


   xs = ys = 64;
   for (a=0; a < 5; a++)
   {
      x = xs;
      y = ys;
      for (b=0; b < 5; b++)
      {
         i = a * 5 + b;
         sub_tile_coord[i].x = x;
         sub_tile_coord[i].y = y;
         x -= 16;
         y -=  8;
      }
      xs += 16;
      ys -=  8;
   }
}


// ==================================================================================
void my_exit(void)
{
   TILE_DATA_S * ptr, * next;
   int         i, x, y, found;


   if (strlen(allegro_error) > 0)
   {
      printf("%s", allegro_error);
      fflush(stdout);
   }

   if (bmp_area  != NULL) destroy_bitmap(bmp_area);
   if (bmp_tiles != NULL) destroy_bitmap(bmp_tiles);

   if (first_tile != NULL)
   {
      printf("\nIn the DS1 of %i*%i tiles, here are the Floor tiles from the image:\n", ds1_width, ds1_width);
      if (floor_layer == 2)
         printf("(Tiles are on Floor Layer # 2 in the ds1)\n");
      for (y=0; y < ds1_width; y++)
      {
         for (x=0; x < ds1_width; x++)
         {
            ptr = first_tile;
            found = FALSE;
            while ((ptr != NULL) && (found == FALSE))
            {
               if ((ptr->ds1_x == x) && (ptr->ds1_y == y))
               {
                  if (ptr->nb_block == 0)
                     printf("0");
                  else
                     printf("*");
                  found = TRUE;
               }
               ptr = ptr->next;
            }
            if (found == FALSE)
               printf("-");
         }
         printf("\n");
      }
   }

   // free tile list
   ptr = first_tile;
   while (ptr != NULL)
   {
      for (i=0; i < 25; i++)
      {
         if (ptr->block[i] != NULL)
            free(ptr->block[i]);
         if (ptr->encoding_data[i] != NULL)
            free(ptr->encoding_data[i]);
      }
      next = ptr->next;
      free(ptr);
      ptr = next;
   }
}


// ==================================================================================
void add_tile(void)
{
   TILE_DATA_S * ptr;
   int         size;


   // new buffer in memory
   size = sizeof(TILE_DATA_S);
   ptr = (TILE_DATA_S *) malloc(size);
   if (ptr == NULL)
   {
      printf("error : can't allocate %i bytes for tile # %li\n", size, tile_count);
      exit(1);
   }
   memset(ptr, 0x00, size);

   // insert to list
   ptr->next = first_tile;
   if (first_tile != NULL)
      first_tile->prec = ptr;
   first_tile = ptr;

   // update tile count
   tile_count++;
}


// ==================================================================================
int block_buffer_count(int x0, int y0)
{
   int x, y, c, nb_solid, buffer_length, row, x_start, x_end;


   buffer_length = 0;
   row           = 0;
   x_start       = xjump[row];
   x_end         = xjump[row] + nbpix[row];
   x             = 0;
   y             = 0;
   while (y < 15)
   {
      // nb_trans
      if ((x < x_start) || (x >= x_end))
         c = 0;
      else
         c = getpixel(bmp_area, x0 + x, y0 + y);
      if (c <= 0)
      {
         x++;
         while ((x < 32) && (
            ((x < x_start) || (x >= x_end) ? 0 : getpixel(bmp_area, x0 + x, y0 + y)) <= 0))
         {
            x++;
         }
      }

      if (x < 32)
      {
         buffer_length++; // nb_trans

         // nb_solid
         nb_solid = 1;
         x++;
         while ((x < 32) && (
            ((x < x_start) || (x >= x_end) ? 0 : getpixel(bmp_area, x0 + x, y0 + y)) > 0))
         {
            x++;
            nb_solid++;
         }
         buffer_length += 1 + nb_solid;
      }

      // EOL
      if (x >= 32)
      {
         buffer_length += 2;
         x = 0;
         y++;
         row++;
         x_start = xjump[row];
         x_end   = xjump[row] + nbpix[row];
      }
   }

   // end
   return buffer_length;
}


// ==================================================================================
void block_buffer_fill(int x0, int y0, UBYTE * ptr)
{
   int x, y, c, nb_trans, nb_solid, buffer_length, row, x_start, x_end;


   buffer_length = 0;
   row           = 0;
   x_start       = xjump[row];
   x_end         = xjump[row] + nbpix[row];
   x             = 0;
   y             = 0;
   while (y < 15)
   {
      // nb_trans
      if ((x < x_start) || (x >= x_end))
         c = 0;
      else
         c = getpixel(bmp_area, x0 + x, y0 + y);
      nb_trans = 0;
      if (c <= 0)
      {
         x++;
         nb_trans = 1;
         while ((x < 32) && (
            ((x < x_start) || (x >= x_end) ? 0 : getpixel(bmp_area, x0 + x, y0 + y)) <= 0))
         {
            x++;
            nb_trans++;
         }
      }

      if (x < 32)
      {
         // put nb_trans
         * ptr = nb_trans;
         ptr++;

         // nb_solid
         nb_solid = 1;
         x++;
         while ((x < 32) && (
            ((x < x_start) || (x >= x_end) ? 0 : getpixel(bmp_area, x0 + x, y0 + y)) > 0))
         {
            x++;
            nb_solid++;
         }

         // put nb_solid
         * ptr = nb_solid;
         ptr++;

         // put solid pixels
         x -= nb_solid;
         while (nb_solid)
         {
            * ptr = getpixel(bmp_area, x0 + x, y0 + y);
            ptr++;
            x++;
            nb_solid--;
         }
      }

      // EOL
      if (x >= 32)
      {
         // put EOL codes
         * ptr = 0;
         ptr++;
         * ptr = 0;
         ptr++;

         // next row
         x = 0;
         y++;
         row++;
         x_start = xjump[row];
         x_end   = xjump[row] + nbpix[row];
      }
   }

   // end
}


// ==================================================================================
void make_tiles_list(void)
{
   int done, nx, ny, ds1_startx, ds1_starty, ds1_x, ds1_y, i;
   int has_pix, has_trans;
   int tile_x, tile_y, row, col, subtile_x, subtile_y, x, y, c;
   int main_index, sub_index;
   int nb_subtile, size, interlace;

   BLOCK_DATA_S * block_ptr;
   UBYTE        * u_ptr;


   tile_x     = -80;
   tile_y     = -40;
   interlace  = 0;
   main_index = starting_main_index;
   sub_index  = 0;

   // nb tiles width
   nx = (80 + bmp_area->w) / 160;
   if ( (80 + bmp_area->w) % 160)
      nx++;

   // nb tiles height
   ny = (40 + bmp_area->h) / 80;
   if ( (40 + bmp_area->h) % 80)
      ny++;

   // ds1 width and height (the ds1 will be a square)
   // the ds1 will have an empty row and an empty column, this is in purpose
   ds1_width = nx + ny;

   // ds1 position of 1st tile
   ds1_x = ds1_startx = 0;
   ds1_y = ds1_starty = nx - 1;


   // for all tiles, image order
   done = FALSE;
   while ( ! done)
   {
      // avoid 2 buggy tiles
      if (main_index == 30)
      {
         if ((sub_index == 0) || (sub_index == 1))
            sub_index = 2;
      }

      // add a tile in memory
      add_tile();

      // process the 25 sub-tiles
      nb_subtile = 0;
      for (i=0; i < 25; i++)
      {
         subtile_x = tile_x + sub_tile_coord[i].x;
         subtile_y = tile_y + sub_tile_coord[i].y;

         // does this sub-tile has at least 1 pixel, and/or at least 1 transparent pixel ?
         has_trans = has_pix = FALSE;
         y = subtile_y;
         for (row = 0; row < 15; row++)
         {
            x = subtile_x + xjump[row];
            for (col = 0; col < nbpix[row]; col++)
            {
               c = getpixel(bmp_area, x, y);
               if (c <= 0)
                  has_trans = TRUE;
               else if (c > 0)
                  has_pix = TRUE;
               if ((has_trans == TRUE) && (has_pix == TRUE))
                  goto ENCODE_THIS_SUBTILE;

               // next pixel
               x++;
            }

            // next row
            y++;
         }

         ENCODE_THIS_SUBTILE:
         if (has_pix == FALSE)
         {
            // empty sub-tile, just skip it
         }
         else
         {
            // new block
            size = sizeof(BLOCK_DATA_S);
            block_ptr = (BLOCK_DATA_S *) malloc(size);
            if (block_ptr == NULL)
            {
               printf("error : can't allocate %i bytes for sub-tile # %i of tile # %li\n",
                  size, nb_subtile, tile_count);
               exit(1);
            }
            memset(block_ptr, 0x00, size);
            first_tile->block[nb_subtile] = block_ptr;

            // fill block datas
            block_ptr->x      = sub_tile_coord[i].x;
            block_ptr->y      = sub_tile_coord[i].y;
            block_ptr->grid_x = 4 - (i % 5);
            block_ptr->grid_y = 4 - (i / 5);

            // encode it
            if (has_trans == TRUE)
            {
               // format = RLE
               block_ptr->format = 0x2005;

               // 1st pass : encoding length
               block_ptr->encoding_length = block_buffer_count(subtile_x, subtile_y);

               // buffer malloc
               u_ptr = (UBYTE *) malloc(block_ptr->encoding_length);
               if (u_ptr == NULL)
               {
                  printf("error : can't allocate %i bytes for RLE encoding "
                         "of sub-tile # %i of tile # %li\n",
                         block_ptr->encoding_length, nb_subtile, tile_count
                  );
                  exit(1);
               }
               first_tile->encoding_data[nb_subtile] = u_ptr;

               // 2nd pass : actual encoding
               block_buffer_fill(subtile_x, subtile_y, u_ptr);
            }
            else
            {
               // format = RAW
               block_ptr->format          = 0x0001;
               block_ptr->encoding_length = 256;

               u_ptr = (UBYTE *) malloc(256);
               if (u_ptr == NULL)
               {
                  printf("error : can't allocate 256 bytes for RAW encoding "
                         "of sub-tile # %i of tile # %li\n",
                         nb_subtile, tile_count
                  );
                  exit(1);
               }
               first_tile->encoding_data[nb_subtile] = u_ptr;

               // read the 256 pixels
               y = subtile_y;
               for (row = 0; row < 15; row++)
               {
                  x = subtile_x + xjump[row];
                  for (col = 0; col < nbpix[row]; col++)
                  {
                     c = getpixel(bmp_area, x, y);
                     if (c < 0)
                        c = 0;
                     * u_ptr = c;
                     u_ptr++;
                     x++;
                  }
                  y++;
               }
            }

            // count this sub-tile
            first_tile->blocks_size += (block_ptr->encoding_length + 20);
            nb_subtile++;
         }
      }

      // fill this tile memory datas
      first_tile->ds1_x        = ds1_x;
      first_tile->ds1_y        = ds1_y;
      first_tile->main_index   = main_index;
      first_tile->sub_index    = sub_index;
      first_tile->nb_block     = nb_subtile;

      // next tile
      tile_x += 160;
      if (tile_x >= bmp_area->w)
      {
         interlace = 1 - interlace;
         if (interlace)
         {
            tile_x = 0;
            ds1_startx++;
         }
         else
         {
            tile_x = -80;
            ds1_starty++;
         }
         ds1_x = ds1_startx;
         ds1_y = ds1_starty;

         tile_y += 40;
         if (tile_y >= bmp_area->h)
            done = TRUE;
      }
      else
      {
         ds1_x++;
         ds1_y--;
      }

      sub_index++;
      if (sub_index >= MAX_SUB_INDEX)
      {
         main_index++;
         sub_index = 0;
      }
   }
}


// ==================================================================================
void make_dt1(FILE * dt1_out)
{
   TILE_DATA_S  * tile  = NULL;
   BLOCK_DATA_S * block = NULL;
   long         nb_tile, dw, file_pointer;
   WORD         w, i;


   tile = first_tile;
   nb_tile = 0;
   while (tile != NULL)
   {
      last_tile = tile;
      if (tile->nb_block > 0)
         nb_tile++;
      tile = tile->next;
   }

   // dt1 header
   dw = 7;
   fwrite( & dw, 4, 1, dt1_out);

   dw = 6;
   fwrite( & dw, 4, 1, dt1_out);

   memset(str_tmp, 0x00, 260);
   fwrite(str_tmp, 260, 1, dt1_out);

   fwrite( & nb_tile, 4, 1, dt1_out);

   dw = 276;
   fwrite( & dw, 4, 1, dt1_out);

   // prepare tile headers
   file_pointer = 276 + (96 * nb_tile);
   tile         = last_tile;
   while (tile != NULL)
   {
      if (tile->nb_block > 0)
      {
         tile->block_file_pointer = file_pointer;
         file_pointer += tile->blocks_size;
      }
      tile = tile->prec;
   }

   // write tile headers
   tile = last_tile;
   while (tile != NULL)
   {
      if (tile->nb_block > 0)
      {
         // Direction
         dw = 3;
         fwrite( & dw, 4, 1, dt1_out);

         // Roof height
         w = 0;
         fwrite( & w, 2, 1, dt1_out);

         // Sound Index
         fputc(1, dt1_out);

         // Flag : Animated
         fputc(0, dt1_out);

         // Height
         dw = -128;
         fwrite( & dw, 4, 1, dt1_out);

         // Width
         dw = 160;
         fwrite( & dw, 4, 1, dt1_out);

         // zeros
         dw = 0;
         fwrite( & dw, 4, 1, dt1_out);

         // Orientation
         dw = 0;
         fwrite( & dw, 4, 1, dt1_out);

         // Main Index
         dw = tile->main_index;
         fwrite( & dw, 4, 1, dt1_out);

         // Sub Index
         dw = tile->sub_index;
         fwrite( & dw, 4, 1, dt1_out);

         // Rarity / Frame Index
         dw = 0;
         fwrite( & dw, 4, 1, dt1_out);

         // unknown, but can cause crash if use same values of other D2 dt1
         fputc(0, dt1_out);
         fputc(0, dt1_out);
         fputc(0, dt1_out);
         fputc(0, dt1_out);

         // Sub-Tile Flags (walkable infos and such)
         for (i=0; i < 25; i++)
            fputc(0x00, dt1_out);

         // zeros
         for (i=0; i < 7; i++)
            fputc(0x00, dt1_out);

         // Block Header Pointer
         dw = tile->block_file_pointer;
         fwrite( & dw, 4, 1, dt1_out);

         // Block Header Length
         dw = tile->blocks_size;
         fwrite( & dw, 4, 1, dt1_out);

         // Number of Blocks
         dw = tile->nb_block;
         fwrite( & dw, 4, 1, dt1_out);

         // zeros
         for (i=0; i < 12; i++)
            fputc(0x00, dt1_out);
      }
      tile = tile->prec;
   }

   // prepare blocks datas
   tile = last_tile;
   file_pointer = 276 + (96 * nb_tile);
   while (tile != NULL)
   {
      if (tile->nb_block > 0)
      {
         file_pointer += (20 * tile->nb_block);
         for (i=0; i < tile->nb_block; i++)
         {
            block = tile->block[i];
            block->pixels_file_pointer = (file_pointer - tile->block_file_pointer);
            file_pointer += block->encoding_length;
         }
      }
      tile = tile->prec;
   }

   // write block headers and blocks pixels datas
   tile = last_tile;
   while (tile != NULL)
   {
      if (tile->nb_block > 0)
      {
         // blocks header
         for (i=0; i < tile->nb_block; i++)
         {
            block = tile->block[i];
            fwrite(block, 20, 1, dt1_out);
         }

         // blocks pixels datas
         for (i=0; i < tile->nb_block; i++)
         {
            block = tile->block[i];
            fwrite(tile->encoding_data[i], block->encoding_length, 1, dt1_out);
         }
      }
      tile = tile->prec;
   }
}


// ==================================================================================
void make_ds1(FILE * out)
{
   TILE_DATA_S  * tile  = NULL;
   long         dw;
   char         * text = "made by TileMaker 2.4";
   int          found, x, y;


   // ds1 header
   dw = 18; // version
   fwrite( & dw, 4, 1, out);

   dw = ds1_width - 1; // width & height
   fwrite( & dw, 4, 1, out);
   fwrite( & dw, 4, 1, out);

   dw = act - 1; // act
   fwrite( & dw, 4, 1, out);

   dw = 0; // substitution type
   fwrite( & dw, 4, 1, out);

   dw = 2; // # string files
   fwrite( & dw, 4, 1, out);
   fwrite(dt1_filename, 1 + strlen(dt1_filename), 1, out);
   fwrite(text, 1 + strlen(text), 1, out);

   dw = 1; // # walls
   fwrite( & dw, 4, 1, out);

   dw = floor_layer; // # floors
   fwrite( & dw, 4, 1, out);

   // walls
   dw = 0;
   // props
   for (y=0; y < ds1_width; y++)
   {
      for (x=0; x < ds1_width; x++)
         fwrite( & dw, 4, 1, out);
   }

   // orientation
   for (y=0; y < ds1_width; y++)
   {
      for (x=0; x < ds1_width; x++)
         fwrite( & dw, 4, 1, out);
   }

   // floors
   if (floor_layer == 2)
   {
      dw = 0;
      for (y=0; y < ds1_width; y++)
      {
         for (x=0; x < ds1_width; x++)
            fwrite( & dw, 4, 1, out);
      }
   }

   for (y=0; y < ds1_width; y++)
   {
      for (x=0; x < ds1_width; x++)
      {
         tile = first_tile;
         found = FALSE;
         while ((tile != NULL) && (found == FALSE))
         {
            if ((tile->nb_block > 0) && (tile->ds1_x == x) && (tile->ds1_y == y))
            {
               found = TRUE;
               dw  = 194; // prop1
               dw |= tile->sub_index  << 8;
               dw |= tile->main_index << 20;
            }
            else
               tile = tile->next;
         }
         if (found == FALSE)
            dw = 0;
         fwrite( & dw, 4, 1, out);
      }
   }

   // shadows
   dw = 0;
   for (y=0; y < ds1_width; y++)
   {
      for (x=0; x < ds1_width; x++)
         fwrite( & dw, 4, 1, out);
   }

   dw = 0; // objects
   fwrite( & dw, 4, 1, out);

   dw = 0; // paths
   fwrite( & dw, 4, 1, out);
}


// ==================================================================================
int main(int argc, char ** argv)
{
   FILE * dt1_out = NULL, * ds1_out;
   int  n;


   allegro_init();

   atexit(my_exit);

   printf("TileMaker v2.5\n"
          "==============\n"
   );
   if (argc < 4)
   {
      printf("syntaxe : tilemaker2 <image filename> <main-index start> <act> [<floor layer>]\n");
      return 1;
   }

   set_color_depth(8); // normaly useless, but just to be sure
   make_sub_coord();

   // get main index
   starting_main_index = atoi(argv[2]);
   if ((starting_main_index < 0) || (starting_main_index > 63))
   {
      printf("error : main index value must be within 0 and 63 (asked : %i)\n",
         starting_main_index);
      fflush(stdout);
      exit(1);
   }

   // get act
   act = atoi(argv[3]);
   if ((act < 1) || (act > 5))
   {
      printf("error : act value must be within 1 and 5 (asked : %i)\n", act);
      fflush(stdout);
      exit(1);
   }

   // get floor layer where to put extracted tiles
   if (argc >= 5)
   {
      floor_layer = atoi(argv[4]);
      if ((floor_layer < 1) || (floor_layer > 2))
      {
         printf("error : floor layer value must be 1 or 2 (asked : %i)\n", floor_layer);
         fflush(stdout);
         exit(1);
      }
   }
   else
      floor_layer = 1;

   // load image to split
   bmp_area = load_bitmap(argv[1], pal_temp);
   if (bmp_area == NULL)
   {
      printf("error : can't load \"%s\"\n", argv[1]);
      fflush(stdout);
      exit(1);
   }

   // make dt1 name
   strcpy(dt1_filename, argv[1]);
   n = strlen(dt1_filename) - 4;
   if (n > 4)
   {
      dt1_filename[n] = 0x00;
      strcat(dt1_filename, ".dt1");
   }
   else
   {
      strcpy(dt1_filename, "tilemaker2_tiles.dt1");
   }

   // make tiles in memory
   printf("spliting image into tiles...\n");
   fflush(stdout);
   make_tiles_list();

   // write dt1 file
   printf("creating dt1 : \"%s\"\n", dt1_filename);
   fflush(stdout);
   dt1_out = fopen(dt1_filename, "wb");
   if (dt1_out == NULL)
   {
      printf("error : can't write \"%s\"\n", dt1_filename);
      fflush(stdout);
      exit(1);
   }
   make_dt1(dt1_out);
   fclose(dt1_out);

   // make ds1 name
   strcpy(str_tmp, argv[1]);
   n = strlen(str_tmp) - 4;
   if (n > 4)
   {
      str_tmp[n] = 0x00;
      strcat(str_tmp, ".ds1");
   }
   else
   {
      strcpy(str_tmp, "tilemaker2_tiles.ds1");
   }

   // write ds1 file
   printf("creating ds1 : \"%s\"\n", str_tmp);
   fflush(stdout);
   ds1_out = fopen(str_tmp, "wb");
   if (ds1_out == NULL)
   {
      printf("error : can't write \"%s\"\n", str_tmp);
      fflush(stdout);
      exit(1);
   }
   make_ds1(ds1_out);
   fclose(ds1_out);

   // end
   return 0;
}
END_OF_MAIN();
