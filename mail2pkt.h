/* --------------------------------------------------------------------------
 * MAIL-TO-PKT v0.1                                            Nov 5th, 1999
 * --------------------------------------------------------------------------
 *
 *   This program reads an email from the standard input and converts it
 *   into a FTN packet. Requires smapilnx and fidoconfig libraries to compile.
 *
 *   Copyright (C) 1999  German Theler
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

/* mail2pkt.h */
int getEncoding(char *s);
void lowercase(char *s);
int main(void);
int readBoundary(char *boundary);
void skip(char *boundary);

/* mime.c */
int cvt_ascii(unsigned char alpha);
int getFile(char *name);




