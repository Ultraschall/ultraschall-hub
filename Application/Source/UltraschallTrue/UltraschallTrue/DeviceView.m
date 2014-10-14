//
//  DeviceView.m
//  UltraschallTrue
//
//  Created by Daniel Lindenfelser on 07/10/14.
//  Copyright (c) 2014 Daniel Lindenfelser. All rights reserved.
//

#import "DeviceView.h"

const int spaceConnector = 30;
const int connectorHeight = 20;
const int spaceCenter = 10;


const int spaceTop = 25;
const int spaceButtom = 5;
const int spaceHeight = 30;


@implementation Conncetor

@end

@interface DeviceView () //note the empty category name
- (void)drawBackgroundWithFrame: (NSRect)frame titleText: (NSString*)titleText;
- (void)drawOutputConnectorWithFrame: (NSRect)frame connectorText: (NSString*)connectorText;
- (void)drawInputConnectorWithFrame: (NSRect)frame connectorText: (NSString*)connectorText;
- (NSPoint)positionOfConnector:(Conncetor*)connector;

@property int outputWidth;
@property int inputWidth;
@property NSString *titleText;

@property (nonatomic)  NSPoint lastDragLocation;
@end

@implementation DeviceView

- (instancetype)init {
    self = [super init];
    if (self) {
        self.outputs = [[NSMutableArray alloc] init];
        self.inputs = [[NSMutableArray alloc] init];
        self.outputWidth = 50;
        self.inputWidth = 50;
    }
    return self;
}

- (instancetype)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];
    if (self) {
        self.outputs = [[NSMutableArray alloc] init];
        self.inputs = [[NSMutableArray alloc] init];
        self.outputWidth = 50;
        self.inputWidth = 50;
    }
    return self;
}

#pragma mark - Input / Output Control

- (Conncetor *)addOutputWithTitle:(NSString*)title {
    Conncetor* connector = [[Conncetor alloc] init];
    connector.title = title;
    connector.type = kConncetorOutput;
    connector.superview = self;
    return [self addOutput:connector];
}

- (Conncetor *)addOutput:(Conncetor *)connector {
    [self.outputs addObject:connector];
    
    NSDictionary* textFontAttributes = @{NSFontAttributeName: [NSFont fontWithName: @"Helvetica" size: 12]};
    NSSize textSize = [connector.title sizeWithAttributes:textFontAttributes];
    
    textSize.width = textSize.width + spaceConnector;
    
    if (self.outputWidth < textSize.width)
        self.outputWidth = textSize.width;
    
    CGFloat width = self.inputWidth + spaceCenter + self.outputWidth;
    if (self.frame.size.width > width)
        width = self.frame.size.width;
    
    self.frame = CGRectMake(self.frame.origin.x,
                            self.frame.origin.y,
                            width,
                            spaceConnector + (MAX(self.inputs.count, self.outputs.count) * connectorHeight));
    return connector;
}

- (Conncetor *)addInputWithTitle:(NSString*)title {
    Conncetor* connector = [[Conncetor alloc] init];
    connector.title = title;
    connector.type = kConncetorInput;
    connector.superview = self;
    return [self addInput:connector];
}

- (Conncetor *)addInput:(Conncetor *)connector {
    [self.inputs addObject:connector];
    
    NSDictionary* textFontAttributes = @{NSFontAttributeName: [NSFont fontWithName: @"Helvetica" size: 12]};
    NSSize textSize = [connector.title sizeWithAttributes:textFontAttributes];
    
    textSize.width = textSize.width + spaceConnector;
    
    if (self.inputWidth < textSize.width)
        self.inputWidth = textSize.width;
    
    CGFloat width = self.inputWidth + spaceCenter + self.outputWidth;
    if (self.frame.size.width > width)
        width = self.frame.size.width;
    
    self.frame = CGRectMake(self.frame.origin.x,
                            self.frame.origin.y,
                            width,
                            spaceConnector + (MAX(self.inputs.count, self.outputs.count) * connectorHeight));
    return connector;
}

#pragma mark - Drawing

