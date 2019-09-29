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
#include <OSCServer.h>

extern "C" {
#include "I2C.h"
#include "SSD1306_OLED.h"
#include "example_app.h"
};

OSCServer oscServer;

#define sampleFreq	50.0f		// sample frequency in Hz

AuxiliaryTask osc2oledTask;		// Auxiliary task to read I2C


/* Oh Compiler-Please leave me as is */
volatile unsigned char flag = 0;

void readOSC(void*);

int localPort = 7562; //port for incoming OSC messages

// Required to init auxiliary tasks
extern int gXenomaiInited;

// Handle Ctrl-C by requesting that the audio rendering stop
void interrupt_handler(int var)
{
    gShouldStop = true;
}

int parseMessage(oscpkt::Message msg){
    
    //rt_printf("received message to: %s\n", msg.addressPattern().c_str());
    clearDisplay();
    //int intArg;
    float floatArg;    

    //if (msg.match("/osc-test").popInt32(intArg).popFloat(floatArg).isOkNoMoreArgs())
    if (msg.match("/osc-test").popFloat(floatArg).isOkNoMoreArgs())
    {
        setTextSize(1);
        setTextColor(WHITE);
        setCursor(10,0);
        print_str("OSC TEST!");
        printNumber(floatArg, DEC);

        Display();

        rt_printf("received float %f\n", floatArg);
    }
  
    std::string trString;
    //const char *ctrStr; = trString.c_str();

    if (msg.match("/tr").popFloat(floatArg).popStr(trString).isOkNoMoreArgs())
    {
        const char *ctrStr = trString.c_str();

        setTextSize(2);
        setTextColor(WHITE);
        setCursor(10,0);
        print_str(ctrStr);

        Display();

        rt_printf("received string %s\n", ctrStr);
        
    }

     if (msg.match("/tr").popStr(trString).isOkNoMoreArgs())
    {
        const char *ctrStr = trString.c_str();

        setTextSize(1);
        setTextColor(WHITE);
        setCursor(10,20);
        print_str(ctrStr);

        Display();

        rt_printf("received string %s\n", ctrStr);
        
    } 	

    if (msg.match("/param1").popFloat(floatArg).isOkNoMoreArgs())
    {
        
        fillRect(10,10, (100*floatArg), 10, WHITE);

        Display();

        //rt_printf("received float %f\n", floatArg);
        
    }   

    return 0;

}

int main(int main_argc, char *main_argv[])
{
	// Setup threading 
	int argc = 0;
	char *const *argv;
    xenomai_init(&argc, &argv);
    gXenomaiInited = 1;
    
    // Set up interrupt handler to catch Control-C and SIGTERM
    signal(SIGINT, interrupt_handler);
    signal(SIGTERM, interrupt_handler);

	// Function to be run, priority, name
	osc2oledTask = Bela_createAuxiliaryTask(readOSC, 95, "osc2oled");
	
    /* Initialize I2C bus and connect to the I2C Device */
    if(init_i2c_dev1(I2C_DEV1_PATH, SSD1306_OLED_ADDR) == 0)
    {
        printf("(Main)i2c-1: Bus Connected to SSD1306\r\n");
    }
    else
    {
        printf("(Main)i2c-1: OOPS! Something Went Wrong\r\n");
        exit(1);
    }

    // Run SDD1306 Initialization Sequence
    display_Init_seq();

    // Clear display 
    clearDisplay();

    Display();

	// OSC 
    //oscClient.setup(remotePort, remoteIp);
    oscServer.setup(localPort);

	int sleepTime = 1000000/sampleFreq;
	while(!gShouldStop)
	{
		Bela_scheduleAuxiliaryTask(osc2oledTask);
  		usleep(sleepTime);
	}
	return false;
	
    Bela_deleteAllAuxiliaryTasks();
}


void cleanup(BelaContext *context, void *userData)
{ }


// Auxiliary task to read the I2C board
void readOSC(void*)
{
	// receive OSC messages, parse them, and send back an acknowledgment
    while (oscServer.messageWaiting()){
        parseMessage(oscServer.popMessage());
        //oscClient.queueMessage(oscClient.newMessage.to("/osc-acknowledge").add(count).add(4.2f).add(std::string("OSC message received")).end());
    }
	
}

