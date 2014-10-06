//
//  UltraschallHub.swift
//  UltraschallHub
//
//  Created by Daniel Lindenfelser on 03.10.14.
//

import Cocoa
import PreferencePanes

class UltraschallHub: NSPreferencePane, NSTableViewDataSource, NSTableViewDelegate {
    var audioEngineManager: AudioEngineManager!

    @IBOutlet var window: NSWindow!
    @IBOutlet weak var settingsTable: NSTableView!
    @IBOutlet weak var refreshButton: NSButton!
    @IBOutlet weak var unloadAndLoadButton: NSButton!
    @IBOutlet weak var presetButton: NSButton!
    
    @IBOutlet weak var statusLabel: NSTextField!
    @IBOutlet weak var baseView: NSView!
    
    class func preferencePaneBundle() -> NSBundle! {
        var bundle = NSBundle(path: NSBundle(forClass: self).bundlePath + "/Contents/Resources")
        return bundle
    }
    
    override func mainViewDidLoad() {
        baseView.invalidateIntrinsicContentSize()
        audioEngineManager = AudioEngineManager()
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
        var documentFolderPath = NSSearchPathForDirectoriesInDomains(NSSearchPathDirectory.DocumentDirectory, NSSearchPathDomainMask.UserDomainMask, true).last as? NSURL
        var savePanel = NSSavePanel()
        savePanel.directoryURL = documentFolderPath
        savePanel.allowedFileTypes = [ "preset" ]
        savePanel.allowsOtherFileTypes = true
        savePanel.extensionHidden = true
        savePanel.canCreateDirectories = true
        savePanel.title = "Save preset..."
        var result = savePanel.runModal()
        
        if (result == NSOKButton) {
            var path = savePanel.URL!.path!
            var status = audioEngineManager.savePreset(path)
        }
    }
    
    @IBAction func loadPreset(sender: AnyObject) {
        var documentFolderPath = NSSearchPathForDirectoriesInDomains(NSSearchPathDirectory.DocumentDirectory, NSSearchPathDomainMask.UserDomainMask, true).last as? NSURL
        var openPanel = NSOpenPanel()
        openPanel.directoryURL = documentFolderPath
        openPanel.allowedFileTypes = [ "preset" ]
        openPanel.allowsOtherFileTypes = true
        openPanel.extensionHidden = true
        openPanel.canChooseFiles = true
        openPanel.title = "Load preset..."
        var result = openPanel.runModal()
        if (result == NSOKButton) {
            var path = openPanel.URL!.path!
            var status = audioEngineManager.loadPreset(path)
            settingsTable.reloadData()
        }
    }
    
    @IBAction func loadDriverSettings(sender: AnyObject) {
        audioEngineManager.loadDriverConfiguration()
        settingsTable.reloadData()
    }
    
    @IBAction func presetButtonPressed(sender: AnyObject) {
        var menu = NSMenu(title: "Presets");
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
        menu.popUpMenuPositioningItem(menu.itemAtIndex(0)!, atLocation: location, inView: baseView)
    }
    
    // MARK: - Driver Status
    func refresh() {
        if (DriverManager().isLoaded("fm.ultraschall.driver.UltraschallHub")) {
            unloadAndLoadButton.title = "Unload Driver"
            setStatusText("Loaded")
        } else {
            unloadAndLoadButton.title = "Load Driver"
            setStatusText("Unloaded")
        }
    }
    
    func setStatusText(text: NSString!) {
        statusLabel.stringValue = "Status: " + text
    }
    
    @IBAction func refreshPressed(sender: AnyObject) {
        // TODO: Real Save Configuration Code
        audioEngineManager.saveConfiguration("/Users/danlin/Desktop/test.plist")
        refresh();
    }
        
    @IBAction func loadUnloadPressed(sender: AnyObject) {
        // TODO: Load and Unload Driver Code
    }
}
