#ifndef Q4XP_FCU_EFIS_PROFILE_H
#define Q4XP_FCU_EFIS_PROFILE_H

#include "fcu-efis-aircraft-profile.h"

#include <map>
#include <string>
#include <unordered_map>

class Q4XPFCUEfisProfile : public FCUEfisAircraftProfile {
    public:
        Q4XPFCUEfisProfile(ProductFCUEfis *product);
        ~Q4XPFCUEfisProfile();

        static bool IsEligible();

        const std::vector<std::string> &displayDatarefs() const override;
        const std::unordered_map<uint16_t, FCUEfisButtonDef> &buttonDefs() const override;
        void updateDisplayData(FCUDisplayData &data) override;

        bool hasEfisLeft() const override {
            return true;
        }

        bool hasEfisRight() const override {
            return true;
        }

        void buttonPressed(const FCUEfisButtonDef *button, XPLMCommandPhase phase) override;
};

#endif
