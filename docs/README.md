[![](markdownAssets/banner.png)](https://github.com/shezik/espnut)  
Eric Smith's Nonpareil emulator ported to ESP32. Supports HP Voyager calculators.  
The successor of shezik/esp15c (now archived).

## Now in code auditing phase!
... before I can get my hands on some hardware to really test it out.

### Features
- Matrix keyboard driver for ESP32-S3 that doesn't support dedicated GPIO interrupt :)
- Basic power management
- State saving & loading
- Graphical menu and file picker

### Todo
- Check if ESP32 LittleFS library can open directories and APIs are used correctly
- File deletion (File picker will officially be file manager with this)
- Logging and accessing logfile
- Customizing state save filename
- Implement emu.setUnlockSpeed()
- Read comments marked with '!!'
- Add ESP32 cpu frequency options to Menu
- Battery level detection

### Coming soon
- PCB file & BOM

You just have to wait ツ  

----------------

### Porting to unicore chips
This project utilizes the auxiliary core of ESP32-S3 to scan the matrix keyboard. You'll need to take care of the xTaskCreatePinnedToCore function called in src/MatrixKeyboard.cpp, though I suppose that ARDUINO_RUNNING_CORE would also be 0 in this case.

Additionally, check commit 7db65a540dc9269db74c296085509b6408cde28f if watchdog is often triggered. You'll probably want to replace yield() with vTaskDelay().

Also confirm that there's enough RAM that fits the ROM files.
