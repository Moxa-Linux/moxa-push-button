/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Name:
 *	C Example code for MOXA Push button Library
 *
 * Description:
 *	Example code for demonstrating the usage of
 *	MOXA Push button Library in C
 *
 * Authors:
 *	2018	Ken CJ Chou	<KenCJ.Chou@moxa.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <moxa/mx_pbtn.h>

void pressed_func(int sec)
{
	printf("  This is in pressed_func, sec: %d\n", sec);
}

void released_func(int sec)
{
	printf("  This is in released_func, sec: %d\n", sec);
}

void hold_func(int sec)
{
	printf("  This is in hold_func, sec: %d\n", sec);
}

int main(int argc, char *argv[])
{
	int ret, btn_id;
	unsigned long hold_sec;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <hold_seconds>\n", argv[0]);
		fprintf(stderr, "	0: function triggered every second\n");
		fprintf(stderr, "	1 ~ 3600: function triggered over hold_seconds\n");
		exit(99);
	}
	hold_sec = atol(argv[1]);

	ret = mx_pbtn_init();
	if (ret < 0) {
		fprintf(stderr, "Error: Initialize Moxa pbtn control library failed\n");
		fprintf(stderr, "Return code: %d\n", ret);
		exit(1);
	}

	printf("Testing Push button:\n");
	printf("========================================\n");

	btn_id = mx_pbtn_open(BUTTON_TYPE_SYSTEM, 1);
	if (btn_id < 0) {
		fprintf(stderr, "Error: Open button failed\n");
		fprintf(stderr, "Return code: %d\n", btn_id);
		exit(1);
	}

	ret = mx_pbtn_pressed_event(btn_id, &pressed_func);
	if (ret < 0) {
		fprintf(stderr, "Error: Set pressed function failed\n");
		fprintf(stderr, "Return code: %d\n", ret);
		exit(1);
	}

	ret = mx_pbtn_released_event(btn_id, &released_func);
	if (ret < 0) {
		fprintf(stderr, "Error: Set released function failed\n");
		fprintf(stderr, "Return code: %d\n", ret);
		exit(1);
	}

	ret = mx_pbtn_hold_event(btn_id, &hold_func, hold_sec);
	if (ret < 0) {
		fprintf(stderr, "Error: Set hold function failed\n");
		fprintf(stderr, "Return code: %d\n", ret);
		exit(1);
	}

	printf("- Start testing. Press button to check the functions.\n");
	printf("  [Ctrl + C] to terminate this process.\n");

	mx_pbtn_wait();

	mx_pbtn_close(btn_id);
	exit(0);
}
