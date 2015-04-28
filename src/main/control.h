/*
 * control.h
 *
 *  Created on: May 3, 2014
 *      Author: underscore
 */

#ifndef __CONTROL_H__
#define __CONTROL_H__

/**
 * Compilation flag to enable local testing.
 */
#define STANDALONE 	0
#define DEBUG		0
#define BITTORRENT  1

#include <stdio.h>
#include <unistd.h>


#if not STADALONE
#include "config.h"
#include "boinc_api.h"
#endif

/**
 * Some methods that only exist on some particular configurations.
 */

char debug_buf[256];

#if STANDALONE
char* boinc_msg_prefix(char* buf, int size) {
	return strcpy(buf, "[STANDALONE]");
}
void boinc_sleep(double s) { sleep((int)s); }
#endif

#if DEBUG
void debug_log(const char* where, const char* what, const char* aux) {
	fprintf(stderr,
			"%s [%s] %s %s\n",
			boinc_msg_prefix(debug_buf, sizeof(debug_buf)), where, what, aux);
}
#endif

#endif /* CONTROL_H_ */
