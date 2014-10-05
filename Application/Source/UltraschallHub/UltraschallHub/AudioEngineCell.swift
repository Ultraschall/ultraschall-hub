//
//  AudioEngineCell.swift
//  UltraschallHub
//
//  Created by Daniel Lindenfelser on 05.10.14.
//  Copyright (c) 2014 Daniel Lindenfelser. All rights reserved.
//

import Cocoa

@IBDesignable
class AudioEngineCell: NSTableCellView {
    @IBOutlet weak var statusLabel: NSTextField!
    @IBOutlet weak var numChannelsPopUpButton: NSPopUpButton!
    
    var engine: AudioEngine?
    
    @IBAction func valueChanged(sender: AnyObject) {
        var value = numChannelsPopUpButton.itemTitleAtIndex(numChannelsPopUpButton.indexOfSelectedItem)
        if let number = value.toInt() {
            if (engine != nil) {
                engine!.engineChannels = number
            }
        }
    }
    
    func setAudioEngine(engine :AudioEngine) {
        statusLabel.editable = true
        self.engine = engine
        statusLabel.stringValue = engine.engineDescription
        numChannelsPopUpButton.selectItemWithTitle(String(engine.engineChannels))
    }
}
