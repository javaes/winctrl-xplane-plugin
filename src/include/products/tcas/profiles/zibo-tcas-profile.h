#ifndef ZIBO_TCAS_PROFILE_H
#define ZIBO_TCAS_PROFILE_H

#include "tcas-aircraft-profile.h"

class ZiboTCASProfile : public TCASAircraftProfile {
    public:
        ZiboTCASProfile(ProductTCAS *product);
        ~ZiboTCASProfile();

        static bool IsEligible();

        const std::unordered_map<uint16_t, TCASButtonDef> &buttonDefs() const override;
        void buttonPressed(const TCASButtonDef *button, XPLMCommandPhase phase) override;
        void updateDisplays() override;
};

#endif
