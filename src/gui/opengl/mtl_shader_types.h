#ifndef GUI_WIDGETS_WX__MTL_SHADER_TYPES_HPP
#define GUI_WIDGETS_WX__MTL_SHADER_TYPES_HPP

/*  $Id: mtl_shader_types.h 43575 2019-08-01 17:49:45Z katargir $
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

// Buffer index values chared between shader and C code to ensure Metal shader buffer inputs match
//   Metal API buffer set calls

typedef struct
{
    simd::float4x4 transfMatrix;
    simd::float4   color;
    float          pointSize;
    int            textureEnvMode;
} UniformsPlain;

typedef struct
{
    simd::float4x4 transfMatrix;
    float          pointSize;
} UniformsColor;

typedef struct
{
    simd::float2 position;
    simd::float2 tangent;
    simd::uchar4 color;
} LineVertexIn;

typedef struct
{
    simd::float4x4 mvMatrix;
    simd::float4x4 prMatrix;
    float lineWidth;
} UniformsLine;

typedef struct
{
    simd::float2 position;
    simd::float4 color;
} VertexInStippledPolygon;

typedef struct
{
    simd::float4x4 mvMatrix;
    simd::float4x4 prMatrix;
} UniformsStippledPolygon;

///
/// kPipelineStateLineStipple
///

typedef struct
{
    short factor;
    short pattern;
} UniformsStippleLine;

typedef struct
{
    simd::float2 position;
    simd::float2 tangent;
    float        distance;
    simd::uchar4 color;
} LineStippleVertexIn;

#endif  //  GUI_WIDGETS_WX__MTL_SHADER_TYPES_HPP