- (void)setTitle:(NSString *)titleText {
    _titleText = titleText;
    NSDictionary* textFontAttributes = @{NSFontAttributeName: [NSFont fontWithName: @"Helvetica-Bold" size: 12]};
    NSSize textSize = [titleText sizeWithAttributes:textFontAttributes];
    if (textSize.width + 40 > self.frame.size.width) {
        self.frame = CGRectMake(self.frame.origin.x,
                                self.frame.origin.y,
                                textSize.width + 40,
                                self.frame.size.height);
    }
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    [self drawBackgroundWithFrame:dirtyRect titleText:self.titleText];
    
    int index = 0;
    for (NSObject *obj in self.inputs) {
        if ([obj class] == [Conncetor class]) {
            index++;
            Conncetor *connector = (Conncetor*)obj;
            NSRect r = CGRectMake(0,
                                  dirtyRect.size.height - (index * connectorHeight) - spaceTop,
                                  self.inputWidth,
                                  connectorHeight);
            [self drawInputConnectorWithFrame:r connectorText:connector.title];
        }
    }
    
    index = 0;
    for (NSObject *obj in self.outputs) {
        if ([obj class] == [Conncetor class]) {
            index++;
            Conncetor *connector = (Conncetor*)obj;
            NSRect r = CGRectMake(dirtyRect.size.width - self.outputWidth,
                                  dirtyRect.size.height - (index * connectorHeight) - spaceTop,
                                  self.outputWidth,
                                  connectorHeight);
            [self drawOutputConnectorWithFrame:r connectorText:connector.title];
        }
    }
}

float treshold(float x,float tr) {
    return (x>0)?((x>tr)?x:tr):-x+tr;
}

-(void)drawLinkFrom:(NSPoint)startPoint to:(NSPoint)endPoint color:(NSColor *)insideColor {
    
    NSPoint p0 = NSMakePoint(startPoint.x,startPoint.y );
    NSPoint p3 = NSMakePoint(endPoint.x,endPoint.y );
    
    NSPoint p1 = NSMakePoint(startPoint.x+treshold((endPoint.x - startPoint.x)/2,50),startPoint.y);
    NSPoint p2 = NSMakePoint(endPoint.x -treshold((endPoint.x - startPoint.x)/2,50),endPoint.y);
    
    NSBezierPath* path = [NSBezierPath bezierPath];
    [path setLineWidth:0];
    [[NSColor grayColor] set];
    [path appendBezierPathWithOvalInRect:NSMakeRect(startPoint.x-2.5,startPoint.y-2.5,5,5)];
    [path fill];
    
    path = [NSBezierPath bezierPath];
    [path setLineWidth:0];
    [insideColor set];
    [path appendBezierPathWithOvalInRect:NSMakeRect(startPoint.x-1.5,startPoint.y-1.5,3,3)];
    [path fill];
    
    path = [NSBezierPath bezierPath];
    [path setLineWidth:0];
    [[NSColor grayColor] set];
    [path appendBezierPathWithOvalInRect:NSMakeRect(endPoint.x-2.5,endPoint.y-2.5,5,5)];
    [path fill];
    
    path = [NSBezierPath bezierPath];
    [path setLineWidth:0];
    [insideColor set];
    [path appendBezierPathWithOvalInRect:NSMakeRect(endPoint.x-1.5,endPoint.y-1.5,3,3)];
    [path fill];
    
    path = [NSBezierPath bezierPath];
    [path setLineWidth:5];
    [path moveToPoint:p0];
    [path curveToPoint:p3 controlPoint1:p1 controlPoint2:p2];
    [[NSColor grayColor] set];
    [path stroke];
    
    
    path = [NSBezierPath bezierPath];
    [path setLineWidth:3];
    [path moveToPoint:p0];
    [path curveToPoint:p3 controlPoint1:p1 controlPoint2:p2];
    [insideColor set];
    [path stroke];
}

- (void)drawLinks:(NSRect)frame {
    for (Conncetor* connector in self.inputs) {
        if (connector.connectedTo != nil) {
            NSPoint end = [self.superview convertPoint:[self positionOfConnector:connector] fromView:self];
            NSPoint start = [self.superview convertPoint:[connector.connectedTo.superview positionOfConnector:connector.connectedTo] fromView:connector.connectedTo.superview];
            [self drawLinkFrom:start to:end color:[NSColor blackColor]];
        }
    }
    
    if (self.dragConnection) {
        if (self.dragConnectionObject != nil) {
            NSColor *color = self.dragValideConnection ? [NSColor redColor] : [NSColor blackColor];
            NSPoint destination = [self.superview convertPoint:self.lastDragLocation fromView:nil];
            if (self.dragConnectionObject.type == kConncetorInput) {
                [self drawLinkFrom:destination to:[self.superview convertPoint:self.dragConnectionStartPoint fromView:self] color:color];
            } else {
                [self drawLinkFrom:[self.superview convertPoint:self.dragConnectionStartPoint fromView:self] to:destination color:color];
            }
        }
    }
}

