#include "q4xp-fmc-profile.h"

#include "config.h"
#include "dataref.h"
#include "product-fmc.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <sstream>

Q4XPFMCProfile::Q4XPFMCProfile(ProductFMC *product) : FMCAircraftProfile(product) {
    product->setAllLedsEnabled(false);

    Dataref::getInstance()->monitorExistingDataref<std::vector<float>>("FJS/Q4XP/Lights/panelText_LIT", [product](const std::vector<float> &brightness) {
        if (brightness.size() <= 2) {
            return;
        }

        uint8_t target = static_cast<uint8_t>(std::clamp(brightness[2], 0.0f, 1.0f) * 255);
        product->setLedBrightness(FMCLed::BACKLIGHT, target);
    },
        this);

    auto font = Font::GlyphData("Q4XP.xpwwf", product->identifierByte, product->hardwareType);
    if (!font.empty()) {
        for (auto &packet : font) {
            product->writeData(packet);
        }
    } else {
        product->setFont(FontVariant::Default);
    }
}

Q4XPFMCProfile::~Q4XPFMCProfile() {
}

bool Q4XPFMCProfile::IsEligible() {
    return Dataref::getInstance()->exists("FJS/Q4XP/cdu1/text_line_0");
}

const std::vector<std::string> &Q4XPFMCProfile::displayDatarefs() const {
    static std::unordered_map<FMCDeviceVariant, std::vector<std::string>> cache;

    auto it = cache.find(product->deviceVariant);
    if (it == cache.end()) {
        const std::string cdu = product->deviceVariant == FMCDeviceVariant::VARIANT_CAPTAIN ? "cdu1" : "cdu2";
        std::vector<std::string> refs;
        static constexpr int ScreenLines = 11;
        for (int i = 0; i < ScreenLines; ++i) {
            refs.push_back("FJS/Q4XP/" + cdu + "/text_line_" + std::to_string(i));
            refs.push_back("FJS/Q4XP/" + cdu + "/style_line_" + std::to_string(i));
        }
        it = cache.emplace(product->deviceVariant, std::move(refs)).first;
    }
    return it->second;
}

