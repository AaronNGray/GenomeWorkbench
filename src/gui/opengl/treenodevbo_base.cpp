/*  $Id: treenodevbo_base.cpp 43891 2019-09-16 13:50:00Z evgeniev $
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
 *
 */

#include <ncbi_pch.hpp>

#include "treenodevbo_base.hpp"

BEGIN_NCBI_SCOPE

void CTreeNodeVboBase::GetVertexBuffer2D(vector<CVect2<float> >& data) const
{
    if (m_NumSegments < 4)
        return;

    float delta = 2*3.141592653589793238463f/m_NumSegments;
    float angle = delta;

    data.push_back(CVect2<float>(-0.5f, 0.5f));

    data.push_back(CVect2<float>(1.0f, 0.0f));
    for (size_t i = 0; i < m_NumSegments - 1; i++) {
        data.push_back(CVect2<float>(cos(angle), sin(angle)));
        angle += delta;
    }
    data.push_back(CVect2<float>(1.0f, 0.0f));
}

END_NCBI_SCOPE
