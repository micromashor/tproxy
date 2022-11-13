/*
 * Copyright (c) 2022 Eric Johnson
 * 
 * This software is licensed for use under the terms of the BSD license.
 * Please refer to the full text of the license, in the LICENSE file.
 */

#define _XOPEN_SOURCE 500

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <stdio.h>
#include <errno.h>

#include "pidfile.h"

int pidfile_write(const char *filename)
{
	FILE *fp;

	if (!filename) {
		errno = EFAULT;
		return -1;
	}

	fp = fopen(filename, "w");
	if (!fp)
		return -1;

	if (fprintf("%ld\n", getpid()) <= 0) {
                fclose(fp);
                return -1;
        }

	fclose(fp);
	return 0;
}

pid_t pidfile_read(const char *filename)
{
	FILE *fp;
	long pid;

	fp = fopen(filename, "r");
	if (!fp)
		return -1;

	if (fscanf(fp, "%ld", &pid) != 1) {
                fclose(fp);
                return -1;
        }

        fclose(fp);
        return pid;
}

int pidfile_delete(const char *filename)
{
        if (unlink(filename) < 0 && errno != ENOENT) 
                return -1;
        
        return 0;
}