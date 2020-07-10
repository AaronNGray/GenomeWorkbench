#ifndef GUI_WIDGETS_SEQ___FLAT_FILE_HEADER_ITEM__HPP
#define GUI_WIDGETS_SEQ___FLAT_FILE_HEADER_ITEM__HPP

/*  $Id: flat_file_header_item.hpp 35449 2016-05-09 20:51:12Z katargir $
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

#include <objtools/format/context.hpp>
#include <gui/widgets/seq/flat_file_text_item.hpp>

BEGIN_NCBI_SCOPE

//
// CFlatFileHeader
//

class CFlatFileHeader : public CFlatFileTextItem
{
public:
    CFlatFileHeader(const string& text) : CFlatFileTextItem(null), m_Text(text) {}
    string GetTextValue() const { return m_Text; }

protected:
    virtual void    x_RenderText(CStyledTextOStream& ostream, CTextPanelContext* context) const;
    virtual void    x_CalcLineCount(CTextPanelContext* /*context*/) { m_LineCount = 1; }

    string m_Text;
};

class CFlatFileSectionHeader : public CFlatFileHeader
{
public:
    CFlatFileSectionHeader(const string& text, bool nucleotide, objects::CFlatFileContext& ctx)
        : CFlatFileHeader(text), m_Nucleotide(nucleotide), m_Ctx(&ctx) {}
	bool IsNucleotide() const { return m_Nucleotide; }

protected:
	bool   m_Nucleotide;

    // To keep from destruction   
    CRef<objects::CFlatFileContext> m_Ctx;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___FLAT_FILE_HEADER_ITEM__HPP