- (void)debugRect: (NSRect)frame {
    NSBezierPath* rectanglePath = [NSBezierPath bezierPathWithRect: frame];
    [NSColor.magentaColor setFill];
    [rectanglePath fill];
}

- (void)drawBackgroundWithFrame: (NSRect)frame titleText: (NSString*)titleText {
    //// Color Declarations
    NSColor* gradientColor = [NSColor colorWithCalibratedRed: 0 green: 0 blue: 0 alpha: 1];
    NSColor* gradientColor2 = [NSColor colorWithCalibratedRed: 0 green: 0 blue: 0 alpha: 0.7];
    NSColor* color = [NSColor colorWithCalibratedRed: 0.3 green: 0.3 blue: 0.3 alpha: 0.7];
    
    //// Gradient Declarations
    NSGradient* gradient = [NSGradient.alloc initWithStartingColor: gradientColor2 endingColor: gradientColor];
    
    
    //// Subframes
    NSRect titleFrame = NSMakeRect(NSMinX(frame), NSMinY(frame) + NSHeight(frame) - 23, NSWidth(frame), 23);
    
    
    //// Body Drawing
    CGFloat bodyCornerRadius = 8;
    NSRect bodyRect = NSMakeRect(NSMinX(frame), NSMinY(frame) + floor((NSHeight(frame) - 23) * 0.00000 + 0.5), NSWidth(frame), NSHeight(frame) - 23 - floor((NSHeight(frame) - 23) * 0.00000 + 0.5));
    NSRect bodyInnerRect = NSInsetRect(bodyRect, bodyCornerRadius, bodyCornerRadius);
    NSBezierPath* bodyPath = NSBezierPath.bezierPath;
    [bodyPath appendBezierPathWithArcWithCenter: NSMakePoint(NSMinX(bodyInnerRect), NSMinY(bodyInnerRect)) radius: bodyCornerRadius startAngle: 180 endAngle: 270];
    [bodyPath appendBezierPathWithArcWithCenter: NSMakePoint(NSMaxX(bodyInnerRect), NSMinY(bodyInnerRect)) radius: bodyCornerRadius startAngle: 270 endAngle: 360];
    [bodyPath lineToPoint: NSMakePoint(NSMaxX(bodyRect), NSMaxY(bodyRect))];
    [bodyPath lineToPoint: NSMakePoint(NSMinX(bodyRect), NSMaxY(bodyRect))];
    [bodyPath closePath];
    [gradient drawInBezierPath: bodyPath angle: -90];
    
    
    //// Title Box Drawing
    NSRect titleBoxRect = NSMakeRect(NSMinX(titleFrame), NSMinY(titleFrame) + NSHeight(titleFrame) - 23, NSWidth(titleFrame), 23);
    CGFloat titleBoxCornerRadius = 8;
    NSRect titleBoxInnerRect = NSInsetRect(titleBoxRect, titleBoxCornerRadius, titleBoxCornerRadius);
    NSBezierPath* titleBoxPath = NSBezierPath.bezierPath;
    [titleBoxPath moveToPoint: NSMakePoint(NSMinX(titleBoxRect), NSMinY(titleBoxRect))];
    [titleBoxPath lineToPoint: NSMakePoint(NSMaxX(titleBoxRect), NSMinY(titleBoxRect))];
    [titleBoxPath appendBezierPathWithArcWithCenter: NSMakePoint(NSMaxX(titleBoxInnerRect), NSMaxY(titleBoxInnerRect)) radius: titleBoxCornerRadius startAngle: 0 endAngle: 90];
    [titleBoxPath appendBezierPathWithArcWithCenter: NSMakePoint(NSMinX(titleBoxInnerRect), NSMaxY(titleBoxInnerRect)) radius: titleBoxCornerRadius startAngle: 90 endAngle: 180];
    [titleBoxPath closePath];
    [color setFill];
    [titleBoxPath fill];
    NSMutableParagraphStyle* titleBoxStyle = NSMutableParagraphStyle.defaultParagraphStyle.mutableCopy;
    titleBoxStyle.alignment = NSLeftTextAlignment;
    
    NSDictionary* titleBoxFontAttributes = @{NSFontAttributeName: [NSFont fontWithName: @"Helvetica-Bold" size: 12], NSForegroundColorAttributeName: NSColor.whiteColor, NSParagraphStyleAttributeName: titleBoxStyle};
    
    NSRect titleBoxInset = NSInsetRect(titleBoxRect, 10, 0);
    [titleText drawInRect: NSOffsetRect(titleBoxInset, 0, 1 - (NSHeight(titleBoxInset) - NSHeight([titleText boundingRectWithSize: titleBoxInset.size options: NSStringDrawingUsesLineFragmentOrigin attributes: titleBoxFontAttributes])) / 2) withAttributes: titleBoxFontAttributes];
}

