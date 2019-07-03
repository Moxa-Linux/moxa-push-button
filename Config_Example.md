## Config Example

### Path 
```
/etc/moxa-configs/moxa-push-button.json
```

### Description

* `CONFIG_VERSION`: The version of config file
* `NUM_OF_ALL_BUTTONS`: The number of all buttons on this device
* `BUTTON_TYPES`: The type of button
  * `TYPE`: Type name
  * `NUM_OF_BUTTONS`: The number of buttons in this type on this device
  * `PATHS`: The filepath of buttons
* `DEFAULT_ACTIONS`: The default action for SYSTEM type button
  * `PRESS_ACTION`: The action when the button is pressed
  * `RELEASE_ACTION`: The action when the button is released
  * `HOLD_ACTION`: The action when the button is hold
    * `SEC`: The seconds that the button is released or hold for
    * `LED_GROUP`, `LED_INDEX`, `LED_STATE`: When the action is triggered, set LED group, index to state
    * `MESSAGE`: When the action is triggered, print this message
    * `EXEC_CMD`: When the action is triggered, execute this command

### Example: UC-5111-LX

```
{
	"CONFIG_VERSION":"1.1.0",

	"NUM_OF_ALL_BUTTONS": 1,

	"BUTTON_TYPES": [
		{
			"TYPE": "SYSTEM",
			"NUM_OF_BUTTONS": 1,
			"PATHS": [
				"/dev/input/event0"
			]
		},
		{
			"TYPE": "USER",
			"NUM_OF_BUTTONS": 0,
			"PATHS": []
		}
	],

	"DEFAULT_ACTIONS": [
		{
			"PRESS_ACTION": [
				{
					"LED_GROUP": 1,
					"LED_INDEX": 3,
					"LED_STATE": "on",
					"MESSAGE": "Button Pressed",
					"EXEC_CMD": ""
				}
			],
			"RELEASE_ACTION": [
				{
					"SEC": 9,
					"LED_GROUP": 1,
					"LED_INDEX": 3,
					"LED_STATE": "off",
					"MESSAGE": "Release Button over 9s - Do nothing",
					"EXEC_CMD": ""
				},
				{
					"SEC": 7,
					"LED_GROUP": 1,
					"LED_INDEX": 3,
					"LED_STATE": "off",
					"MESSAGE": "Release Button in 7~9s - ENTER reset to default",
					"EXEC_CMD": ""
				},
				{
					"SEC": 4,
					"LED_GROUP": 1,
					"LED_INDEX": 3,
					"LED_STATE": "off",
					"MESSAGE": "Release Button in 4~7s - Do nothing",
					"EXEC_CMD": ""
				},
				{
					"SEC": 2,
					"LED_GROUP": 1,
					"LED_INDEX": 3,
					"LED_STATE": "off",
					"MESSAGE": "Release Button in 2~4s - ENTER Diagnostic function",
					"EXEC_CMD": ""
				},
				{
					"SEC": 0,
					"LED_GROUP": 1,
					"LED_INDEX": 3,
					"LED_STATE": "off",
					"MESSAGE": "Release Button within 2s - Do nothing",
					"EXEC_CMD": ""
				}
			],
			"HOLD_ACTION": [
				{
					"SEC": 9,
					"LED_GROUP": 1,
					"LED_INDEX": 3,
					"LED_STATE": "on",
					"MESSAGE": "Push Button over 9s",
					"EXEC_CMD": ""
				},
				{
					"SEC": 7,
					"LED_GROUP": 1,
					"LED_INDEX": 3,
					"LED_STATE": "blink",
					"MESSAGE": "Push Button over 7s",
					"EXEC_CMD": ""
				},
				{
					"SEC": 4,
					"LED_GROUP": 1,
					"LED_INDEX": 3,
					"LED_STATE": "on",
					"MESSAGE": "Push Button over 4s",
					"EXEC_CMD": ""
				},
				{
					"SEC": 2,
					"LED_GROUP": 1,
					"LED_INDEX": 3,
					"LED_STATE": "blink",
					"MESSAGE": "Push Button over 2s",
					"EXEC_CMD": ""
				}
			]
		}
	]
}
```
