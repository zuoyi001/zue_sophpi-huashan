#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <syslog.h>

#include "cvi_log.h"

static int syslog_level(const int lvl)
{
	switch (lvl)
	{
	case ZF_LOG_VERBOSE:
		return LOG_DEBUG;
	case ZF_LOG_DEBUG:
		return LOG_DEBUG;
	case ZF_LOG_INFO:
		return LOG_INFO;
	case ZF_LOG_WARN:
		return LOG_WARNING;
	case ZF_LOG_ERROR:
		return LOG_ERR;
	case ZF_LOG_FATAL:
		return LOG_EMERG;
	default:
		assert(!"can't be");
		return LOG_EMERG;
	}
}

static void syslog_output_callback(const zf_log_message *msg, void *arg)
{
	(void)arg;
	/* p points to the log message end. By default, message is not terminated
	 * with 0, but it has some space allocated for EOL area, so there is always
	 * some place for terminating zero in the end (see ZF_LOG_EOL_SZ define in
	 * zf_log.c).
	 */
	*msg->p = 0;
	syslog(syslog_level(msg->lvl), "%s", msg->tag_b);
}

static void syslog_output_close(void)
{
    closelog();
}

static void syslog_output_open(void)
{
	openlog("cvi", LOG_CONS|LOG_PERROR|LOG_PID, LOG_USER);
    //openlog("cvi", LOG_NDELAY | LOG_PID, LOG_DAEMON);

	const unsigned put_mask = ZF_LOG_PUT_STD & !ZF_LOG_PUT_CTX;
	zf_log_set_output_v(put_mask, 0, syslog_output_callback);
    atexit(syslog_output_close);
}

static FILE *g_log_file;

static void file_output_callback(const zf_log_message *msg, void *arg)
{
	(void)arg;
	*msg->p = '\n';
	fwrite(msg->buf, msg->p - msg->buf + 1, 1, g_log_file);
	fflush(g_log_file);
}

static void file_output_close(void)
{
	fclose(g_log_file);
}

static void file_output_open(const char *const log_path)
{
	g_log_file = fopen(log_path, "a");
	if (!g_log_file)
	{
		ZF_LOGW("Failed to open log file %s", log_path);
		return;
	}
	atexit(file_output_close);
	zf_log_set_output_v(ZF_LOG_PUT_STD, 0, file_output_callback);
}

void CVI_LOG_INIT(void) {
    char *log_level = getenv(CVI_LOG_LEVEL_VAR_NAME);
    if (log_level) {
        switch(log_level[0]) {
        case 'N':
            CVI_LOG_SET_LEVEL(CVI_LOG_NONE);
            break;
        case 'F':
            CVI_LOG_SET_LEVEL(CVI_LOG_FATAL);
            break;
        case 'E':
            CVI_LOG_SET_LEVEL(CVI_LOG_ERROR);
            break;
        case 'W':
            CVI_LOG_SET_LEVEL(CVI_LOG_WARN);
            break;
        case 'I':
            CVI_LOG_SET_LEVEL(CVI_LOG_INFO);
            break;
        case 'D':
            CVI_LOG_SET_LEVEL(CVI_LOG_DEBUG);
            break;
        case 'V':
            CVI_LOG_SET_LEVEL(CVI_LOG_VERBOSE);
            break;
        default:
            CVI_LOG_SET_LEVEL(CVI_LOG_INFO);
            break;
        }
    }

    char *log_filename = getenv(CVI_LOG_FILE_VAR_NAME);
    if (log_filename) {
        file_output_open(log_filename);
        return;
    }

    char *log_syslog = getenv(CVI_LOG_SYSLOG_VAR_NAME);
    if (log_syslog && strcmp(log_syslog, "true") == 0) {
        syslog_output_open();
    }

    return;
}

void CVI_LOG_SET_LEVEL(const int lvl) {
    zf_log_set_output_level(lvl);
}

void CVI_LOG_SET_TAG(const char *const tag) {
    zf_log_set_tag_prefix(tag);
}
