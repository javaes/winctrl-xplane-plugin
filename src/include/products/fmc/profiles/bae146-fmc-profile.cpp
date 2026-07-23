#include "bae146-fmc-profile.h"

#include "appstate.h"
#include "dataref.h"
#include "product-fmc.h"
#include "logger.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <limits>
#include <regex>

// The FJCC UFMC exposes every key as a "Read - Write" dataref: set it to 1 to
// simulate a press and it returns to zero automatically. Display text lives in
// FJCC/UFMC/LINE_1..14 as strings with ";<code>" colour escapes, identical to
// the FPS748 UFMC handling.
static const std::string kPrefix = "FJCC/UFMC/";

BAE146FMCProfile::BAE146FMCProfile(ProductFMC *product) : FMCAircraftProfile(product) {
    datarefRegex = std::regex(kPrefix + "LINE_([0-9]+)");

    product->setAllLedsEnabled(false);
    product->setFont(FontVariant::Font737);

    // The BAe 146 is a Thranda airframe using stock X-Plane electrical and
    // lighting datarefs, so the backlight follows the panel brightness rheostat
    // and is gated on battery power (the same datarefs the UFMC plugin reads).
    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("sim/cockpit2/electrical/instrument_brightness_ratio", [this, product](const std::vector<float> &brightness) {
        bool hasPower = Dataref::getInstance()->get<bool>("sim/cockpit/electrical/battery_on");

        float panel = 0.0f;
        for (size_t i = 0; i < brightness.size() && i < 4; i++) {
            panel = std::max(panel, brightness[i]);
        }

        uint8_t backlight = hasPower ? static_cast<uint8_t>(panel * 255) : 0;
        uint8_t screens = hasPower ? 255 : 0;

        if (backlight == lastBacklightSent && screens == lastScreensSent) {
            return;
        }
        lastBacklightSent = backlight;
        lastScreensSent = screens;

        product->setLedBrightness(FMCLed::BACKLIGHT, backlight);
        product->setLedBrightness(FMCLed::SCREEN_BACKLIGHT, screens);
        product->setLedBrightness(FMCLed::OVERALL_LEDS_BRIGHTNESS, screens);
    },
        this);

    Dataref::getInstance()->monitorExistingDataref<bool>("sim/cockpit/electrical/battery_on", [](bool batteryOn) {
        Dataref::getInstance()->executeChangedCallbacksForDataref("sim/cockpit2/electrical/instrument_brightness_ratio");
    },
        this);

    // EXEC annunciator. The BAe 146 has a single FMC, so drive the light from the
    // pilot exec flag regardless of the connected device variant.
    Dataref::getInstance()->monitorExistingDataref<bool>((kPrefix + "Exec_Light_on_Pilot").c_str(), [product](bool enabled) {
        product->setLedBrightness(FMCLed::PFP_EXEC, enabled ? 1 : 0);
        product->setLedBrightness(FMCLed::MCDU_STATUS, enabled ? 1 : 0);
    },
        this);

    Logger::getInstance()->info("FMC: BAe 146 (FJCC UFMC) profile active\n");
    Dataref::getInstance()->executeChangedCallbacksForDataref("sim/cockpit2/electrical/instrument_brightness_ratio");
}

bool BAE146FMCProfile::IsEligible() {
    std::string icao = Dataref::getInstance()->get<std::string>("sim/aircraft/view/acf_ICAO");
    // JF BAe 146: -100 = B461, -200 = B462, -300 = B463. Also require the FJCC
    // UFMC to be loaded so we never claim another B46x airframe.
    return icao.starts_with("B46") && Dataref::getInstance()->exists((kPrefix + "LINE_1").c_str());
}

const std::vector<std::string> &BAE146FMCProfile::displayDatarefs() const {
    static const std::vector<std::string> datarefs = {
        kPrefix + "LINE_1",
        kPrefix + "LINE_2",
        kPrefix + "LINE_3",
        kPrefix + "LINE_4",
        kPrefix + "LINE_5",
        kPrefix + "LINE_6",
        kPrefix + "LINE_7",
        kPrefix + "LINE_8",
        kPrefix + "LINE_9",
        kPrefix + "LINE_10",
        kPrefix + "LINE_11",
        kPrefix + "LINE_12",
        kPrefix + "LINE_13",
        kPrefix + "LINE_14"};

    return datarefs;
}

