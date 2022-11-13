/*
 * Copyright (c) 2022 Eric Johnson
 * 
 * This software is licensed for use under the terms of the BSD license.
 * Please refer to the full text of the license, in the LICENSE file.
 */

#ifndef _PROC_H
#define _PROC_H

extern const char *proc_name;

void proc_title_set(const char *fmt, ...) __attribute__((__format__(__printf__, 1, 2)));
int proc_init(char ***argv);

#endif /* _PROC_H */