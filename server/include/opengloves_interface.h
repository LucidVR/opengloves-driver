#pragma once

#include <functional>
#include <string>

namespace og {

    enum Hand {
        kHandLeft,
        kHandRight
    };

    struct BluetoothConfiguration {
        std::string name;
    };

    struct SerialConfiguration {
        std::string port_name;
    };

    struct EncodingConfiguration {
        unsigned int max_analog_value;
    };

    struct LegacyConfiguration {
        Hand hand;

        SerialConfiguration serial_configuration;
        BluetoothConfiguration bluetooth_configuration;

        EncodingConfiguration encoding_configuration;
    };

    class Device {

    };

    class Server {
    public:

        /**
         * Used to provide legacy configuration values (explicitly setting comm ports, bluetooth name, encoding, etc.)
         * Not needed for newer versions of the firmware.
         * Not calling this before starting to probe for devices is fine, but means that any devices running firmware
         * where we can't get data from them will be dropped.
         */
        void SetLegacyConfiguration(const LegacyConfiguration &configuration);

        /***
         * Start looking for devices. The callback will be called for every new device found.
         */
        int StartProber(std::function<void(Device *device)>);

        int StopProber();

        ~OGServer();
    };
}