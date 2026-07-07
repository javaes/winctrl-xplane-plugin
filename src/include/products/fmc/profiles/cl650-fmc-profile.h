#ifndef CL650_FMC_PROFILE_H
#define CL650_FMC_PROFILE_H

#include "fmc-aircraft-profile.h"

class CL650FMCProfile : public FMCAircraftProfile {
    public:
        CL650FMCProfile(ProductFMC *product);
        ~CL650FMCProfile();

        static bool IsEligible();
        const std::vector<std::string> &displayDatarefs() const override;
        const std::vector<FMCButtonDef> &buttonDefs() const override;
        const std::unordered_map<FMCKey, const FMCButtonDef *> &buttonKeyMap() const override;
        const std::map<char, FMCTextColor> &colorMap() const override;
        void mapCharacter(std::vector<uint8_t> *buffer, uint8_t character, bool isFontSmall) override;
        void updatePage(std::vector<std::vector<char>> &page) override;
        void buttonPressed(const FMCButtonDef *button, XPLMCommandPhase phase) override;
};

#endif // CL650_FMC_PROFILE_H
