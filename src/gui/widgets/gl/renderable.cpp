/*  $Id: renderable.cpp 14565 2007-05-18 12:32:01Z dicuccio $
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
#include <algorithm>

#include <gui/widgets/gl/irenderable.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CRenderableImpl
CRenderableImpl::CRenderableImpl()
:   m_Visible(true)
{
}


CRenderableImpl::~CRenderableImpl()
{
}


TVPRect CRenderableImpl::GetVPRect() const
{
    return m_VPRect;
}


void CRenderableImpl::SetVPRect(const TVPRect& rc)
{
    m_VPRect = rc;
}


TModelRect CRenderableImpl::GetModelRect() const
{
    return m_ModelRect;
}


void CRenderableImpl::SetModelRect(const TModelRect& rc)
{
    m_ModelRect = rc;
}


TVPPoint CRenderableImpl::PreferredSize()
{
    return TVPPoint(m_VPRect.Width(), m_VPRect.Height()); // trivial default implementation
}


bool CRenderableImpl::IsVisible()
{
    return m_Visible;
}


void CRenderableImpl::SetVisible(bool set)
{
    m_Visible = set;
}


bool CRenderableImpl::NeedTooltip(CGlPane& /*pane*/, TVPUnit vp_x, TVPUnit vp_y)
{
    _VERIFY(m_VPRect.PtInRect(vp_x, vp_y));
    return false;
}


string CRenderableImpl::GetTooltip()
{
    return "Error";
}


END_NCBI_SCOPE
