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

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <fidoconfig/fidoconfig.h>

#include "mime.c"
#include "mail2pkt.h"

char *getMBox(void)
{
    struct passwd *p;
    char *buff = malloc(255);
    int uid = getuid();

    while ((p = getpwent()))
        if (uid == p->pw_uid)
            sprintf(buff, "%s%s", MAILSPOOLDIR, p->pw_name);

    return buff;

}

int mailFile(char *name)
{
    int c;
    FILE *from;
    FILE *to;
    char *mbox = malloc(255);

    mbox = getMBox();
    from = fopen(name, "r");
    to = fopen(mbox, "a");

    while ((c = fgetc(from)) != EOF)
        fputc(c, to);
    fclose(to);
    fclose(from);

    return 0;

}


char *makeTempFile(char *inbound)
{
    int c;
    FILE *temp;
    char *buff = malloc(strlen(inbound)+13);
    char *name = malloc(13);

    strcpy(name, "pkt2mail.tmp");
    findName(inbound, name);
    sprintf(buff, "%s%s", inbound, name);

    if ((temp = fopen(buff, "wb")) == NULL) {
        strcpy(buff, "");
        return buff;
    }

    while ((c = getchar()) != EOF)
        fputc(c, temp);
    fclose(temp);

    return buff;
}

int readBoundary(char *boundary, FILE *file)
{
    char buff[255];

    do {
        fgets(buff, 255, file);

        if (feof(file))
            return -1;
            
        if (strncasecmp(buff, "Content-type: ", 13) == 0) {
            strncpy(buff, strchr(buff, '=')+1, 254);
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


int findName(char *inbound, char *name)
{
    FILE *file;
    char foo[255];
    char bar[255];

    strcpy(bar, name);
    sprintf(foo, "%s%s", inbound, bar);
    while((file = fopen(foo, "r")) != NULL) {
        sprintf(bar, "%04x.%s", (unsigned int)time(0), name+9);
        sprintf(foo, "%s%s", inbound, bar);
    };

    strcpy(name, bar);

    return 0;
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


int main(int argc, char *argv[])
{
    s_fidoconfig *config;
    char *tmpName;
    FILE *tmpFile;
    char buff[255];
    char boundary[255];
    char inbound[255];
    char logdir[255];
    char name[255];
    int encoding = TEXT;

    /* check out the arguments */
    config = readConfig();
    if (argc == 1) {
        strncpy(inbound, config->protInbound, 254);
        strncpy(logdir, config->logFileDir, 254);
    } else if (argc == 2) {
        strncpy(inbound, argv[1], 254);
        if (inbound[strlen(inbound)-1] != '/')
            strcat(inbound, "/");
        strncpy(logdir, config->logFileDir, 254);
    } else {
        fprintf(stderr, "Usage: mail2pkt [inbound]\n
  If inbound is not present, protected inbound from fidoconfig is used.\n
  See manual page for details.\n");
        disposeConfig(config);
        return 1;
    }

    /* get all the stdin to a file, so if something goes wrong, we can
       save the whole message */
    tmpName = malloc(strlen(inbound)+13);
    tmpName = makeTempFile(config->tempInbound);

    /* if we can't write the temp file */
    if (tmpName[0] == 0) {
        int c;
        char *mboxFile = malloc(255);
        FILE *mbox;

        log("[!] Can't open a tempfile for writing in the temp inbound.\n", logdir);
        log("[!] Mail saved in your mailbox.\n", logdir);

        mboxFile = getMBox();

        if ((mbox = fopen(mboxFile, "a")) == NULL)
            return 2;

        while ((c = getchar()) != EOF)
            fputc(c, mbox);
        fclose(mbox);

        return 2;
    }

    if ((tmpFile = fopen(tmpName, "r")) == NULL) {
        log("[!] Can't open tempfile for reading in the temp inbound.\n", logdir);
        return 3;
    }

    /* read the headers, and get the boundary */
    if (readBoundary(boundary, tmpFile) == -1) {
        log("Message has no attachments, so it is not for us!\n", logdir);
        log("Mail saved in your mailbox.\n", logdir);

        fclose(tmpFile);
        mailFile(tmpName);
        return 4;
    }

    do {

        while (strncmp(fgets(buff, 255, tmpFile), boundary, strlen(boundary)) && strncmp(buff, "\n", 1)) {
            strcpy(name, "");

            do {
                /* parse the headers and get important info */
                
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
                        log("File is encoded with an unsupported algorithm. Currently only BASE64 is supported.\n", logdir);
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
            } while (strcmp(fgets(buff, 255, tmpFile), "\n") != 0);

            /* A header section has finnished. What must we do? */

            if ((encoding == BASE64) && (name[0] != 0)) {
                /* according to Matthias docs, all the files we create
                must be lower case */
                lowercase(name);

                /* rename this bundle to fit a name that is not already
                in use */
                findName(inbound, name);
                
                sprintf(buff, "%s%s", inbound, name);
                if (fromBase64(buff, tmpFile) == 0) {
                    sprintf(buff, "Received %s OK.\n", name);
                    log(buff, logdir);
                } else {
                    sprintf(buff, "Error while processing %s.\n", name);
                    log(buff, logdir);
                    return -1;
                }
            } else if (encoding == TEXT)
                skip(boundary, tmpFile);
        }
    } while(buff[strlen(boundary)] != '-');

    fclose(tmpFile);
    remove(tmpName);
    disposeConfig(config);

    return 0;
}
