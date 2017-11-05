/*
 *	a potentially useful set of subroutines for use with CIS 415
 *	project 1
 */

#include <unistd.h>
#include <stdlib.h>
#include "p1fxns.h"
#include <string.h>
#include <errno.h>

/*
 *	p1getline - return EOS-terminated character array from fd
 *
 *	returns number of characters in buf as result, 0 if end of file
 */
int p1getline(int fd, char buf[], int size) {
    int i;
    char c;
    int max = size - 1; /* must leave room for EOS */

    i = 0;
    for (i = 0; i < max; i++) {
        if (read(fd, &c, 1) == 0)
            break;
        buf[i] = c;
        if (c == '\n') {
            i++;
            break;
        }
    }
    buf[i] = '\0';
    return i;
}

/*
 *	p1strchr - return the array index of leftmost occurrence of 'c' in 'buf'
 *
 *	return -1 if not found
 */
int p1strchr(char buf[], char c) {
    int i;

    for (i = 0; buf[i] != '\0'; i++)
        if (buf[i] == c)
            return i;
    return -1;
}

/*
 *	p1getword - fetch next blank-separated word from buffer into word
 *
 *	return value is index into buffer for next search or -1 if at end
 *
 *	N.B. assumes that word[] is large enough to hold the next word
 */
static char *singlequote = "'";
static char *doublequote = "\"";
static char *whitespace = " \t";

int p1getword(char buf[], int i, char word[]) {
    char *tc, *p;

    /* skip leading white space */
    while(p1strchr(whitespace, buf[i]) != -1)
        i++;
    /* buf[i] is now '\0' or a non-blank character */
    if (buf[i] == '\0')
        return -1;
    p = word;
    switch(buf[i]) {
    case '\'': tc = singlequote; i++; break;
    case '"': tc = doublequote; i++; break;
    default: tc = whitespace; break;
    }
    while (buf[i] != '\0') {
        if (p1strchr(tc, buf[i]) != -1)
            break;
        *p++ = buf[i];
        i++;
    }
    /* either at end of string or have found one of the terminators */
    if (buf[i] != '\0') {
        if (tc != whitespace) {
            i++;	/* skip over terminator */
        }
    }
    *p = '\0';
    return i;
}

/*
 *	p1strlen - return length of string
 */
int p1strlen(char *s) {
    char *p = s;

    while (*p != '\0')
        p++;
    return (p - s);
}

/*
 *
 *	p1strdup - duplicate string on heap
 *
 */
char *p1strdup(char *s) {
    int n = p1strlen(s) + 1;
    char *p = (char *)malloc(n);
    int i;

    if (p != NULL) {
        for (i = 0; i < n; i++)
            p[i] = s[i];
    }
    return p;
}

/*
 *	p1putint - display integer in decimal on file descriptor
 */
static char *digits = "0123456789";

void p1putint(int fd, int number) {
    char buf[25];
    int n, i;

    if (number == 0) {
        buf[0] = '0';
        i = 1;
    } else {
        for (n = number, i = 0; n != 0; i++) {
            buf[i] = digits[n % 10];
            n /= 10;
        }
    }
    while (--i >= 0)
        write(fd, buf+i, 1);
    fsync(fd);
}

/*
 *	p1putstr - display string on file descriptor
 */
void p1putstr(int fd, char *s) {
    int n = p1strlen(s);
    write(fd, s, n);
    //while (*s != '\0') {
        //write(fd, s, 1);
        //s++;
    //}
    fsync(fd);
}

/*
 *
 *	p1perror(int fd, char *str) - writes 'str' and string describing
 *                                    the last error on 'fd'
 */
void p1perror(int fd, char *str) {
    char *p = strerror(errno);

    p1putstr(fd, str);
    p1putstr(fd, " - ");
    p1putstr(fd, p);
    p1putstr(fd, "\n");
}

/*
 *	p1atoi - convert string to integer
 */
int p1atoi(char *s) {
    int ans;

    for (ans = 0; *s >= '0' && *s <= '9'; s++)
        ans = 10 * ans + (int) (*s - '0');
    return ans;
}

/*
 *	p1itoa - format integer as decimal string
 */
void p1itoa(int number, char *buf) {
    char tmp[25];
    int n, i, negative;
    static char digits[] = "0123456789";

    if (number == 0) {
        tmp[0] = '0';
        i = 1;
    } else {
        if ((n = number) < 0) {
            negative = 1;
            n = -n;
        } else
            negative = 0;
        for (i = 0; n != 0; i++) {
            tmp[i] = digits[n % 10];
	    n /= 10;
        }
        if (negative) {
            tmp[i] = '-';
            i++;
        }
    }
    while (--i >= 0)
       *buf++ = tmp[i];
    *buf = '\0';
}

/*
 *	p1strcpy - copy str2 into str1
 */
void p1strcpy(char *str1, char *str2) {
    while ((*str1++ = *str2++) != '\0')
        ;
}

/*
 *	p1strcat - concatenate str2 onto str1
 */
void p1strcat(char *str1, char *str2) {
    char *p;

    for (p = str1; *p != '\0'; p++)	/* find end of string */
        ;
    p1strcpy(p, str2);
}

/*
 *	p1strneq - determine if the first n characters of two strings are equal
 *
 *	returns 1 if the first n characters are equal, 0 if not
 */
int p1strneq(const char *s1, const char *s2, int n){
    int i;

    for (i = 0; i < n; i++) {
        if (s1[i] != s2[i])
            return 0;	/* return false because chars at index i are unequal */
    }
    return 1;
}

/*
 *	p1strpack - pack justified strings into buffer
 *
 *	packs st into buf, with a field width fw, appending fill character fc
 *      to st to make up fw;
 *      if fw < 0, packs fc before st, up to a field width of |fw|;
 *	appends '\0' to buf, returns a pointer to the '\0' as the function value
 *
 *	examples:
 *	  p1strpack("1", 5, '0', buf) packs "10000" into buf, returns buf+5
 *	  p1strpack("3", -5, '0', buf) packs "00003" into buf, returns buf+5
 *	  p1strpack("3", 0, ' ', buf) packs "3" into buf, returns buf+1
 */
char *p1strpack(char *st, int fw, char fc, char *buf){
    char *p = buf;
    int n = p1strlen(st);
    int nb;

    if (fw < 0)
        nb = (-fw - n);
    else
        nb = fw - n;
    if (fw < 0)
        while (nb-- > 0)
            *p++ = fc;
    while (*st != '\0')
        *p++ = *st++;
    if (fw >= 0)
        while (nb-- > 0)
            *p++ = fc;
    *p = '\0';
    return p;
}
