#include "cl650-fcu-efis-profile.h"

#include "dataref.h"
#include "product-fcu-efis.h"

#include <algorithm>
#include <XPLMUtilities.h>

CL650FCUEfisProfile::CL650FCUEfisProfile(ProductFCUEfis *product) : FCUEfisAircraftProfile(product) {
    Dataref::getInstance()->monitorExistingDataref<float>("CL650/lamps/integ/P2LM_plt_glrshld", [product](float val) {
        uint8_t target = static_cast<uint8_t>(std::clamp(val, 0.0f, 1.0f) * 255);
        product->setLedBrightness(FCUEfisLed::BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::EXPED_BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::EFISR_BACKLIGHT, target);
        product->setLedBrightness(FCUEfisLed::EFISL_BACKLIGHT, target);
        product->forceStateSync();
    });

    Dataref::getInstance()->monitorExistingDataref<float>("CL650/lamps/glareshield/FCP/ap_eng_1", [product](float val) {
        product->setLedBrightness(FCUEfisLed::AP1_GREEN, val > 0.5f ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<float>("CL650/lamps/glareshield/FCP/ap_eng_2", [product](float val) {
        product->setLedBrightness(FCUEfisLed::AP2_GREEN, val > 0.5f ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<float>("CL650/lamps/glareshield/FCP/appr_1", [product](float val) {
        product->setLedBrightness(FCUEfisLed::APPR_GREEN, val > 0.5f ? 1 : 0);
    });

    Dataref::getInstance()->monitorExistingDataref<float>("CL650/lamps/glareshield/ats_L", [product](float val) {
        product->setLedBrightness(FCUEfisLed::ATHR_GREEN, val > 0.5f ? 1 : 0);
    });
}

CL650FCUEfisProfile::~CL650FCUEfisProfile() {
    Dataref::getInstance()->unbind("CL650/lamps/integ/P2LM_plt_glrshld");
    Dataref::getInstance()->unbind("CL650/lamps/glareshield/FCP/ap_eng_1");
    Dataref::getInstance()->unbind("CL650/lamps/glareshield/FCP/ap_eng_2");
    Dataref::getInstance()->unbind("CL650/lamps/glareshield/FCP/appr_1");
    Dataref::getInstance()->unbind("CL650/lamps/glareshield/ats_L");
}

bool CL650FCUEfisProfile::IsEligible() {
    return Dataref::getInstance()->exists("CL650/CDU/1/screen/text_line0");
}

const std::vector<std::string> &CL650FCUEfisProfile::displayDatarefs() const {
    static const std::vector<std::string> datarefs = {};
    return datarefs;
}

const std::unordered_map<uint16_t, FCUEfisButtonDef> &CL650FCUEfisProfile::buttonDefs() const {
    static const std::unordered_map<uint16_t, FCUEfisButtonDef> buttons = {
        // FCU main panel
        {0,  {"SPD/MACH", "CL650/FCP/ias_mach"}},
        {3,  {"AP1",   "CL650/FCP/ap_eng"}},
        {4,  {"AP2",   "CL650/FCP/ap_eng"}},
        {5,  {"A/THR", "CL650/glareshield/ATS"}},
        {8,  {"APPR",  "CL650/FCP/appr_mode"}},

        // Speed encoder
        {9,  {"SPD DEC", "CL650/FCP/speed_down"}},
        {10, {"SPD INC", "CL650/FCP/speed_up"}},

        // Heading encoder
        {13, {"HDG DEC",  "CL650/FCP/hdg_down"}},
        {14, {"HDG INC",  "CL650/FCP/hdg_up"}},
        {15, {"HDG PUSH", "CL650/FCP/hdg_mode"}},
        {16, {"HDG PULL", "CL650/FCP/hdy_sync"}},

        // Altitude encoder
        {17, {"ALT DEC",  "CL650/FCP/alt_down"}},
        {18, {"ALT INC",  "CL650/FCP/alt_up"}},
        {19, {"ALT PUSH", "CL650/FCP/vnav_mode"}},
        {20, {"ALT PULL", "CL650/FCP/alt_mode"}},

        // Vertical Speed encoder
        {21, {"VS DEC",  "CL650/FCP/vs_pitch_down"}},
        {22, {"VS INC",  "CL650/FCP/vs_pitch_up"}},
        {23, {"VS PUSH", "CL650/FCP/vs_mode"}},

        // EFIS Left — FD + Baro
        {32, {"L_FD", "CL650/FCP/fd1"}},
        {39, {"L_BARO PUSH", "CL650/DCP/1/baro_push"}},
        {41, {"L_BARO DEC",  "CL650/DCP/1/baro_down"}},
        {42, {"L_BARO INC",  "CL650/DCP/1/baro_up"}},

        // EFIS Right — FD + Baro
        {64, {"R_FD", "CL650/FCP/fd2"}},
        {71, {"R_BARO PUSH", "CL650/DCP/2/baro_push"}},
        {73, {"R_BARO DEC",  "CL650/DCP/2/baro_down"}},
        {74, {"R_BARO INC",  "CL650/DCP/2/baro_up"}},
    };

    return buttons;
}

void CL650FCUEfisProfile::updateDisplayData(FCUDisplayData &data) {
    data.displayEnabled = false;
}

void CL650FCUEfisProfile::buttonPressed(const FCUEfisButtonDef *button, XPLMCommandPhase phase) {
    if (!button || button->dataref.empty() || phase == xplm_CommandContinue) {
        return;
    }

    auto datarefManager = Dataref::getInstance();
    if (phase == xplm_CommandBegin) {
        datarefManager->executeCommand(button->dataref.c_str());
    }
}
