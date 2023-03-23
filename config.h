#ifndef _CONFIG_H
#define _CONFIG_H

//#define DEBUG_ON  //uncomment to enable console debug
#define ONKYO_PIN 2  //13
#define ONKYO_A_9010 //ONKYO_TX8020

/*
* AMPS MODELS COMMAND LISTS
*/

#ifdef ONKYO_A_9010
static std::map<String, byte>
  cmds = {
    { "on", 0x02F },
    { "off", 0x0DA },
    { "toggleonoff", 0x004 },
    { "volup", 0x002 },
    { "voldown", 0x003 },
    { "mute", 0x005 },
    { "sourcenext", 0x0D5 },
    { "sourceprev", 0x0D6 },
    { "cd", 0x020 },
    { "linein", 0x0e3 },
    { "dock", 0x170 }
  };
#endif

#ifdef ONKYO_TX_SR304
static std::map<String, byte>
  cmds = {
    { "on", 0x1AF },
    { "off", 0x1AE },
    { "mute", 0x1A4 },
    { "volup", 0x1A2 },
    { "voldown", 0x1A3 },
    { "cd", 0x20 },
    { "tape", 0x70 },
    { "dvd", 0x120 },
    { "hdd", 0x170 },
    { "video2", 0x1A0 }
  };
#endif


#ifndef DEB
#ifdef DEBUG_ON
#define DEB(...) Serial.print(__VA_ARGS__)
#define DEBUG(...) Serial.println(__VA_ARGS__)
#else
#define DEB(...)
#define DEBUG(...)
#endif
#endif

#endif