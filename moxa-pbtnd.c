/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Name:
 *	MOXA Push button Daemon
 *
 * Description:
 *	Daemon that loads default actions for system type push button.
 *
 * Authors:
 *	2014	SZ Lin		<sz.lin@moxa.com>
 *	2017	Fero JD Zhou	<FeroJD.Zhou@moxa.com>
 *	2018	Ken CJ Chou	<KenCJ.Chou@moxa.com>
 */

#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include <moxa/mx_led.h>

#include "mx_pbtn.h"

#define CONF_FILE "/etc/moxa-configs/moxa-push-button.json"

static struct json_object *config;

/*
 * json-c utilities
 */

static struct json_object *obj_get_obj(struct json_object *obj, char *key)
{
	struct json_object *val;

	if (!json_object_object_get_ex(obj, key, &val)) {
		fprintf(stderr, "json-c parsing error at key: %s\n", key);
		exit(-1);
	}
	return val;
}

static inline int obj_get_int(struct json_object *obj, char *key)
{
	return json_object_get_int(obj_get_obj(obj, key));
}

static inline const char *obj_get_str(struct json_object *obj, char *key)
{
	return json_object_get_string(obj_get_obj(obj, key));
}

static inline struct array_list *obj_get_arr(struct json_object *obj, char *key)
{
	return json_object_get_array(obj_get_obj(obj, key));
}

static inline struct json_object *arr_get_obj(struct array_list *arr, int idx)
{
	return array_list_get_idx(arr, idx);
}

static inline int arr_get_int(struct array_list *arr, int idx)
{
	return json_object_get_int(array_list_get_idx(arr, idx));
}

static inline const char *arr_get_str(struct array_list *arr, int idx)
{
	return json_object_get_string(array_list_get_idx(arr, idx));
}

static inline struct array_list *arr_get_arr(struct array_list *arr, int idx)
{
	return json_object_get_array(array_list_get_idx(arr, idx));
}

/*
 * main part
 */

static void init(void)
{
	struct json_object *obj;
	struct array_list *arr;

	obj = json_object_from_file(CONF_FILE);
	if (obj == NULL) {
		fprintf(stderr, "failed to open config file: %s\n", CONF_FILE);
		exit(-1);
	}

	arr = obj_get_arr(obj, "DEFAULT_ACTIONS");
	config = arr_get_obj(arr, 0);
}

void do_action(struct json_object *action)
{
	int group, index;
	const char *state;

	group = obj_get_int(action, "LED_GROUP");
	index = obj_get_int(action, "LED_INDEX");
	state = obj_get_str(action, "LED_STATE");

	mx_led_set_type_all(LED_TYPE_PROGRAMMABLE, LED_STATE_OFF);

	if (strcmp(state, "off") == 0) {
		mx_led_set_brightness(LED_TYPE_PROGRAMMABLE,
			group, index, LED_STATE_OFF);
	} else if (strcmp(state, "on") == 0) {
		mx_led_set_brightness(LED_TYPE_PROGRAMMABLE,
			group, index, LED_STATE_ON);
	} else if (strcmp(state, "blink") == 0) {
		mx_led_set_brightness(LED_TYPE_PROGRAMMABLE,
			group, index, LED_STATE_BLINK);
	}

	printf("%s\n", obj_get_str(action, "MESSAGE"));
	system(obj_get_str(action, "EXEC_CMD"));
}

void pressed_func(int sec)
{
	struct array_list *actions;
	struct json_object *action;

	actions = obj_get_arr(config, "PRESS_ACTION");
	action = arr_get_obj(actions, 0);
	do_action(action);
}

void released_func(int sec)
{
	struct array_list *actions;
	struct json_object *action;
	int i, t;

	actions = obj_get_arr(config, "RELEASE_ACTION");
	for (i = 0; i < array_list_length(actions); i++) {
		action = arr_get_obj(actions, i);
		t = obj_get_int(action, "SEC");

		if (sec >= t) {
			do_action(action);
			break;
		}
	}
}

void hold_func(int sec)
{
	struct array_list *actions;
	struct json_object *action;
	int i, t;

	actions = obj_get_arr(config, "HOLD_ACTION");
	for (i = 0; i < array_list_length(actions); i++) {
		action = arr_get_obj(actions, i);
		t = obj_get_int(action, "SEC");

		if (sec > t) {
			break;
		} else if (sec == t) {
			do_action(action);
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	int btn_id;

	init();
	mx_pbtn_init();
	mx_led_init();

	btn_id = mx_pbtn_open(BUTTON_TYPE_SYSTEM, 1);
	if (btn_id < 0)
		exit(-1);

	mx_pbtn_pressed_event(btn_id, &pressed_func);
	mx_pbtn_released_event(btn_id, &released_func);
	mx_pbtn_hold_event(btn_id, &hold_func, DURATION_EVERY);

	mx_pbtn_wait();

	mx_pbtn_close(btn_id);

	exit(0);
}
