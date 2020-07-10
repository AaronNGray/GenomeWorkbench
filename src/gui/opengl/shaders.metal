
/*  $Id: shaders.metal 43575 2019-08-01 17:49:45Z katargir $
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

#include <metal_stdlib>
using namespace metal;

#import "mtl_shader_types.h"

// Vertex shader outputs and fragment shader inputs
typedef struct
{
    // The [[position]] attribute of this member indicates that this value is the clip space
    // position of the vertex when this structure is returned from the vertex function
    float4 clipSpacePosition [[position]];
    
    // Since this member does not have a special attribute, the rasterizer interpolates
    // its value with the values of the other triangle vertices and then passes
    // the interpolated value to the fragment shader for eaxh fragment in the triangle
    float4 color;
    float pointSize[[point_size]];

} RasterizerData;

vertex RasterizerData
vertexShaderPlain(uint vertexID [[vertex_id]],
             constant UniformsPlain& uniforms[[buffer(0)]],
             constant packed_float3* vertices [[buffer(1)]])
{
    RasterizerData out;
    out.clipSpacePosition = uniforms.transfMatrix * float4(vertices[vertexID], 1.0);
    out.color = uniforms.color;
    out.pointSize = uniforms.pointSize;
    return out;
}

vertex RasterizerData
vertexShaderColor(uint vertexID [[vertex_id]],
             constant UniformsColor& uniforms [[buffer(0)]],
             constant packed_float3* vertices [[buffer(1)]],
             constant packed_float4* colors [[buffer(2)]])
{
    RasterizerData out;
    out.clipSpacePosition = uniforms.transfMatrix * float4(vertices[vertexID], 1.0);
    out.color = colors[vertexID];
    out.pointSize = uniforms.pointSize;
    return out;
}

vertex RasterizerData
vertexShaderPlain2D(uint vertexID [[vertex_id]],
             constant UniformsPlain& uniforms[[buffer(0)]],
             constant packed_float2* vertices [[buffer(1)]])
{
    RasterizerData out;
    out.clipSpacePosition = uniforms.transfMatrix * float4(vertices[vertexID], 0.0, 1.0);
    out.color = uniforms.color;
    out.pointSize = uniforms.pointSize;
    return out;
}

vertex RasterizerData
vertexShaderColor2D(uint vertexID [[vertex_id]],
             constant UniformsColor& uniforms [[buffer(0)]],
             constant packed_float2* vertices [[buffer(1)]],
             constant packed_float4* colors [[buffer(2)]])
{
    RasterizerData out;
    out.clipSpacePosition = uniforms.transfMatrix * float4(vertices[vertexID], 0.0, 1.0);
    out.color = colors[vertexID];
    out.pointSize = uniforms.pointSize;
    return out;
}

vertex RasterizerData
vertexShaderColor2DUC(uint vertexID [[vertex_id]],
             constant UniformsColor& uniforms [[buffer(0)]],
             constant packed_float2* vertices [[buffer(1)]],
             constant packed_uchar4* colors [[buffer(2)]])
{
    RasterizerData out;
    out.clipSpacePosition = uniforms.transfMatrix * float4(vertices[vertexID], 0.0, 1.0);
#if __METAL_VERSION__ >= 120
    out.color = static_cast<float4>(colors[vertexID])/255.f;
#else
    for (int i = 0;  i < 4;  ++i) {
        out.color[i] = colors[vertexID][i]/255.f;
    }
#endif
    out.pointSize = uniforms.pointSize;
    return out;
}

// Fragment function
fragment float4 fragmentShader(RasterizerData in [[stage_in]])
{
    // We return the color we just set which will be written to our color attachment.
    return in.color;
}

struct TexturedOutVertex
{
    float4 position [[position]];
    float2 texCoords;
    float4 color;
};

vertex TexturedOutVertex
vertexShaderTexture(uint vertexID [[vertex_id]],
             constant UniformsPlain& uniforms [[buffer(0)]],
             constant packed_float3* vertices [[buffer(1)]],
             constant packed_float2* texCoords [[buffer(2)]])
{
    TexturedOutVertex out;
    out.position = uniforms.transfMatrix * float4(vertices[vertexID], 1.0);
    out.texCoords = texCoords[vertexID];
    out.color = uniforms.color;
    return out;
}

fragment half4 fragmentShaderTexture(
    TexturedOutVertex vert [[stage_in]],
    texture2d<float, access::sample> tex[[texture(0)]],
    sampler samplr [[sampler(0)]])
{
    float4 c = vert.color*tex.sample(samplr, vert.texCoords);
    return half4(c.r, c.g, c.b, c.a);
}

struct TexturedAlphaOutVertex
{
    float4 position [[position]];
    float2 texCoords;
    float4 color;
    int    textureEnvMode;
};

vertex TexturedAlphaOutVertex
vertexShaderTextureAlpha(uint vertexID [[vertex_id]],
             constant UniformsPlain& uniforms [[buffer(0)]],
             constant packed_float3* vertices [[buffer(1)]],
             constant packed_float2* texCoords [[buffer(2)]])
{
    TexturedAlphaOutVertex out;
    out.position = uniforms.transfMatrix * float4(vertices[vertexID], 1.0);
    out.texCoords = texCoords[vertexID];
    out.color = uniforms.color;
    out.textureEnvMode = uniforms.textureEnvMode;
    return out;
}

fragment half4 fragmentShaderTextureAlpha(
    TexturedAlphaOutVertex vert [[stage_in]],
    texture2d<float, access::sample> tex[[texture(0)]],
    sampler samplr [[sampler(0)]])
{
    float4 c = tex.sample(samplr, vert.texCoords);
    if (vert.textureEnvMode)
        c.a *= vert.color.a;
    return half4(vert.color.r, vert.color.g, vert.color.b, c.a);
}

///
/// kPipelineStateLine
///

struct LineOutVertex
{
    float4 position [[position]];
    uchar4 color;
};

vertex LineOutVertex
vertexShaderLine(uint vertexID [[vertex_id]],
             constant UniformsLine& uniforms [[buffer(0)]],
             constant LineVertexIn* vertices [[buffer(1)]])
{
    LineOutVertex out;
    float4 tangent = uniforms.mvMatrix*float4(vertices[vertexID].tangent, 0, 0);
    float2 delta = uniforms.lineWidth*0.5f*normalize(float2(-tangent.y, tangent.x));
    float4 pos = uniforms.mvMatrix*float4(vertices[vertexID].position, 0, 1);
    out.position = uniforms.prMatrix*(pos + float4(delta, 0, 0));
    out.color = vertices[vertexID].color;
    return out;
}

fragment half4 fragmentShaderLine(LineOutVertex in [[stage_in]])
{
    return half4(in.color)/255.f;
}

///
/// kPipelineStateLineSmooth
///

struct LineSmoothOutVertex
{
    float4 position [[position]];
    float2 delta;
    uchar4 color;
};

constant float feather = 1;

vertex LineSmoothOutVertex
vertexShaderLineSmooth(uint vertexID [[vertex_id]],
             constant UniformsLine& uniforms [[buffer(0)]],
             constant LineVertexIn* vertices [[buffer(1)]])
{
    LineSmoothOutVertex out;
    float4 tangent = uniforms.mvMatrix*float4(vertices[vertexID].tangent, 0, 0);
    float2 delta = (uniforms.lineWidth*0.5f + feather)*normalize(float2(-tangent.y, tangent.x));
    float4 pos = uniforms.mvMatrix*float4(vertices[vertexID].position, 0, 1);
    out.position = uniforms.prMatrix*(pos + float4(delta, 0, 0));
    out.color = vertices[vertexID].color;
    out.delta = delta;
    return out;
}

fragment half4 fragmentShaderLineSmooth(
        LineSmoothOutVertex in [[stage_in]],
        constant UniformsLine& uniforms [[buffer(0)]])
{

    half d = length(in.delta) - uniforms.lineWidth*.5f + feather;
    half a = in.color.a/255.f;

    if (d > 2*feather)
        a = 0;
    else if (d > 0)
        a *=  1 - d/(2*feather);
        
    return half4(half3(in.color.rgb)/255.f, a);
}

///
/// kPipelineStateLineStipple
///

struct LineStippleOutVertex
{
    float4 position [[position]];
    float distance;
    uchar4 color;
};

vertex LineStippleOutVertex
vertexShaderLineStipple(uint vertexID [[vertex_id]],
             constant UniformsLine& uniforms [[buffer(0)]],
             constant LineStippleVertexIn* vertices [[buffer(1)]])
{
    LineStippleOutVertex out;
    float4 tangent = uniforms.mvMatrix*float4(vertices[vertexID].tangent, 0, 0);
    float2 delta = uniforms.lineWidth*0.5f*normalize(float2(-tangent.y, tangent.x));
    float4 pos = uniforms.mvMatrix*float4(vertices[vertexID].position, 0, 1);
    out.position = uniforms.prMatrix*(pos + float4(delta, 0, 0));
    out.color = vertices[vertexID].color;
    out.distance = vertices[vertexID].distance;;
    return out;
}

fragment half4 fragmentShaderLineStipple(
        LineStippleOutVertex in [[stage_in]],
        constant UniformsStippleLine& uniforms [[buffer(0)]])
{
    unsigned int v = uniforms.pattern&(1<<(0xF&(unsigned int)(in.distance/uniforms.factor)));
    half a = v ? in.color.a/255.f : 0.f;
    return half4(half3(in.color.rgb)/255.f, a);
}

vertex TexturedOutVertex
vertexShader2DTexture1D(uint vertexID [[vertex_id]],
             constant UniformsPlain& uniforms [[buffer(0)]],
             constant packed_float2* vertices [[buffer(1)]],
             constant float* texCoords [[buffer(2)]])
{
    TexturedOutVertex out;
    out.position = uniforms.transfMatrix * float4(vertices[vertexID], 0.0, 1.0);
    out.texCoords = float2(texCoords[vertexID], 0.0);
    return out;
}

vertex RasterizerData
vertexShaderTreeNode(uint vertexID [[vertex_id]],
             constant UniformsPlain& uniforms [[buffer(0)]],
             constant packed_float2* vertices [[buffer(1)]],
             constant packed_float4* colors [[buffer(2)]])
{
    RasterizerData out;
    out.clipSpacePosition = uniforms.transfMatrix * float4(vertices[vertexID], 0.0, 1.0);
    float4 color = uniforms.color, vcolor = colors[vertexID];
    out.color = color*(1 - vcolor.a) + float4(1,1,1,color.a)*vcolor.a;
    out.pointSize = uniforms.pointSize;
    return out;
}

typedef struct
{
    float4 clipSpacePosition [[position]];
    float4 color;
    float2 screenPosition;
} VertexOutStippledPolygon;

vertex VertexOutStippledPolygon
vertexShaderStippledPolygon(uint vertexID [[vertex_id]],
             constant UniformsStippledPolygon& uniforms [[buffer(0)]],
             constant VertexInStippledPolygon* vertices [[buffer(1)]])
{
    VertexOutStippledPolygon out;
    out.color = vertices[vertexID].color;
    float4 screenPos = uniforms.mvMatrix*float4(vertices[vertexID].position, 0, 1);
    out.screenPosition = screenPos.xy;
    out.clipSpacePosition = uniforms.prMatrix*screenPos;
    return out;
}

fragment half4 fragmentShaderStippledPolygon(
             VertexOutStippledPolygon in [[stage_in]],
             constant unsigned char* mask [[buffer(0)]])
{
    unsigned int x = 0x1F&(unsigned int)in.screenPosition.x;
    unsigned int y = 0x1F&(unsigned int)in.screenPosition.y;
    int a = 1&(mask[y*4 + (x>>3)] >> (7&~x));
    return half4(half3(in.color.rgb), in.color.a*a);
}
