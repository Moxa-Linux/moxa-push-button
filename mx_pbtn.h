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

#ifndef _MOXA_PBTN_H
#define _MOXA_PBTN_H

enum button_type {
	BUTTON_TYPE_SYSTEM = 0,
	BUTTON_TYPE_USER = 1
};

#define DURATION_EVERY 0

#ifdef __cplusplus
extern "C" {
#endif

extern int mx_pbtn_init(void);
extern int mx_pbtn_open(int type, int index);
extern int mx_pbtn_close(int btn_id);
extern int mx_pbtn_wait(void);
extern int mx_pbtn_is_pressed(int btn_id);
extern int mx_pbtn_pressed_event(int btn_id, void (*func)(int));
extern int mx_pbtn_released_event(int btn_id, void (*func)(int));
extern int mx_pbtn_hold_event(int btn_id, void (*func)(int), unsigned long duration);

#ifdef __cplusplus
}
#endif

#endif /* _MOXA_PBTN_H */

