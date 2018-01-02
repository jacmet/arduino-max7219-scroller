#include <SPI.h> /* pull in SPI driver */

#if defined (__AVR__)
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#endif

#include "font.h"
#include "msgs.h"

/* gpios */
#define SPI_MOSI	12
#define SPI_CLK		11
#define SPI_CS		10

/* number of max7219 devices in chain */
#define DEVICES		5

/* scroll speed (delay time in ms) */
#define SCROLL_DELAY	70

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

static byte screen[8*DEVICES];
static char text[256];
static byte cpos = 0; /* character position of left screen border */
static byte ppos = 0; /* pixel (column) position of left screen border */

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

	return pgm_read_byte(&font_width[(byte)c]);
}

static int char_offset(char c)
{
	if (c < 32 || c > 127)
		c = ' ';

	c -= 32;

	return pgm_read_word(&font_offset[(byte)c]);
}

static byte char_column(char c, byte i)
{
	int ofs = char_offset(c);

	return pgm_read_byte(&font_data[ofs + i]);
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

static void setup_msg(void)
{
	int i;

	for (i = 0; i < DEVICES * 2; i++)
		text[i] = ' ';

	randomSeed(analogRead(0) | (analogRead(1) << 8));

	i = random(MSGS);

	strcpy_P(&text[DEVICES*2], (char*)pgm_read_word(&(msgs[i])));

	Serial.print(text); Serial.print("\r\n");
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
		send_cmd(i + 1, 0);

	send_cmd(OP_SHUTDOWN, 1);

	setup_msg();
}

static void scroll_refresh(void)
{
	byte cp, i, x, y, w;
	char c;

	/* clear screen */
	for (i = 0; i < sizeof(screen); i++)
		screen[i] = 0;

	cp = cpos;
	c = text[cp];
	i = ppos;
	w = char_width(c);
	/* populate screen */
	for (x = 0; x < (DEVICES * 8); x++) {
		byte b = char_column(c, i);
		byte mask = 1 << (x & 7);

		for (y = 0; y < 8; y++) {
			if (b & (1 << (7-y)))
				screen[y*DEVICES + (x >> 3)] |= mask;
		}

		i++;

		if (i >= w) {
			cp++;
			c = text[cp];
			i = 0;
			w = char_width(c);
		}


	}

	/* send to display */
	for (y = 0; y < 8; y++) {
		digitalWrite(SPI_CS, LOW);

		for (x = 0; x < DEVICES; x++) {
			shiftOut(SPI_MOSI, SPI_CLK, MSBFIRST, y + 1);
			shiftOut(SPI_MOSI, SPI_CLK, MSBFIRST, screen[y * DEVICES + x]);
		}
		digitalWrite(SPI_CS, HIGH);
	}
}

void loop(void)
{
	scroll_refresh();

	ppos++;
	if (ppos >= char_width(text[cpos])) {
		ppos = 0;
		cpos++;

		if (cpos >= strlen(text))
			power_down();
	}

	delay(SCROLL_DELAY);
}
