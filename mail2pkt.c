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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <fidoconfig.h>

#include "mime.c"
#include "mail2pkt.h"


int readBoundary(char *boundary, FILE *file)
{
    char buff[255];

    do {
        fgets(buff, 255, file);

        if (feof(file))
            return -1;
            
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

    } while ((strncmp(buff, boundary, (strlen(boundary) == 0)?10:strlen(boundary)) != 0)) ;

    return 0;
}

int skip(char *boundary, FILE *file)
{
    char buff[255];

    do {
        fgets(buff, 255, file);
        if (feof(file))
            return -1;
            
    } while(strncmp(buff, boundary, strlen(boundary)-1) != 0);

    return 0;
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

int log(char *string, char *dir)
{
    char *name;
    char date[40];
    FILE *logFile;
    time_t t;

    time(&t);
    strftime(date, 40, "%a %d %b - %H:%M:%S", localtime(&t));

    if ((name = malloc(strlen(dir) + 13)) == NULL)
        return -2;
    sprintf(name, "%smail2pkt.log", dir);

    if ((logFile = fopen(name, "a")) == NULL)
        return -1;
    fprintf(logFile, "%s  %s", date, string);
    fclose(logFile);

    return 0;
}


int main(void)
{
    s_fidoconfig *config;

    char buff[255];
    char boundary[255];
    char name[255];

    /* Default enconding */
    int encoding = TEXT;

    config = readConfig();

    /* Get the file pointer... */
    /* read the headers, and get the boundary */
    if (readBoundary(boundary, stdin) == -1) {
        log("Can't find a valid boundary! Seems like the message has no attachments, so it is not for us!", config->logFileDir);
        return -1;
    }

    do {
        while (strcmp(fgets(buff, strlen(boundary), stdin), boundary+1)) {
            fseek(stdin, -strlen(boundary)+1, SEEK_CUR);
            strcpy(name, "");

            do {
                /* read a line */
                fgets(buff, 255, stdin);

                /* parse it */
                /* Let's start having a look at the content-type header...*/
                if (strncasecmp(buff, "Content-type: ", 13) == 0) {
                /* if it's a text body, we will skip it later */
                    if (strncasecmp(buff+14, "text/plain", 10) == 0)
                        encoding = TEXT;
     
                    /* if it has an attachment, the file name is just after the
                       last '=' */
                    if (strchr(buff, '=') != NULL) {
                        strcpy(name, strchr(buff, '=')+1);
                        if (name[0] == '"') {
                            strcpy(name, name+1);
                            name[strlen(name)-2] = 0;
                        } else
                            name[strlen(name)-1] = 0;
                    }
                }
    
                /* now, the encoding scheme...*/
                if (strncasecmp(buff, "Content-Transfer-Encoding: ", 26) == 0)
                    if (strncasecmp(buff+27, "base64", 6) == 0)
                    encoding = BASE64;
                    else {
                        log("File is encoded with an unsupported algorithm. Currently only BASE64 is supported.", config->logFileDir);
                        return -2;
                    }
    
                /* if this header is present, then we can get the file name from
                   here too... */

                if (strncasecmp(buff, "Content-Disposition: ", 21) == 0) {
                    strcpy(name, strchr(buff, '=')+1);
                    if (name[0] == '"') {
                        strcpy(name, name+1);
                        name[strlen(name)-2] = 0;
                    } else
                        name[strlen(name)-1] = 0;
                }
            } while (strcmp(buff, "\n") != 0);

            /* A header section has finnished. What must we do? */

            if ((encoding == BASE64) && (name[0] != 0)) {
                /* according to Matthias docs, all the files we create
                must be lower case */
                lowercase(name);
                sprintf(buff, "%s%s", config->protInbound, name);
                if (getFile(buff, stdin) == 0) {
                    sprintf(buff, "Received %s.\n", name);
                    log(buff, config->logFileDir);
                } else {
                    sprintf(buff, "Error while processing %s.\n", name);
                    log(buff, config->logFileDir);
                }
            } else if (encoding == TEXT)
                skip(boundary, stdin);
        }
    } while(fgetc(stdin) != '-');


    fclose(stdin);

    return 0;
}
