/*  $Id: mtlcanvas.cpp 44588 2020-01-27 21:00:35Z katargir $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Roman Katargin
 *
 * File Description:
 */

#include <ncbi_pch.hpp>

#ifdef __WXOSX_COCOA__

#import <QuartzCore/CAMetalLayer.h>

#include <gui/widgets/wx/mtlcanvas.hpp>
#include <gui/opengl/mtldata.hpp>

#include <wx/log.h> 
#include <wx/osx/private.h>

@interface wxMetalView : NSView<CALayerDelegate>
@end

const MTLPixelFormat kColorPixelFormat =  MTLPixelFormatBGRA8Unorm;

@implementation wxMetalView

+ (void)initialize
{
    static BOOL initialized = NO;
    if (!initialized)
    {
        initialized = YES;
        wxOSXCocoaClassAddWXMethods( self );
    }
}

- (id)initWithFrame: (NSRect)frame
{
    if (!(self = [super initWithFrame:frame]))
        return self;

    self.wantsLayer = YES;
    self.layerContentsRedrawPolicy = NSViewLayerContentsRedrawDuringViewResize;

    return self;
}

- (CALayer*)makeBackingLayer
{
    CAMetalLayer* metalLayer = [CAMetalLayer layer];
    metalLayer.device = ncbi::Metal().GetMTLDevice();
    metalLayer.delegate = self;
    metalLayer.pixelFormat = kColorPixelFormat;
    metalLayer.autoresizingMask = kCALayerHeightSizable | kCALayerWidthSizable;
    metalLayer.needsDisplayOnBoundsChange = YES;
    return metalLayer;
}

- (CAMetalLayer*)metalLayer
{
    return (CAMetalLayer*)self.layer;
}

- (void)setFrameSize:(NSSize)newSize
{
    [super setFrameSize:newSize];
    self.metalLayer.drawableSize = newSize;
}

- (void)viewDidChangeBackingProperties
{
    [super viewDidChangeBackingProperties];
    self.layer.contentsScale = [[self window] backingScaleFactor];
}

- (void)displayLayer:(CALayer*)layer
{
    wxWidgetCocoaImpl* viewimpl = (wxWidgetCocoaImpl*) wxWidgetImpl::FindFromWXWidget( self );
    if (viewimpl) {
        ncbi::CMTLCanvas* wnd = (ncbi::CMTLCanvas*)viewimpl->GetWXPeer();
        if (wnd)
            wnd->Redraw();
    }
}

@end

BEGIN_NCBI_SCOPE

class wxMetalCocoaImpl : public wxWidgetCocoaImpl
{
public:
    wxMetalCocoaImpl(wxWindowMac* wxpeerr, wxMetalView *v);

    static wxWidgetImplType* CreateMetal(
        wxWindowMac* wxpeer,
        wxWindowMac* parent,
        wxWindowID id,
        const wxPoint& pos,
        const wxSize& size,
        long style,
        long extraStyle);
};

wxMetalCocoaImpl::wxMetalCocoaImpl(wxWindowMac *wxpeer, wxMetalView *v)
: wxWidgetCocoaImpl(wxpeer, v, Widget_UserKeyEvents)
{
}

wxWidgetImplType* wxMetalCocoaImpl::CreateMetal(
        wxWindowMac* wxpeer,
        wxWindowMac* WXUNUSED(parent),
        wxWindowID WXUNUSED(id),
        const wxPoint& pos,
        const wxSize& size,
        long WXUNUSED(style),
        long WXUNUSED(extraStyle))

{
    if (!Metal().InitPipelineStates(kColorPixelFormat))
        return nullptr;
    
    NSRect r = wxOSXGetFrameForControl( wxpeer, pos, size );
    wxMetalView* view = [[wxMetalView alloc] initWithFrame:r];
    
    return new wxMetalCocoaImpl( wxpeer, view );
}

wxIMPLEMENT_CLASS(CMTLCanvas, wxWindow);

int CMTLCanvas::m_InstCount = 0;

CMTLCanvas::CMTLCanvas(wxWindow *parent,
                       wxWindowID winId,
                       const wxPoint& pos,
                       const wxSize& size,
                       long style,
                       const wxString& name)
{
    ++m_InstCount;
    Create(parent, winId, pos, size, style, name); 

    m_ClearColor[0] = 1;
    m_ClearColor[1] = 1;
    m_ClearColor[2] = 1;
    m_ClearColor[3] = 0;
}

void CMTLCanvas::SetClearColor(float r, float g, float b, float a)
{
    m_ClearColor[0] = r;
    m_ClearColor[1] = g;
    m_ClearColor[2] = b;
    m_ClearColor[3] = a;
}

CMTLCanvas::~CMTLCanvas()
{
    if (--m_InstCount == 0)
        Metal().DeletePipelineStates();
}

bool CMTLCanvas::Create(wxWindow *parent,
                        wxWindowID winId,
                        const wxPoint& pos,
                        const wxSize& size,
                        long style,
                        const wxString& name)
{
    DontCreatePeer();

    if (!wxWindow::Create(parent, winId, pos, size, style, name))
        return false;

    wxOSXWidgetImpl* const peer = wxMetalCocoaImpl::CreateMetal(this, parent, winId, pos, size, style, GetExtraStyle());
    if (!peer)
        return false;

    SetPeer(peer);

    return true;
}

void CMTLCanvas::Redraw()
{
    @autoreleasepool {
        wxMetalView* view = (wxMetalView*)GetHandle();

        // Create a new command buffer for each render pass to the current drawable
        id<MTLCommandBuffer> commandBuffer = [Metal().GetCommandQueue() commandBuffer];
        commandBuffer.label = @"MyCommand";

        id<CAMetalDrawable> drawable = view.metalLayer.nextDrawable;
        if (drawable)
        {
            id<MTLTexture> texture = drawable.texture;
            MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];

            renderPassDescriptor.colorAttachments[0].texture = texture;
            renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
            renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], m_ClearColor[3]);
            // Create a render command encoder so we can render into something
            id<MTLRenderCommandEncoder> renderEncoder =
                [commandBuffer renderCommandEncoderWithDescriptor: renderPassDescriptor];
            renderEncoder.label = @"MyRenderEncoder";

            [renderEncoder setRenderPipelineState: Metal().GetPipelineState(0)];

            wxSize clientSize = GetClientSize();
            Metal().SetClientSize({ (uint)clientSize.x, (uint)clientSize.y });
            Metal().SetEncoder(renderEncoder);
            x_Render();
            Metal().SetEncoder(nil);

            [renderEncoder endEncoding];

            // Schedule a present once the framebuffer is complete using the current drawable
            [commandBuffer presentDrawable:drawable];
        }

        // Finalize rendering here & push the command buffer to the GPU
        [commandBuffer commit];
    }
}

END_NCBI_SCOPE

#endif // __WXOSX_COCOA__

