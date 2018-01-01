# Makefile for arduino build. Uses arduino-mk
# (https://github.com/sudar/Arduino-Makefile or apt-get install arduino-mk)
# and python / python-pil for gfx conversion

# building for a Arduino nano
BOARD_TAG = nano328

MONITOR_PORT = /dev/ttyUSB0

all: font.h

font.h: font2h.py
	curl -s https://www.espruino.com/modules/Font6x8.js | ./font2h.py > $@

include /usr/share/arduino/Arduino.mk
