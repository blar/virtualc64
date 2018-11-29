//
// This file is part of VirtualC64 - A user-friendly Commodore 64 emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
//

import Foundation

class ExportScreenshotController : UserDialogController {
    
    static let PNG = 0
    static let JPG = 1
    static let TIFF = 2
    
    var type = 0
    var savePanel: NSSavePanel!
    
    override func showSheet(withParent: MyController, completionHandler:(() -> Void)? = nil) {
        
        parent = withParent
        parentWindow = parent.window
        c64 = parent.mydocument.c64
        
        // Create save panel
        savePanel = NSSavePanel()
        savePanel.allowedFileTypes = ["png"]
        savePanel.prompt = "Export"
        savePanel.title = "Export"
        savePanel.nameFieldLabel = "Export As:"
        savePanel.accessoryView = window?.contentView
        
        // Run panel as sheet
        savePanel.beginSheetModal(for: parent.window!, completionHandler: { result in
            if result == .OK {
                self.export()
            }
        })
    }
    
    @discardableResult func export() -> Bool {
        
        let image = parent.metalScreen.screenshot()
        var data: Data?
        
        print("targetFormat = \(type)")
        
        switch type {
        case ExportScreenshotController.PNG:
            track("Exporting to PNG")
            data = image?.pngRepresentation
            break;
            
        case ExportScreenshotController.JPG:
            track("Exporting to JPG")
            data = image?.jpgRepresentation
            break;

        case ExportScreenshotController.TIFF:
            track("Exporting to TIFF")
            data = image?.tiffRepresentation
            break;

        default:
            track("Unknown format")
            break;
        }
        
        // Get URL from panel
        guard let url = savePanel.url else {
            return false
        }
        
        // Export
        track("Exporting to file \(url)")
        do {
            try data?.write(to: url, options: .atomic)
            return true
        } catch {
            track("Cannot export screenshot")
            return false
        }
    }
    
    @IBAction func selectPNG(_ sender: Any!) {
        track()
        savePanel.allowedFileTypes = ["png"]
        type = ExportScreenshotController.PNG
    }
    
    @IBAction func selectJPG(_ sender: Any!) {
        track()
        savePanel.allowedFileTypes = ["jpg"]
        type = ExportScreenshotController.JPG
    }
    
    @IBAction func selectTIFF(_ sender: Any!) {
        track()
        savePanel.allowedFileTypes = ["tiff"]
        type = ExportScreenshotController.TIFF
    }
}
