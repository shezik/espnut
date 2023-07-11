# Copyright Notice for Binary Distribution

## Contents
- [espnut](#espnut)
- [Nonpareil](#nonpareil)
- [arduino-esp32](#arduino-esp32)
- [GEM](#gem)
- [U8g2](#u8g2)
- [u8g2_font_6x12_tr](#u8g2_font_6x12_tr)
- [u8g2_font_tom_thumb_4x6_mr](#u8g2_font_tom_thumb_4x6_mr)

## espnut
Copyright (C) 2023&ensp;&ensp;shezik

espnut is free software; you can redistribute it and/or modify it  
under the terms of the GNU General Public License version 2 as  
published by the Free Software Foundation.&ensp;&ensp;Note that the permission  
to redistribute or modify espnut under the terms of any later  
version of the General Public License is denied by the author  
of Nonpareil, Eric L. Smith, according to his notice.

espnut is distributed in the hope that it will be useful, but  
WITHOUT ANY WARRANTY; without even the implied warranty of  
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
General Public License for more details.

You should have received a copy of the GNU General Public License  
along with this program (in the file "COPYING" or "[LICENSE](LICENSE)"); if not,  
write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,  
Boston, MA 02111, USA.

## Nonpareil
**Note that the relevant code used in this project is a MODIFIED version.**

The modifications are focused on function `bool nut_read_object_file (nut_reg_t *nut_reg, const char *fn)` in [proc_nut.cpp](src/proc_nut.cpp).

----------------------------------------------------------------

Copyright 1995, 2003, 2004, 2005 Eric L. Smith &lt;eric@brouhaha.com&gt;

Nonpareil is free software; you can redistribute it and/or modify it  
under the terms of the GNU General Public License version 2 as  
published by the Free Software Foundation.&ensp;&ensp;Note that I am not  
granting permission to redistribute or modify Nonpareil under the  
terms of any later version of the General Public License.

Nonpareil is distributed in the hope that it will be useful, but  
WITHOUT ANY WARRANTY; without even the implied warranty of  
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
General Public License for more details.

You should have received a copy of the GNU General Public License  
along with this program (in the file "[COPYING](LICENSE)"); if not, write to the  
Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,  
MA 02111, USA.

## arduino-esp32
Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD

Licensed under the Apache License, Version 2.0 (the "License");  
you may not use this file except in compliance with the License.  
You may obtain a copy of the License at

&ensp;&ensp;&ensp;&ensp;<http://www.apache.org/licenses/LICENSE-2.0>

Unless required by applicable law or agreed to in writing, software  
distributed under the License is distributed on an "AS IS" BASIS,  
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
See the License for the specific language governing permissions and  
limitations under the License.

## GEM
**Note that the relevant code used in this project is a MODIFIED version.**

Modifications on GEM_u8g2.h of spirik/GEM@^1.4.0:  
Line 134: `virtual void drawMenu();`  
Line 141: `protected:`

Modifications to GEMPage.h in spirik/GEM@^1.4.0:
Line 59: `protected:`

----------------------------------------------------------------

GEM (a.k.a. Good Enough Menu) - Arduino library for creation of graphic multi-level menu with  
editable menu items, such as variables (supports int, byte, float, double, boolean, char[17] data types)  
and option selects. User-defined callback function can be specified to invoke when menu item is saved.

Supports buttons that can invoke user-defined actions and create action-specific  
context, which can have its own enter (setup) and exit callbacks as well as loop function.

Supports:
- AltSerialGraphicLCD library by Jon Green (<http://www.jasspa.com/serialGLCD.html>);
- U8g2 library by olikraus (<https://github.com/olikraus/U8g2_Arduino>);
- Adafruit GFX library by Adafruit (<https://github.com/adafruit/Adafruit-GFX-Library>).

For documentation visit:  
<https://github.com/Spirik/GEM>

Copyright (c) 2018-2022 Alexander 'Spirik' Spiridonov

This library is free software; you can redistribute it and/or  
modify it under the terms of the GNU Lesser General Public  
License as published by the Free Software Foundation; either  
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,  
but WITHOUT ANY WARRANTY; without even the implied warranty of  
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  
Lesser General Public License for more details.

You should have received [a copy](docs/markdownAssets/lgpl-3.0.md) of the GNU Lesser General Public License  
along with this library.&ensp;&ensp;If not, see &lt;<http://www.gnu.org/licenses/>&gt;.

## U8g2
Universal 8bit Graphics Library (<https://github.com/olikraus/u8g2>)

Copyright (c) 2016, olikraus@gmail.com  
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list 
  of conditions and the following disclaimer.
  
* Redistributions in binary form must reproduce the above copyright notice, this 
  list of conditions and the following disclaimer in the documentation and/or other 
  materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND  
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,  
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF  
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE  
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR  
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT  
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;  
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER  
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,  
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)  
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF  
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

## u8g2_font_6x12_tr
Copyright information is available on the [x.org](https://www.x.org/archive/X11R7.5/doc/LICENSE.html) server.

Copyright string from the font file: "Public domain font. Share and enjoy." or "These glyphs are unencumbered"

## u8g2_font_tom_thumb_4x6_mr
u8g2_font_tom_thumb_4x6_mr (c) 1999 Brian J. Swetland, licensed under CC-BY 3.0

Tom-Thumb font is available at <https://robey.lag.net/2010/01/23/tiny-monospace-font.html>.

From the above refered web site:

> Brian’s page implies that his font was licensed under the MIT license, so since I did these modifications in my free time, the same license applies here. Feel free to download the BDF file below if you would find this useful, or would like to modify it further for some other nefarious purposes. (Update from 2015: As per comments below, Brian has authorized his font to be released under the CC0 or CC-BY 3.0 license. Therefore, this font may also be used under either CC0 or CC-BY 3.0 license.)
