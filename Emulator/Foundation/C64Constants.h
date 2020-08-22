// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _C64_CONSTANTS_H
#define _C64_CONSTANTS_H

#include "Aliases.h"

//
// PAL constants
//

// PAL clock frequency in Hz
static const u32 PAL_CLOCK_FREQUENCY = 985249;

// CPU cycles per second in PAL mode
static const unsigned PAL_CYCLES_PER_SECOND = PAL_CLOCK_FREQUENCY;

// Pixel aspect ratio in PAL mode
static const double PAL_PIXEL_ASPECT_RATIO = 0.9365;


// Horizontal screen parameters

// Width of left VBLANK area in PAL mode
static const u16 PAL_LEFT_VBLANK = 76;

// Width of left border in PAL mode
static const u16 PAL_LEFT_BORDER_WIDTH = 48;

// Width of canvas area in PAL mode
static const u16 PAL_CANVAS_WIDTH = 320;

// Width of right border in PAL mode
static const u16 PAL_RIGHT_BORDER_WIDTH = 37;

// Width of right VBLANK area in PAL mode
static const u16 PAL_RIGHT_VBLANK = 23;

// Total width of a rasterline (including VBLANK) in PAL mode
static const u16 PAL_WIDTH = 504; // 76 + 48 + 320 + 37 + 23

// Number of drawn pixels per rasterline in PAL mode
static const u16 PAL_PIXELS = 405; // 48 + 320 + 37


// Vertical screen parameters

// Number of VBLANK lines at top in PAL mode
static const u16 PAL_UPPER_VBLANK = 16;

// Heigt of upper boder in PAL mode
static const u16 PAL_UPPER_BORDER_HEIGHT = 35;

// Height of canvas area in PAL mode
static const u16 PAL_CANVAS_HEIGHT = 200;

// Lower border height in PAL mode
static const u16 PAL_LOWER_BORDER_HEIGHT = 49;

// Number of VBLANK lines at bottom in PAL mode
static const u16 PAL_LOWER_VBLANK = 12;

// Total height of a frame (including VBLANK) in PAL mode
static const u16 PAL_HEIGHT = 312; // 16 + 35 + 200 + 49 + 12

// Number of drawn rasterlines per frame in PAL mode
static const u16 PAL_RASTERLINES = 284; // 35 + 200 + 49

// Number of viewable rasterlines per frame in PAL mode
static const u16 PAL_VISIBLE_RASTERLINES = 284; // was 292


//
// NTSC constants
//

// NTSC clock frequency in Hz
static const u32 NTSC_CLOCK_FREQUENCY = 1022727;

// CPU cycles per second in NTSC mode
static const unsigned NTSC_CYCLES_PER_SECOND = NTSC_CLOCK_FREQUENCY;

// Pixel aspect ratio in NTSC mode
static const double NTSC_PIXEL_ASPECT_RATIO = 0.75;


// Horizontal screen parameters

// Width of left VBLANK area in NTSC mode
static const u16 NTSC_LEFT_VBLANK = 77;

// Width of left border in NTSC mode
static const u16 NTSC_LEFT_BORDER_WIDTH = 55;

// Width of canvas area in NTSC mode
static const u16 NTSC_CANVAS_WIDTH = 320;

// Width of right border in NTSC mode
static const u16 NTSC_RIGHT_BORDER_WIDTH = 53;

// Width of right VBLANK area in NTSC mode
static const u16 NTSC_RIGHT_VBLANK = 15;

// Total width of a rasterline (including VBLANK) in NTSC mode
static const u16 NTSC_WIDTH = 520; // 77 + 55 + 320 + 53 + 15

// Number of drawn pixels per rasterline in NTSC mode
static const u16 NTSC_PIXELS = 428; // 55 + 320 + 53

// Number of viewable pixels per rasterline in NTSC mode
static const u16 NTSC_VISIBLE_PIXELS = 418;


// Vertical screen parameters

// Number of VBLANK lines at top in NTSC mode
static const u16 NTSC_UPPER_VBLANK = 16;

// Heigt of upper boder in NTSC mode
static const u16 NTSC_UPPER_BORDER_HEIGHT = 10;

// Height of canvas area in NTSC mode
static const u16 NTSC_CANVAS_HEIGHT = 200;

// Lower border height in NTSC mode
static const u16 NTSC_LOWER_BORDER_HEIGHT = 25;

// Number of VBLANK lines at bottom in NTSC mode
static const u16 NTSC_LOWER_VBLANK = 12;

// Total height of a frame (including VBLANK) in NTSC mode
static const u16 NTSC_HEIGHT = 263; // 16 + 10 + 200 + 25 + 12

#endif