const std::vector<FMCButtonDef> &Q4XPFMCProfile::buttonDefs() const {
    static std::unordered_map<FMCDeviceVariant, std::vector<FMCButtonDef>> cache;

    auto it = cache.find(product->deviceVariant);
    if (it == cache.end()) {
        const std::string fms = product->deviceVariant == FMCDeviceVariant::VARIANT_CAPTAIN ? "fms1" : "fms2";

        std::vector<FMCButtonDef> buttons = {
            {FMCKey::LSK1L, "FJS/Q4XP/" + fms + "/lsk_l1", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::LSK2L, "FJS/Q4XP/" + fms + "/lsk_l2", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::LSK3L, "FJS/Q4XP/" + fms + "/lsk_l3", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::LSK4L, "FJS/Q4XP/" + fms + "/lsk_l4", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::LSK5L, "FJS/Q4XP/" + fms + "/lsk_l5", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::LSK1R, "FJS/Q4XP/" + fms + "/lsk_r1", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::LSK2R, "FJS/Q4XP/" + fms + "/lsk_r2", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::LSK3R, "FJS/Q4XP/" + fms + "/lsk_r3", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::LSK4R, "FJS/Q4XP/" + fms + "/lsk_r4", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::LSK5R, "FJS/Q4XP/" + fms + "/lsk_r5", FMCDatarefType::EXECUTE_CMD_ONCE},

            // UNS-1 only has 5 LSKs per side — LSK6 mapped empty
            {FMCKey::LSK6L, ""},
            {FMCKey::LSK6R, ""},

            {std::vector<FMCKey>{FMCKey::MCDU_DIR, FMCKey::PFP_INIT_REF}, "FJS/Q4XP/" + fms + "/dto", FMCDatarefType::EXECUTE_CMD_ONCE},
            {std::vector<FMCKey>{FMCKey::MCDU_PERF, FMCKey::PFP3_N1_LIMIT}, "FJS/Q4XP/" + fms + "/perf", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::MCDU_DATA, "FJS/Q4XP/" + fms + "/data", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::MCDU_FPLN, "FJS/Q4XP/" + fms + "/fpl", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::PFP_LEGS, "FJS/Q4XP/" + fms + "/fpl", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::PFP_ROUTE, "FJS/Q4XP/" + fms + "/fpl", FMCDatarefType::EXECUTE_CMD_ONCE},
            {std::vector<FMCKey>{FMCKey::MCDU_RAD_NAV, FMCKey::PFP4_NAV_RAD, FMCKey::PFP7_NAV_RAD}, "FJS/Q4XP/" + fms + "/nav", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::MCDU_FUEL_PRED, "FJS/Q4XP/" + fms + "/fuel", FMCDatarefType::EXECUTE_CMD_ONCE},
            {std::vector<FMCKey>{FMCKey::MCDU_ATC_COMM, FMCKey::PFP4_ATC}, "FJS/Q4XP/" + fms + "/tune", FMCDatarefType::EXECUTE_CMD_ONCE},
            {std::vector<FMCKey>{FMCKey::MCDU_INIT, FMCKey::PFP4_VNAV, FMCKey::PFP7_VNAV}, "FJS/Q4XP/" + fms + "/vnav", FMCDatarefType::EXECUTE_CMD_ONCE},
            {std::vector<FMCKey>{FMCKey::MCDU_AIRPORT, FMCKey::PFP_DEP_ARR}, "FJS/Q4XP/" + fms + "/list", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::MENU, "FJS/Q4XP/" + fms + "/menu", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::PFP_EXEC, "FJS/Q4XP/" + fms + "/key_enter", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::MCDU_OVERFLY, "FJS/Q4XP/" + fms + "/key_enter", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::MCDU_EMPTY_TOP_RIGHT, "FJS/Q4XP/" + fms + "/pwr", FMCDatarefType::EXECUTE_CMD_ONCE},
            {std::vector<FMCKey>{FMCKey::CLR, FMCKey::PFP_DEL}, "FJS/Q4XP/" + fms + "/key_back", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEY0, "FJS/Q4XP/" + fms + "/key_0", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEY1, "FJS/Q4XP/" + fms + "/key_1", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEY2, "FJS/Q4XP/" + fms + "/key_2", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEY3, "FJS/Q4XP/" + fms + "/key_3", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEY4, "FJS/Q4XP/" + fms + "/key_4", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEY5, "FJS/Q4XP/" + fms + "/key_5", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEY6, "FJS/Q4XP/" + fms + "/key_6", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEY7, "FJS/Q4XP/" + fms + "/key_7", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEY8, "FJS/Q4XP/" + fms + "/key_8", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEY9, "FJS/Q4XP/" + fms + "/key_9", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYA, "FJS/Q4XP/" + fms + "/key_A", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYB, "FJS/Q4XP/" + fms + "/key_B", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYC, "FJS/Q4XP/" + fms + "/key_C", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYD, "FJS/Q4XP/" + fms + "/key_D", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYE, "FJS/Q4XP/" + fms + "/key_E", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYF, "FJS/Q4XP/" + fms + "/key_F", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYG, "FJS/Q4XP/" + fms + "/key_G", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYH, "FJS/Q4XP/" + fms + "/key_H", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYI, "FJS/Q4XP/" + fms + "/key_I", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYJ, "FJS/Q4XP/" + fms + "/key_J", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYK, "FJS/Q4XP/" + fms + "/key_K", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYL, "FJS/Q4XP/" + fms + "/key_L", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYM, "FJS/Q4XP/" + fms + "/key_M", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYN, "FJS/Q4XP/" + fms + "/key_N", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYO, "FJS/Q4XP/" + fms + "/key_O", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYP, "FJS/Q4XP/" + fms + "/key_P", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYQ, "FJS/Q4XP/" + fms + "/key_Q", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYR, "FJS/Q4XP/" + fms + "/key_R", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYS, "FJS/Q4XP/" + fms + "/key_S", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYT, "FJS/Q4XP/" + fms + "/key_T", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYU, "FJS/Q4XP/" + fms + "/key_U", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYV, "FJS/Q4XP/" + fms + "/key_V", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYW, "FJS/Q4XP/" + fms + "/key_W", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYX, "FJS/Q4XP/" + fms + "/key_X", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYY, "FJS/Q4XP/" + fms + "/key_Y", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::KEYZ, "FJS/Q4XP/" + fms + "/key_Z", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::PLUSMINUS, "FJS/Q4XP/" + fms + "/plusminus", FMCDatarefType::EXECUTE_CMD_ONCE},

            // Arrows: UP=prev, DOWN=next, LEFT=back
            {FMCKey::MCDU_PAGE_UP, "FJS/Q4XP/" + fms + "/prev", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::MCDU_PAGE_DOWN, "FJS/Q4XP/" + fms + "/next", FMCDatarefType::EXECUTE_CMD_ONCE},
            {FMCKey::PAGE_PREV, "FJS/Q4XP/" + fms + "/back", FMCDatarefType::EXECUTE_CMD_ONCE},

            // Remaining keys — no UNS-1 equivalent
            {FMCKey::BRIGHTNESS_UP, ""},
            {FMCKey::BRIGHTNESS_DOWN, ""},
            {FMCKey::PAGE_NEXT, ""},
            {FMCKey::MCDU_EMPTY_BOTTOM_LEFT, ""},
            {FMCKey::MCDU_SEC_FPLN, ""},
            {FMCKey::SLASH, ""},
            {FMCKey::PERIOD, ""},
            {FMCKey::SPACE, ""},
            {FMCKey::PROG, ""},
            {FMCKey::PFP_HOLD, ""},
            {FMCKey::PFP_FIX, ""},
            {FMCKey::PFP3_CLB, ""},
            {FMCKey::PFP3_CRZ, ""},
            {FMCKey::PFP3_DES, ""},
            {FMCKey::PFP4_FMC_COMM, ""},
            {FMCKey::PFP7_ALTN, ""},
            {FMCKey::PFP7_FMC_COMM, ""},
        };

        it = cache.emplace(product->deviceVariant, std::move(buttons)).first;
    }
    return it->second;
}

