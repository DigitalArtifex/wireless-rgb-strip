wireless-rgb-strip
========================================================

This is an Arduino project that incorporates WiFly module to control light settings wirelessly. The current version of this project is 0.0.2 ALPHA. The software works, but please expect bugs and/or instability issues. In the current version of the code, I recommend using only 150 LED's max on Arduino Uno based products.

Usage Example:

	james@James-Linux:~$ telnet 192.168.1.132 2000
	Trying 192.168.1.132...
	Connected to 192.168.1.132.
	Escape character is '^]'.
	?set:current:red=127&green=127&blue=127
	0
	?show:current:
	0
	^]
	telnet> close
	Connection closed.

Currently, you are able to use the following commands:
	Syntax: 
		command:parameter:value\r\n

	set
		current
			Sets the current RGB value. (0-127)
			Example: ?set:current:red=127&green=127&blue=127
		target
			Sets the target RGB value. (0-127)
			Example: ?set:target:red=127&green=127&blue=127
		animation
			Sets the animation type and length in ms
			Example: ?set:animation:id=1&length=180000

			Note: You must set current and target RGB values before setting an
			animation value.

	show 
		current
			Shows the current RGB value. (0-127)
			Example: ?show:current:
		target
			Shows the target RGB value. (0-127)
			Example: ?show:target:

Currently, the following response statuses are returned upon receiving a new line:
* AOK = 0
* ERR_CMD = 1
* ERR_PARAM = 2
* ERR_TARGET = 4
* ERR_LINE = 8

Note: It is technically possible for status to contain each error. This will
   be utilized in a future version, so make sure to check for each error.

Note: It can take up to 4 seconds for the device to act upon a show command on Arduino Uno Based Products

Currently, these are the following implemented animations:
* Fade In (SUNSRISE) = 1
* Fade Out (SUNSET) = 2
* Show Current Color = 3
* Show Target Color = 4

========================================================
Software Dependencies:

This project has several dependancies, that are included under the 'dep' folder of this directory. These libraries should be installed to your Arduino directory located in your home folder.

========================================================
Change Log

July 3rd, 2014:
	* Fixed set animation command
	* Implemented unique ID creation
	* Implemented configuration struct
	* Moved color settings to configuration struct
	* Now storing configuration in EEPROM
	* Moved show current and target actions to the animation state machine
	* Reduced flash consumption
	* Cleaned up the sketch a bit

July 2nd, 2014:
	* Increased speed and stability
	* Removed memory tracking files used in the WiFiSerial examples
	* Further reduced memory usage
	* Switched ByteArray implementation to pure C
	* Added SUNSET (Fade Out) animation to the animation state machine
	* Device now responds with a single byte status code
