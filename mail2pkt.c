/* --------------------------------------------------------------------------
 * MAIL-TO-PKT v0.2                                            Apr 19th, 2000
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

#include <fidoconf/fidoconf.h>
#include <fidoconf/common.h>

#include "mime.c"
#include "mail2pkt.h"


/* ---------------------------------------------------------------------
 * getMBox
 *   returns a string containing the file that is used to store incoming
 *   mail for a certain system and user.
 * ---------------------------------------------------------------------
 */
char *getMBox(void)
{
    struct passwd *p;
    char *spool = malloc(255);
    FILE *dir;
    int uid = getuid();

    /* look where could mail boxes be in different systems */
    if ((dir = fopen("/var/mail", "r")) != NULL)
        strcpy(spool, "/var/mail/");
    else if ((dir = fopen("/var/spool/mail", "r")) != NULL)
        strcpy(spool, "/var/spool/mail/");
    else if ((dir = fopen("/usr/spool/mail", "r")) != NULL)
        strcpy(spool, "/usr/spool/mail/");
    else if ((dir = fopen("/usr/mail", "r")) != NULL)
        strcpy(spool, "/usr/mail/");
    else
        return NULL;

    fclose(dir);

    /* add the user name to the spool */
    while ((p = getpwent()))
        if (uid == p->pw_uid)
            strcat(spool, p->pw_name);

    return spool;

}


/* ---------------------------------------------------------------------
 * mailFile
 *   appends a text file to the user's incoming mailbox.
 * ---------------------------------------------------------------------
 */
int mailFile(char *name)
{
    FILE *from;
    FILE *to;
    int c;
    char *mbox = malloc(255);

    if ((mbox = getMBox()) == NULL)
        return -1;

    if ((from = fopen(name, "r")) == NULL)
        return -1;

    if ((to = fopen(mbox, "a")) == NULL)
        return -1;

    /* write the file to the mailbox */
    while ((c = fgetc(from)) != EOF)
        fputc(c, to);

    fclose(to);
    fclose(from);

    return 0;

}

/* ---------------------------------------------------------------------
 * makeTempFile
 *   reads the standard input and creates a swap file in directory dir
 *   with the contents of the standard input.
 *   returns the name of the file.
 * ---------------------------------------------------------------------
 */
char *makeTempFile(char *dir)
{
    int c;
    FILE *temp;
    char *buff = malloc(strlen(dir)+13);
    char *name = malloc(13);

    /* let's start with this name. If it is already in use, find a new one */
    strcpy(name, "pkt2mail.tmp");
    findName(dir, name);
    sprintf(buff, "%s%s", dir, name);

    if ((temp = fopen(buff, "wb")) == NULL)
        return NULL;

    while ((c = getchar()) != EOF)
        fputc(c, temp);

    fclose(temp);

    return buff;
}


/* ---------------------------------------------------------------------
 * readBoundary
 *   parses a text file containing a plain email and tries to get the
 *   message boundary
 *   WARNING: though I had no problems with this routine, it is not
 *   guaranteed that this will work. If you find a better routine, please
 *   commit it in.
 * ---------------------------------------------------------------------
 */
