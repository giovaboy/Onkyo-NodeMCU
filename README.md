# Onkyo NodeMCU

Use any NodeMCU ESP82666 board to drive you Onkyo Amplifier via REST commands.

## Description
This project will help you to create an usb powered little box that will expose some REST API GET commands in you local network to drive your offline amplifier.
 
## Tools needed
What I'm using:
#### HW
* Mini D1 NodeMCU
* cable with 3.5mm mono audio jack
* Onkyo amplifier supporting RI (Onkyo A-9010)
#### SW
* Arduino IDE
* [OnkyoRI library](https://github.com/docbender/Onkyo-RI)

### Usage
During the first boot, your nodeMCU board will boot in AP mode (SSID: `Onkyo NodeMCU`, no password), connect to it and point your browser at `192.168.4.1`, 
Here you will be able to insert your local wifi SSID and password.

The board will store SSID and password data, then it'll restart and try to connect. If everything is ok, you'll find a new device in your home network. Test it with your browser reaching the IP address it got from your DHCP (ex.: `http://192.168.1.13`)

If you've got a different amplifer model, please update those RI codes. There are some on the OnkyoRI library page.
You can try to find out yours by using the `custom` command

### REST GET commands implemented
 * on (ex.: `http://192.168.1.13/on`)
 * off
 * volup
 * voldown
 * mute
 * source
 * custom (custom command via `cmd` parameter) (ex.: `http://192.168.1.13/custom?cmd=0x02F`)
 * initialize (reset wireless settings)

 ## Onkyo A-9010 RI Codes
| Action | Command | Notes |
| ---- | ---- | ---- |
| Turn On | 0x02F | |
| Turn Off | 0x0DA | |
| On/Off toggle | 0x004 | |
| Volume up | 0x002 | |
| Volume down | 0x003 | |
| Mute toggle | 0x005 | |
| Next source | 0x0d5 | |
| Previous source | 0x0d6 | |
| CD source | 0x020 | |
| Line-in | 0x0e3 | |
| Change source to Dock | 0x170 | |
