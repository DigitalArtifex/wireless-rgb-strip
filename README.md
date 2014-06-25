wireless-rgb-strip
==================

This is an Arduino project that incorporates WiFly module to control light settings wirelessly. The current version of this project is 0.0.1 ALPHA. The software works, but please expect bugs and/or instability issues.

Usage Example:

	james@James-Linux:~$ telnet 192.168.1.132 2000
	Trying 192.168.1.132...
	Connected to 192.168.1.132.
	Escape character is '^]'.
	?set:current:red=127&green=127&blue=127
	?show:current:
	^]
	telnet> close
	Connection closed.


Software Dependencies:
This project has several dependancies, that are included under the 'dep' folder of this directory. These libraries should be installed to your Arduino directory located in your home folder.