const std::vector<FMCButtonDef> &BAE146FMCProfile::buttonDefs() const {
    static const std::vector<FMCButtonDef> buttons = {
        {FMCKey::LSK1L, kPrefix + "LK1", FMCDatarefType::SET_VALUE},
        {FMCKey::LSK2L, kPrefix + "LK2", FMCDatarefType::SET_VALUE},
        {FMCKey::LSK3L, kPrefix + "LK3", FMCDatarefType::SET_VALUE},
        {FMCKey::LSK4L, kPrefix + "LK4", FMCDatarefType::SET_VALUE},
        {FMCKey::LSK5L, kPrefix + "LK5", FMCDatarefType::SET_VALUE},
        {FMCKey::LSK6L, kPrefix + "LK6", FMCDatarefType::SET_VALUE},
        {FMCKey::LSK1R, kPrefix + "RK1", FMCDatarefType::SET_VALUE},
        {FMCKey::LSK2R, kPrefix + "RK2", FMCDatarefType::SET_VALUE},
        {FMCKey::LSK3R, kPrefix + "RK3", FMCDatarefType::SET_VALUE},
        {FMCKey::LSK4R, kPrefix + "RK4", FMCDatarefType::SET_VALUE},
        {FMCKey::LSK5R, kPrefix + "RK5", FMCDatarefType::SET_VALUE},
        {FMCKey::LSK6R, kPrefix + "RK6", FMCDatarefType::SET_VALUE},
        {std::vector<FMCKey>{FMCKey::PFP_INIT_REF, FMCKey::MCDU_INIT}, kPrefix + "INITREF", FMCDatarefType::SET_VALUE},
        {std::vector<FMCKey>{FMCKey::PFP_ROUTE, FMCKey::MCDU_SEC_FPLN}, kPrefix + "RTE", FMCDatarefType::SET_VALUE},
        {std::vector<FMCKey>{FMCKey::PFP4_ATC, FMCKey::MCDU_ATC_COMM}, kPrefix + "ATC", FMCDatarefType::SET_VALUE},
        {std::vector<FMCKey>{FMCKey::PFP4_VNAV, FMCKey::MCDU_DATA, FMCKey::PFP7_VNAV}, kPrefix + "VNAV", FMCDatarefType::SET_VALUE},
        {std::vector<FMCKey>{FMCKey::PFP_FIX, FMCKey::MCDU_EMPTY_BOTTOM_LEFT}, kPrefix + "FIX", FMCDatarefType::SET_VALUE},
        {std::vector<FMCKey>{FMCKey::PFP_LEGS, FMCKey::MCDU_FPLN, FMCKey::MCDU_DIR}, kPrefix + "LEGS", FMCDatarefType::SET_VALUE},
        {std::vector<FMCKey>{FMCKey::PFP_DEP_ARR, FMCKey::MCDU_AIRPORT}, kPrefix + "DEPARR", FMCDatarefType::SET_VALUE},
        {FMCKey::PFP_HOLD, kPrefix + "HOLD", FMCDatarefType::SET_VALUE},
        {std::vector<FMCKey>{FMCKey::PFP4_FMC_COMM, FMCKey::PFP7_FMC_COMM}, kPrefix + "FMCCOM", FMCDatarefType::SET_VALUE},
        {FMCKey::PROG, kPrefix + "PROG", FMCDatarefType::SET_VALUE},
        {std::vector<FMCKey>{FMCKey::PFP_EXEC, FMCKey::MCDU_EMPTY_TOP_RIGHT}, kPrefix + "EXEC", FMCDatarefType::SET_VALUE},
        {FMCKey::MENU, kPrefix + "MENU", FMCDatarefType::SET_VALUE},
        {std::vector<FMCKey>{FMCKey::PFP4_NAV_RAD, FMCKey::MCDU_RAD_NAV, FMCKey::PFP7_NAV_RAD}, kPrefix + "NAVRAD", FMCDatarefType::SET_VALUE},
        {FMCKey::PAGE_PREV, kPrefix + "PREVPAGE", FMCDatarefType::SET_VALUE},
        {FMCKey::PAGE_NEXT, kPrefix + "NEXTPAGE", FMCDatarefType::SET_VALUE},
        {FMCKey::KEY1, kPrefix + "1", FMCDatarefType::SET_VALUE},
        {FMCKey::KEY2, kPrefix + "2", FMCDatarefType::SET_VALUE},
        {FMCKey::KEY3, kPrefix + "3", FMCDatarefType::SET_VALUE},
        {FMCKey::KEY4, kPrefix + "4", FMCDatarefType::SET_VALUE},
        {FMCKey::KEY5, kPrefix + "5", FMCDatarefType::SET_VALUE},
        {FMCKey::KEY6, kPrefix + "6", FMCDatarefType::SET_VALUE},
        {FMCKey::KEY7, kPrefix + "7", FMCDatarefType::SET_VALUE},
        {FMCKey::KEY8, kPrefix + "8", FMCDatarefType::SET_VALUE},
        {FMCKey::KEY9, kPrefix + "9", FMCDatarefType::SET_VALUE},
        {FMCKey::PERIOD, kPrefix + "punto", FMCDatarefType::SET_VALUE},
        {FMCKey::KEY0, kPrefix + "0", FMCDatarefType::SET_VALUE},
        {FMCKey::PLUSMINUS, kPrefix + "menos", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYA, kPrefix + "A", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYB, kPrefix + "B", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYC, kPrefix + "C", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYD, kPrefix + "D", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYE, kPrefix + "E", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYF, kPrefix + "F", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYG, kPrefix + "G", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYH, kPrefix + "H", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYI, kPrefix + "I", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYJ, kPrefix + "J", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYK, kPrefix + "K", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYL, kPrefix + "L", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYM, kPrefix + "M", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYN, kPrefix + "N", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYO, kPrefix + "O", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYP, kPrefix + "P", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYQ, kPrefix + "Q", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYR, kPrefix + "R", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYS, kPrefix + "S", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYT, kPrefix + "T", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYU, kPrefix + "U", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYV, kPrefix + "V", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYW, kPrefix + "W", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYX, kPrefix + "X", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYY, kPrefix + "Y", FMCDatarefType::SET_VALUE},
        {FMCKey::KEYZ, kPrefix + "Z", FMCDatarefType::SET_VALUE},
        {FMCKey::SPACE, kPrefix + "espacio", FMCDatarefType::SET_VALUE},
        {std::vector<FMCKey>{FMCKey::PFP_DEL, FMCKey::MCDU_OVERFLY}, kPrefix + "DEL", FMCDatarefType::SET_VALUE},
        {FMCKey::SLASH, kPrefix + "barra", FMCDatarefType::SET_VALUE},
        {FMCKey::CLR, kPrefix + "CLR", FMCDatarefType::SET_VALUE}};

    return buttons;
}

