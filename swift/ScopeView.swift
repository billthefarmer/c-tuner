
//
//  Scope.swift
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

class ScopeView: TunerView
{
    let kTextSize: CGFloat = 10
    var max: Double = 0

    override func mouseDown(with event: NSEvent)
    {
        if (event.type == .leftMouseDown)
        {
            audioData.filter = !audioData.filter
            needsDisplay = true;
        }
    }

    override func draw(_ dirtyRect: NSRect)
    {
        super.draw(dirtyRect)

        // Drawing code here.
        NSBezierPath.fill(rect)

        // Dark green graticule
        let darkGreen = NSColor(red: 0, green: 0.5, blue: 0, alpha: 1.0)
        darkGreen.set()

        // Move the origin
        let transform = AffineTransform(translationByX: 0, byY: NSMidY(rect))
        (transform as NSAffineTransform).concat()
        NSGraphicsContext.current!.shouldAntialias = false;

        // Draw graticule
        for x in stride(from: NSMinX(rect), to: NSMaxX(rect), by: 6)
        {
            NSBezierPath.strokeLine(from: NSMakePoint(x, NSMaxY(rect) / 2),
                                    to: NSMakePoint(x, -NSMaxY(rect) / 2))
        }

        for y in stride(from: 0, to: NSHeight(rect) / 2, by: 6)
        {
            NSBezierPath.strokeLine(from: NSMakePoint(NSMinX(rect), y),
                                    to: NSMakePoint(NSMaxX(rect), y))
            NSBezierPath.strokeLine(from: NSMakePoint(NSMinX(rect), -y),
                                    to: NSMakePoint(NSMaxX(rect), -y))
        }

        if (scopeData.data == nil)
        {
            return
        }

        // Initialise sync
        var maxdx: Double = 0
        var dx: Double = 0
        var n = 0

        if (width < 1)
        {
            return
        }

        for i in 1 ..< Int(width)
        {
	    dx = scopeData.data[i] - scopeData.data[i - 1]
	    if (maxdx < dx)
	    {
	        maxdx = dx
	        n = i
	    }

	    if (maxdx < 0.0 && dx > 0.0)
            {
	        break
            }
        }

        // Calculate scale
        if (max < 0.125)
        {
	    max = 0.125
        }

        let yscale = max / Double(height / 2)
        max = 0.0

        // Green trace
        NSColor.green.set()
        NSGraphicsContext.current!.shouldAntialias = true;

        // Draw the trace
        let path = NSBezierPath()
        path.move(to: NSZeroPoint)

        for i in 0 ..< Int(width)
        {
	    if (max < abs(scopeData.data[n + i]))
            {
	        max = abs(scopeData.data[n + i])
            }

	    let y = scopeData.data[n + i] / yscale
	    path.line(to: NSMakePoint(NSMinX(rect) + CGFloat(i), CGFloat(y)))
        }

        path.stroke()

        // Show F if filtered
        if (audioData.filter == true)
        {
	    // Select font
            let font = NSFont.systemFont(ofSize: kTextSize)
            let attribs: [NSAttributedStringKey: Any] =
              [.foregroundColor: NSColor.yellow,
               .font: font]
            "F".draw(at: NSMakePoint(NSMinX(rect) + 2, -NSMidY(rect)),
                     withAttributes: attribs)
        }
    }
}
