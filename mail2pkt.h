/* --------------------------------------------------------------------------
 * MAIL-TO-PKT v0.2                                            Apr 6th, 2000
 * --------------------------------------------------------------------------
 *
 *   This program is a procmail filter to automatically decode FTN packets
 *   from BASE64 encoded email attachments.
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

#define VERSION          "0.2"

#define MAILSPOOLDIR     "/var/spool/mail/"

#define TEXT             0
#define BASE64           1

/* not yet implemented */
#define QUOTED_PRINTABLE 2
#define UUENCODED        3
#define NONE             4

int log(char *string, char *dir);
int readBoundary(char *boundary, FILE *file);
int skip(char *boundary, FILE *file);
int main(int argc, char *argv[]);
int findName(char *inbound, char *name);
int mailFile(char *name);
char *makeTempFile(char *inbound);
char *getMBox(void);
void lowercase(char *s);