const std::unordered_map<FMCKey, const FMCButtonDef *> &Q4XPFMCProfile::buttonKeyMap() const {
    static std::unordered_map<FMCDeviceVariant, std::unordered_map<FMCKey, const FMCButtonDef *>> cache;

    auto it = cache.find(product->deviceVariant);
    if (it == cache.end()) {
        std::unordered_map<FMCKey, const FMCButtonDef *> map;
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
        it = cache.emplace(product->deviceVariant, std::move(map)).first;
    }
    return it->second;
}

const std::map<char, FMCTextColor> &Q4XPFMCProfile::colorMap() const {
    static const std::map<char, FMCTextColor> colMap = {
        {0x00, FMCTextColor::COLOR_WHITE},
        {0x01, FMCTextColor::COLOR_WHITE},
        {0x02, FMCTextColor::COLOR_GREEN},
        {0x03, FMCTextColor::COLOR_YELLOW},
        {0x04, FMCTextColor::COLOR_GREEN},
        {0x05, FMCTextColor::COLOR_MAGENTA},
        {0x06, FMCTextColor::COLOR_GREEN},
        {0x07, FMCTextColor::COLOR_CYAN},
        {0x0B, FMCTextColor::COLOR_GREEN},
        {0x40, FMCTextColor::withBackgroundColor(FMCTextColor::COLOR_BLACK, FMCTextColor::COLOR_WHITE)},  // inverted (ACCEPT)
    };
    return colMap;
}

