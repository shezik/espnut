[![](markdownAssets/banner-dev.png)](https://github.com/shezik/espnut)  
An ESP32 port of Eric Smith's Nonpareil emulator. Designed for HP Voyager calculator emulation.  
The successor of shezik/esp15c (now archived).  
**To perform a firmware update, visit [espnut-updater](https://github.com/shezik/espnut-updater).**

### Features
- Matrix keyboard driver for ESP32-S3 that doesn't support dedicated GPIO interrupt :)
- Power management
- State saving & loading
- Graphical menu and file manager
- Synthetic programming support for HP-15C (y<sup>x</sup> + ON)

### Todo (Sorted by priority)
- Remove excess variables saved in state file
- deleting empty directories
- Logging and accessing logfile
- Read comments marked with '!!'
- Apply ROM patches dynamically
- Add ESP32 cpu frequency options to Menu

Schematics are available [here](docs/Schematics).

--------------------------------------------------------------------------------------------------------------------------------

### Porting to unicore chips
This project utilizes the auxiliary core of ESP32-S3 to scan the matrix keyboard. You'll need to take care of the `xTaskCreatePinnedToCore` function called in *src/MatrixKeyboard.cpp*, though I suppose that ARDUINO_RUNNING_CORE would also be 0 in this case.

Additionally, check commit 7db65a if watchdog is often triggered. You'll probably want to replace `yield()` with `vTaskDelay()`.

Also confirm that there's enough RAM that fits the ROM files.