const std::unordered_map<FMCKey, const FMCButtonDef *> &BAE146FMCProfile::buttonKeyMap() const {
    static std::unordered_map<FMCKey, const FMCButtonDef *> map;
    if (map.empty()) {
        const auto &buttons = buttonDefs();
        for (const auto &button : buttons) {
            std::visit([&](auto &&k) {
                using T = std::decay_t<decltype(k)>;
                if constexpr (std::is_same_v<T, FMCKey>) {
                    map[k] = &button;
                } else {
                    for (const auto &key : k) {
                        map[key] = &button;
                    }
                }
            },
                button.key);
        }
    }
    return map;
}

const std::map<char, FMCTextColor> &BAE146FMCProfile::colorMap() const {
    static const std::map<char, FMCTextColor> colMap = {
        {'c', FMCTextColor::COLOR_CYAN},
        {'g', FMCTextColor::COLOR_GREEN},
        {'p', FMCTextColor::COLOR_MAGENTA},
        {'w', FMCTextColor::COLOR_WHITE},
        {'m', FMCTextColor::COLOR_MAGENTA},
        {'y', FMCTextColor::COLOR_YELLOW},
        {'i', FMCTextColor::withBackgroundColor(FMCTextColor::COLOR_WHITE, FMCTextColor::COLOR_GREY)}, // i = Inverted
    };

    return colMap;
}

void BAE146FMCProfile::mapCharacter(std::vector<uint8_t> *buffer, uint8_t character, bool isFontSmall) {
    switch (character) {
        case '#':
            buffer->insert(buffer->end(), FMCSpecialCharacter::OUTLINED_SQUARE.begin(), FMCSpecialCharacter::OUTLINED_SQUARE.end());
            break;

        case '=':
            buffer->insert(buffer->end(), FMCSpecialCharacter::DEGREES.begin(), FMCSpecialCharacter::DEGREES.end());
            break;

        default:
            buffer->push_back(character);
            break;
    }
}

