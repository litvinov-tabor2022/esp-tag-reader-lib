#ifndef TAGREADER_H
#define TAGREADER_H

#include "Arduino.h"
#include <mutex>
#include "EasyMFRC522.h"

#ifndef TAGREADER_DEBUG
#define TAGREADER_DEBUG false
#endif

#ifndef MFRC_UID_LENGTH
#define MFRC_UID_LENGTH 16
#endif

#ifdef MFRC_SIGNAL
#ifdef MFRC_SIGNAL_PIN
#define MFRC_SIGNAL_PIN 2
#endif
#endif

#ifndef MFRC_RESET_TIMEOUT
#define MFRC_RESET_TIMEOUT 500
#endif

typedef unsigned char u8;
typedef int8_t i8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef unsigned long long u64;

class MFRCTagReader {
public:
    MFRCTagReader(Stream *logger, u8 pinSS, u8 pinRST) : logger(logger) {
        // I would put this to #begin. But every time I try to do that, it stops working and... fuck it, I can live with this.
        rfid = new EasyMFRC522(pinSS, pinRST);
        rfid->init();
        device = rfid->getMFRC522();
    }

    bool begin();

    void addOnConnectCallback(const std::function<void(byte *)> &callback);

    void addOnDisconnectCallback(const std::function<void(void)> &callback);

    bool isTagConnected() const { return tagConnectedPublic; }

    bool write(byte *data, int size);

    bool read(byte *byte, int size);

    void checkTagPresented();

private:
    void onTagConnected(byte *uuid);

    void onTagDisconnected();


    Stream *logger;

    EasyMFRC522 *rfid;
    MFRC522 *device;
    bool tagConnected = false;
    bool tagConnectedPublic = false;
    u64 resettingTagWriteSince = 0;
    u64 resettingTagReadSince = 0;
    byte lastUID[MFRC_UID_LENGTH]{};
    std::vector<std::function<void(byte *)>> onDetectCallbacks;
    std::vector<std::function<void()>> onDisconnectCallbacks;
};


#endif //TAGREADER_H
