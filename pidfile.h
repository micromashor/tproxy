/*
 * Copyright (c) 2022 Eric Johnson
 * 
 * This software is licensed for use under the terms of the BSD license.
 * Please refer to the full text of the license, in the LICENSE file.
 */

#ifndef _PIDFILE_H
#define _PIDFILE_H

#include <unistd.h>

int pidfile_create(const char *filename);

pid_t pidfile_read(const char *filename);

int pidfile_destroy(const char *filename);

#endif /* _PIDFILE_H */