- (void)drawOutputConnectorWithFrame: (NSRect)frame connectorText: (NSString*)connectorText {
    //// Color Declarations
    NSColor* color2 = [NSColor colorWithCalibratedRed: 1 green: 1 blue: 1 alpha: 1];
    
    //// Oval Drawing
    NSBezierPath* ovalPath = [NSBezierPath bezierPathWithOvalInRect: NSMakeRect(NSMinX(frame) + NSWidth(frame) - 14.5, NSMinY(frame) + floor((NSHeight(frame) - 8) * 0.54167) + 0.5, 8, 8)];
    [color2 setFill];
    [ovalPath fill];
    [NSColor.blackColor setStroke];
    [ovalPath setLineWidth: 1];
    [ovalPath stroke];
    
    
    //// Text Drawing
    NSRect textRect = NSMakeRect(NSMinX(frame), NSMinY(frame), NSWidth(frame) - 20, NSHeight(frame));
    NSMutableParagraphStyle* textStyle = NSMutableParagraphStyle.defaultParagraphStyle.mutableCopy;
    textStyle.alignment = NSRightTextAlignment;
    
    NSDictionary* textFontAttributes = @{NSFontAttributeName: [NSFont fontWithName: @"Helvetica" size: 12], NSForegroundColorAttributeName: NSColor.whiteColor, NSParagraphStyleAttributeName: textStyle};
    
    [connectorText drawInRect: NSOffsetRect(textRect, 0, 1 - (NSHeight(textRect) - NSHeight([connectorText boundingRectWithSize: textRect.size options: NSStringDrawingUsesLineFragmentOrigin attributes: textFontAttributes])) / 2) withAttributes: textFontAttributes];
}

- (void)drawInputConnectorWithFrame: (NSRect)frame connectorText: (NSString*)connectorText {
    //// Color Declarations
    NSColor* color2 = [NSColor colorWithCalibratedRed: 1 green: 1 blue: 1 alpha: 1];
    
    //// Oval Drawing
    NSBezierPath* ovalPath = [NSBezierPath bezierPathWithOvalInRect: NSMakeRect(NSMinX(frame) + 6.5, NSMinY(frame) + floor((NSHeight(frame) - 8) * 0.54167) + 0.5, 8, 8)];
    [color2 setFill];
    [ovalPath fill];
    [NSColor.blackColor setStroke];
    [ovalPath setLineWidth: 1];
    [ovalPath stroke];
    
    
    //// Text Drawing
    NSRect textRect = NSMakeRect(NSMinX(frame) + 20, NSMinY(frame), floor((NSWidth(frame) - 20) * 1.00000 + 0.5), NSHeight(frame));
    NSMutableParagraphStyle* textStyle = NSMutableParagraphStyle.defaultParagraphStyle.mutableCopy;
    textStyle.alignment = NSLeftTextAlignment;
    
    NSDictionary* textFontAttributes = @{NSFontAttributeName: [NSFont fontWithName: @"Helvetica" size: 12], NSForegroundColorAttributeName: NSColor.whiteColor, NSParagraphStyleAttributeName: textStyle};
    
    [connectorText drawInRect: NSOffsetRect(textRect, 0, 1 - (NSHeight(textRect) - NSHeight([connectorText boundingRectWithSize: textRect.size options: NSStringDrawingUsesLineFragmentOrigin attributes: textFontAttributes])) / 2) withAttributes: textFontAttributes];
}

#pragma mark - Mouse Events and Helper

