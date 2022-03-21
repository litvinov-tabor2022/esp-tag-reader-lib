#include "MFRCTagReader.h"

bool MFRCTagReader::begin() {
#ifdef MFRC_SIGNAL
    pinMode(MFRC_SIGNAL_PIN, OUTPUT);
    digitalWrite(MFRC_SIGNAL_PIN, LOW);
#endif

    device->PCD_SoftPowerDown();
    device->PCD_SoftPowerUp();
    device->PCD_Reset();
    device->PCD_Init();

    const bool selfTestOk = device->PCD_PerformSelfTest();
    Serial.printf("RFID self-test: %s\n", selfTestOk ? "ok" : "failed");
    device->PCD_Init();

    if (!selfTestOk) return false;

    return true;
}

void MFRCTagReader::addOnConnectCallback(const std::function<void(byte *)> &callback) {
    this->onDetectCallbacks.push_back(callback);
}

void MFRCTagReader::onTagConnected(byte *uuid) {
#ifdef MFRC_SIGNAL
    digitalWrite(MFRC_SIGNAL_PIN, HIGH);
#endif

    for (auto &callback: onDetectCallbacks) callback(uuid);
}

void MFRCTagReader::addOnDisconnectCallback(const std::function<void(void)> &callback) {
    this->onDisconnectCallbacks.push_back(callback);
}

void MFRCTagReader::onTagDisconnected() {
#ifdef MFRC_SIGNAL
    digitalWrite(MFRC_SIGNAL_PIN, LOW);
#endif

    for (auto &callback: onDisconnectCallbacks) callback();
}

bool MFRCTagReader::write(byte *data, int size) {
    resettingTagWriteSince = millis();

    for (int i = 0; i < 5; i++) {
        rfid->unselectMifareTag(true);
        delay(50);
        if (rfid->detectTag()) {
            if (rfid->writeRaw(1, data, size) > 0) {
                return true;
            }
        }
    }

    logger->println("Write failure!!! Cancelling reset, disconnecting");
    resettingTagWriteSince = 0;
    tagConnectedPublic = false;
    tagConnected = false;
    onTagDisconnected();

    return false;
}

bool MFRCTagReader::read(byte *byte, const int size) {
    resettingTagReadSince = millis();

    for (int i = 0; i < 5; i++) {
        rfid->unselectMifareTag(true);
        delay(50);
        if (rfid->detectTag()) {
            const int readResult = rfid->readRaw(1, byte, size);

            // all data has to be read
            if (readResult == size) {
                return true;
            }
        }
    }

    logger->println("Read failure!!! Cancelling reset, disconnecting");
    resettingTagReadSince = 0;
    tagConnectedPublic = false;
    tagConnected = false;
    onTagDisconnected();

    return false;
}

void MFRCTagReader::checkTagPresented() {
    const u64 now = millis();

    if (resettingTagReadSince > 0 && now - resettingTagReadSince >= MFRC_RESET_TIMEOUT) {
        logger->println("Read-resetting timeout, disconnecting");
        resettingTagReadSince = 0;
        resettingTagWriteSince = 0;
        tagConnectedPublic = false;
        tagConnected = false;
        onTagDisconnected();
        return;
    }

    if (resettingTagWriteSince > 0 && now - resettingTagWriteSince >= MFRC_RESET_TIMEOUT) {
        logger->println("Write-resetting timeout, disconnecting");
        resettingTagWriteSince = 0;
        resettingTagReadSince = 0;
        tagConnectedPublic = false;
        tagConnected = false;
        onTagDisconnected();
        return;
    }

    if (!tagConnected) {
        if (device->PICC_IsNewCardPresent() && device->PICC_ReadCardSerial()) {
            tagConnected = true;
            tagConnectedPublic = true;

            std::copy(device->uid.uidByte, device->uid.uidByte + MFRC_UID_LENGTH, lastUID);

            if (resettingTagWriteSince > 0) {
                if (TAGREADER_DEBUG) logger->println("Not connecting, tag being write-reset; tag reset done");
                resettingTagWriteSince = 0;
            } else {
                if (resettingTagReadSince > 0) {
                    if (TAGREADER_DEBUG) logger->println("Not connecting, tag being read-reset; tag reset done");
                    resettingTagReadSince = 0;
                } else {
                    if (TAGREADER_DEBUG) logger->println("Connecting, not being reset");
                    onTagConnected(device->uid.uidByte);
                }
            }
        }
    }

    if (tagConnected) {
        uint8_t control = 0x00;
        for (int i = 0; i < 3; i++) {
            if (!device->PICC_IsNewCardPresent()) {
                if (device->PICC_ReadCardSerial()) {
                    control |= 0x16;
                }
                if (device->PICC_ReadCardSerial()) {
                    control |= 0x16;
                }
                control += 0x1;
            }
            control += 0x4;
        }
        if (control == 13 || control == 14) {
            //card is still there
            return;
        }
    }

    if (tagConnected) {
        if (resettingTagWriteSince == 0) {
            if (resettingTagReadSince == 0) {
                tagConnectedPublic = false;
                if (TAGREADER_DEBUG) logger->println("Disconnecting, not being reset");
                onTagDisconnected();
            } else {
                if (TAGREADER_DEBUG) logger->println("Not disconnecting, tag being read-reset; tag reset done");
            }
        } else {
            tagConnectedPublic = true; // don't admit the tag is reconnecting
            if (TAGREADER_DEBUG) logger->println("Not disconnecting, tag being write-reset");
        }
    }

    tagConnected = false;
    device->PICC_HaltA();
    device->PCD_StopCrypto1();
    rfid->unselectMifareTag(true);
}
