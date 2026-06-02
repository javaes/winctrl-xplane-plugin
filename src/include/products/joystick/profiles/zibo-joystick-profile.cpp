#include "zibo-joystick-profile.h"

#include "appstate.h"
#include "dataref.h"
#include "product-joystick.h"

#include <algorithm>
#include <cmath>

ZiboJoystickProfile::ZiboJoystickProfile(USBDevice *product) : JoystickAircraftProfile(product) {
    auto joystick = static_cast<ProductJoystick *>(product);
    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("laminar/B738/electric/panel_brightness", [joystick](const std::vector<float> &panelBrightness) {
        if (panelBrightness.size() < 4) {
            return;
        }

        bool hasPower = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/avionics_on");
        uint8_t target = hasPower ? panelBrightness[3] * 255 : 0;
        joystick->setLedBrightness(target);

        if (!hasPower) {
            joystick->setVibration(0);
        }
    });

    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("laminar/B738/electric/panel_brightness");
    });
}

ZiboJoystickProfile::~ZiboJoystickProfile() {
    Dataref::getInstance()->unbind("sim/cockpit/electrical/avionics_on");
    Dataref::getInstance()->unbind("laminar/B738/electric/panel_brightness");
    Dataref::getInstance()->unbind("sim/flightmodel/failures/onground_any");
}

bool ZiboJoystickProfile::IsEligible() {
    return Dataref::getInstance()->exists("zibomod/Aircraft_Path");
}
