/*##############################################################################
# Copyright (C) MOXA Inc. All rights reserved.
#
# This software is distributed under the terms of the
# MOXA License.  See the file COPYING-MOXA for details.
#
# Purpose : Push button device
# Return : 0 on success; Other wise, modification failed.
# Author : Lock Lin
# Date : 2014-03-17
# Version : 1.0
# Notes :
# 2016-2017 Fero JD Zhou <FeroJD.Zhou@moxa.com>
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
#include <json-c/json.h>

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)    fprintf(stderr, fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif

#define CONFIG_FILE "/etc/moxa-configs/moxa-push-button.json"

struct action {
	int interval;
	const char *led_group;
	const char *led_idx;
	const char *led_state;
	const char *debug_msg;
	const char *cmd;
	struct action *release;
};

const char *get_string_from_obj(struct json_object *obj, char *key)
{
	struct json_object *val;

	json_object_object_get_ex(obj, key, &val);
	return json_object_get_string(val);
}

void load_action_from_object(struct action *ac, struct json_object *obj)
{
	ac->interval = atoi(get_string_from_obj(obj, "INTERVAL"));
	ac->led_group = get_string_from_obj(obj, "LED_GROUP");
	ac->led_idx = get_string_from_obj(obj, "LED_IDX");
	ac->led_state = get_string_from_obj(obj, "LED_STATE");
	ac->debug_msg = get_string_from_obj(obj, "DEBUG_MSG");
	ac->cmd = get_string_from_obj(obj, "EXEC_CMD");
}

struct json_object *get_json_obj_from_key(struct json_object *obj, char *key)
{
	struct json_object *val;

	if (!json_object_object_get_ex(obj, key, &val)) {
		printf("Parsing error at key:%s\n", key);
		exit(-1);
	}
	return val;
}

void load_configuration(struct action **list, char *config_name, int *len)
{
	struct json_object *obj = json_object_from_file(config_name);
	struct array_list *arr;
	int i;

	obj = get_json_obj_from_key(obj, "BUTTON1");
	obj = get_json_obj_from_key(obj, "ACTION");
	arr = json_object_get_array(obj);
	*len = array_list_length(arr);
	*list = (struct action *)malloc(sizeof(struct action)*(*len));
	for (i = 0; i < *len; i++) {
		obj = array_list_get_idx(arr, i);
		load_action_from_object(&(*list)[i], obj);
		obj = get_json_obj_from_key(obj, "RELEASE");
		(*list)[i].release =
(struct action *)malloc(sizeof(struct action));
		load_action_from_object((*list)[i].release, obj);
	}
}

static void action_unit_handler(struct action *ac)
{
	if (strcmp(ac->led_idx, "all") == 0)
		turn_all_led(ac->led_state);
	else
		onoff_led(ac->led_group, ac->led_idx, ac->led_state);
}

static void action_handler(int fd, struct action *list, int len)
{
	/*FIXME: We are not sure whether the json_object handles self-destruct*/
	/*automatically or not. May need to run valgrind to check.*/
	struct action *cur_action;
	int i;

	for (i = 0; i < len; i++) {
		turn_all_led("off");
		cur_action = &list[i];
		printf("debug message: %s\n", cur_action->debug_msg);
		action_unit_handler(cur_action);
		if (mx_btn_event_detect(fd, cur_action->interval) == 1) {
			printf("enter during mode(button released!!)\n");
			cur_action = list[i].release;
			action_unit_handler(cur_action);
			exec_cmd(cur_action->cmd);
			break;
		} else {
			printf("enter outside mode\n");
		}
	}
}

int main(int argc, char **argv)
{
	int i, fd, ret, rd, len=0;
	struct input_event ev[64];
	struct action *list;

	memset(ev, 0, sizeof(struct input_event)*64);
	fd = mx_open_device();

	if (fd < 0) {
		perror("mx-pbtn");
		return 1;
	}

	load_configuration(&list, CONFIG_FILE, &len);

	turn_all_led("off");
	DEBUG_PRINT("Testing ... (interrupt to exit)\n");

	while (1) {
		rd = read(fd, ev, sizeof(struct input_event) * 64);

		if (rd < (int) sizeof(struct input_event)) {
			perror("\nmx-pbtn: error reading");
			return 1;
		}

		if (ev[0].value == 0)
			continue;
		else {
			turn_all_led("off");
			action_handler(fd, list, len);
		}
	}
	for (i=0; i<len; i++) {
		free(list[i].release);
	}
	free(list);
	mx_close_device(fd);
}

