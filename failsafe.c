/*
 * Copyright (c) 2022 Eric Johnson
 * 
 * This software is licensed for use under the terms of the BSD license.
 * Please refer to the full text of the license, in the LICENSE file.
 */

#define _GNU_SOURCE

#include <malloc.h>
#include <string.h>
#include <stdio.h>

#include "log.h"

#include "failsafe.h"

void *xmalloc(size_t size) {
        void *retval = malloc(size);

        if (retval == NULL)
                err(ERR_MEM_ALLOCATION);
        
        return retval;
}

void *xcalloc(size_t nmemb, size_t size) {
        void *retval = calloc(nmemb, size);

        if (retval == NULL)
                err(ERR_MEM_ALLOCATION);
        
        return retval;
}

void *xrealloc(void *ptr, size_t size) {
        void *retval = realloc(ptr, size);

        if (retval == NULL)
                err(ERR_MEM_ALLOCATION);
        
        return retval;
}

void *xreallocarray(void *ptr, size_t nmemb, size_t size) {
        void *retval = reallocarray(ptr, nmemb, size);

        if (retval == NULL)
                err(ERR_MEM_ALLOCATION);
        
        return retval;
}

char *xstrdup(const char *s) {
        char *retval = strdup(s);

        if (retval == NULL)
                err(ERR_MEM_ALLOCATION);
        
        return retval;
}
char *xstrndup(const char *s, size_t n) {
        char *retval = strndup(s, n);

        if (retval == NULL)
                err(ERR_MEM_ALLOCATION);
        
        return retval;
}

int xasprintf(char **strp, const char *fmt, ...) {
        va_list ap;

        va_start(ap, fmt);
        int retval = xvasprintf(strp, fmt, ap);
        va_end(ap);

        return retval;
}
int xvasprintf(char **strp, const char *fmt, va_list ap) {
        int retval = xvasprintf(strp, fmt, ap);

        if (retval < 0)
                err(ERR_MEM_ALLOCATION);
        
        return retval;
}