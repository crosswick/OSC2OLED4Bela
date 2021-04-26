/*
 ____  _____ _        _
| __ )| ____| |      / \
|  _ \|  _| | |     / _ \
| |_) | |___| |___ / ___ \
|____/|_____|_____/_/   \_\

The platform for ultra-low latency audio and sensor processing

http://bela.io

A project of the Augmented Instruments Laboratory within the
Centre for Digital Music at Queen Mary University of London.
http://www.eecs.qmul.ac.uk/~andrewm

(c) 2016 Augmented Instruments Laboratory: Andrew McPherson,
  Astrid Bin, Liam Donovan, Christian Heinrichs, Robert Jack,
  Giulio Moro, Laurel Pardue, Victor Zappi. All rights reserved.

  ola mundo

The Bela software is distributed under the GNU Lesser General Public License
(LGPL 3.0), available here: https://www.gnu.org/licenses/lgpl-3.0.txt
*/

#include <Bela.h>
#include <xenomai/init.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <cmath>
#include <math.h>
#include <libraries/OscReceiver/OscReceiver.h>

extern "C" {
#include "I2C.h"
#include "SSD1306_OLED.h"
#include "example_app.h"
};

OscReceiver oscReceiver;
int localPort = 7562; //port for incoming OSC messages
volatile unsigned char flag = 0; // needed by the silly example_app

// Handle Ctrl-C by requesting that the audio rendering stop
void interrupt_handler(int var)
{
	gShouldStop = true;
}

int parseMessage(oscpkt::Message msg, void* arg)
{

	//printf("received message to: %s\n", msg.addressPattern().c_str());
	clearDisplay();
	float floatArg;

	//if (msg.match("/osc-test").popInt32(intArg).popFloat(floatArg).isOkNoMoreArgs())
	if (msg.match("/osc-test").popFloat(floatArg).isOkNoMoreArgs())
	{
		printf("received /osc-test float %f\n", floatArg);
		setTextSize(1);
		setTextColor(WHITE);
		setCursor(10,0);
		print_str("OSC TEST!");
		printNumber(floatArg, DEC);
		Display();
	}

	std::string trString;
	if (msg.match("/tr").popFloat(floatArg).popStr(trString).isOkNoMoreArgs())
	{
		const char *ctrStr = trString.c_str();
		printf("received /tr float %f string %s\n", floatArg, ctrStr);
		setTextSize(2);
		setTextColor(WHITE);
		setCursor(10,0);
		print_str(ctrStr);
		Display();
	}

	if (msg.match("/tr").popStr(trString).isOkNoMoreArgs())
	{
		const char *ctrStr = trString.c_str();
		printf("received string %s\n", ctrStr);
		setTextSize(1);
		setTextColor(WHITE);
		setCursor(10,20);
		print_str(ctrStr);
		Display();
	}

	if (msg.match("/param1").popFloat(floatArg).isOkNoMoreArgs())
	{
		printf("received /param1 float %f\n", floatArg);
		fillRect(10,10, (100*floatArg), 10, WHITE);
		Display();
	}
	return 0;
}

int main(int main_argc, char *main_argv[])
{
	// Currently required by OscReceiver
	int argc = 0;
	char *const *argv;
	xenomai_init(&argc, &argv);

	// Set up interrupt handler to catch Control-C and SIGTERM
	signal(SIGINT, interrupt_handler);
	signal(SIGTERM, interrupt_handler);

	/* Initialize I2C bus and connect to the I2C Device */
	if(init_i2c_dev1(I2C_DEV1_PATH, SSD1306_OLED_ADDR) == 0)
	{
		printf("(Main)i2c-1: Bus Connected to SSD1306\n");
	}
	else
	{
		fprintf(stderr, "(Main)i2c-1: OOPS! Something Went Wrong\n");
		return 1;
	}

	// Run SDD1306 Initialization Sequence
	display_Init_seq();

	// Clear display
	clearDisplay();

	setTextSize(1);
	setTextColor(WHITE);
	setCursor(10,0);
	print_str("OSC TEST!");

	Display();

	// OSC
	oscReceiver.setup(localPort, parseMessage);
	while(!gShouldStop)
	{
		usleep(100000);
	}
	return false;
}