void Q4XPFMCProfile::mapCharacter(std::vector<uint8_t> *buffer, uint8_t character, bool isFontSmall) {
    switch (character) {
        case '#':
            buffer->insert(buffer->end(), FMCSpecialCharacter::OUTLINED_SQUARE.begin(), FMCSpecialCharacter::OUTLINED_SQUARE.end());
            break;

        case '<':
            buffer->insert(buffer->end(), FMCSpecialCharacter::ARROW_LEFT.begin(), FMCSpecialCharacter::ARROW_LEFT.end());
            break;

        case '>':
            buffer->insert(buffer->end(), FMCSpecialCharacter::ARROW_RIGHT.begin(), FMCSpecialCharacter::ARROW_RIGHT.end());
            break;

        case 30:
            buffer->insert(buffer->end(), FMCSpecialCharacter::ARROW_UP.begin(), FMCSpecialCharacter::ARROW_UP.end());
            break;

        case 31:
            buffer->insert(buffer->end(), FMCSpecialCharacter::ARROW_DOWN.begin(), FMCSpecialCharacter::ARROW_DOWN.end());
            break;

        case '`':
            buffer->insert(buffer->end(), FMCSpecialCharacter::DEGREES.begin(), FMCSpecialCharacter::DEGREES.end());
            break;

        case '^':
            buffer->insert(buffer->end(), FMCSpecialCharacter::TRIANGLE.begin(), FMCSpecialCharacter::TRIANGLE.end());
            break;

        default:
            buffer->push_back(character);
            break;
    }
}

