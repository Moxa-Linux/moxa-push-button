# API References

---
### int mx_pbtn_init(void)

Initialize Moxa push button library.

#### Return value
* 0 on success.
* negative numbers on error.

---
### int mx_pbtn_open(int type, int index)

Open a push button by button type and index.

#### Parameters
* type: BUTTON_TYPE_SYSTEM or BUTTON_TYPE_USER
* index: button index

#### Return value
* 0 or positive integer: button ID for manipulate the button by other APIs
* negative numbers on error.

---
### int mx_pbtn_close(int btn_id)

Close a push button.

#### Parameters
* btn_id: button ID returned by "mx_pbtn_open"

#### Return value
* 0 on success.
* negative numbers on error.

---
### int mx_pbtn_start(int btn_id)

Start listening on a push button.

#### Parameters
* btn_id: button ID returned by "mx_pbtn_open"

#### Return value
* 0 on success.
* negative numbers on error.

---
### int mx_pbtn_stop(int btn_id)

Stop listening on a push button.

#### Parameters
* btn_id: button ID returned by "mx_pbtn_open"

#### Return value
* 0 on success.
* negative numbers on error.

---
### int mx_pbtn_wait(void)

Check if there is any button being listened on, if so, hang the process. This API can be
used for daemon.

#### Return value
* 0 on success.
* negative numbers on error.

---
### int mx_pbtn_is_pressed(int btn_id)

Get the state of a button.

#### Parameters
* btn_id: button ID returned by "mx_pbtn_open"

#### Return value
* 0 if the button is released.
* 1 if the button is pressed.
* negative numbers on error.

---
### int mx_pbtn_pressed_event(int btn_id, void (*func)(int))

Register action on button pressed.

#### Parameters
* btn_id: button ID returned by "mx_pbtn_open"
* func: a function pointer which will be invoked on button pressed.

#### Return value
* 0 on success.
* negative numbers on error.

---
### int mx_pbtn_released_event(int btn_id, void (*func)(int))

Register action on button released.

#### Parameters
* btn_id: button ID returned by "mx_pbtn_open"
* func: a function pointer which will be invoked on button released.

#### Return value
* 0 on success.
* negative numbers on error.

---
### int mx_pbtn_hold_event(int btn_id, void (*func)(int), unsigned long duration)

Register action on button hold.

#### Parameters
* btn_id: button ID returned by "mx_pbtn_open"
* func: a function pointer which will be invoked on button pressed.
* duration: the time that button being hold to trigger action (in seconds)
  * range: 1-3600
  * 0 for keep triggering every second 

#### Return value
* 0 on success.
* negative numbers on error.

---