#ifndef GUI_WIDGETS_SEQ___FLAT_FILE_FEATURE_COLLAPSED__HPP
#define GUI_WIDGETS_SEQ___FLAT_FILE_FEATURE_COLLAPSED__HPP

/*  $Id: flat_file_feature_collapsed.hpp 29707 2014-01-31 16:59:45Z katargir $
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

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <gui/widgets/seq/flat_file_text_item.hpp>

BEGIN_NCBI_SCOPE

class CFlatFileFeatureCollapsed : public CFlatFileTextItem
{
public:
    CFlatFileFeatureCollapsed(CConstRef<objects::IFlatItem>& item)
      : CFlatFileTextItem(item) {}

    // Expand object will handle selection
    virtual bool    IsSelectable() const { return false; }
    virtual int     GetIndent() const { return 5; }
protected:
    virtual void    x_RenderText(CStyledTextOStream& ostream, CTextPanelContext* context) const;
    virtual void    x_CalcLineCount(CTextPanelContext* /*context*/) { m_LineCount = 1; }

    string x_GetText(CTextPanelContext* context) const;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___FLAT_FILE_FEATURE_COLLAPSED__HPP
