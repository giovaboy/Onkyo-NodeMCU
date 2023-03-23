# Onkyo NodeMCU

Use any NodeMCU ESP8266 board to drive your Onkyo Amplifier via REST commands.

## Description
This project will help you to create an usb powered little box that will expose some REST API GET commands to you local network to drive your offline amplifier.
 
## Tools needed
What I'm using:
#### HW
* Mini D1 NodeMCU
* two wires cable with 3.5mm mono audio jack
* Onkyo amplifier supporting RI (Onkyo A-9010)
#### SW
* Arduino IDE
* [OnkyoRI library](https://github.com/docbender/Onkyo-RI)

## Usage
During the first boot, your nodeMCU board will boot in AP mode (SSID: `Onkyo NodeMCU`, no password), connect to it and navigate with your browser to `http://192.168.4.1`. 
Here you will be able to select your local wifi SSID and insert your password.

The board will store SSID and password data, then it'll restart and try to connect. If everything is ok, you'll find a new device in your home network. Test it with your browser reaching the IP address it got from your DHCP (ex.: `http://192.168.1.13`)

## Configuration
Please, take a look at the config.h file, there you have to select your amplifier model, the GPIO pin used or enable/disable debug.

## REST GET commands
Those are commands available indipendently by the amplifier model.
 * custom (custom command via `cmd` parameter) (ex.: `http://192.168.1.13/custom?cmd=0x02F`)
 * initialize (reset wireless settings)

Here below there are specific amplifiers commands already stored in the config.h file, you need to define your model before compiling and flashing your NodeMCU board.

 ### Onkyo A-9010 RI Codes
| Action | Command | REST GET |
| ---- | ---- | ---- |
| Turn On | 0x02F | on |
| Turn Off | 0x0DA | off |
| On/Off toggle | 0x004 | toggleonoff |
| Volume up | 0x002 | volup |
| Volume down | 0x003 | voldown |
| Mute toggle | 0x005 | mute |
| Next source | 0x0d5 | sourcenext |
| Previous source | 0x0d6 | sourceprev |
| CD source | 0x020 | cd |
| Line-in | 0x0e3 | linein |
| Change source to Dock | 0x170 | dock |

### Onkyo ONKYO_TX_SR304 RI Codes
| Action | Command | REST GET |
| ---- | ---- | ---- |
| Turn On | 0x1AF | on |
| Turn Off | 0x1AE | off |
| Mute | 0x1A4 | mute |
| Volume up | 0x1A2 | volup |
| Volume down | 0x1A3 | voldown |
| CD source | 0x20 | cd |
| Tape source | 0x70 | tape |
| DVD source | 0x120 | dvd |
| HDD source | 0x170 | hdd |
| Video2 source | 0x1A0 | video2 |
