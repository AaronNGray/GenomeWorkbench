/*  $Id: igraph.cpp 23921 2011-06-22 14:36:01Z falkrb $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/opengl.h>

#include <gui/graph/igraph.hpp>

BEGIN_NCBI_SCOPE

IGraph::~IGraph()
{
}

IGraphDataSource::~IGraphDataSource()
{
}

/////////////////////////////////////////////////////////////////////////////
/// CGraphBase

CGraphBase::CGraphBase()
:   m_pDataSource(NULL),
    m_Color(0.5f, 0.5f, 0.5f),
    m_TextColor(0.0f, 0.0f, 0.0f)
{
}

CGraphBase::~CGraphBase()
{
}

bool    CGraphBase::SetDataSource(IGraphDataSource* pDS)
{
    m_pDataSource = pDS;
    CalculateLimits();
    return true;
}

IGraphDataSource*   CGraphBase::GetDataSource()
{
    return m_pDataSource;
}

const TModelRect&    CGraphBase::GetLimits()    const
{
    return m_Limits;
}

void    CGraphBase::Render(CGlPane*, TElemVector*)
{
}

/////////////////////////////////////////////////////////////////////////////
/// class CGraphDotMarker

void    CGraphDotMarker::RenderMarker(TModelUnit cX, TModelUnit cY, TModelUnit MarkerW, TModelUnit MarkerH, EMarkerType Type)
{
    if(Type != eNone) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      TModelUnit Rx = MarkerW / 2;
      TModelUnit Ry = MarkerH / 2;

      switch(Type) {
      case eDiamond: {
          glBegin(GL_QUADS);
              glVertex2d(cX + Rx, cY);
              glVertex2d(cX, cY + Ry);
              glVertex2d(cX - Rx, cY);
              glVertex2d(cX, cY - Ry);
          glEnd();
          break;
      }
      case eRect: {
          glBegin(GL_QUADS);
              glVertex2d(cX + Rx, cY + Ry);
              glVertex2d(cX + Rx, cY - Ry);
              glVertex2d(cX - Rx, cY - Ry);
              glVertex2d(cX - Rx, cY + Ry);
          glEnd();
          break;
      }
      case eTriangle: {
          glBegin(GL_TRIANGLES);
              glVertex2d(cX, cY + Ry);
              glVertex2d(cX + Rx, cY - Ry);
              glVertex2d(cX - Rx, cY - Ry);
          glEnd();
          break;
      }
      case eCross: {
          glBegin(GL_LINES);
              glVertex2d(cX - Rx, cY - Ry);
              glVertex2d(cX + Rx, cY + Ry);
              glVertex2d(cX - Rx, cY + Ry);
              glVertex2d(cX + Rx, cY - Ry);
          glEnd();
          break;
      }
      default: _ASSERT(false);
      }// switch
    }
}

END_NCBI_SCOPE