void BAE146FMCProfile::updatePage(std::vector<std::vector<char>> &page) {
    page = std::vector<std::vector<char>>(ProductFMC::PageLines, std::vector<char>(ProductFMC::PageCharsPerLine * ProductFMC::PageBytesPerChar, ' '));

    auto datarefManager = Dataref::getInstance();
    for (const auto &ref : displayDatarefs()) {
        std::smatch match;
        if (!std::regex_match(ref, match, datarefRegex)) {
            continue;
        }

        int lineNum = std::stoi(match[1]);
        int lineIndex = lineNum - 1;

        if (lineIndex < 0 || lineIndex >= ProductFMC::PageLines) {
            continue;
        }

        std::string text = datarefManager->getCached<std::string>(ref.c_str());
        if (text.empty()) {
            continue;
        }

        // Count visible characters (skip ';X' color escapes, stop at 0x00)
        auto visibleLength = [&](const std::string &s) {
            int len = 0;
            for (int i = 0; i < (int) s.size(); ++i) {
                if ((unsigned char) s[i] == 0x00) {
                    break;
                }
                if (s[i] == ';' && i + 1 < (int) s.size()) {
                    ++i;
                    continue;
                }
                if (s[i] == '[' && i + 1 < (int) s.size() && s[i + 1] == ']') {
                    ++i;
                    ++len;
                    continue;
                }
                ++len;
            }
            return len;
        };

        // If too long, strip spaces from the middle outward until it fits
        while (visibleLength(text) > (int) ProductFMC::PageCharsPerLine) {
            int mid = text.size() / 2;
            int found = -1;
            for (int d = 0; d <= mid; ++d) {
                if (mid + d < (int) text.size() && text[mid + d] == ' ') {
                    found = mid + d;
                    break;
                }
                if (mid - d >= 0 && text[mid - d] == ' ') {
                    found = mid - d;
                    break;
                }
            }
            if (found == -1) {
                break;
            }
            text.erase(found, 1);
        }

        char currentColor = 'w';
        bool fontSmall = lineIndex % 2 == 1;
        int displayPos = 0;

        for (int i = 0; i < text.size() && displayPos < ProductFMC::PageCharsPerLine; ++i) {
            unsigned char c = text[i];
            if (c == 0x00) {
                break;
            }

            if (c == ';' && i + 1 < text.size()) {
                char colorCode = text[i + 1];
                currentColor = colorCode;
                i++; // Skip the color code character
                continue;
            }

            if (c == '[' && i + 1 < text.size() && text[i + 1] == ']') {
                product->writeLineToPage(page, lineIndex, displayPos, "#", currentColor, fontSmall);
                displayPos++;
                i++; // Skip the closing bracket
                continue;
            }

            if (c != 0x20) {
                product->writeLineToPage(page, lineIndex, displayPos, std::string(1, toupper(c)), currentColor, fontSmall);
            }
            displayPos++;
        }
    }
}

void BAE146FMCProfile::buttonPressed(const FMCButtonDef *button, XPLMCommandPhase phase) {
    if (!button || button->dataref.empty() || phase == xplm_CommandContinue) {
        return;
    }

    auto datarefManager = Dataref::getInstance();
    if (button->datarefType == FMCDatarefType::SET_VALUE || button->datarefType == FMCDatarefType::SET_VALUE_PHASED) {
        double value = std::fabs(button->value) < std::numeric_limits<double>::epsilon() ? 1.0 : button->value;
        if (button->datarefType == FMCDatarefType::SET_VALUE && phase != xplm_CommandBegin) {
            return;
        }

        datarefManager->set<double>(button->dataref.c_str(), phase == xplm_CommandBegin ? value : 0.0);
    } else if (phase == xplm_CommandBegin && button->datarefType == FMCDatarefType::ADJUST_VALUE) {
        double currentValue = datarefManager->get<double>(button->dataref.c_str());
        datarefManager->set<double>(button->dataref.c_str(), currentValue + button->value);
    } else if (phase == xplm_CommandBegin && button->datarefType == FMCDatarefType::EXECUTE_CMD_ONCE) {
        datarefManager->executeCommand(button->dataref.c_str());
    } else {
        datarefManager->executeCommand(button->dataref.c_str(), phase);
    }
}
