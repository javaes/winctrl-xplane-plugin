#include "toliss-nws-profile.h"

#include "dataref.h"
#include "product-nws.h"

TolissNWSProfile::TolissNWSProfile(ProductNWS *product) : NWSAircraftProfile(product) {
    Dataref::getInstance()->monitorExistingDataref<float>("AirbusFBW/PanelBrightnessLevel", [product](float brightness) {
        bool hasPower = Dataref::getInstance()->getCached<bool>("sim/cockpit/electrical/avionics_on");
        uint8_t backlightBrightness = hasPower ? brightness * 255 : 0;

        product->setLedBrightness(NWSLed::BACKLIGHT, backlightBrightness);
    },
        this);

    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/avionics_on", [](bool poweredOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("AirbusFBW/PanelBrightnessLevel");
    },
        this);
}

bool TolissNWSProfile::IsEligible() {
    return Dataref::getInstance()->exists("AirbusFBW/PanelBrightnessLevel");
}
