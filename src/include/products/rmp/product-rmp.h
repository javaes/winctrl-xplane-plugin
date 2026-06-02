#ifndef PRODUCT_RMP_H
#define PRODUCT_RMP_H

#include "rmp-aircraft-profile.h"
#include "usbdevice.h"

#include <set>

enum class RMPDeviceVariant : unsigned char {
    VARIANT_CAPTAIN = 0,
    VARIANT_STBY = 1,
    VARIANT_FIRSTOFFICER = 2
};

enum class RMPLed : int {
    BACKLIGHT = 0,
    LCD_BRIGHTNESS = 1,
    OVERALL_LEDS_BRIGHTNESS = 2,

    _START = 3,
    VHF1 = 3,
    VHF2 = 4,
    VHF3 = 5,
    LOAD = 6,
    HF1 = 7,
    SEL = 8,
    HF2 = 9,
    AM = 10,
    NAV = 11,
    VOR = 12,
    ILS = 13,
    GLS = 14,
    MLS = 15,
    ADF = 16,
    _END = 16,
};

class ProductRMP : public USBDevice {
    private:
        RMPAircraftProfile *profile;
        int menuItemId;
        int displayUpdateFrameCounter = 0;
        uint64_t lastButtonStateLo;
        uint32_t lastButtonStateHi;
        std::set<int> pressedButtonIndices;
        uint8_t packetNumber = 1;

        void setProfileForCurrentAircraft();
        void parseSegment(const std::string &text, int expectedLength, std::string &outDigits, uint16_t &colonMask, int digitOffset);

    public:
        ProductRMP(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName, RMPDeviceVariant variant);
        ~ProductRMP();

        static constexpr unsigned char IdentifierByte = 0x80;

        const RMPDeviceVariant deviceVariant;

        const char *classIdentifier() override;
        const char *activeProfileName() const override;
        bool connect() override;
        void update() override;
        void blackout() override;
        void didReceiveData(int reportId, uint8_t *report, int reportLength) override;
        void didReceiveButton(uint16_t hardwareButtonIndex, bool pressed, uint8_t count = 1) override;

        void setAllLedsEnabled(bool enabled);
        void setLedBrightness(RMPLed led, uint8_t brightness);
        void setDisplayText(const std::string &active, const std::string &stby);
};

#endif
