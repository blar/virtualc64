/*!
 * @header      C64.h
 * @brief       This file is part of VirtualC64.
 * @author      Dirk W. Hoffmann, www.dirkwhoffmann.de
 * @copyright   2018 Dirk W. Hoffmann
 */
/*              This program is free software; you can redistribute it and/or modify
 *              it under the terms of the GNU General Public License as published by
 *              the Free Software Foundation; either version 2 of the License, or
 *              (at your option) any later version.
 *
 *              This program is distributed in the hope that it will be useful,
 *              but WITHOUT ANY WARRANTY; without even the implied warranty of
 *              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *              GNU General Public License for more details.
 *
 *              You should have received a copy of the GNU General Public License
 *              along with this program; if not, write to the Free Software
 *              Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef CONFIGURE_H
#define CONFIGURE_H

// Snapshot version number of this release
#define V_MAJOR 3
#define V_MINOR 1
#define V_SUBMINOR 0

// Disable assertion checking (Uncomment in release build)
// #define NDEBUG

// Default debug level for all components (Set to 1 in release build)
#define DEBUG_LEVEL 1

#endif 



// RELEASE NOTES (Version 3.2)
//
// Added support for cartridges of type Kingsoft, StarDos, and Atomic Power.
// The Rom dialog has been reworked and can be opend manually from the hardware preferences.
// CRT monitor emulation has been improved. Furthermore, the emulator preferences window has been redesign to control the video options more easily. 
// Fixed a bug in the VIC implementation that caused too many rasterline IRQs to be triggered under some circumstances.
// Fixed a bug in the VIC implementation that caused too many sprite-sprite and sprite-background IRQs to be triggered.
// Improved the emulation of VICII's sprite shift register.
// Fixed multiple issues in the sprite debug panel.
//
// TODOs for the next release:
//
//
// Map right Mac shift key to right Commodore key
//
//
// CLEANUP:
// Don't use mount() for inserting disks. Use insertDisk instead
// Replace Mouse* C64::mouse by (enum) MouseModel C64::mouse
//
// OPTIMIZATION:
// Update IEC bus inside CIA and VIA. Use delay flags if neccessary
// Use a simpler implementation for the raster irq trigger. Edge sensitive matching value
// Call CA1 action in VIA class only if the pin value really has changed.
//
// Add setter API for SID stuff
//

