/*
 * Copyright (c) 2022 Eric Johnson
 * 
 * This software is licensed for use under the terms of the BSD license.
 * Please refer to the full text of the license, in the LICENSE file.
 */

#ifndef _LOG_H
#define _LOG_H

#include <stddef.h>

/* Logging Channels, as defined in log.c */

enum {
	LOGCH_FATAL,
	LOGCH_ERROR,
	LOGCH_WARN,
	LOGCH_INFO,
	LOGCH_DEBUG
};

typedef int log_level_t;

#define LOGCHF_ERRNO (1 << 31)

const char *log_level_name(log_level_t num);
log_level_t log_level_num(const char *name);

/* Syslogd-like facility codes */

typedef enum {
	LOG_FACILITY_AUTH,
	LOG_FACILITY_AUTHPRIV,
	LOG_FACILITY_DAEMON,
	LOG_FACILITY_LOCAL0,
	LOG_FACILITY_LOCAL1,
	LOG_FACILITY_LOCAL2,
	LOG_FACILITY_LOCAL3,
	LOG_FACILITY_LOCAL4,
	LOG_FACILITY_LOCAL5,
	LOG_FACILITY_LOCAL6,
	LOG_FACILITY_LOCAL7,
	LOG_FACILITY_LPR,
	LOG_FACILITY_MAIL,
	LOG_FACILITY_NEWS,
	LOG_FACILITY_CRON,
	LOG_FACILITY_FTP,
	LOG_FACILITY_KERN,
	LOG_FACILITY_SYSLOG,
	LOG_FACILITY_UUCP,
	LOG_FACILITY_USER
} log_facility_t;

const char *log_facility_name(log_facility_t num);
log_facility_t log_facility_num(const char *name);

/* Preloaded Error Codes */

typedef enum { ERR_INTERNAL, ERR_MEM_ALLOCATION } errcode_t;

void err(errcode_t);

/* Log control functions and macros */

#define LOG_F_SYSLOG   1
#define LOG_F_STDERR   2
#define LOG_F_DEVERRNO 4

const char *log_init(const char *argv0, log_facility_t facility, int flags);

int log_fd_add(int fd);
int log_fd_rm(int fd);

log_level_t log_setlevel(log_level_t level);

typedef void (*log_hook_fn_t)(const char *msg, size_t len, size_t taglen, void *arg);

int log_hook_add(log_hook_fn_t hook, void *arg);
int log_hook_rm(log_hook_fn_t hook, void *arg);

/* Generic Logging functions and macros */

void internal_log_direct(const char *file, unsigned long line, const char *func,
			 log_level_t level, const char *fmt, ...)
	__attribute__((__format__(__printf__, 5, 6)));

#define log(level, ...) \
	internal_log_direct(__FILE__, __LINE__, __func__, level, __VA_ARGS__)

#endif /* _LOG_H */