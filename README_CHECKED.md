Checked final version notes

- Folder name is now MediDispenser to match MediDispenser.ino.
- app_webserver.h/cpp is used instead of webserver.h/cpp to avoid conflict with ESP32 WebServer.h on Windows.
- STEPS_PER_REV is 4096, so STEPS_PER_SLOT is 512 for 8 carousel positions.
- Refill handler rotates one slot when the carousel is at the loading bay or marked empty, then resets state to Sunday and 7/7.
- API status reports Loading Bay when dosesThisWeek >= 7.
- Carousel state is saved to Preferences after movement/reset.

Hardware note: without a home/limit sensor, software cannot detect if the 28BYJ-48 physically skipped steps. Use separate 5V power for ULN2003 motor supply and common GND with ESP32.
