/*****************************************************************************/
/*                                                                           */
/* GBAconv 1.00 (c) by Frederic Cambus 2002-2006                             */
/* http://gbaconv.sourceforge.net                                            */
/*                                                                           */
/* PCX to GBA Converter                                                      */
/*                                                                           */
/* Created:      2002/12/09                                                  */
/* Last Updated: 2006/01/13                                                  */
/*                                                                           */
/* This program is free software; you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation; either version 2 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                           */
/*****************************************************************************/

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *input_file;
unsigned char *input_file_buffer;
int input_file_size;
struct stat input_file_stat;

FILE *output_file;

struct pcx_header
{
   char ID;
   char version;
   char encoding;
   char bits_per_pixel;
   short int x_min;
   short int y_min;
   short int x_max;
   short int y_max;
   short x_resolution;
   short y_resolution;
   char palette[48];
   char reserved;
   char number_of_bit_planes;
   short bytes_per_line;
   short palette_type;
   short x_screen_size;
   short y_screen_size;
   char filler[54];
} pcx_header;

unsigned char pcx_image_palette[768];
unsigned char *pcx_buffer;
int pcx_buffer_size;

int loop;

int current_byte;
int offset;
int run_count;
int run_position;



/*****************************************************************************/
/* MAIN                                                                      */
/*****************************************************************************/

int main (int argc, char *argv[])
{
   printf("-------------------------------------------------------------------------------\n");
   printf("      PCX to GBA Converter - GBAconv 1.00 (c) by Frederic Cambus 2002-2006\n");
   printf("-------------------------------------------------------------------------------\n\n");

   if (argc!=4)
   {
      printf("USAGE: pcx2gba input.pcx output.inc array_name (Input File must be 8-bpp PCX)\n\n");
      exit(0);
   }



/*****************************************************************************/
/* LOAD INPUT FILE                                                           */
/*****************************************************************************/

   stat (argv[1], &input_file_stat);
   input_file_size=input_file_stat.st_size;

   input_file_buffer=malloc(input_file_size);

   if (input_file_buffer==NULL)
   {
      printf("ERROR: Cannot allocate memory\n\n");
      exit(-1);
   }

   input_file=fopen(argv[1],"rb");
   if (input_file==NULL)
   {
      printf("ERROR: Cannot open file %s\n\n",argv[1]);
      exit(-1);
   }

   fread(input_file_buffer,input_file_size,1,input_file);
   fclose(input_file);



/*****************************************************************************/
/* CHECK THAT THE FILE IS A VALID 8-bpp PCX                                  */
/*****************************************************************************/

   memcpy(&pcx_header,input_file_buffer,128);

   if (pcx_header.bits_per_pixel!=8)
   {
      printf("ERROR: Input File is not 8-bpp\n\n");
      exit(-1);
   }



/*****************************************************************************/
/* UNCOMPRESS RLE ENCODED PCX INPUT FILE                                     */
/*****************************************************************************/

   pcx_buffer_size=(pcx_header.x_max+1)*(pcx_header.y_max+1);
   pcx_buffer=malloc(pcx_buffer_size);

   while (loop<input_file_size-768-128)
   {
      current_byte=input_file_buffer[loop+128];

      if (current_byte>192)
      {
         run_count=current_byte-192;

         for (run_position=0;run_position<run_count;run_position++)
         {
            pcx_buffer[offset+run_position]=input_file_buffer[loop+128+1];
         }
         offset+=run_count;
         loop+=2;
      }
      else
      {
         pcx_buffer[offset]=current_byte;
         offset++;
         loop++;
      }
   }

   for (loop=0;loop<768;loop++)
   {
      pcx_image_palette[loop]=(input_file_buffer[input_file_size-768+loop]/8);
   }



/*****************************************************************************/
/* CREATE OUTPUT FILE                                                        */
/*****************************************************************************/

   output_file=fopen(argv[2],"w");
   if (output_file==NULL)
   {
      printf("ERROR: Cannot create file %s\n\n",argv[2]);
      exit(-1);
   }

   printf("INPUT  FILE: %s (%ix%ix%i-bpp)\n",argv[1],pcx_header.x_max+1,pcx_header.y_max+1,pcx_header.bits_per_pixel);
   printf("OUTPUT FILE: %s\n\n",argv[2]);

   fprintf(output_file,"const u16 %s_palette[] = {\n", argv[3]);

   for (loop=0;loop<256;loop++)
   {
      fprintf(output_file,"0x%x,",(pcx_image_palette[loop*3] | pcx_image_palette[(loop*3)+1]<<5 | pcx_image_palette[(loop*3)+2]<<10));
   }

   fseek(output_file,ftell(output_file)-1,0);
   fprintf(output_file,"};\n\n");

   fprintf(output_file,"const u16 %s[] = {\n", argv[3]);

   for (loop=0;loop<pcx_buffer_size/2;loop++)
   {
      fprintf(output_file,"0x%x,",(pcx_buffer[loop*2] | pcx_buffer[(loop*2)+1]<<8));
   }

   fseek(output_file,ftell(output_file)-1,0);
   fprintf(output_file,"};\n");



/*****************************************************************************/
/* TERMINATE PROGRAM                                                         */
/*****************************************************************************/

   fclose(output_file);
   free(input_file_buffer);

   printf("Successfully created file %s\n\n",argv[2]);

   return (0);
}
