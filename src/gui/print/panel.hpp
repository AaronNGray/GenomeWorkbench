#ifndef GUI_UTILS___PANEL__HPP
#define GUI_UTILS___PANEL__HPP

/*  $Id: panel.hpp 14565 2007-05-18 12:32:01Z dicuccio $
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
 * Authors:  Peter Meric
 *
 * File Description:
 *   CPanel - represents a physical PDF page
 */

#include <corelib/ncbiobj.hpp>
#include <gui/print/print_options.hpp>


BEGIN_NCBI_SCOPE


class CPdfObject;

class CPanel : public CObject
{
public:
    typedef CPrintOptions::TAlignment TAlignment;

public:
    TAlignment m_HAlign;
    TAlignment m_VAlign;

    unsigned int m_Col;
    unsigned int m_Row;
    unsigned int m_PanelNum;

    unsigned int m_HPageOffset;
    unsigned int m_VPageOffset;


    CRef<CPdfObject> m_Panel;
};


END_NCBI_SCOPE


#endif  // GUI_UTILS___PANEL__HPP
