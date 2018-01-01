#include <SPI.h> /* pull in SPI driver */

#if defined (__AVR__)
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#endif

#include "font.h"

/* gpios */
#define SPI_MOSI	12
#define SPI_CLK		11
#define SPI_CS		10

/* number of max7219 devices in chain */
#define DEVICES		5

/* line buffer (op + data for each device) */
static byte line[DEVICES * 2];

#define OP_NOOP		0
#define OP_ROW0		1
#define OP_ROW1		2
#define OP_ROW2		3
#define OP_ROW3		4
#define OP_ROW4		5
#define OP_ROW5		6
#define OP_ROW6		7
#define OP_ROW7		8
#define OP_DECODEMODE	9
#define OP_INTENSITY	10
#define OP_SCANLIMIT	11
#define OP_SHUTDOWN	12
#define OP_DISPLAYTEST	15

static void send_cmd(byte cmd, byte val)
{
	int i;

	digitalWrite(SPI_CS, LOW);

	for (i = 0; i < DEVICES; i++) {
		shiftOut(SPI_MOSI, SPI_CLK, MSBFIRST, cmd);
		shiftOut(SPI_MOSI, SPI_CLK, MSBFIRST, val);
	}

	/* latch the data onto the display */
	digitalWrite(SPI_CS, HIGH);
}

static void power_down(void)
{
	send_cmd(OP_SHUTDOWN, 0);

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	cli();
	sleep_enable();
	sei();
	sleep_mode();
	sleep_cpu();
	while (1) ;
}

static int char_width(char c)
{
	if (c < 32 || c > 127)
		c = ' ';

	c -= 32;

	return pgm_read_byte(&font_width[c]);
}

static int char_offset(char c)
{
	if (c < 32 || c > 127)
		c = ' ';

	c -= 32;

	return pgm_read_word(&font_offset[c]);
}

static int str_width(const char *s)
{
	int width = 0;
	while (s && *s) {
		width += char_width(*s);
		s++;
	}

	return width;
}

void setup(void)
{
	int i;

	Serial.begin(115200);
	Serial.print("hello world\n");

	pinMode(SPI_MOSI, OUTPUT);
	pinMode(SPI_CLK, OUTPUT);
	pinMode(SPI_CS, OUTPUT);
	digitalWrite(SPI_CS, HIGH);

	/* setup max7219s */
	send_cmd(OP_DISPLAYTEST, 0);
	send_cmd(OP_SCANLIMIT, 7);
	send_cmd(OP_DECODEMODE, 0);
	send_cmd(OP_INTENSITY, 8);

	/* clear display */
	for (i = 0; i < 8; i++)
		send_cmd(i + 1, 1 << i);

	send_cmd(OP_SHUTDOWN, 1);
}

static int frame;

void loop(void)
{
	int i, x, y;

	if (frame == 0) {
		Serial.print(str_width("hello"));
		for (y = 0; y < 8;  y++) {
			for (i = 0; i < DEVICES: i += 2)
				line[i] = y+1;
		
	}
	frame++;

//	for (i = 0; i < 8; i++)
//		send_cmd(i + 1, 1 << (i+frame));

	delay(1000);
	if (frame > 3) {
		power_down();
		Serial.print("powered down\n");
	}
}
