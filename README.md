# ESP NFC tag reader library

This library uses [MFRC22](https://github.com/pablo-sampaio/easy_mfrc522) library under the hood but adds more high-level API for reading
and writing from/to tags. Developed and tested with the
standard [MFRC22](https://lastminuteengineers.com/how-rfid-works-rc522-arduino-tutorial/) module and ESP32.

Usage:

The class needs a logger to be provided and two pins: SS and RST. The logger is used only for errors reporting, nothing is logged out by
default (unless you use the `MFRC_DEBUG` build flag).

Don't forget to call `checkTagPresented()` in some loop - unless you do that, tags are not detected...

_Tip: The reader uses SPI bus, and you should always synchronize work on it. Unless you do so, weird things may happen..._ 

```c++
#include <Arduino.h>
#include <MFRCTagReader.h>

MFRCTagReader reader(&Serial, 25, 32);

setup() {
    Serial.begin(115200);
}

loop() {
    reader.checkTagPresented();
}

```

## Build flags

`MFRC_SIGNAL_PIN` - pin with LED/buzzer to signal connected tag (HIGH=connected, LOW=disconnected); default UNDEFINED (== don't show)  
`MFRC_DEBUG` - whether debug logging should be shown; default UNDEFINED (== don't log)  

`MFRC_UID_LENGTH` - length of tag'sUID; default `16`  
`MFRC_RESET_TIMEOUT` - timeout for read/write reset; default `500`  