- (NSPoint)positionOfConnector:(Conncetor*)connector {
    NSRect dirtyRect = self.frame;
    
    int index = 0;
    for (NSObject *obj in self.inputs) {
        if ([obj class] == [Conncetor class]) {
            index++;
            if (connector == (Conncetor*)obj) {
                NSRect frame = CGRectMake(0,
                                          dirtyRect.size.height - (index * connectorHeight) - spaceTop,
                                          self.inputWidth,
                                          connectorHeight);
                return NSMakePoint(NSMinX(frame) + 10.5, NSMinY(frame) + floor((NSHeight(frame) - 4) * 0.54167) + 0.5);
            }
        }
    }
    
    index = 0;
    for (NSObject *obj in self.outputs) {
        if ([obj class] == [Conncetor class]) {
            index++;
            if (connector == (Conncetor*)obj) {
                NSRect frame = CGRectMake(dirtyRect.size.width - self.outputWidth,
                                          dirtyRect.size.height - (index * connectorHeight) - spaceTop,
                                          self.outputWidth,
                                          connectorHeight);
                return NSMakePoint(NSMinX(frame) + NSWidth(frame) - 10.5, NSMinY(frame) + floor((NSHeight(frame) - 4) * 0.54167) + 0.5);
            }
        }
    }
    return NSMakePoint(0, 0);
}

- (ConncetorType)hitConnectorAtPoint: (NSPoint)point {
    point = [self convertPoint:point fromView:self.superview];
    NSRect dirtyRect = self.frame;
   
    for (int index = 1; index <= self.inputs.count; index++) {
        NSRect frame = CGRectMake(0,
                                  dirtyRect.size.height - (index * connectorHeight) - spaceTop,
                                  self.inputWidth,
                                  connectorHeight);
        frame = NSMakeRect(NSMinX(frame) + 6.5,
                           NSMinY(frame) + floor((NSHeight(frame) - 8) * 0.54167) + 0.5,
                           8,
                           8);
        if (NSPointInRect(point, frame))
            return kConncetorInput;
    }
    
    for (int index = 1; index <= self.outputs.count; index++) {
        NSRect frame = CGRectMake(dirtyRect.size.width - self.outputWidth,
                                  dirtyRect.size.height - (index * connectorHeight) - spaceTop,
                                  self.outputWidth,
                                  connectorHeight);
        frame = NSMakeRect(NSMinX(frame) + NSWidth(frame) - 14.5,
                           NSMinY(frame) + floor((NSHeight(frame) - 8) * 0.54167) + 0.5,
                           8,
                           8);
        if (NSPointInRect(point, frame))
            return kConncetorOutput;
    }

    
    return kConncetorNone;
}

- (Conncetor*)connectorAtPoint: (NSPoint)point {
    point = [self convertPoint:point fromView:self.superview];
    NSRect dirtyRect = self.frame;
    int index = 0;
    for (NSObject *obj in self.inputs) {
        if ([obj class] == [Conncetor class]) {
            Conncetor *connector = (Conncetor*)obj;
            index++;
            NSRect frame = CGRectMake(0,
                                      dirtyRect.size.height - (index * connectorHeight) - spaceTop,
                                      self.inputWidth,
                                      connectorHeight);
            frame = NSMakeRect(NSMinX(frame) + 6.5,
                               NSMinY(frame) + floor((NSHeight(frame) - 8) * 0.54167) + 0.5,
                               8,
                               8);
            if (NSPointInRect(point, frame))
            return connector;
        }
    }
    
    index = 0;
    for (NSObject *obj in self.outputs) {
        if ([obj class] == [Conncetor class]) {
            Conncetor *connector = (Conncetor*)obj;
            index++;
            NSRect frame = CGRectMake(dirtyRect.size.width - self.outputWidth,
                                      dirtyRect.size.height - (index * connectorHeight) - spaceTop,
                                      self.outputWidth,
                                      connectorHeight);
            frame = NSMakeRect(NSMinX(frame) + NSWidth(frame) - 14.5,
                               NSMinY(frame) + floor((NSHeight(frame) - 8) * 0.54167) + 0.5,
                               8,
                               8);
            if (NSPointInRect(point, frame))
                return connector;
        }
    }
    
    return nil;
}

- (BOOL)hitTestTitleAtPoint: (NSPoint)point {
    NSRect titleFrame = NSMakeRect(NSMinX(self.frame), NSMinY(self.frame) + NSHeight(self.frame) - 23, NSWidth(self.frame), 23);
    return NSPointInRect(point, titleFrame);
}

