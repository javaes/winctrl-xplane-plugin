#ifndef JOYSTICK_AIRCRAFT_PROFILE_H
#define JOYSTICK_AIRCRAFT_PROFILE_H

#include <string>
#include <vector>

class USBDevice;

class JoystickAircraftProfile {
    protected:
        USBDevice *product;

    public:
        JoystickAircraftProfile(USBDevice *product) : product(product) {};
        virtual ~JoystickAircraftProfile() = default;

        virtual void update() {};
};

#endif