int readBoundary(char *boundary, FILE *file)
{
    char *buff = malloc(255);

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

/* ---------------------------------------------------------------------
 * skip
 *   ignores a part of a message until the next one (until the end of the
 *   boundary).
 * ---------------------------------------------------------------------
 */
int skip(char *boundary, FILE *file)
{
    char *buff = malloc(255);

    do {
        fgets(buff, 255, file);
        if (feof(file))
            return -1;

    } while(strncmp(buff, boundary, strlen(boundary)-1) != 0);

    return 0;
}

/* ---------------------------------------------------------------------
 * lowercase
 *   replaces uppercase letters (A-Z) in string s with lowercase letters
 *   (a-z). This is done because of a doc in husky-base that says that
 *   every created file must be in lowercase.
 * ---------------------------------------------------------------------
 */
void lowercase(char *s)
{
    int i=0;

    while(s[i] != 0) {
        if ((s[i] >= 'A') && (s[i] <= 'Z'))
            s[i] += 'a' - 'A';
        i++;
    }
}

/* ---------------------------------------------------------------------
 * findName
 *   given a file name (name) and a directory (dir), if the name is not
 *   being used, it does nothing. If it _is_ used, makes a new one up. If
 *   this new one is already used, makes a new one, etc, until it finds
 *   one that suits.
 * ---------------------------------------------------------------------
 */
int findName(char *dir, char *name)
{
    FILE *file;
    char *foo = malloc(255);
    char *bar = malloc(255);

    /* check if the names ends in .out, .cut or .dut and translate to .pkt */
    strcpy(bar, name);
    if ((bar[strlen(bar)-2] == 'u') && (bar[strlen(bar)-1] == 't'))
        strcpy(bar+strlen(bar)-3, "pkt");

    sprintf(foo, "%s%s", dir, bar);
    while((file = fopen(foo, "r")) != NULL) {
        /* the file name is in use, make a new one up */
        sprintf(bar, "%04x.%s", (unsigned int)time(0), name+9);
        sprintf(foo, "%s%s", dir, bar);
    };

    strcpy(name, bar);

    return 0;
}

/* ---------------------------------------------------------------------
 * log
 *   logs an action (string) in the logfile defined in c with the level
 *   level. It checks if level is defined in fidoconfig before logging.
 * ---------------------------------------------------------------------
 */
int log(char *string, s_fidoconfig *c, int level)
{
    char *name;
    char *date = malloc(40);
    FILE *logFile;
    time_t t;

    if (c->loglevels[0] == 0 || strchr(c->loglevels, level+'0') != NULL) {
        time(&t);
        strftime(date, 40, "%a %d %b  %H:%M:%S", localtime(&t));

        if ((name = malloc(strlen(c->logFileDir) + 13)) == NULL)
            return -2;

        sprintf(name, "%smail2pkt.log", c->logFileDir);

        if ((logFile = fopen(name, "a")) == NULL)
            return -1;
        fprintf(logFile, "%d   %s   %s", level, date, string);
        fclose(logFile);
    }

    return 0;
}



int main(int argc, char *argv[])
{
    FILE *tmpFile;
    s_fidoconfig *config;
    char *tmpName;
    char *buff = malloc(255);
    char *boundary = malloc(255);
    char *inbound = malloc(255);
    char *logdir = malloc(255);
    char *name = malloc(255);
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

        log("[!] Can't open a tempfile for writing in the temp inbound.\n", config, 1);
        log("[!] Mail saved in your mailbox.\n", config, 1);

        mboxFile = getMBox();

        if ((mbox = fopen(mboxFile, "a")) == NULL)
            return 2;

        while ((c = getchar()) != EOF)
            fputc(c, mbox);
        fclose(mbox);

        return 2;
    }

    if ((tmpFile = fopen(tmpName, "r")) == NULL) {
        log("[!] Can't open tempfile for reading in the temp inbound.\n", config, 1);
        return 3;
    }

    /* read the headers, and get the boundary */
    if (readBoundary(boundary, tmpFile) == -1) {
        log("Message has no attachments, so it is not for us!\n", config, 1);
        log("Mail saved in your mailbox.\n", config, 1);

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
                if (strncasecmp(buff, "Content-Transfer-Encoding: ", 26) == 0) {
                    if (strncasecmp(buff+27, "base64", 6) == 0) {
                        encoding = BASE64;
                    } else {
                        log("File is encoded with an unsupported algorithm. Currently only BASE64 is supported.\n", config, 2);
                        return -2;
                    }
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
                    log(buff, config, 3);
                } else {
                    sprintf(buff, "[!] Error while processing %s.\n", name);
                    log(buff, config, 1);
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
