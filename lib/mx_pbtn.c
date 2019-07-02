/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Name:
 *	MOXA Push button Library
 *
 * Description:
 *	Library for setting Push button to trigger actions on input events.
 *
 * Authors:
 *	2014	SZ Lin		<sz.lin@moxa.com>
 *	2017	Fero JD Zhou	<FeroJD.Zhou@moxa.com>
 *	2018	Ken CJ Chou	<KenCJ.Chou@moxa.com>
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/file.h>
#include <linux/input.h>
#include <json-c/json.h>
#include <mx_pbtn.h>

#define CONF_FILE "/etc/moxa-configs/moxa-push-button.json"
#define CONF_VER_SUPPORTED "1.1.*"
#define MAX_FILEPATH_LEN 256	/* reserved length for file path */

struct button_struct {
	int fd;
	pthread_t thread;
	int is_opened;
	int is_pressed;
	void (*pressed_func)(int sec);
	void (*released_func)(int sec);
	void (*hold_func)(int sec);
	int hold_duration;
};

static int lib_initialized;
static struct json_object *config;
static struct button_struct *buttons;
extern char mx_errmsg[256];

/*
 * json-c utilities
 */

static inline int obj_get_obj(struct json_object *obj, char *key, struct json_object **val)
{
	if (!json_object_object_get_ex(obj, key, val)) {
		sprintf(mx_errmsg, "json-c: can\'t get key: \"%s\"", key);
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
		sprintf(mx_errmsg, "json-c: can\'t get index: %d", idx);
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
 * static functions
 */

static int check_config_version_supported(const char *conf_ver)
{
	int cv[2], sv[2];

	if (sscanf(conf_ver, "%d.%d.%*s", &cv[0], &cv[1]) < 0) {
		sprintf(mx_errmsg, "sscanf: %s: %s", conf_ver, strerror(errno));
		return -1; /* E_SYSFUNCERR */
	}

	if (sscanf(CONF_VER_SUPPORTED, "%d.%d.%*s", &sv[0], &sv[1]) < 0) {
		sprintf(mx_errmsg, "sscanf: %s: %s", CONF_VER_SUPPORTED, strerror(errno));
		return -1; /* E_SYSFUNCERR */
	}

	if (cv[0] != sv[0] || cv[1] != sv[1]) {
		sprintf(mx_errmsg, "Config version not supported, need to be %s", CONF_VER_SUPPORTED);
		return -4; /* E_UNSUPCONFVER */
	}
	return 0;
}

static int get_button(int btn_id, struct button_struct **button)
{
	int num_of_all_btns;

	if (obj_get_int(config, "NUM_OF_ALL_BUTTONS", &num_of_all_btns) < 0)
		return -5; /* E_CONFERR */

	if (btn_id < 0 || btn_id > num_of_all_btns - 1) {
		sprintf(mx_errmsg, "Button ID out of index: %d", btn_id);
		return -2; /* E_INVAL */
	}

	*button = &buttons[btn_id];
	return 0;
}

static int detect_input(int fd)
{
	fd_set read_fds;
	struct timeval timeout;

	FD_ZERO(&read_fds);
	FD_SET(fd, &read_fds);

	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	return select(fd + 1, &read_fds, NULL, NULL, &timeout);
}

static void handle_event(struct button_struct *button)
{
	int sec = 0;

	if (button->pressed_func != NULL)
		button->pressed_func(0);

	while (1) {
		sec++;

		if (detect_input(button->fd) == 1) {
			button->is_pressed = 0;
			if (button->released_func != NULL)
				button->released_func(sec - 1);
			break;
		}

		if (button->hold_func != NULL) {
			if (button->hold_duration == DURATION_EVERY)
				button->hold_func(sec);
			else if (sec == button->hold_duration)
				button->hold_func(sec);
		}
	}
}

static void *thread_start(void *arg)
{
	struct button_struct *button = (struct button_struct *) arg;
	struct input_event events[64];
	int read_len;

	memset(events, 0, sizeof(struct input_event) * 64);

	while (1) {
		read_len = read(button->fd, events
			, sizeof(struct input_event) * 64);

		if (read_len < (int) sizeof(struct input_event)) {
			perror("read input error");
			/* to do: error handling */
			return NULL;
		}

		if (events[0].value == 0) {
			continue;
		} else {
			button->is_pressed = 1;
			handle_event(button);
		}
	}
	return NULL;
}

/*
 * APIs
 */

int mx_pbtn_init(void)
{
	int ret, num_of_all_btns, i;
	const char *conf_ver;

	if (lib_initialized)
		return 0;

	config = json_object_from_file(CONF_FILE);
	if (config == NULL) {
		sprintf(mx_errmsg, "json-c: load file %s failed", CONF_FILE);
		return -5; /* E_CONFERR */
	}

	if (obj_get_str(config, "CONFIG_VERSION", &conf_ver) < 0)
		return -5; /* E_CONFERR */

	ret = check_config_version_supported(conf_ver);
	if (ret < 0)
		return ret;

	if (obj_get_int(config, "NUM_OF_ALL_BUTTONS", &num_of_all_btns) < 0)
		return -5; /* E_CONFERR */

	buttons = (struct button_struct *)
		malloc(num_of_all_btns * sizeof(struct button_struct));
	if (buttons == NULL) {
		sprintf(mx_errmsg, "malloc: %s", strerror(errno));
		return -1; /* E_SYSFUNCERR */
	}

	for (i = 0; i < num_of_all_btns; i++) {
		buttons[i].is_opened = 0;
		buttons[i].pressed_func = NULL;
		buttons[i].released_func = NULL;
		buttons[i].hold_func = NULL;
	}

	lib_initialized = 1;
	return 0;
}

int mx_pbtn_open(int type, int index)
{
	struct array_list *btn_types, *btn_paths;
	struct json_object *btn_type_info;
	struct button_struct *button;
	const char *filepath;
	int ret, btn_id;

	if (!lib_initialized) {
		sprintf(mx_errmsg, "Library is not initialized");
		return -3; /* E_LIBNOTINIT */
	}

	if (obj_get_arr(config, "BUTTON_TYPES", &btn_types) < 0)
		return -5; /* E_CONFERR */

	if (arr_get_obj(btn_types, BUTTON_TYPE_SYSTEM, &btn_type_info) < 0)
		return -5; /* E_CONFERR */

	btn_id = index - 1;
	if (type == BUTTON_TYPE_USER) {
		int sys_btn_num;

		if (obj_get_int(btn_type_info, "NUM_OF_BUTTONS", &sys_btn_num) < 0)
			return -5; /* E_CONFERR */
		btn_id += sys_btn_num;
		if (arr_get_obj(btn_types, BUTTON_TYPE_USER, &btn_type_info) < 0)
			return -5; /* E_CONFERR */
	}

	ret = get_button(btn_id, &button);
	if (ret < 0)
		return ret;

	if (button->is_opened == 1)
		return 0;

	if (obj_get_arr(btn_type_info, "PATHS", &btn_paths) < 0)
		return -5; /* E_CONFERR */

	if (arr_get_str(btn_paths, index - 1, &filepath) < 0)
		return -5; /* E_CONFERR */

	button->fd = open(filepath, O_RDONLY);
	if (button->fd < 0) {
		sprintf(mx_errmsg, "open %s: %s", filepath, strerror(errno));
		return -1; /* E_SYSFUNCERR */
	}
	flock(button->fd, LOCK_EX);

	pthread_create(&button->thread, NULL, thread_start, button);
	button->is_opened = 1;
	button->is_pressed = 0;
	return btn_id;
}

int mx_pbtn_close(int btn_id)
{
	struct button_struct *button;
	int ret;

	if (!lib_initialized) {
		sprintf(mx_errmsg, "Library is not initialized");
		return -3; /* E_LIBNOTINIT */
	}

	ret = get_button(btn_id, &button);
	if (ret < 0)
		return ret;

	if (button->is_opened == 0)
		return 0;

	pthread_cancel(button->thread);
	close(button->fd);
	button->pressed_func = NULL;
	button->released_func = NULL;
	button->hold_func = NULL;
	button->is_opened = 0;
	return 0;
}

int mx_pbtn_wait(void)
{
	struct button_struct *button;
	int ret, btn_num, i = 0;

	if (!lib_initialized) {
		sprintf(mx_errmsg, "Library is not initialized");
		return -3; /* E_LIBNOTINIT */
	}

	if (obj_get_int(config, "NUM_OF_ALL_BUTTONS", &btn_num) < 0)
		return -5; /* E_CONFERR */

	while (i < btn_num) {
		ret = get_button(i, &button);
		if (ret < 0)
			return ret;

		if (button->is_opened == 1) {
			pthread_join(button->thread, NULL);
			i = 0;
		} else {
			i++;
		}
	}
	return 0;
}

int mx_pbtn_is_pressed(int btn_id)
{
	struct button_struct *button;
	int ret;

	if (!lib_initialized) {
		sprintf(mx_errmsg, "Library is not initialized");
		return -3; /* E_LIBNOTINIT */
	}

	ret = get_button(btn_id, &button);
	if (ret < 0)
		return ret;

	if (button->is_opened != 1) {
		sprintf(mx_errmsg, "Button is not opened");
		return -70; /* E_PBTN_NOTOPEN */
	}

	return button->is_pressed;
}

int mx_pbtn_pressed_event(int btn_id, void (*func)(int))
{
	struct button_struct *button;
	int ret;

	if (!lib_initialized) {
		sprintf(mx_errmsg, "Library is not initialized");
		return -3; /* E_LIBNOTINIT */
	}

	ret = get_button(btn_id, &button);
	if (ret < 0)
		return ret;

	button->pressed_func = func;
	return 0;
}

int mx_pbtn_released_event(int btn_id, void (*func)(int))
{
	struct button_struct *button;
	int ret;

	if (!lib_initialized) {
		sprintf(mx_errmsg, "Library is not initialized");
		return -3; /* E_LIBNOTINIT */
	}

	ret = get_button(btn_id, &button);
	if (ret < 0)
		return ret;

	button->released_func = func;
	return 0;
}

int mx_pbtn_hold_event(int btn_id, void (*func)(int), unsigned long duration)
{
	struct button_struct *button;
	int ret;

	if (!lib_initialized) {
		sprintf(mx_errmsg, "Library is not initialized");
		return -3; /* E_LIBNOTINIT */
	}

	ret = get_button(btn_id, &button);
	if (ret < 0)
		return ret;

	if (duration > 3600) {
		sprintf(mx_errmsg, "Duration out of range: %ld", duration);
		return -2; /* E_INVAL */
	}

	button->hold_func = func;
	button->hold_duration = duration;
	return 0;
}