- (BOOL)acceptsFirstMouse:(NSEvent *)e {
    return YES;
}

static NSComparisonResult DraggedViewAboveOtherViewsComparator(NSView* view1, NSView* view2, void* context) {
    if ([view1 isKindOfClass:[DeviceView class]]) {
        DeviceView *view = (DeviceView*)view1;
        if (view.drag) {
            return NSOrderedDescending;
        }
    }
    else if ([view2 isKindOfClass:[DeviceView class]]) {
        DeviceView *view = (DeviceView*)view2;
        if (view.drag) {
            return NSOrderedAscending;
        }
    }
    
    return NSOrderedSame;
}

- (void)mouseDown:(NSEvent *)theEvent {
    NSPoint mousePoint = [self.superview convertPoint:[theEvent locationInWindow] fromView:nil];
    if ([self hitTestTitleAtPoint:mousePoint]) {
        self.drag = true;
        [self.superview sortSubviewsUsingFunction:DraggedViewAboveOtherViewsComparator context:nil];
        // Convert to superview's coordinate space
        self.lastDragLocation = [self.superview convertPoint:[theEvent locationInWindow] fromView:nil];
    } if ([self hitConnectorAtPoint:mousePoint]) {
        self.dragConnection = true;
        self.dragConnectionObject = [self connectorAtPoint:mousePoint];
        self.dragConnectionStartPoint = [self positionOfConnector:self.dragConnectionObject];
        self.dragValideConnection = NO;
    }
}

- (void)mouseDragged:(NSEvent *)theEvent {
    if (self.drag) {
        // We're working only in the superview's coordinate space, so we always convert.
        NSPoint newDragLocation = [self.superview convertPoint:[theEvent locationInWindow] fromView:nil];
        NSPoint thisOrigin = [self frame].origin;
        thisOrigin.x += (-self.lastDragLocation.x + newDragLocation.x);
        thisOrigin.y += (-self.lastDragLocation.y + newDragLocation.y);
        [self setFrameOrigin:thisOrigin];
        self.lastDragLocation = newDragLocation;
    } else if (self.dragConnection) {
        NSPoint newDragLocation = [self.superview convertPoint:[theEvent locationInWindow] fromView:nil];
        self.lastDragLocation = newDragLocation;
        self.dragValideConnection = NO;
        for (NSView* view in self.superview.subviews) {
            if ([view class] == [DeviceView class]) {
                DeviceView* deviceView = (DeviceView*)view;
                if (deviceView != self) {
                    if ([deviceView hitConnectorAtPoint:self.lastDragLocation]) {
                        if ([deviceView connectorAtPoint:self.lastDragLocation].type != self.dragConnectionObject.type) {
                            self.dragValideConnection = YES;
                        }
                    }
                }
            }
        }
    }
    [self.superview setNeedsDisplay:YES];
}
         
- (void)mouseUp:(NSEvent *)theEvent {
    if (self.drag) {
        self.drag = false;
    } else if (self.dragConnection) {
        for (NSView* view in self.superview.subviews) {
            if ([view class] == [DeviceView class]) {
                DeviceView* deviceView = (DeviceView*)view;
                if (deviceView != self) {
                    if ([deviceView hitConnectorAtPoint:self.lastDragLocation]) {
                        if ([deviceView connectorAtPoint:self.lastDragLocation].type != self.dragConnectionObject.type) {
                            self.dragConnectionObject.connectedTo = [deviceView connectorAtPoint:self.lastDragLocation];
                            self.dragConnectionObject.connectedTo.connectedTo = self.dragConnectionObject;
                            
                            if (self.dragConnectionObject.type == kConncetorOutput) {
                                if (self.delegate) {
                                    [self.delegate connectInput:self.dragConnectionObject withOutput:self.dragConnectionObject.connectedTo];
                                }
                            } else {
                                if (self.delegate) {
                                    [self.delegate connectInput:self.dragConnectionObject.connectedTo withOutput:self.dragConnectionObject];
                                }
                            }
                        }
                    }
                }
            }
        }
        
        self.dragConnection = false;
        self.dragConnectionObject = nil;
        self.dragConnectionStartPoint = NSMakePoint(0, 0);
        [self.superview setNeedsDisplay:YES];
    }
}


@end
