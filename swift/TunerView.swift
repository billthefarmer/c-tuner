//
//  TunerView.swift
//  Tuner
//
//  Created by Bill Farmer on 08/10/2017.
//  Copyright © 2017 Bill Farmer. All rights reserved.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

import Cocoa

class TunerView: NSView
{
    var rect: NSRect = NSRect()
    var width: CGFloat = 0
    var height: CGFloat = 0

    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Drawing code here.
        rect = DrawEdge(dirtyRect)
        width = NSWidth(rect)
        height = NSHeight(rect)
   }

    func DrawEdge(_ rect: NSRect) -> NSRect
    {
        // Save context
        NSGraphicsContext.saveGraphicsState()

        // Set colour
        NSColor.gray.set()

        // Draw edge
        let path = NSBezierPath(roundedRect: rect, xRadius: 8, yRadius: 8)
        path.lineWidth = 2
        path.stroke()

        // Restore context before clip
        NSGraphicsContext.restoreGraphicsState()

        // Create inset
        let inset = NSInsetRect(rect, 2, 2)
        __NSRectClip(inset)

        return inset
    }

    // keyDown
    func keyDown(event: NSEvent)
    {
        let key = event.characters
        switch key!.lowercased()
        {
        case "d":
            audioData.downsample = !audioData.downsample

        case "f":
            audioData.filter = !audioData.filter

        case "l":
            displayData.lock = !displayData.lock

        case "m":
            displayData.multiple = !displayData.multiple

        case "s":
            strobeData.enable = !strobeData.enable

        case "z":
            spectrumData.zoom = !spectrumData.zoom

        default:
            NSLog("Key %s", key!)
        }
    }
}
