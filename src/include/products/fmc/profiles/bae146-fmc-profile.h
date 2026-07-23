#ifndef BAE146_FMC_PROFILE_H
#define BAE146_FMC_PROFILE_H

#include "fmc-aircraft-profile.h"

#include <regex>

// JustFlight / Thranda BAe 146 Professional. The FMC is driven by the shared
// FJCC UFMC plugin (Javier Cortes), same family as the FPS748 profile but with
// the "FJCC/UFMC/" dataref namespace. Display lines are read as strings and the
// keys are write-1 datarefs that auto-reset ("Set to 1 for simulate pressing").
class BAE146FMCProfile : public FMCAircraftProfile {
    private:
        std::regex datarefRegex;
        int lastBacklightSent = -1;
        int lastScreensSent = -1;

    public:
        BAE146FMCProfile(ProductFMC *product);

        static bool IsEligible();

        const std::vector<std::string> &displayDatarefs() const override;
        const std::vector<FMCButtonDef> &buttonDefs() const override;
        const std::unordered_map<FMCKey, const FMCButtonDef *> &buttonKeyMap() const override;
        const std::map<char, FMCTextColor> &colorMap() const override;
        void mapCharacter(std::vector<uint8_t> *buffer, uint8_t character, bool isFontSmall) override;
        void updatePage(std::vector<std::vector<char>> &page) override;
        void buttonPressed(const FMCButtonDef *button, XPLMCommandPhase phase) override;
};

#endif
