//
//  UltraschallHub.swift
//  UltraschallHub
//
//  Created by Daniel Lindenfelser on 03.10.14.
//  Copyright (c) 2014 Daniel Lindenfelser. All rights reserved.
//

import Cocoa
import PreferencePanes

class UltraschallHub: NSPreferencePane, NSTableViewDataSource, NSTableViewDelegate {
    let audioEngineManager: AudioEngineManager! = AudioEngineManager()

    @IBOutlet weak var settingsTable: NSTableView!
    
    @IBOutlet weak var statusBubble: StatusBubble!
    
    @IBOutlet weak var refreshButton: NSButton!
    @IBOutlet weak var unloadAndLoadButton: NSButton!
    @IBOutlet weak var presetButton: NSButton!
    
    @IBOutlet weak var statusLabel: NSTextField!
    
    class func preferencePaneBundle() -> NSBundle! {
        var bundle = NSBundle(path: NSBundle(forClass: self).bundlePath + "/Contents/Resources")
        return bundle
    }
    
    override func mainViewDidLoad() {
        mainView.invalidateIntrinsicContentSize()
        refresh()
    }
    
    // MARK: - Table View
    func numberOfRowsInTableView(aTableView: NSTableView!) -> Int
    {
        return audioEngineManager.audioEngines.count
    }
    
    func tableView(tableView: NSTableView, viewForTableColumn tableColumn: NSTableColumn?, row: Int) -> NSView? {
        if var audioEngine = self.audioEngineManager.engineAtIndex(row) {
            if var view = settingsTable.makeViewWithIdentifier("AudioEngineCell", owner: self) as? AudioEngineCell {
                view.setAudioEngine(audioEngine)
                return view
            }
        }
        
        return nil
    }
    
    @IBAction func insertNewRow(sender: AnyObject) {
        audioEngineManager.addEngine(AudioEngine())
        settingsTable.reloadData()
    }
    
    @IBAction func removeSelectedRows(sender: AnyObject) {
        var indexes = settingsTable.selectedRowIndexes
        settingsTable.removeRowsAtIndexes(indexes, withAnimation: NSTableViewAnimationOptions.EffectFade);
        indexes.enumerateIndexesUsingBlock { (index, _) in
            if var audioEngine = self.audioEngineManager.engineAtIndex(index) {
                self.audioEngineManager.removeEngine(audioEngine.engineIdentifier)
            }
        }
    }
    
    // MARK: - Presets
    
    @IBAction func newPreset(sender: AnyObject) {
        audioEngineManager.newPreset()
        settingsTable.reloadData()
    }
    
    @IBAction func savePreset(sender: AnyObject) {
        let savePanel = NSSavePanel()
        savePanel.allowedFileTypes = [ "preset" ]
        savePanel.allowsOtherFileTypes = true
        savePanel.extensionHidden = true
        savePanel.canCreateDirectories = true
        savePanel.title = "Save preset..."
        let result = savePanel.runModal()
        
        if (result == NSOKButton) {
            if let url = savePanel.URL {
                if let path = url.path {
                    // TODO: handle fails
                    var status = audioEngineManager.savePreset(path)
                }
            }
        }
    }
    
    @IBAction func loadPreset(sender: AnyObject) {
        let openPanel = NSOpenPanel()
        openPanel.allowedFileTypes = [ "preset" ]
        openPanel.allowsOtherFileTypes = true
        openPanel.extensionHidden = true
        openPanel.canChooseFiles = true
        openPanel.title = "Load preset..."
        let result = openPanel.runModal()
        if (result == NSOKButton) {
            if let url = openPanel.URL {
                if let path = url.path {
                    // TODO: handle fails
                    var status = audioEngineManager.loadPreset(path)
                    settingsTable.reloadData()
                }
            }
        }
    }
    
    @IBAction func loadDriverSettings(sender: AnyObject) {
        audioEngineManager.loadDriverConfiguration()
        settingsTable.reloadData()
    }
    
    @IBAction func presetButtonPressed(sender: AnyObject) {
        let menu = NSMenu(title: "Presets");
        menu.insertItemWithTitle("Current Settings...", action: "loadDriverSettings:", keyEquivalent: "", atIndex: 0)
        menu.insertItem(NSMenuItem.separatorItem(), atIndex: 1)
        menu.insertItemWithTitle("New Preset...", action: "newPreset:", keyEquivalent: "", atIndex: 2)
        menu.insertItemWithTitle("Save Preset...", action: "savePreset:", keyEquivalent: "", atIndex: 3)
        menu.insertItemWithTitle("Load Preset...", action: "loadPreset:", keyEquivalent: "", atIndex: 4)
        
        for item: AnyObject in menu.itemArray {
            if let menuItem = item as? NSMenuItem {
                menuItem.target = self
            }
        }
        
        var location = presetButton.frame.origin
        menu.popUpMenuPositioningItem(menu.itemAtIndex(0)!, atLocation: location, inView: mainView)
    }
    
    // MARK: - Driver Status
    func refresh() {
        if (DriverManager().isLoaded("fm.ultraschall.driver.UltraschallHub")) {
            self.setDriverLoaded(true);
        } else {

            self.setDriverLoaded(false);
        }
    }
    
    func setDriverLoaded(loaded: Bool!) {
        if (loaded == true) {
            statusLabel.stringValue = "Driver: loaded"
            unloadAndLoadButton.title = "Unload Driver"
            statusBubble.active = true
            statusBubble.needsDisplay = true
        } else {
            statusLabel.stringValue = "Driver: unloaded"
            unloadAndLoadButton.title = "Load Driver"
            statusBubble.active = false
            statusBubble.needsDisplay = true
        }
    }
    
    @IBAction func refreshPressed(sender: AnyObject) {
        // TODO: Real Save Configuration Code
        audioEngineManager.saveConfiguration("/Users/danlin/Desktop/test.plist")
        refresh();
    }
        
    @IBAction func loadUnloadPressed(sender: AnyObject) {
        // TODO: Load and Unload Driver Code
        statusBubble.active = !statusBubble.active;
        setDriverLoaded(statusBubble.active);
    }
}