void Q4XPFMCProfile::updatePage(std::vector<std::vector<char>> &page) {
    page = std::vector<std::vector<char>>(ProductFMC::PageLines, std::vector<char>(ProductFMC::PageCharsPerLine * ProductFMC::PageBytesPerChar, ' '));

    auto datarefManager = Dataref::getInstance();
    const std::string cdu = product->deviceVariant == FMCDeviceVariant::VARIANT_CAPTAIN ? "cdu1" : "cdu2";

    // Replace unicode symbols with single-byte placeholders
    const std::vector<std::pair<std::string, unsigned char>> symbols = {
        {"\u2190", '<'},  {"\u2192", '>'},  {"\u2191", 30},  {"\u2193", 31},
        {"\u2610", '#'},  {"\u00B0", '`'},  {"\u0394", '^'},
        {"\u2194", '<'},  {"\u2196", '<'},  {"\u2197", '>'},  {"\u2198", '>'},  {"\u2199", '<'},
        {"\u21E6", '<'},  {"\u21E8", '>'},  {"\u21E7", 30},  {"\u21E9", 31},
        {"\u2500", '-'},  {"\u2502", '|'},  {"\u250C", '+'}, {"\u2510", '+'}, {"\u2514", '+'}, {"\u2518", '+'},
        {"\u251C", '|'},  {"\u2524", '|'},  {"\u252C", '+'}, {"\u2534", '+'}, {"\u253C", '+'},
        {"\u2550", '='},  {"\u2551", '|'},  {"\u2554", '+'}, {"\u2557", '+'}, {"\u255A", '+'}, {"\u255D", '+'},
        {"\u2560", '+'},  {"\u2563", '+'},  {"\u2566", '+'}, {"\u2569", '+'}, {"\u256C", '+'},
        {"\u256D", '|'},  {"\u256E", '|'},  {"\u256F", '|'},  {"\u2570", '|'},
        {"\u23A1", '+'},  {"\u23A4", '+'},  {"\u23A7", '{'},  {"\u23AB", '}'},
        {"\u27E6", '['},  {"\u27E7", ']'},
    };

    static constexpr int ScreenLines = 11;

    for (int lineNum = 0; lineNum < ScreenLines; ++lineNum) {
        std::string textDataref = "FJS/Q4XP/" + cdu + "/text_line_" + std::to_string(lineNum);
        std::string styleDataref = "FJS/Q4XP/" + cdu + "/style_line_" + std::to_string(lineNum);

        std::string text = datarefManager->getCached<std::string>(textDataref.c_str());
        if (text.empty()) {
            continue;
        }

        std::vector<unsigned char> styleBytes = datarefManager->getCached<std::vector<unsigned char>>(styleDataref.c_str());

        for (const auto &symbol : symbols) {
            size_t pos = 0;
            while ((pos = text.find(symbol.first, pos)) != std::string::npos) {
                text.replace(pos, symbol.first.length(), std::string(1, static_cast<char>(symbol.second)));
                pos += 1;
            }
        }

        // Map remaining high-byte characters to ASCII equivalents
        for (size_t i = 0; i < text.size(); ++i) {
            unsigned char c = static_cast<unsigned char>(text[i]);
            if (c <= 127) { continue; }
            switch (c) {
                case 0xB0: case 0xB1: case 0xB2: text[i] = ' '; break;
                case 0xB3: case 0xDD: case 0xDE: text[i] = '|'; break;
                case 0xC4: case 0xDC: case 0xDF: text[i] = '-'; break;
                case 0xBA:                     text[i] = '|'; break;
                case 0xCD:                     text[i] = '='; break;
                case 0xBF: case 0xC0: case 0xD9: case 0xDA:
                case 0xB4: case 0xC3: case 0xC1: case 0xC2: case 0xC5:
                case 0xB5: case 0xB6: case 0xB7: case 0xB8: case 0xB9:
                case 0xBB: case 0xBC: case 0xBD: case 0xBE:
                case 0xC6: case 0xC7: case 0xC8: case 0xC9:
                case 0xCA: case 0xCB: case 0xCC: case 0xCE: case 0xCF:
                case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xD4:
                case 0xD5: case 0xD6: case 0xD7: case 0xD8:
                    text[i] = '+'; break;
                case 0xDB: text[i] = '#'; break;
                default:   text[i] = ' '; break;
            }
        }

        int displayLine = lineNum;
        if (displayLine >= ProductFMC::PageLines) {
            break;
        }

        for (int i = 0; i < text.size() && i < ProductFMC::PageCharsPerLine; ++i) {
            char c = text[i];
            if (c == 0x00) {
                continue;
            }

            bool fontSmall = false;
            unsigned char styleByte = (i < styleBytes.size()) ? styleBytes[i] : 0x00;
            fontSmall = (styleByte & 0xF0) == 0x00;

            bool isInverted = (styleByte & 0x40);
            if (c == ' ' && !isInverted) {
                continue;
            }

            unsigned char colorIdx = styleByte & 0x0F;
            if (styleByte & 0x40) {
                colorIdx = 0x40;  // inverted/reverse video
            }

            product->writeLineToPage(page, displayLine, i, std::string(1, c), colorIdx, fontSmall);
        }
    }
}

void Q4XPFMCProfile::buttonPressed(const FMCButtonDef *button, XPLMCommandPhase phase) {
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
    } else if (phase == xplm_CommandBegin && button->datarefType == FMCDatarefType::EXECUTE_MULTIPLE_CMD_ONCE) {
        std::stringstream ss(button->dataref);
        std::string item;
        std::vector<std::string> commands;
        while (std::getline(ss, item, ',')) {
            commands.push_back(item);
        }

        for (const auto &cmd : commands) {
            datarefManager->executeCommand(cmd.c_str());
        }
    } else if (phase == xplm_CommandBegin && button->datarefType == FMCDatarefType::EXECUTE_CMD_ONCE) {
        datarefManager->executeCommand(button->dataref.c_str());
    } else {
        datarefManager->executeCommand(button->dataref.c_str(), phase);
    }
}
