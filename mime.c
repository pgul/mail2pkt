/* --------------------------------------------------------------------------
 * MAIL-TO-PKT v0.2                                            Apr 6th, 2000
 * --------------------------------------------------------------------------
 *
 *   This file is part of mail2pkt, and contains mime base64 decoding
 *   routines.
 *   This is the HUSKY-DEPENDANT version.
 *   Get the latest version from http://husky.physcip.uni-stuttgart.de
 *
 *   Copyright (C) 1999-2000  German Theler
 *       Email: german@linuxfreak.com
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

#include "mime.h"

int fromBase64(char *name, FILE *from)
{
    FILE *to;
    char buffer[255];
    int i;
    int a, b, c, d;
    int x, y, z;

    if ((to = fopen(name, "wb")) == NULL)
        return -1;

    while ((i = getc(from)) != '\n') {
        ungetc(i, from);
        fgets(buffer, 254, from);

        for (i = 0; (buffer[i] != '\n') && (buffer[i] != '\0'); i += 4) {
            a = strchr(base64, buffer[i]) - base64;
            b = strchr(base64, buffer[i+1]) - base64;
            c = strchr(base64, buffer[i+2]) - base64;
            d = strchr(base64, buffer[i+3]) - base64;

            x = (((a << 6) | b) >> 4) & 0xFF;
            y = (((((a << 6) | b) << 6) | c) >> 2) & 0xFF;
            z = ((((((a << 6) | b) << 6) | c) << 6) | d) & 0xFF;

            fputc(x, to);

            if (c != 64)
                fputc(y, to);
	    else {
	        fclose(to);
		return 0;
	    }

            if (d != 64)
                fputc(z, to);
	    else {
		fclose(to);
		return 0;
	    }
        }
    }
    fclose(to);
    
    return 0;
}



