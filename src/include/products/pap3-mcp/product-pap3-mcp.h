#ifndef PRODUCT_PAP3MCP_H
#define PRODUCT_PAP3MCP_H

#include "pap3-mcp-aircraft-profile.h"
#include "usbdevice.h"

#include <array>
#include <map>
#include <set>
#include <vector>

class ProductPAP3MCP : public USBDevice {
    public:
        // A/T ARM switch hardware variant. The magnetic switch is a maintained
        // toggle held up by a solenoid; the standard switch is a momentary,
        // spring-loaded toggle with no solenoid that snaps back on its own.
        enum class ATSwitchType {
            Magnetic,
            Standard,
        };

    private:
        uint8_t packetNumber = 1;
        PAP3MCPAircraftProfile *profile;
        int menuItemId;
        PAP3MCPDisplayData displayData;
        int lastUpdateCycle;
        int displayUpdateFrameCounter = 0;
        std::set<int> pressedButtonIndices;

        uint64_t lastButtonStateLo = 0;
        uint32_t lastButtonStateHi = 0;

        ATSwitchType atSwitchType = ATSwitchType::Magnetic;
        bool standardModeATArmed = false;

        void setProfileForCurrentAircraft();
        void loadATSwitchType(const std::string &value);

    public:
        ProductPAP3MCP(HIDDeviceHandle hidDevice, uint16_t vendorId, uint16_t productId, std::string vendorName, std::string productName);
        ~ProductPAP3MCP();

        static constexpr unsigned char IdentifierByte = 0x0C;

        const char *classIdentifier() override;
        const char *activeProfileName() const override;
        bool connect() override;
        void update() override;
        void blackout() override;
        void didReceiveData(int reportId, uint8_t *report, int reportLength) override;
        void didReceiveButton(uint16_t hardwareButtonIndex, bool pressed, uint8_t count = 1) override;
        void forceStateSync() override;

        void updateDisplays(bool force = true);

        void setAllLedsEnabled(bool enable);
        void setLedBrightness(PAP3MCPLed led, uint8_t brightness);
        void setATSolenoid(bool engaged);

        void initializeDisplays();
        void clearDisplays();
        void sendLCDDisplay(const std::string &speed, int heading, int altitude, const std::string &vs, int crsCapt, int crsFo);
        void sendRawLCDPayload(const std::array<uint8_t, 32> &payload);
        void sendLCDCommit();
};

#endif
