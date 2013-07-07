/**
 * Libstatsd
 * 
 * @author Axel Etcheverry (http://twitter.com/euskadi31)
 * @copyright Copyright (c) 2013 Axel Etcheverry
 * @license http://creativecommons.org/licenses/MIT/deed.fr MIT
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "statsd.h"

/* will change the original string */
static void cleanup(char *stat) {
    char *p;
    for (p = stat; *p; p++) {
        if (*p == ':' || *p == '|' || *p == '@') {
            *p = '_';
        }
    }
}

static int should_send(float sample_rate) {
    if (sample_rate < 1.0) {
        float p = ((float)random() / RAND_MAX);
        return sample_rate > p;
    } else {
        return 1;
    }
}

static int send_stat(
    statsd_t *link, 
    char *stat, 
    size_t value, 
    const char *type, 
    float sample_rate
) {
    char message[MAX_MSG_LEN];

    if (!should_send(sample_rate)) {
        return 0;
    }

    statsd_prepare(
        link, 
        stat, 
        value, 
        type, 
        sample_rate, 
        message, 
        MAX_MSG_LEN, 
        0
    );

    return statsd_send(link, message);
}

statsd_t *statsd_init_with_namespace(
    const char *host, 
    int port, 
    const char *ns_
) {
    size_t len = strlen(ns_);

    statsd_t *temp = statsd_init(host, port);

    if ((temp->ns = malloc(len + 2)) == NULL) {
        perror("malloc");
        return NULL;
    }

    strcpy(temp->ns, ns_);
    temp->ns[len++] = '.';
    temp->ns[len] = 0;

    return temp;
}

statsd_t *statsd_init(const char *host, int port) {
    statsd_t *temp = malloc(sizeof(statsd_t));

    if ((temp->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("socket");
        return NULL;
    }

    memset(&temp->server, 0, sizeof(temp->server));

    temp->server.sin_family = AF_INET;
    temp->server.sin_port = htons(port);

    struct addrinfo *result = NULL, hints;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int error;

    if ((error = getaddrinfo(host, NULL, &hints, &result))) {
        fprintf(stderr, "%s\n", gai_strerror(error));
        return NULL;
    }

    memcpy(
        &(temp->server).sin_addr, 
        &((struct sockaddr_in*)result->ai_addr)->sin_addr, 
        sizeof(struct in_addr)
    );

    freeaddrinfo(result);

    if (inet_aton(host, &(temp->server).sin_addr) == 0) {
        perror("inet_aton");
        return NULL;
    }

    srandom(time(NULL));

    return temp;
}

void statsd_finalize(statsd_t *link) {
    // close socket
    if (link->sock != -1) {
        close(link->sock);
        link->sock = -1;
    }

    // freeing ns
    if (link->ns) {
        free(link->ns);
        link->ns = NULL;
    }

    // free sockaddr_in
    free(&link->server);

    // free whole link
    //free(&link);
}

int statsd_send(statsd_t *link, const char *message) {
    int slen = sizeof(link->server);

    if (sendto(
        link->sock, 
        message, 
        strlen(message), 
        0, 
        (struct sockaddr *) &link->server, 
        slen
    ) == -1) {
        perror("sendto");
        return -1;
    }

    return 0;
}

void statsd_prepare(
    statsd_t *link, 
    char *stat, 
    size_t value, 
    const char *type, 
    float sample_rate, 
    char *message, 
    size_t buflen, 
    int lf
) {
    cleanup(stat);

    if (sample_rate == 1.0) {
        snprintf(
            message, 
            buflen, 
            "%s%s:%zd|%s%s", 
            link->ns ? link->ns : "", 
            stat, 
            value, 
            type, 
            lf ? "\n" : ""
        );
    } else {
        snprintf(
            message, 
            buflen, 
            "%s%s:%zd|%s|@%.2f%s", 
            link->ns ? link->ns : "", 
            stat, 
            value, 
            type, 
            sample_rate, 
            lf ? "\n" : ""
        );
    }
}


int statsd_count(statsd_t *link, char *stat, size_t value, float sample_rate) {
    return send_stat(link, stat, value, "c", sample_rate);
}

int statsd_dec(statsd_t *link, char *stat, float sample_rate) {
    return statsd_count(link, stat, -1, sample_rate);
}

int statsd_inc(statsd_t *link, char *stat, float sample_rate) {
    return statsd_count(link, stat, 1, sample_rate);
}

int statsd_gauge(statsd_t *link, char *stat, size_t value) {
    return send_stat(link, stat, value, "g", 1.0);
}

int statsd_timing(statsd_t *link, char *stat, size_t ms) {
    return send_stat(link, stat, ms, "ms", 1.0);
}