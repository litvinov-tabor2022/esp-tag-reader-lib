# esp-tag-reader-lib

Usage:
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

## Envs

`MFRC_SIGNAL_PIN` - pin with LED/buzzer to signal connected tag (HIGH=connected, LOW=disconnected); default UNDEFINED (== don't show)
`MFRC_DEBUG` - whether debug logging should be shown; default UNDEFINED (== don't log)

`MFRC_UID_LENGTH` - length of tag'sUID; default `16`
`MFRC_RESET_TIMEOUT` - timeout for read/write reset; default `500`

