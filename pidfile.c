/*
 * Copyright (c) 2022 Eric Johnson
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * Redistributions of works must retain the original copyright notice, this list
 * of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the original copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * Neither the name of the W3C nor the names of its contributors may be used to
 * endorse or promote products derived from this work without specific prior
 * written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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