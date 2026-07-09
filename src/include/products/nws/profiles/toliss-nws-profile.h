#ifndef TOLISS_NWS_PROFILE_H
#define TOLISS_NWS_PROFILE_H

#include "nws-aircraft-profile.h"

class ProductNWS;

class TolissNWSProfile : public NWSAircraftProfile {
    public:
        TolissNWSProfile(ProductNWS *product);

        static bool IsEligible();
};

#endif
