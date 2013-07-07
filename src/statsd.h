/**
 * Libstatsd
 * 
 * @author Axel Etcheverry (http://twitter.com/euskadi31)
 * @copyright Copyright (c) 2013 Axel Etcheverry
 * @license http://creativecommons.org/licenses/MIT/deed.fr MIT
 */
#ifndef __STATSD_H
#define __STATSD_H

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX_MSG_LEN 100
#define STATSD_DEFAULT_PORT 8125


typedef struct {
    struct sockaddr_in server;
    int sock;
    char *ns;
} statsd_t;

/**
 * Initialize statsd connexion
 * 
 * @param  host of statsd server
 * @param  port of statsd server
 */
statsd_t *statsd_init(const char *host, int port);

/**
 * Initialize statsd connexion
 * 
 * @param  host of statsd server
 * @param  port of statsd server
 * @param  ns prefix
 */
statsd_t *statsd_init_with_namespace(
    const char *host, 
    int port, 
    const char *ns
);

/**
 * 
 * @param link
 */
void statsd_finalize(statsd_t *link);

/**
 * Write the stat line to the provided buffer,
 * type can be "c", "g" or "ms"
 * lf - whether line feed needs to be added
 * 
 * @param link
 * @param stat
 * @param value
 * @param type
 * @param sample_rate
 * @param buf
 * @param buflen
 * @param lf
 */
void statsd_prepare(
    statsd_t *link, 
    char *stat, 
    size_t value, 
    const char *type, 
    float sample_rate, 
    char *buf, 
    size_t buflen, 
    int lf
);

/**
 * manually send a message, which might be composed of several lines. 
 * Must be null-terminated
 * 
 * @param  link
 * @param  message
 */
int statsd_send(statsd_t *link, const char *message);

/**
 * 
 * @param  link
 * @param  stat
 * @param  sample_rate
 */
int statsd_inc(statsd_t *link, char *stat, float sample_rate);

/**
 * 
 * @param  link
 * @param  stat
 * @param  sample_rate
 */
int statsd_dec(statsd_t *link, char *stat, float sample_rate);

/**
 * 
 * @param  link
 * @param  stat
 * @param  count
 * @param  sample_rate
 */
int statsd_count(statsd_t *link, char *stat, size_t count, float sample_rate);

/**
 * 
 * @param  link
 * @param  stat
 * @param  value
 */
int statsd_gauge(statsd_t *link, char *stat, size_t value);

/**
 * 
 * @param  link
 * @param  stat
 * @param  ms
 */
int statsd_timing(statsd_t *link, char *stat, size_t ms);

#endif