#include "esphome.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>

const uint16_t IR_PIN = 12;

// For IR
IRsend irsend(IR_PIN);
// End for IR

// convert single hex char to dec
uint8_t hex2dec(const char hex) {
    if (hex >= 48 && hex <= 57) { // 0 ~ 9
        return hex - 48;
    } else if (hex >= 65 && hex <= 70) { // A ~ F
        return hex - 55;
    } else if (hex >= 97 && hex <= 102) { // a ~ f
        return hex - 87;
    } else {
        return 0;
    }
}

// convert multi hex chars to dec
uint16_t hex2dec(const char* payload, const uint8_t len) {
    uint16_t res = 0;
    for (uint8_t i = 0; i < len; i++) {
        res = res * 16 + hex2dec(payload[i]);
    }
    return res;
}

void send_ir(const char* payload, const uint16_t length) {
    /* 
        payload format:
        ----------------------------------
        | field       | length  | offset |
        ----------------------------------
        ----------------------------------
        | repeat      | 1 byte  | 0      |
        ----------------------------------
        | header mark | 3 bytes | 1      |
        | header gap  | 3 bytes | 4      |
        ----------------------------------
        | one mark    | 3 bytes | 7      |
        | one space   | 3 bytes | 10     |
        ----------------------------------
        | zero mark   | 3 bytes | 13     |
        | zero space  | 3 bytes | 16     |
        ----------------------------------
        | footer mark | 3 bytes | 19     |
        | gap?        | 4 bytes | 22     |
        ----------------------------------
        | data        |         | 22 / 26|
        ----------------------------------
    */

    const uint8_t repeat = hex2dec(payload + 0, 1);
    const uint16_t headermark = hex2dec(payload + 1, 3);
    const uint16_t headerspace = hex2dec(payload + 4, 3);
    const uint16_t onemark = hex2dec(payload + 7, 3);
    const uint16_t onespace = hex2dec(payload + 10, 3);
    const uint16_t zeromark = hex2dec(payload + 13, 3);
    const uint16_t zerospace = hex2dec(payload + 16, 3);
    const uint16_t footermark = hex2dec(payload + 19, 3);
    uint16_t gap = 0;
    uint8_t dataoffset = 22;
    if (repeat > 0) {
        gap = hex2dec(payload + 22, 4);
        dataoffset = 26;
    }

    const uint8_t datalength = (length - dataoffset) / 2;
    uint8_t data[datalength];
    for (uint8_t i = 0; i < datalength; i ++) {
        data[i] = hex2dec(payload + dataoffset + i * 2, 2);
    }

    // send ir signal ...
    irsend.sendGeneric(headermark, headerspace, onemark, onespace,
                       zeromark, zerospace, footermark, gap,
                       data, datalength, 38, false, repeat, 50);
}

class IRSender : public Component {
    public:
        void setup() override {

            // start ir send module
            irsend.begin();
        }
};
