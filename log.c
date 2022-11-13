/*
 * Copyright (c) 2022 Eric Johnson
 * 
 * This software is licensed for use under the terms of the BSD license.
 * Please refer to the full text of the license, in the LICENSE file.
 */

/**
 * This library is designed to include the following logging interfaces:
 * - stderr
 * - external logging files
 * - syslogd(8)
 * - logging callbacks, which can be registered by the user program
*/

#define _GNU_SOURCE

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <syslog.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "configuration.h"
#include "failsafe.h"
#include "proc.h"

#include "log.h"

#define ARRAY_NMEMB(arr) (sizeof(arr) / sizeof(*(arr)))

/* Static Declarations */

struct log_hook {
	log_hook_fn_t func;
	void *uarg;
};

static int *log_fds = NULL;
static size_t log_fds_count = 0;
static struct log_hook *log_hooks = NULL;
static size_t log_hooks_count = 0;
static log_level_t log_level_main = LOGCH_INFO;
static int log_flags = 0;

/* Log Channel Definitions */

#define LCHOPT_DIE 0x01 /* Exit the program */

static const struct {
	const char *name;
	int sysloglevel;
	unsigned int opts;
} log_level_infos[] = { [LOGCH_FATAL] = { "fatal", LOG_ALERT, LCHOPT_DIE },
			[LOGCH_ERROR] = { "error", LOG_ERR, 0 },
			[LOGCH_WARN] = { "warn", LOG_WARNING, 0 },
			[LOGCH_INFO] = { "info", LOG_INFO, 0 },
			[LOGCH_DEBUG] = { "debug", LOG_INFO, 0 } };

const char *log_level_name(log_level_t ch)
{
	if (ch < 0 || ch >= (int) ARRAY_NMEMB(log_level_infos))
		return NULL;

	return log_level_infos[ch].name;
}

log_level_t log_level_num(const char *name)
{
	if (!name)
		return -1;

	for (size_t i = 0; i < ARRAY_NMEMB(log_level_infos); i++) {
		if (strcasecmp(name, log_level_infos[i].name) == 0)
			return i;
	}

	return -1;
}

/* Preloaded Error Code Definitions */

#define ERRINFO_ERRNO 0x01 /* Print the contents of errno */

static const struct {
	const char *msg;
	log_level_t level;
	int flags;
} errors[] = { { "Unknown Internal Error", LOGCH_FATAL, 0 },
	       { "Failed to allocate memory", LOGCH_FATAL, ERRINFO_ERRNO } };

void err(errcode_t err)
{
	int level = errors[err].level;

	if (errors[err].flags & ERRINFO_ERRNO)
		level |= LOGCHF_ERRNO;

	internal_log_direct(NULL, 0, NULL, level, "%s", errors[err].msg);
}

/* Logging Facilities */

#define LOG_FACINFO_F_WARN 1

static struct {
	const char *name;
	int syslog_facility;
	int flags;
} log_facilities[] = {
	[LOG_FACILITY_AUTH] = { "auth", LOG_AUTH, 0 },
	[LOG_FACILITY_AUTHPRIV] = { "authpriv", LOG_AUTHPRIV, 0 },
	[LOG_FACILITY_DAEMON] = { "daemon", LOG_DAEMON, 0 },
	[LOG_FACILITY_LOCAL0] = { "local0", LOG_LOCAL0, 0 },
	[LOG_FACILITY_LOCAL1] = { "local1", LOG_LOCAL1, 0 },
	[LOG_FACILITY_LOCAL2] = { "local2", LOG_LOCAL2, 0 },
	[LOG_FACILITY_LOCAL3] = { "local3", LOG_LOCAL3, 0 },
	[LOG_FACILITY_LOCAL4] = { "local4", LOG_LOCAL4, 0 },
	[LOG_FACILITY_LOCAL5] = { "local5", LOG_LOCAL5, 0 },
	[LOG_FACILITY_LOCAL6] = { "local6", LOG_LOCAL6, 0 },
	[LOG_FACILITY_LOCAL7] = { "local7", LOG_LOCAL7, 0 },
	[LOG_FACILITY_LPR] = { "lpr", LOG_LPR, 0 },
	[LOG_FACILITY_MAIL] = { "mail", LOG_MAIL, 0 },
	[LOG_FACILITY_NEWS] = { "news", LOG_NEWS, 0 },
	[LOG_FACILITY_CRON] = { "cron", LOG_CRON, 0 },
	[LOG_FACILITY_FTP] = { "ftp", LOG_FTP, 0 },
	[LOG_FACILITY_KERN] = { "kern", LOG_KERN, LOG_FACINFO_F_WARN },
	[LOG_FACILITY_SYSLOG] = { "syslog", LOG_SYSLOG, LOG_FACINFO_F_WARN },
	[LOG_FACILITY_UUCP] = { "uucp", LOG_UUCP, 0 },
	[LOG_FACILITY_USER] = { "user", LOG_USER, 0 }
};

