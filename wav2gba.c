/*****************************************************************************/
/*                                                                           */
/* GBAconv 1.00 (c) by Frederic Cambus 2002-2006                             */
/* http://gbaconv.sourceforge.net                                            */
/*                                                                           */
/* WAV to GBA Converter                                                      */
/*                                                                           */
/* Created:      2002/12/10                                                  */
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
char *input_file_buffer;
int input_file_size;
struct stat input_file_stat;

FILE *output_file;

struct wave_header
{
   char chunk_ID[4];
   unsigned int chunk_size;
   char format[4];
   char fmt_chunk[4];
   unsigned int fmt_chunk_size;
   unsigned short int audio_format;
   unsigned short int channels;
   unsigned int sample_rate;
   unsigned int byte_rate;
   unsigned short int block_align;
   unsigned short int bits_per_sample;
   char data_chunk[4];
   unsigned int data_chunk_size;
} wave_header;

int loop;



/*****************************************************************************/
/* MAIN                                                                      */
/*****************************************************************************/

int main (int argc, char *argv[])
{
   printf("-------------------------------------------------------------------------------\n");
   printf("      WAV to GBA Converter, cgbasound version\n");
   printf("-------------------------------------------------------------------------------\n\n");

   if (argc!=4)
   {
      printf("USAGE: wav2gba input.wav output.inc array_name (Input File must be 8-bit, MONO)\n\n");
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
/* CHECK THAT THE FILE IS A VALID 8-bit MONO WAV                             */
/*****************************************************************************/

   memcpy(&wave_header,input_file_buffer,44);

   if (wave_header.channels!=1)
   {
      printf("ERROR: Input File is not MONO\n\n");
      exit(-1);
   }

   if (wave_header.bits_per_sample!=8)
   {
      printf("ERROR: Input File is not 8-bit\n\n");
      exit(-1);
   }

   if (wave_header.sample_rate != 16000)
   {
      printf("ERROR: Input not 16kHz\n\n");
      exit(-1);
   }



/****************************************************************************/
/* CREATE OUTPUT FILE                                                        */
/*****************************************************************************/

   output_file=fopen(argv[2],"w");
   if (output_file==NULL)
   {
      printf("ERROR: Cannot create file %s\n\n",argv[2]);
      exit(-1);
   }

   printf("INPUT  FILE: %s (8-bit, MONO, %i Hz)\n",argv[1],wave_header.sample_rate);
   printf("OUTPUT FILE: %s\n\n",argv[2]);

   fprintf(output_file,"static const s8 %s[%u] = {\n", argv[3], input_file_size - 44 + 272);

   int line = 0;
   for (loop = 44; loop < input_file_size; loop++)
   {
      if (!line) fprintf(output_file, "\t");
      fprintf(output_file, "0x%02x,", input_file_buffer[loop]+128);
      if (line == 17) {
         line = 0;
         fputs("\n", output_file);
      } else {
         line++;
      }
   }
   if (line != 17) fputs("\n", output_file);
   fprintf(output_file,"};\n\n");

   fprintf(output_file, "static const u32 %s_size = sizeof(%s);\n", argv[3], argv[3]);


/*****************************************************************************/
/* TERMINATE PROGRAM                                                         */
/*****************************************************************************/

   fclose(output_file);
   free(input_file_buffer);

   return (0);
}
