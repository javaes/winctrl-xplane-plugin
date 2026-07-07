#ifndef ZIBO_RMP_PROFILE_H
#define ZIBO_RMP_PROFILE_H

#include "rmp-aircraft-profile.h"

#include <string>
#include <vector>

class ZiboRMPProfile : public RMPAircraftProfile {
    private:
        std::vector<std::string> _displayDatarefs;
        const char *rtpName() const;

    public:
        ZiboRMPProfile(ProductRMP *product);

        static bool IsEligible();
        const std::vector<std::string> &displayDatarefs() const override;
        const std::unordered_map<uint16_t, RMPButtonDef> &buttonDefs() const override;

        void buttonPressed(const RMPButtonDef *button, XPLMCommandPhase phase) override;

        void updateDisplays() override;
};

#endif
