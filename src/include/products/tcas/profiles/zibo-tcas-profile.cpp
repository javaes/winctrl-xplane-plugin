#include "zibo-tcas-profile.h"

#include "dataref.h"
#include "product-tcas.h"

#include <cstdio>

ZiboTCASProfile::ZiboTCASProfile(ProductTCAS *product) : TCASAircraftProfile(product) {
    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("sim/cockpit2/electrical/panel_brightness_ratio", [product](const std::vector<float> &brightness) {
        bool hasPower = Dataref::getInstance()->getCached<bool>("sim/cockpit/electrical/battery_on");
        uint8_t backlight = (hasPower && !brightness.empty()) ? static_cast<uint8_t>(brightness[0] * 255) : 0;
        product->setLedBrightness(TCASLed::BACKLIGHT, backlight);
        product->setLedBrightness(TCASLed::LCD_BRIGHTNESS, hasPower ? 255 : 0);
        product->setLedBrightness(TCASLed::OVERALL_LEDS_BRIGHTNESS, hasPower ? 255 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/battery_on", [](bool) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("sim/cockpit2/electrical/panel_brightness_ratio");
    });

    Dataref::getInstance()->executeChangedCallbacksForDataref("sim/cockpit/electrical/battery_on");
}

ZiboTCASProfile::~ZiboTCASProfile() {
    Dataref::getInstance()->unbind("sim/cockpit2/electrical/panel_brightness_ratio");
    Dataref::getInstance()->unbind("sim/cockpit/electrical/battery_on");
}

bool ZiboTCASProfile::IsEligible() {
    return Dataref::getInstance()->exists("laminar/B738/knob/transponder_stby");
}

const std::unordered_map<uint16_t, TCASButtonDef> &ZiboTCASProfile::buttonDefs() const {
    static const std::unordered_map<uint16_t, TCASButtonDef> buttons = {
        // Keypad digits — shift-in to sim/cockpit2/radios/actuators/transponder_code
        {0, {"Keypad 1", "sim/cockpit2/radios/actuators/transponder_code", TCASDatarefType::SET_VALUE_PHASED, 1}},
        {1, {"Keypad 2", "sim/cockpit2/radios/actuators/transponder_code", TCASDatarefType::SET_VALUE_PHASED, 2}},
        {2, {"Keypad 3", "sim/cockpit2/radios/actuators/transponder_code", TCASDatarefType::SET_VALUE_PHASED, 3}},
        {3, {"Keypad 4", "sim/cockpit2/radios/actuators/transponder_code", TCASDatarefType::SET_VALUE_PHASED, 4}},
        {4, {"Keypad 5", "sim/cockpit2/radios/actuators/transponder_code", TCASDatarefType::SET_VALUE_PHASED, 5}},
        {5, {"Keypad 6", "sim/cockpit2/radios/actuators/transponder_code", TCASDatarefType::SET_VALUE_PHASED, 6}},
        {6, {"Keypad 7", "sim/cockpit2/radios/actuators/transponder_code", TCASDatarefType::SET_VALUE_PHASED, 7}},
        {7, {"Keypad 0", "sim/cockpit2/radios/actuators/transponder_code", TCASDatarefType::SET_VALUE_PHASED, 0}},
        {8, {"Keypad CLR", "sim/cockpit2/radios/actuators/transponder_code", TCASDatarefType::SET_VALUE, 0}},
        {9, {"Ident", "laminar/B738/push_button/transponder_ident_dn", TCASDatarefType::EXECUTE_CMD_PHASED}},

        // Transponder mode selector
        {10, {"XPDR STBY", "laminar/B738/knob/transponder_stby", TCASDatarefType::EXECUTE_CMD_PHASED}},
        {12, {"XPDR ON", "laminar/B738/knob/transponder_alton", TCASDatarefType::EXECUTE_CMD_PHASED}},
        {15, {"ALT RPTG OFF", "laminar/B738/knob/transponder_altoff", TCASDatarefType::EXECUTE_CMD_PHASED}},
        {22, {"TCAS TA", "laminar/B738/knob/transponder_ta", TCASDatarefType::EXECUTE_CMD_PHASED}},
        {23, {"TCAS TA/RA", "laminar/B738/knob/transponder_tara", TCASDatarefType::EXECUTE_CMD_PHASED}},
    };
    return buttons;
}

void ZiboTCASProfile::buttonPressed(const TCASButtonDef *button, XPLMCommandPhase phase) {
    if (!button || button->dataref.empty() || phase == xplm_CommandContinue) {
        return;
    }

    auto dm = Dataref::getInstance();

    if (button->dataref == "sim/cockpit2/radios/actuators/transponder_code") {
        if (phase != xplm_CommandBegin) {
            return;
        }
        if (button->datarefType == TCASDatarefType::SET_VALUE) {
            // CLR: write 0
            dm->set<int>("sim/cockpit2/radios/actuators/transponder_code", 0);
        } else {
            // Digit: shift current code left and append new digit
            int current = dm->get<int>("sim/cockpit2/radios/actuators/transponder_code");
            int newCode = (current * 10 + static_cast<int>(button->value)) % 10000;
            dm->set<int>("sim/cockpit2/radios/actuators/transponder_code", newCode);
        }
        return;
    }

    dm->executeCommand(button->dataref.c_str(), phase);
}

void ZiboTCASProfile::updateDisplays() {
    if (!product) {
        return;
    }

    int code = Dataref::getInstance()->get<int>("sim/cockpit2/radios/actuators/transponder_code");
    char buf[5];
    snprintf(buf, sizeof(buf), "%04d", code);
    product->setLCDText(std::string(buf));
}