const char *log_facility_name(log_facility_t num)
{
	if (num < 0 || num >= ARRAY_NMEMB(log_facilities)) {
		errno = EINVAL;
		return NULL;
	}

	return log_facilities[num].name;
}

log_facility_t log_facility_num(const char *name)
{
	if (!name)
		return LOG_FACILITY_USER;
	for (size_t i = 0; i < ARRAY_NMEMB(log_facilities); i++) {
		if (strcasecmp(name, log_facilities[i].name) == 0)
			return i;
	}
	return LOG_FACILITY_USER;
}

/* Log control functions */

const char *log_init(const char *argv0, log_facility_t facility, int flags)
{
	if (!argv0 || index(argv0, '%'))
		return "invalid argv0 value";
	if (facility < 0 || facility >= ARRAY_NMEMB(log_facilities))
		return "invalid log facility";
	if (flags & ~(LOG_F_SYSLOG | LOG_F_STDERR | LOG_F_DEVERRNO))
		return "illegal flags";

	log_flags = flags;

	if (flags & LOG_F_SYSLOG) {
		openlog(proc_name, LOG_NDELAY | LOG_NOWAIT | LOG_PID,
			log_facilities[facility].syslog_facility);

		if (log_facilities[facility].flags & LOG_FACINFO_F_WARN)
			internal_log_direct(
				NULL, 0, "log_init", LOGCH_WARN,
				"dangerous log facility specified: %s",
				log_facilities[facility].name);
	}

	return 0;
}

int log_fd_add(int fd)
{
	if (fd < 0)
		return -1;

	if (write(fd, "", 0) < 0)
		return -1;

	for (size_t i = 0; i < log_fds_count; i++) {
		if (log_fds[i] == fd) {
			errno = EEXIST;
			return -1;
		}
	}

	++log_fds_count;

	log_fds = xreallocarray(log_fds, log_fds_count, sizeof(*log_fds));

	log_fds[log_fds_count - 1] = fd;

	return 0;
}

#define scoot_up(arr, i, nmemb) \
	memmove(&(arr[i]), &(arr[i + 1]), (nmemb - i) * sizeof(*arr))

int log_fd_rm(int fd)
{
	for (size_t i = 0; i < log_fds_count; i++) {
		if (log_fds[i] != fd)
			continue;

		scoot_up(log_fds, i, log_fds_count);

		--log_fds_count;

		log_fds = xreallocarray(log_fds, log_fds_count, sizeof(*log_fds));

		return 0;
	}

	errno = ENOENT;
	return -1;
}

int log_hook_add(log_hook_fn_t hook, void *arg)
{
	if (!hook)
		return -1;

	for (size_t i = 0; i < log_hooks_count; i++) {
		if (log_hooks[i].func == hook) {
			errno = EEXIST;
			return -1;
		}
	}

	++log_hooks_count;

	log_hooks = xreallocarray(log_hooks, log_hooks_count, sizeof(*log_hooks));

	log_hooks[log_hooks_count - 1].func = hook;
	log_hooks[log_hooks_count - 1].uarg = arg;

	return 0;
}

int log_hook_rm(log_hook_fn_t hook, void *arg)
{
	for (size_t i = 0; i < log_hooks_count; i++) {
		if (log_hooks[i].func != hook)
			continue;
		if (log_hooks[i].uarg != arg)
			continue;

		scoot_up(log_hooks, i, log_hooks_count);

		--log_hooks_count;

		log_hooks = xreallocarray(log_hooks, log_hooks_count,
				     sizeof(*log_hooks));

		return 0;
	}

	errno = ENOENT;
	return -1;
}

log_level_t log_setlevel(log_level_t level)
{
	if (level == -1)
		return log_level_main;

	if (level < 0 || !log_level_name(level)) {
		errno = EINVAL;
		return -1;
	}

	return log_level_main = level;
}

