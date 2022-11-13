/*
 * Copyright (c) 2022 Eric Johnson
 * 
 * This software is licensed for use under the terms of the BSD license.
 * Please refer to the full text of the license, in the LICENSE file.
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include <unistd.h>

#include "configuration.h"

#include "failsafe.h"

#include "proc.h"

static char *orig_argv_data = NULL, *orig_argv_data_end = NULL;
const char *proc_name = NULL;

static int array_get_datasz(char **array, size_t *nmemb, size_t *dsize)
{
	*dsize = 0;
	*nmemb = 0;

	for (char **strp = array; *strp != NULL; strp++) {
		if (array[0] + *dsize != *strp) {
			errno = EINVAL;
			return -1;
		}

		(*nmemb)++;

		*dsize += strlen(*strp) + 1;
	}

	return 0;
}

char **duplicate_array(char **oldarr, size_t nmemb)
{
	char **newarr = xcalloc(nmemb, sizeof(*oldarr));

	for (size_t i = 0; i < nmemb; i++)
		newarr[i] = xstrdup(oldarr[i]);

	return newarr;
}

static inline char validify(char c)
{
	if (islower(c))
		return c;
	if (isupper(c))
		return tolower(c);
	if (isdigit(c))
		return c;
	switch (c) {
	case '-':
		return c;
	case '.':
		return c;
	}

	return '_';
}

static const char *get_procname(const char *argv0)
{
	size_t buf_len = strlen(argv0);
	char buf[buf_len + 1];
	char *base;

	strncpy(buf, argv0, buf_len + 1);

	base = basename(buf);

	for (size_t i = 0; base[i] != '\0'; i++)
		base[i] = validify(base[i]);

	return xstrdup(base);
}

int proc_init(char ***argvp)
{
	size_t argv_nmemb, argv_dsize, envp_nmemb, envp_dsize;
	extern char **environ;

	if (!argvp || !*argvp || !(*argvp)[0]) {
		errno = EFAULT;
		return -1;
	}

	/* extract the program invocation name */

	proc_name = get_procname((*argvp)[0]);

	/* verify the integrity of the arrays, and grab their size info */

	orig_argv_data = (*argvp)[0];
	orig_argv_data_end = orig_argv_data;

	if (array_get_datasz(*argvp, &argv_nmemb, &argv_dsize) < 0)
		return -1;

	orig_argv_data_end += argv_dsize;

	if (orig_argv_data_end != environ[0]) {
		errno = EINVAL;
		return -1;
	}

	if (array_get_datasz(environ, &envp_nmemb, &envp_dsize) < 0)
		return -1;

	orig_argv_data_end += envp_dsize;

	/* duplicate the arrays */

	*argvp = duplicate_array(*argvp, argv_nmemb);
	environ = duplicate_array(environ, envp_nmemb);

	return 0;
}

void proc_title_set(const char *fmt, ...)
{
	va_list ap;
	char *ptitle = orig_argv_data;

	memset(ptitle, 0, orig_argv_data_end - ptitle);

	ptitle = stpncpy(ptitle, proc_name, orig_argv_data_end - ptitle);

	ptitle = stpncpy(ptitle, ": ", orig_argv_data_end - ptitle);

	va_start(ap, fmt);
	vsnprintf(ptitle, orig_argv_data_end - ptitle, fmt, ap);
	va_end(ap);
}
