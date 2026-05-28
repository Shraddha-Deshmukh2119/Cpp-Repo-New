#include <cstdlib>

// Linker wrapper helpers for MinGW/GCC.
// The test targets pass -Wl,--wrap=getch / _getch / system, so we must provide
// __wrap_getch, __wrap__getch, and __wrap_system symbols.
extern "C" {

int __wrap_getch(void)
{
    // Return Enter (carriage return) so menu loops don't block.
    return '\r';
}

int __wrap__getch(void)
{
    return '\r';
}

int __wrap_system(const char* command)
{
    (void)command;
    return 0;
}

} // extern "C"

