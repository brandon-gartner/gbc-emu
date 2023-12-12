#ifndef EMULATOR_H
#define EMULATOR_H

class emulator {
    public:
        enum status
        {
            running,
            paused
        };

        void start();
        emulator::status get_status();
        void set_status(emulator::status new_status);

    private:
        status current;
};


#endif