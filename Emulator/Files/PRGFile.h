// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AnyCollection.h"

class PRGFile : public AnyCollection {

public:
    
    static bool isCompatibleName(const std::string &name);
    static bool isCompatibleStream(std::istream &stream);
    
    static PRGFile *makeWithFileSystem(class FSDevice &fs);

    
    //
    // Initializing
    //
    
    PRGFile() : AnyCollection() { }
    PRGFile(usize capacity) : AnyCollection(capacity) { }
    
    
    //
    // Methods from C64Object
    //
    
    const char *getDescription() const override { return "PRGFile"; }
    
    
    //
    // Methods from AnyFile
    //

    FileType type() const override { return FILETYPE_PRG; }
    
    
    //
    // Methods from AnyCollection
    //

    PETName<16> collectionName() override;
    u64 collectionCount() const override;
    PETName<16> itemName(unsigned nr) const override;
    u64 itemSize(unsigned nr) const override;
    u8 readByte(unsigned nr, u64 pos) const override;
};
