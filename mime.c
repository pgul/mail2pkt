/* --------------------------------------------------------------------------
 * MAIL-TO-PKT v0.2                                           Jan 22nd, 2000
 * --------------------------------------------------------------------------
 *
 *   This program is a procmail filter to automatically decode FTN packets
 *   from BASE64 encoded email attachments.
 *
 *   Copyright (C) 1999-2000  German Theler
 *       Email: kuroshivo@bigfoot.com
 *        Fido: 4:905/210
 *
 *
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation version 2.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *                        
 * --------------------------------------------------------------------------
 */

char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
                  "0123456789+/";

int cvt_ascii( unsigned char alpha )
{
   if      ( (alpha >= 'A') && (alpha <= 'Z') ) return (int)(alpha - 'A');
   else if ( (alpha >= 'a') && (alpha <= 'z') )
        return 26 + (int)(alpha - 'a');
   else if ( (alpha >= '0') && (alpha <= '9' ) )
        return 52 + (int)(alpha - '0');
   else if ( alpha == '+' ) return 62;
   else if ( alpha == '/' ) return 63;
   else if ( alpha == '=' ) return -2;
   else                     return -1;
}


int getFile(char *name, FILE *file)
{
    FILE *out;
    int c;
    unsigned char blivit;
    unsigned long accum = 0;
    unsigned long value;
    char buff[80];
    int decode_state = 0;
    int shift = 0;
    int index;
    int quit = 0;
    int cycle_flag = 0;

    if ((out = fopen(name, "wb")) == NULL)
        return -2;

    while ((c = fgetc(file)) != '-') {
        ungetc(c, file);
        fgets(buff, 80, file);

        if (feof(file))
            return -1;
        else {
            cycle_flag = 1;

            if ((decode_state == 1) && ((buff[0] == '\n') || (buff[0] < '+')))
               return -1;
        }

        if (decode_state == 0) {
            for (index = 0; (buff[index] != '\n') && (buff[index] != '\0') && (decode_state >= 0); index++ ) {
                if (((buff[index] >= 'A') && (buff[index] <= 'Z')) ||
                   ((buff[index] >= 'a') && (buff[index] <= 'z')) ||
                   ((buff[index] >= '0') && (buff[index] <= '9')) ||
                   (buff[index] == '+') || (buff[index] == '/') ||
                   (buff[index] == '=')) {
                    decode_state = 1;
                } else {
                    return -3;
                }
            }
        }

        if (quit != 0)
            buff[0] = '\0';

        for (index = 0; (buff[index] != '\n') && (buff[index] != '\0'); index++) {
            value = cvt_ascii(buff[index]);

            if (value < 64) {
                accum <<= 6;
                shift += 6;
                accum |= value;
                if (shift >= 8) {
                    shift -= 8;
                    value = accum >> shift;
                    blivit = (unsigned char)value & 0xFFl;
                    fputc(blivit, out);
                }
            } else {
                quit = 1;
                break;
            }
        }
    }

   fclose(out);

   return 0;

}
