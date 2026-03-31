#include "audio.h"
#ifdef _WIN32
#include <windows.h>
#endif

void shootSound() {
#ifdef _WIN32
    Beep(1000, 100);
#endif
}

void explosionSound() {
#ifdef _WIN32
    Beep(400, 200);
#endif
}