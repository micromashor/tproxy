/*
 * Copyright (c) 2022 Eric Johnson
 * 
 * This software is licensed for use under the terms of the BSD license.
 * Please refer to the full text of the license, in the LICENSE file.
 */

#ifndef _FAILSAFE_H
#define _FAILSAFE_H

#include <stddef.h>
#include <stdarg.h>

void *xmalloc(size_t size);
void *xcalloc(size_t nmemb, size_t size);
void *xrealloc(void *ptr, size_t size);
void *xreallocarray(void *ptr, size_t nmemb, size_t size);

char *xstrdup(const char *s);
char *xstrndup(const char *s, size_t n);

int xasprintf(char **strp, const char *fmt, ...);
int xvasprintf(char **strp, const char *fmt, va_list ap);

#endif /* _FAILSAFE_H */