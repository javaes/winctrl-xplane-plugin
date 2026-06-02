#include "toliss-rmp-profile.h"

#include "dataref.h"
#include "product-rmp.h"

#include <cstdio>

const char *TolissRMPProfile::comRadio() const {
    switch (product->deviceVariant) {
        case RMPDeviceVariant::VARIANT_CAPTAIN:
            return "com1";
        case RMPDeviceVariant::VARIANT_STBY:
            return "com3";
        case RMPDeviceVariant::VARIANT_FIRSTOFFICER:
            return "com2";
    }
    return "com1";
}

std::string TolissRMPProfile::formatFrequency(int hz) {
    if (hz <= 0) {
        return "      ";
    }
    int mhz = hz / 1000000;
    int khz = (hz % 1000000) / 1000;
    char buf[8];
    std::snprintf(buf, sizeof(buf), "%3d.%03d", mhz, khz);
    return std::string(buf);
}

TolissRMPProfile::TolissRMPProfile(ProductRMP *product) : RMPAircraftProfile(product) {
    Dataref::getInstance()->monitorExistingDataref<float>("AirbusFBW/PanelBrightnessLevel", [product](float brightness) {
        bool hasEssentialBusPower = Dataref::getInstance()->get<bool>("AirbusFBW/FCUAvail");
        bool hasPower = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on");
        uint8_t backlightBrightness = hasPower ? brightness * 255 : 0;

        product->setLedBrightness(RMPLed::BACKLIGHT, backlightBrightness);
        product->setLedBrightness(RMPLed::LCD_BRIGHTNESS, hasEssentialBusPower ? 255 : 0);
        product->setLedBrightness(RMPLed::OVERALL_LEDS_BRIGHTNESS, hasEssentialBusPower ? 255 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("AirbusFBW/FCUAvail", [](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/PanelBrightnessLevel");
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [this](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/PanelBrightnessLevel");
        updateDisplays();
    });

    std::string activeRef = std::string("sim/cockpit2/radios/actuators/") + comRadio() + "_frequency_hz_833";
    std::string stbyRef = std::string("sim/cockpit2/radios/actuators/") + comRadio() + "_standby_frequency_hz_833";

    Dataref::getInstance()->monitorExistingDataref<int>(activeRef.c_str(), [this](int hz) {
        updateDisplays();
    });

    Dataref::getInstance()->monitorExistingDataref<int>(stbyRef.c_str(), [this](int hz) {
        updateDisplays();
    });
}

TolissRMPProfile::~TolissRMPProfile() {
    Dataref::getInstance()->unbind("AirbusFBW/PanelBrightnessLevel");
    Dataref::getInstance()->unbind("AirbusFBW/FCUAvail");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
    std::string activeRef = std::string("sim/cockpit2/radios/actuators/") + comRadio() + "_frequency_hz_833";
    std::string stbyRef = std::string("sim/cockpit2/radios/actuators/") + comRadio() + "_standby_frequency_hz_833";
    Dataref::getInstance()->unbind(activeRef.c_str());
    Dataref::getInstance()->unbind(stbyRef.c_str());
}

bool TolissRMPProfile::IsEligible() {
    return Dataref::getInstance()->exists("AirbusFBW/PanelBrightnessLevel");
}

const std::unordered_map<uint16_t, RMPButtonDef> &TolissRMPProfile::buttonDefs() const {
    static std::unordered_map<RMPDeviceVariant, std::unordered_map<uint16_t, RMPButtonDef>> cache;

    if (cache.find(product->deviceVariant) == cache.end()) {
        cache[product->deviceVariant] = {
            {0, {"Flip frequencies", std::string("sim/radios/") + comRadio() + "_standy_flip"}},
            {1, {"VHF 1", ""}},
            {2, {"VHF 2", ""}},
            {3, {"VHF 3", ""}},
            {4, {"LOAD", ""}},
            {5, {"HF1", ""}},
            {6, {"HF2", ""}},
            {7, {"AM", ""}},
            {8, {"Knob Large L", ""}},
            {9, {"Knob Large R", ""}},
            {10, {"Knob Small L", ""}},
            {11, {"Knob Small R", ""}},
            {12, {"Knob depress", ""}},
            {13, {"NAV", ""}},
            {14, {"VOR", ""}},
            {15, {"ILS", ""}},
            {16, {"GLS", ""}},
            {17, {"MLS", ""}},
            {18, {"ADF", ""}},
            {19, {"Switch On", ""}},
            {20, {"Switch Off", ""}},
        };
    }

    return cache[product->deviceVariant];
}

void TolissRMPProfile::buttonPressed(const RMPButtonDef *button, XPLMCommandPhase phase) {
    if (!button || button->dataref.empty() || phase == xplm_CommandContinue) {
        return;
    }

    auto datarefManager = Dataref::getInstance();
    if (button->datarefType == RMPDatarefType::SET_VALUE) {
        if (phase != xplm_CommandBegin) {
            return;
        }

        datarefManager->set<int>(button->dataref.c_str(), static_cast<int>(button->value));
    } else {
        datarefManager->executeCommand(button->dataref.c_str(), phase);
    }
}

void TolissRMPProfile::updateDisplays() {
    if (!product) {
        return;
    }

    bool hasPower = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on");
    if (!hasPower) {
        product->setDisplayText("      ", "      ");
        return;
    }

    std::string activeRef = std::string("sim/cockpit2/radios/actuators/") + comRadio() + "_frequency_hz_833";
    std::string stbyRef = std::string("sim/cockpit2/radios/actuators/") + comRadio() + "_standby_frequency_hz_833";
    int activeHz = Dataref::getInstance()->get<int>(activeRef.c_str());
    int stbyHz = Dataref::getInstance()->get<int>(stbyRef.c_str());

    product->setDisplayText(formatFrequency(activeHz), formatFrequency(stbyHz));
}
