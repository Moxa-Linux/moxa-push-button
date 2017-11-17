/*##############################################################################
# Copyright (C) MOXA Inc. All rights reserved.
#
# This software is distributed under the terms of the
# MOXA License.  See the file COPYING-MOXA for details.
#
# Purpose : MOXA LED library
# Return : 0 on success; Other wise, modification failed.
# Author : Lock Lin
# Date : 2014-03-17
# Version : 1.0
# Notes :
##############################################################################*/

#include <stdint.h>
#include <linux/input.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#define MAXLINE 128

#define DEVICE_NODE	"/dev/input/event0"

int mx_btn_event_detect(int fd, int sec)
{
	struct timeval timeout;
	fd_set read_fds;

	FD_ZERO(&read_fds);
	FD_SET(fd, &read_fds);

	timeout.tv_sec = sec;
	timeout.tv_usec = 0;

	return select(fd + 1, &read_fds, NULL, NULL, &timeout);
}

int onoff_led(char *group, char *idx, char *status)
{
	FILE *fp;
	int rt_val;
	char cmd[MAXLINE];

	memset(cmd, '\0', MAXLINE);
	snprintf(cmd, sizeof(cmd), "/sbin/mx-led-ctl -p %s -i %s %s",
group, idx, status);

	fp = popen(cmd, "r");
	if (NULL == fp) {
		perror("Cannot execute cmd\n");
		return -1;
	}
	rt_val = pclose(fp);
	return rt_val;
}

int turn_all_led(char *state)
{
	FILE *fp;
	int rt_val;
	char cmd[MAXLINE];

	memset(cmd, '\0', MAXLINE);
	snprintf(cmd, sizeof(cmd), "/sbin/mx-led-ctl --all-programmable %s",
state);

	fp = popen(cmd, "r");
	if (NULL == fp) {
		perror("Cannot execute cmd\n");
		return -1;
	}
	rt_val = pclose(fp);
	return rt_val;
}

void exec_cmd(char *bin_name)
{
	system(bin_name);
#if 0
	char **cmd[32];
	FILE *fp;
	char rt_val[32];
	char cmd[MAXLINE];

	memset(cmd, '\0', MAXLINE);
	memset(rt_val, '\0', sizeof(rt_val));
	snprintf(cmd, sizeof(cmd), "%s", bin_name);
	fp = popen(cmd, "r");
	printf("cmd=%s\n", cmd);
	if (NULL == fp) {
		perror("Cannot execute cmd\n");
		exit(1);
	}
	fgets(rt_val, 32, fp);
	pclose(fp);
	printf("Exit\n");
#endif
}

int mx_open_device(void)
{
	return open(DEVICE_NODE, O_RDONLY);
}

int mx_close_device(int fd)
{
	close(fd);
}
