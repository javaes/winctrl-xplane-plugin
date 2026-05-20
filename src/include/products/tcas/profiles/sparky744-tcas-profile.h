#ifndef SPARKYB744_TCAS_PROFILE_H
#define SPARKYB744_TCAS_PROFILE_H

#include "tcas-aircraft-profile.h"

#include <string>

class SparkyB744TCASProfile : public TCASAircraftProfile {
    public:
        SparkyB744TCASProfile(ProductTCAS *product);
        ~SparkyB744TCASProfile();

        static bool IsEligible();

        const std::unordered_map<uint16_t, TCASButtonDef> &buttonDefs() const override;
        void buttonPressed(const TCASButtonDef *button, XPLMCommandPhase phase) override;
        void updateDisplays() override;
};

#endif
