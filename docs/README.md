[![](markdownAssets/banner.png)](https://github.com/shezik/espnut)  
Eric Smith's Nonpareil emulator ported to ESP32. Supports HP Voyager calculators.  
The successor of shezik/esp15c (now archived).

## ~Now in code auditing phase!~
actually we're still under development.

### Features
- Matrix keyboard driver for ESP32-S3 that doesn't support dedicated GPIO interrupt :)
- Power management
- State saving & loading
- Graphical menu and file manager

### Todo (Sorted by importance)
- deleting empty directories
- Logging and accessing logfile
- Read comments marked with '!!'
- Add ESP32 cpu frequency options to Menu

### Coming soon
- PCB & BOM

You just have to wait, like, soon? ツ  

----------------

### Porting to unicore chips
This project utilizes the auxiliary core of ESP32-S3 to scan the matrix keyboard. You'll need to take care of the `xTaskCreatePinnedToCore` function called in *src/MatrixKeyboard.cpp*, though I suppose that ARDUINO_RUNNING_CORE would also be 0 in this case.

Additionally, check commit 7db65a if watchdog is often triggered. You'll probably want to replace `yield()` with `vTaskDelay()`.

Also confirm that there's enough RAM that fits the ROM files.
