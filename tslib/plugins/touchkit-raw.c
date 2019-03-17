
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>

#include "config.h"
#include "tslib-private.h"

/*
 * TouchKit RS232 driver
 * (controller type SAT4000UR)
 * 2009.07  Fred Salabartan
 *
 * Packet format for position report (5 bytes):
 * [0] 1 0 0 0 0 0 0 t = header (t=touched)
 * [1] 0 0 0 0 X X X X = X coord most significant bits
 * [2] 0 X X X X X X X = X coord less significant bits
 * [3] 0 0 0 0 Y Y Y Y = Y coord most significant bits
 * [4] 0 Y Y Y Y Y Y Y = Y coord less significant bits
 *
 * Problem: sometimes some packets overlap, so it is possible
 * to find a new packet in the middle of another packet.
 * -> check that no byte in the packet (but the first one)
 *    have its first bit set (0x80 = start)
 *
 * This file is placed under the LGPL.  Please see the file
 * COPYING for more details.
*/

enum {
	PACKET_SIZE = 8,
	BUFFER_SIZE = 100,
	PACKET_START_SIGNATURE1 = 0x50,
	PACKET_START_SIGNATURE2 = 0x52,
	PACKET_END_SIGNATURE = 0x0D
};

/* Is is a start of packet ? */
#define IsStart1(x) ((x) == PACKET_START_SIGNATURE1)
#define IsStart2(x) ((x) == PACKET_START_SIGNATURE2)
#define IsEnd(x) ((x) == PACKET_END_SIGNATURE)

static int touchkit_init(int dev)
{
	struct termios tty;

	tcgetattr(dev, &tty);
	tty.c_iflag = IGNBRK | IGNPAR;
	tty.c_oflag = 0;
	tty.c_lflag = 0;
	tty.c_line = 0;
	tty.c_cc[VTIME] = 0;
	tty.c_cc[VMIN] = 1;
	tty.c_cflag = CS8 | CREAD | CLOCAL | HUPCL;
	tty.c_cflag |= B115200;
	tcsetattr(dev, TCSAFLUSH, &tty);

	return 1;
}

static int touchkit_read(struct tslib_module_info *inf, struct ts_sample *samp,
			 int nr)
{
	static int initDone = 0;
	static unsigned char buffer[BUFFER_SIZE];	/* enough space for 2 "normal" packets */
	static int pos = 0;

	struct tsdev *ts = inf->dev;

	if (initDone == 0) {
		initDone = touchkit_init(ts->fd);
		if (initDone == -1)
			return -1;
	}
	/* read some new bytes (enough for 1 normal packet) */
	int ret = read(ts->fd, buffer + pos, PACKET_SIZE);
	if (ret <= 0)
		return -1;

	pos += ret;
	if (pos < PACKET_SIZE)
		return 0;

	/* find start */
	int p;
	int total = 0;
	for (p = 0; p < pos; ++p)
		if (IsStart1(buffer[p]) || IsStart2(buffer[p])) {
			/* we have enough data for a packet ? */
			if (p + PACKET_SIZE > pos) {
				if (p > 0) {
                                        /*
					 * we have found a start >0, it means we have garbage
					 * at beginning of buffer
					 * so let's shift data to ignore this garbage
                                         */
					memcpy(buffer, buffer + p, pos - p);
					pos -= p;
				}
				break;
			}

			unsigned char *data = buffer + p;

			/* check if all bytes are ok (no 'start' embedded) */
			if (!IsEnd(buffer[p + PACKET_SIZE - 1])) {
#ifdef DEBUG
				fprintf(stderr,
					"End can't be found within packet [%02X %02X %02X %02X %02X %02X %02X %02X] p=%d, pos=%d\n",
					data[0], data[1], data[2], data[3],
					data[4], data[5], data[6], data[7], p, pos );
#endif
				continue;
			}
			/* now let's decode it */
			samp->x		= (data[1] & 0x00FF) | (data[2] & 0x00FF) << 8;
			samp->y		= (data[3] & 0x00FF) | (data[4] & 0x00FF) << 8;
			samp->pressure	= (data[5] & 0x00FF) | (data[6] & 0x00FF) << 8;
			gettimeofday(&samp->tv, NULL);
#ifdef DEBUG
			fprintf(stderr,
				"RAW -------------------------> data=[%02X %02X %02X %02X %02X %02X %02X %02X]  x=%d y=%d pres=%d\n",
				data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
				samp->x, samp->y, samp->pressure);
#endif
			samp++;

			/* now remove it */
			memcpy(buffer, buffer + p + PACKET_SIZE,
			       pos - p - PACKET_SIZE);
			pos -= p + PACKET_SIZE;
			total = 1;
			break;
		}

	return total;
}

static const struct tslib_ops touchkit_ops = {
	.read = touchkit_read,
};

TSAPI struct tslib_module_info *touchkit_mod_init(struct tsdev *dev, const char *params)
{
	struct tslib_module_info *m;

	m = malloc(sizeof(struct tslib_module_info));
	if (m == NULL)
		return NULL;

	m->ops = &touchkit_ops;
	return m;
}

#ifndef TSLIB_STATIC_TOUCHKIT_MODULE
	TSLIB_MODULE_INIT(touchkit_mod_init);
#endif