#ifndef TAGREADER_H
#define TAGREADER_H

#include "Arduino.h"
#include <mutex>
#include <vector>
#include "EasyMFRC522.h"

#ifndef MFRC_UID_LENGTH
#define MFRC_UID_LENGTH 16
#endif

#ifndef MFRC_RESET_TIMEOUT
#define MFRC_RESET_TIMEOUT 500
#endif

class MFRCTagReader {
public:
    MFRCTagReader(Stream *logger, uint8_t pinSS, uint8_t pinRST) : logger(logger) {
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
    uint64_t resettingTagWriteSince = 0;
    uint64_t resettingTagReadSince = 0;
    byte lastUID[MFRC_UID_LENGTH]{};
    std::vector<std::function<void(byte *)>> onDetectCallbacks;
    std::vector<std::function<void()>> onDisconnectCallbacks;
};


#endif //MFRCTagReader_H
