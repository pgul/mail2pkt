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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fidoconfig.h>

#include "mime.c"
#include "mail2pkt.h"

int readBoundary(char *boundary)
{
    char buff[255];

    do {
        fgets(buff, 255, stdin);

        if (strncasecmp(buff, "Content-type: ", 13) == 0) {
            strcpy(buff, strchr(buff, '=')+1);
            if (buff[0] == '"') {
                sprintf(boundary, "--%s", buff+1);
                boundary[strlen(boundary)-2] = 0;
            } else {
                sprintf(boundary, "--%s", buff);
                boundary[strlen(boundary)-1] = 0;
            }
        }

    } while (strncmp(buff, boundary, (strlen(boundary) == 0)?10:strlen(boundary)) != 0);

    return 0;
}

void skip(char *boundary)
{
    char buff[255];

    do {
        fgets(buff, 255, stdin);
    } while(strncmp(buff, boundary, strlen(boundary)-1) != 0);
}

void lowercase(char *s)
{
    int i=0;

    while(s[i] != 0) {
        if ((s[i] >= 'A') && (s[i] <= 'Z'))
            s[i] += 'a' - 'A';

        i++;
    }
}

int main(void)
{
    s_fidoconfig *c;
    char buff[255];
    char boundary[255];
    char name[255];

    int encoding = TEXT;

    /* read fidoconfig configuration */
    c = readConfig();

    /* read the headers, and get the boundary */
    readBoundary(boundary);

    while (!feof(stdin)) {

      strcpy(name, "");
    
      do {

          /* read a line */
          fgets(buff, 255, stdin);

          /* parse it */
          if (strncasecmp(buff, "Content-type: ", 13) == 0) {
              if (strncasecmp(buff+14, "text/plain", 10) == 0)
                  encoding = TEXT;

              if (strchr(buff, '=') != NULL) {
                  strcpy(name, strchr(buff, '=')+1);
                  if (name[0] == '"') {
                      strcpy(name, name+1);
                      name[strlen(name)-2] = 0;
                  } else {
                      name[strlen(name)-1] = 0;
                  }
              }
          }

          if (strncasecmp(buff, "Content-Transfer-Encoding: ", 26) == 0)
              if (strncasecmp(buff+27, "base64", 6) == 0)
                  encoding = BASE64;

          if (strncasecmp(buff, "Content-Disposition: ", 21) == 0) {
              strcpy(name, strchr(buff, '=')+1);
              if (name[0] == '"') {
                  strcpy(name, name+1);
                  name[strlen(name)-2] = 0;
              } else {
                  name[strlen(name)-1] = 0;
              }
          }
      } while (strcmp(buff, "\n") != 0);

      /* A header section have finnished. What must we do? */

      if ((encoding == BASE64) && (name[0] != 0)) {
          lowercase(name);
          sprintf(buff, "%s%s", c->protInbound, name);
          getFile(buff);
      } else if (encoding == TEXT)
          skip(boundary);

    }

    return 0;
}
