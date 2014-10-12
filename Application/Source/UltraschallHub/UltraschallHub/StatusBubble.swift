//
//  StatusBubble.swift
//  UltraschallHub
//
//  Created by Daniel Lindenfelser on 12/10/14.
//  Copyright (c) 2014 Daniel Lindenfelser. All rights reserved.
//

import Cocoa

class StatusBubble: NSView {

    var active: Bool = true;
    
    override func drawRect(dirtyRect: NSRect) {
        super.drawRect(dirtyRect)
        var frame = dirtyRect;
        
        if (self.active == true) {
            let greenBorder = NSColor(calibratedRed: 0.135, green: 0.649, blue: 0.155, alpha: 1)
            let fill = NSColor(calibratedRed: 0.16, green: 0.766, blue: 0.195, alpha: 1)
            
            let ovalPath = NSBezierPath(ovalInRect: NSMakeRect(NSMinX(frame) + 4, NSMinY(frame) + 4, NSWidth(frame) - 8, NSHeight(frame) - 8))
            fill.setFill()
            ovalPath.fill()
            greenBorder.setStroke()
            ovalPath.lineWidth = 1
            ovalPath.stroke()
        } else {
            let greyBorder = NSColor(calibratedRed: 0.7, green: 0.7, blue: 0.7, alpha: 1)
            
            let ovalPath = NSBezierPath(ovalInRect: NSMakeRect(NSMinX(frame) + 4, NSMinY(frame) + 4, NSWidth(frame) - 8, NSHeight(frame) - 8))
            greyBorder.setStroke()
            ovalPath.lineWidth = 1
            ovalPath.stroke()
        }
        // Drawing code here.
    }
    
}
