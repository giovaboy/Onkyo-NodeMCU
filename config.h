#ifndef _CONFIG_H
#define _CONFIG_H

#define DEBUG_ON

#define ONKYO_A9010

#ifdef ONKYO_A9010
static std::map<String, byte>
  cmds = {
    { "on", 0x02F },
    { "off", 0x0DA },
    { "mute", 0x005 },
    { "volup", 0x002 },
    { "voldown", 0x003 },
    { "sourcenext", 0x0D5 },
    { "sourceprev", 0x0D6 }
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