#ifndef JOYSTICK_AIRCRAFT_PROFILE_H
#define JOYSTICK_AIRCRAFT_PROFILE_H

#include "owner-cleanup.h"

#include <string>
#include <vector>

class USBDevice;

class JoystickAircraftProfile {
    protected:
        USBDevice *product;

    public:
        JoystickAircraftProfile(USBDevice *product) : product(product) {};
        virtual ~JoystickAircraftProfile() {
            cleanupOwner(this);
        }

        virtual void update() {};
};

#endif
