//
//  GradiantView.swift
//  UltraschallHub
//
//  Created by Daniel Lindenfelser on 04.10.14.
//  Copyright (c) 2014 Daniel Lindenfelser. All rights reserved.
//

import Cocoa

public class GradiantView: NSView {

    override public func drawRect(dirtyRect: NSRect) {
        super.drawRect(dirtyRect)
        drawBackground(dirtyRect)
    }
    
    private func drawBackground(frame: NSRect) {
        //// Color Declarations
        let gradientColor1 = NSColor(calibratedRed: 0.975, green: 0.975, blue: 0.975, alpha: 1)
        let gradientColor2 = NSColor(calibratedRed: 0.951, green: 0.951, blue: 0.951, alpha: 1)
        let borderColor = NSColor(calibratedRed: 0.569, green: 0.569, blue: 0.569, alpha: 1)
        
        //// Gradient Declarations
        let gradient = NSGradient(startingColor: gradientColor1, endingColor: gradientColor2)
        
        //// Rectangle Drawing
        let rectanglePath = NSBezierPath(rect: NSMakeRect(NSMinX(frame) + 0.5, NSMinY(frame) + 0.5, NSWidth(frame) - 1.0, NSHeight(frame) - 1.0))
        gradient.drawInBezierPath(rectanglePath, angle: -90)
        borderColor.setStroke()
        rectanglePath.lineWidth = 1
        rectanglePath.stroke()
    }

}
