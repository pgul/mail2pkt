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

#define TEXT             0
#define QUOTED_PRINTABLE 1
#define BASE64           2
#define NONE             3

/* mail2pkt.c */
int log(char *string, char *dir);
int readBoundary(char *boundary, FILE *file);
int skip(char *boundary, FILE *file);
int main(void);
void lowercase(char *s);

/* mime.c */
int cvt_ascii(unsigned char alpha);
int getFile(char *name, FILE *file);


