// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AnyFile.h"

class TAPFile : public AnyFile {
 
public:

    static bool isCompatibleName(const std::string &name);
    static bool isCompatibleStream(std::istream &stream);
    
    
    //
    // Initializing
    //
    
public:

    TAPFile() : AnyFile() { }
    const char *getDescription() override { return "TAPFile"; }

    
    //
    // Methods from AnyFile
    //
    
    FileType type() override { return FILETYPE_TAP; }
    PETName<16> getName() override;
    
    
    //
    // Retrieving tape information
    //
    
    // Returns the TAP version (0 = original layout, 1 = updated layout)
    TAPVersion version() { return (TAPVersion)data[0x000C]; }
    
    // Returns the beginning of the data area
    u8 *getData() { return data + 0x14; }
    
    // Returns the size of the data area in bytes
    size_t getDataSize() { return size - 0x14; }
    
    
    //
    // Repairing
    //
    
public:
    
    void repair() override;
};
