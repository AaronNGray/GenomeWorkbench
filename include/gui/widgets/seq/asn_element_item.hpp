#ifndef GUI_WIDGETS_SEQ___ASN_ELEMENT_ITEM__HPP
#define GUI_WIDGETS_SEQ___ASN_ELEMENT_ITEM__HPP

/*  $Id: asn_element_item.hpp 39821 2017-11-08 16:18:05Z katargir $
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

#include <serial/serialdef.hpp>

#include <gui/widgets/text_widget/text_select_item.hpp>

#include <gui/widgets/wx/ibioseq_editor.hpp>


BEGIN_NCBI_SCOPE

class CMemberId;
class CStyledTextOStream;
class CAsnBioContext;

class CAsnElementItem : public CTextSelectItem
{
public:
    friend class CAsnViewContext;

    CAsnElementItem(const CMemberId& memberId,
                    TTypeInfo type,
                    TConstObjectPtr ptr,
                    size_t indent,
                    CAsnBioContext* bioContext) :
                        m_MemberId(memberId),
                        m_Type(type),
                        m_Ptr(ptr),
                        m_Indent(indent),
                        m_TrailComma(true),
                        m_BioContext(bioContext),
                        m_EditFlags() {}

    CAsnElementItem(const CAsnElementItem& element) :
                        m_MemberId(element.m_MemberId),
                        m_Type(element.m_Type),
                        m_Ptr(element.m_Ptr),
                        m_Indent(element.m_Indent),
                        m_TrailComma(element.m_TrailComma),
                        m_BioContext(element.m_BioContext),
                        m_EditFlags(element.m_EditFlags) {}

    virtual int     GetIndent() const;

    void            SetTrailComma(bool fTrailComma) { m_TrailComma = fTrailComma; }
    bool            GetTrailComma() const { return m_TrailComma; }

    TTypeInfo         GetType() const { return m_Type; }
    TConstObjectPtr   GetObjectPtr() const { return m_Ptr; }

    void SetEditFlags(int flags) { m_EditFlags = flags; }
    int  GetEditFlags() const { return m_EditFlags; }

    virtual const   CConstRef<CObject> GetAssosiatedObject() const;

    virtual bool    IsSelectable() const { return (m_EditFlags != 0); }

    static void GetRealObject(TTypeInfo& type, TConstObjectPtr& ptr);
    static void GetSelectionObject(TTypeInfo& type, TConstObjectPtr& ptr);

    CEditObject GetEditObject(objects::CScope& scope);

protected:
    virtual void    x_RenderText(CStyledTextOStream& ostream, CTextPanelContext* context) const;

    virtual void    RenderValue(CStyledTextOStream& ostream, CTextPanelContext* context,
                                TTypeInfo type, TConstObjectPtr ptr) const = 0;

    static string   x_GetMemberId(const CMemberId& memberId);

    static void     x_NormalizeType(TTypeInfo& type, TConstObjectPtr& ptr);
    void            x_RenderComma(CStyledTextOStream& ostream) const;

    const CMemberId&  m_MemberId;
    TTypeInfo         m_Type;
    TConstObjectPtr   m_Ptr;
    size_t            m_Indent;
    bool              m_TrailComma;
    CAsnBioContext*   m_BioContext;

    int               m_EditFlags;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___ASN_ELEMENT_ITEM__HPP
