//
//  AudioEngineCell.swift
//  UltraschallHub
//
//  Created by Daniel Lindenfelser on 05.10.14.
//  Copyright (c) 2014 Daniel Lindenfelser. All rights reserved.
//

import Cocoa

@IBDesignable
class AudioEngineCell: NSTableCellView, NSTextFieldDelegate {
    @IBOutlet weak var statusLabel: NSTextField!
    @IBOutlet weak var numChannelsPopUpButton: NSPopUpButton!
    
    var engine: AudioEngine?
    
    override func controlTextDidChange(obj: NSNotification) {
        if (engine != nil) {
            engine!.engineDescription = statusLabel.stringValue
        }
    }
    
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
        statusLabel.delegate = self
    }
}
