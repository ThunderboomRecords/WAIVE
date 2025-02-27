#include "DragSource.h"
#import <Cocoa/Cocoa.h>

// Custom class to conform to NSDraggingSource
@interface DraggingSourceView : NSView <NSDraggingSource>
@end

@implementation DraggingSourceView

// Required methods for NSDraggingSource
- (NSDragOperation)draggingSession:(NSDraggingSession *)session sourceOperationMaskForDraggingContext:(NSDraggingContext)context {
    return NSDragOperationCopy;
}

@end

void DragSource::startDraggingFile(const std::string &filePath, void* nativeView) {
    @autoreleasepool {
        NSString *nsFilePath = [NSString stringWithUTF8String:filePath.c_str()];
        NSURL *fileURL = [NSURL fileURLWithPath:nsFilePath];

        if (!fileURL) return;

        NSArray *fileArray = @[fileURL];

        // Set up pasteboard with file URLs
        NSPasteboard *pasteboard = [NSPasteboard pasteboardWithName:NSPasteboardNameDrag];
        [pasteboard clearContents];
        [pasteboard writeObjects:fileArray];

        // Create the drag item
        NSDraggingItem *dragItem = [[NSDraggingItem alloc] initWithPasteboardWriter:fileURL];
        NSSize dragSize = NSMakeSize(32, 32);  // Placeholder drag image size
        [dragItem setDraggingFrame:NSMakeRect(0, 0, dragSize.width, dragSize.height) contents:nil];

        // Convert native view to NSView and ensure it is a DraggingSourceView
        NSView *view = (__bridge NSView*)nativeView;
        DraggingSourceView *dragSourceView = (DraggingSourceView*)view;

        // Start the drag session
        [dragSourceView beginDraggingSessionWithItems:@[dragItem] event:[NSApp currentEvent] source:dragSourceView];
    }
}
