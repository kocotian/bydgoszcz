/* See licenses/LIBSL file for copyright and license details. */

#ifndef _UTIL_H
#define _UTIL_H

#define MAX(A, B)               ((signed)(A) > (signed)(B) ? (signed)(A) : (signed)(B))
#define MIN(A, B)               ((signed)(A) < (signed)(B) ? (signed)(A) : (signed)(B))
#define BETWEEN(X, A, B)        ((signed)(A) <= (signed)(X) && (signed)(X) <= (signed)(B))

#define UMAX(A, B)              ((unsigned)(A) > (unsigned)(B) ? (unsigned)(A) : (unsigned)(B))
#define UMIN(A, B)              ((unsigned)(A) < (unsigned)(B) ? (unsigned)(A) : (unsigned)(B))
#define UBETWEEN(X, A, B)       ((unsigned)(A) <= (unsigned)(X) && (unsigned)(X) <= (unsigned)(B))

#define LEN(X)                  (sizeof (X) / sizeof (*X))

void die(const char *fmt, ...);
void *ecalloc(size_t nmemb, size_t size);

#endif
