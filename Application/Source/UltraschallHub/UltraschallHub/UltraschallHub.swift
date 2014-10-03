//
//  UltraschallHub.swift
//  UltraschallHub
//
//  Created by Daniel Lindenfelser on 03.10.14.
//  Copyright (c) 2014 Daniel Lindenfelser. All rights reserved.
//

import Cocoa
import PreferencePanes
import SecurityInterface


class UltraschallHub: NSPreferencePane {
    @IBOutlet weak var refreshButton: NSButton!
    @IBOutlet weak var unloadAndLoadButton: NSButton!
    
    @IBOutlet weak var statusLabel: NSTextField!
    @IBOutlet weak var baseView: NSView!

    override func mainViewDidLoad() {
        
    }
    
    func setStatusText(text: NSString!) {
        statusLabel.stringValue = "Status: " + text;
    }
    
    @IBAction func refreshPressed(sender: AnyObject) {
        reloadDriver();
    }
    
    func isDriverLoaded() -> Bool! {
        return true;
    }
    
    @IBAction func loadUnloadPressed(sender: AnyObject) {
        
    }
    
    func unloadDriver() {
    }
    
    func loadDriver() {
    }
    
    func reloadDriver() {
        unloadDriver();
        loadDriver();
    }
}
