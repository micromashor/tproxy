/*
 * Copyright (c) 2022 Eric Johnson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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