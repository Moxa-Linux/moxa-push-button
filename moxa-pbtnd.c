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
extern char mx_errmsg[256];

/*
 * json-c utilities
 */

static inline int obj_get_obj(struct json_object *obj, char *key, struct json_object **val)
{
	if (!json_object_object_get_ex(obj, key, val)) {
		fprintf(stderr, "json-c: can\'t get key: \"%s\"", key);
		return -1;
	}
	return 0;
}

static int obj_get_int(struct json_object *obj, char *key, int *val)
{
	struct json_object *tmp;

	if (obj_get_obj(obj, key, &tmp) < 0)
		return -1;

	*val = json_object_get_int(tmp);
	return 0;
}

static int obj_get_str(struct json_object *obj, char *key, const char **val)
{
	struct json_object *tmp;

	if (obj_get_obj(obj, key, &tmp) < 0)
		return -1;

	*val = json_object_get_string(tmp);
	return 0;
}

static int obj_get_arr(struct json_object *obj, char *key, struct array_list **val)
{
	struct json_object *tmp;

	if (obj_get_obj(obj, key, &tmp) < 0)
		return -1;

	*val = json_object_get_array(tmp);
	return 0;
}

static int arr_get_obj(struct array_list *arr, int idx, struct json_object **val)
{
	if (arr == NULL || idx >= arr->length) {
		fprintf(stderr, "json-c: can\'t get index: %d", idx);
		return -1;
	}

	*val = array_list_get_idx(arr, idx);
	return 0;
}

static int arr_get_int(struct array_list *arr, int idx, int *val)
{
	struct json_object *tmp;

	if (arr_get_obj(arr, idx, &tmp) < 0)
		return -1;

	*val = json_object_get_int(tmp);
	return 0;
}

static int arr_get_str(struct array_list *arr, int idx, const char **val)
{
	struct json_object *tmp;

	if (arr_get_obj(arr, idx, &tmp) < 0)
		return -1;

	*val = json_object_get_string(tmp);
	return 0;
}

static int arr_get_arr(struct array_list *arr, int idx, struct array_list **val)
{
	struct json_object *tmp;

	if (arr_get_obj(arr, idx, &tmp) < 0)
		return -1;

	*val = json_object_get_array(tmp);
	return 0;
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
		fprintf(stderr, "json-c: load file %s failed\n", CONF_FILE);
		exit(1);
	}

	if (obj_get_arr(obj, "DEFAULT_ACTIONS", &arr) < 0)
		exit(1);

	if (arr_get_obj(arr, 0, &config) < 0)
		exit(1);
}

void do_action(struct json_object *action)
{
	int group, index;
	const char *state, *message, *cmd;

	if (obj_get_int(action, "LED_GROUP", &group) < 0)
		exit(1);
	if (obj_get_int(action, "LED_INDEX", &index) < 0)
		exit(1);
	if (obj_get_str(action, "LED_STATE", &state) < 0)
		exit(1);

	if (mx_led_set_type_all(LED_TYPE_PROGRAMMABLE, LED_STATE_OFF) < 0) {
		fprintf(stderr, "%s\n", mx_errmsg);
		exit(2);
	}

	if (strcmp(state, "off") == 0) {
		if (mx_led_set_brightness(LED_TYPE_PROGRAMMABLE,
			group, index, LED_STATE_OFF) < 0) {
			fprintf(stderr, "%s\n", mx_errmsg);
			exit(2);
		}
	} else if (strcmp(state, "on") == 0) {
		if (mx_led_set_brightness(LED_TYPE_PROGRAMMABLE,
			group, index, LED_STATE_ON) < 0) {
			fprintf(stderr, "%s\n", mx_errmsg);
			exit(2);
		}
	} else if (strcmp(state, "blink") == 0) {
		if (mx_led_set_brightness(LED_TYPE_PROGRAMMABLE,
			group, index, LED_STATE_BLINK) < 0) {
			fprintf(stderr, "%s\n", mx_errmsg);
			exit(2);
		}
	}

	if (obj_get_str(action, "MESSAGE", &message) < 0)
		exit(1);
	printf("%s\n", message);

	if (obj_get_str(action, "EXEC_CMD", &cmd) < 0)
		exit(1);
	system(cmd);
}

void pressed_func(int sec)
{
	struct array_list *actions;
	struct json_object *action;

	if (obj_get_arr(config, "PRESS_ACTION", &actions) < 0)
		exit(1);

	if (arr_get_obj(actions, 0, &action) < 0)
		exit(1);

	do_action(action);
}

void released_func(int sec)
{
	struct array_list *actions;
	struct json_object *action;
	int i, t;

	if (obj_get_arr(config, "RELEASE_ACTION", &actions) < 0)
		exit(1);
	for (i = 0; i < array_list_length(actions); i++) {
		if (arr_get_obj(actions, i, &action) < 0)
			exit(1);

		if (obj_get_int(action, "SEC", &t) < 0)
			exit(1);

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

	if (obj_get_arr(config, "HOLD_ACTION", &actions) < 0)
		exit(1);
	for (i = 0; i < array_list_length(actions); i++) {
		if (arr_get_obj(actions, i, &action) < 0)
			exit(1);

		if (obj_get_int(action, "SEC", &t) < 0)
			exit(1);

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
	if (mx_pbtn_init() < 0) {
		fprintf(stderr, "Initialize Moxa push button library failed\n");
		fprintf(stderr, "%s\n", mx_errmsg);
		exit(3);
	}
	if (mx_led_init() < 0) {
		fprintf(stderr, "Initialize Moxa LED control library failed\n");
		fprintf(stderr, "%s\n", mx_errmsg);
		exit(2);
	}

	btn_id = mx_pbtn_open(BUTTON_TYPE_SYSTEM, 1);
	if (btn_id < 0) {
		fprintf(stderr, "%s\n", mx_errmsg);
		exit(3);
	}

	if (mx_pbtn_pressed_event(btn_id, &pressed_func) < 0) {
		fprintf(stderr, "%s\n", mx_errmsg);
		exit(3);
	}
	if (mx_pbtn_released_event(btn_id, &released_func) < 0) {
		fprintf(stderr, "%s\n", mx_errmsg);
		exit(3);
	}
	if (mx_pbtn_hold_event(btn_id, &hold_func, DURATION_EVERY) < 0) {
		fprintf(stderr, "%s\n", mx_errmsg);
		exit(3);
	}

	if (mx_pbtn_wait() < 0) {
		fprintf(stderr, "%s\n", mx_errmsg);
		exit(3);
	}

	if (mx_pbtn_close(btn_id) < 0) {
		fprintf(stderr, "%s\n", mx_errmsg);
		exit(3);
	}

	exit(0);
}