static void log_to_external_facilities(const char *msg, size_t len,
				       size_t pfxlen, int sysloglevel)
{
	if (log_flags & LOG_F_STDERR) {
		write(STDERR_FILENO, msg + pfxlen, len - pfxlen);
	}
	if (log_flags & LOG_F_SYSLOG) {
		syslog(sysloglevel, "%s", msg + pfxlen);
	}
	for (size_t i = 0; i < log_fds_count; i++) {
		write(log_fds[i], msg, len);
	}
	for (size_t i = 0; i < log_hooks_count; i++) {
		log_hooks[i].func(msg, len, pfxlen, log_hooks[i].uarg);
	}
}

/* Generic Logging Functions */

void internal_log_direct(const char *file, unsigned long line, const char *func,
			 log_level_t level, const char *fmt, ...)
{
	int errno_save = errno, printloc;
	va_list ap;
	char *fmtstrp, *fmtstrend;
	const char *lvlname;
	size_t fmtstrlen, taglen, loclen, lvllen, errnolen, msglen;

	int append_errno_flag = (level & LOGCHF_ERRNO) != 0;

	level &= ~LOGCHF_ERRNO;

	lvlname = log_level_name(level);
	if (!lvlname)
		return;

	if (log_level_main < level)
		return;

	if (!file || !line || !func)
		printloc = 0;
	else
		printloc = 1;

	/*
                Parts of a logmessage:
                tag:      "%s[%llu]: ", proc_name, pid
                location: "(at %s:%d, in %s)", __FILE__, __LINE__, __func__
                level:    "%s: ", log_level_name(level)
                message:  "%s", fmt
                errno:    ": %s (%s = %d)", strerrordesc_np(errno), strerrorname_np(errno), errno
                newline:  "\n"
        */

	taglen = snprintf(NULL, 0, "%s[%llu]: ", proc_name,
			  (unsigned long long) getpid());
	lvllen = snprintf(NULL, 0, "%s", log_level_name(level));
	loclen = printloc ? snprintf(NULL, 0, " (at %s:%lu, in %s)", file, line,
				     func) :
			    0;
	errnolen = snprintf(
		NULL, 0, log_flags & LOG_F_DEVERRNO ? ": %s (%s = %d)" : ": %s",
		strerrordesc_np(errno_save), strerrorname_np(errno_save),
		errno);

	fmtstrlen = taglen + loclen + lvllen + strlen(fmt) + 2 + errnolen +
		    1 /* \n */;

	fmtstrlen += 1; // for '\0' byte

	char fmtstr[fmtstrlen + 1];

	memset(fmtstr, -1, fmtstrlen + 1);

	fmtstr[fmtstrlen] = -1;

	fmtstrp = fmtstr;
	fmtstrend = fmtstr + fmtstrlen;

	fmtstrp += snprintf(fmtstrp, fmtstrend - fmtstrp,
			    "%s[%llu]: ", proc_name,
			    (unsigned long long) getpid());
	fmtstrp += snprintf(fmtstrp, fmtstrend - fmtstrp, "%s",
			    log_level_name(level));
	fmtstrp += printloc ?
			   snprintf(fmtstrp, fmtstrend - fmtstrp,
				    " (at %s:%lu, in %s)", file, line, func) :
			   0;
	fmtstrp = stpncpy(fmtstrp, ": ", fmtstrend - fmtstrp);
	fmtstrp = stpncpy(fmtstrp, fmt, fmtstrend - fmtstrp);
	fmtstrp += append_errno_flag ?
			   snprintf(fmtstrp, fmtstrend - fmtstrp,
				    log_flags & LOG_F_DEVERRNO ?
					    ": %s (%s = %d)" :
					    ": %s",
				    strerrordesc_np(errno_save),
				    strerrorname_np(errno_save), errno) :
			   0;
	fmtstrp = stpncpy(fmtstrp, "\n", fmtstrend - fmtstrp);
	fputs(fmtstr, stderr);

	va_start(ap, fmt);
	msglen = vsnprintf(NULL, 0, fmtstr, ap);
	va_end(ap);

	char msgbuf[msglen + 1];

	va_start(ap, fmt);
	msglen = vsnprintf(msgbuf, msglen + 1, fmtstr, ap);
	va_end(ap);

	log_to_external_facilities(msgbuf, msglen, taglen,
				   log_level_infos[level].sysloglevel);

	if (log_level_infos[level].opts & LCHOPT_DIE) {
		exit(255);
	}
}
