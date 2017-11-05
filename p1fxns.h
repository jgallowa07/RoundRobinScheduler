/*
 *	a potentially useful set of subroutines for use with CIS 415
 *	project 1
 */

#ifndef _P1FXNS_H_
#define _P1FXNS_H_

/*
 *	p1getline - return EOS-terminated character array from fd
 *
 *	returns number of characters in buf as result, 0 if end of file
 */
int p1getline(int fd, char buf[], int size);

/*
 *	p1strchr - return the array index of leftmost occurrence of 'c' in 'buf'
 *
 *	return -1 if not found
 */
int p1strchr(char buf[], char c);

/*
 *	p1getword - fetch next blank-separated word from buffer into word
 *
 *	return value is index into buffer for next search or -1 if at end
 *
 *	N.B. assumes that word[] is large enough to hold the next word
 */
int p1getword(char buf[], int i, char word[]);

/*
 *	p1strlen - return length of string
 */
int p1strlen(char *s);

/*
 *
 *	p1strdup - duplicate string on heap
 *
 */
char *p1strdup(char *s);

/*
 *	p1putint - display integer in decimal on file descriptor
 */
void p1putint(int fd, int number);

/*
 *	p1putstr - display string on file descriptor
 */
void p1putstr(int fd, char *s);

/*
 *
 *	p1perror(int fd, char *str) - writes 'str' and string describing
 *                                    the last error on 'fd'
 */
void p1perror(int fd, char *str);

/*
 *	p1atoi - convert string to integer
 */
int p1atoi(char *s);

/*
 *	p1itoa - format integer as decimal string
 */
void p1itoa(int number, char *buf);

/*
 *	p1strcpy - copy str2 into str1
 */
void p1strcpy(char *str1, char *str2);

/*
 *	p1strcat - concatenate str2 onto str1
 */
void p1strcat(char *str1, char *str2);

/*
 *	p1strneq - determine if the first n characters of two strings are equal
 *
 *	returns 1 if the first n characters are equal, 0 if not
 */
int p1strneq(const char *s1, const char *s2, int n);

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
char *p1strpack(char *st, int fw, char fc, char *buf);

#endif	/* _P1FXNS_H_ */
