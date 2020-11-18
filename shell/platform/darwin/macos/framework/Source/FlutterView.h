// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterResizableBackingStoreProvider.h"

/**
 * Listener for view resizing.
 */
@protocol FlutterViewReshapeListener <NSObject>
/**
 * Called when the view's backing store changes size.
 */
- (void)viewDidReshape:(nonnull NSView*)view;
@end

/**
 * View capable of acting as a rendering target and input source for the Flutter
 * engine.
 */
@interface FlutterView : NSView

#ifdef SHELL_ENABLE_METAL
- (instancetype)initWithMTLDevice:(id<MTLDevice>)device
                     commandQueue:(id<MTLCommandQueue>)commandQueue
                  reshapeListener:(id<FlutterViewReshapeListener>)reshapeListener
    NS_DESIGNATED_INITIALIZER;
#endif

- (nullable instancetype)initWithFrame:(NSRect)frame
                           mainContext:(nonnull NSOpenGLContext*)mainContext
                       reshapeListener:(nonnull id<FlutterViewReshapeListener>)reshapeListener
    NS_DESIGNATED_INITIALIZER;

- (nullable instancetype)initWithMainContext:(nonnull NSOpenGLContext*)mainContext
                             reshapeListener:
                                 (nonnull id<FlutterViewReshapeListener>)reshapeListener;

- (nullable instancetype)initWithFrame:(NSRect)frameRect
                           pixelFormat:(nullable NSOpenGLPixelFormat*)format NS_UNAVAILABLE;
- (nonnull instancetype)initWithFrame:(NSRect)frameRect NS_UNAVAILABLE;
- (nullable instancetype)initWithCoder:(nonnull NSCoder*)coder NS_UNAVAILABLE;
- (nonnull instancetype)init NS_UNAVAILABLE;

/**
 * Flushes the OpenGL context and flips the surfaces. Expected to be called on raster thread.
 */
- (void)present;

/**
 * Ensures that a backing store with requested size exists and returns the descriptor. Expected to
 * be called on raster thread.
 */
- (nonnull FlutterBackingStoreDescriptor*)backingStoreForSize:(CGSize)size;

@end
