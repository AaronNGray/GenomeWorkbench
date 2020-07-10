#ifndef GUI_UTILS__PAGE_HANDLER_HPP
#define GUI_UTILS__PAGE_HANDLER_HPP

/*  $Id: page_handler.hpp 29699 2014-01-30 19:10:19Z falkrb $
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
 *   CPageHandler - manage pages in a PDF document
 */

#include <corelib/ncbiobj.hpp>
#include <gui/print/print_utils.hpp>
#include "page_buffers.hpp"
#include "panel.hpp"


BEGIN_NCBI_SCOPE


class CPdfObject;
class CPrintOptions;
class CPdfObjectWriter;
class CPdfArray;
class CIdGenerator;
class CPageGrid;
class CPanelGrid;
class CPdfFontHandler;


class CPageHandler
{
private:
    typedef CRef<CPdfObject> TPdfObjectRef;
    typedef CUnit::TPdfUnit TPdfUnit;
    typedef pair<TPdfUnit, TPdfUnit> TSize;
    typedef pair<TPdfUnit, TPdfUnit> TOffset;
    typedef CPrintOptions::TAlignment TAlignment;

    enum Side
    {
        eTop,
        eBottom,
        eLeft,
        eRight
    };

public:
    CPageHandler(const CPageBuffers& buffers,
                 CPdfObjectWriter& obj_writer,
                 CIdGenerator* objid_gen,
                 CPdfFontHandler* font_handler
                );
    virtual ~CPageHandler();

    TPdfObjectRef GetObject(void) const;
    void SetOptions(const CPrintOptions& options);
    void AddContent(const TPdfObjectRef& content);\
    void AddAnnot(const TPdfObjectRef& content);
    void Clear();   

    unsigned int WritePages(void);
    void WritePageTree(const CRef<CPdfDictionary>& pagedict);

protected:
    void x_Update(const CRef<CPdfDictionary>& pagedict);
    const TSize x_GetScaledPageSize(void) const;

    void x_UpdatePageTreeRoot(const CRef<CPdfDictionary>& pagedict);
    void x_PrintText(CPdfObject& obj, const CPrintOptions::SText& txt, TPdfUnit yoffset);
    TPdfObjectRef x_CreateHeaderFooter(void);
    TPdfObjectRef x_CreatePrintersMarks(const CRef<CPanel>& panel);
    TPdfObjectRef x_CreatePageClipObject(void);
    TPdfObjectRef x_CreateContentXForm(void);
    TPdfObjectRef x_CreatePageXForm(const CRef<CPanel>& panel);

    void x_CreatePage(TPdfObjectRef page, 
                      CRef<CPdfArray>& contents,
                      CRef<CPdfArray>& annots);

    pair<float, float> x_GetPageScaling(TPdfUnit w,
                                        TPdfUnit h,
                                        bool keepAspect = true
                                       ) const;

    TOffset x_GetPanelOffset(const CRef<CPanel>& panel) const;
    void x_DrawCornerCropMarks(CPdfObject& obj, TPdfUnit margin) const;
    CRef<CPdfArray> x_DrawPanelLink(CPdfObject& obj,
                                    unsigned int panel_num,
                                    TPdfUnit x,
                                    TPdfUnit y,
                                    enum Side side = eTop
                                   ) const;
    void x_DrawTriangle(CPdfObject& obj,
                        TPdfUnit x,
                        TPdfUnit y,
                        TPdfUnit width,
                        TPdfUnit height
                       ) const;

private:
    CPdfObjectWriter* m_ObjWriter;
    CRef<CIdGenerator> m_ObjIdGenerator;
    CRef<CPdfFontHandler> m_FontHandler;

    bool m_Finalized;
    const CPrintOptions* m_Options;

    vector<TPdfObjectRef> m_Pages;
    vector<TPdfObjectRef> m_StdContents;
    TPdfObjectRef m_PageTreeRootObj;
    TPdfObjectRef m_ContentPagesXForm;

    TPdfObjectRef m_Clip;
    vector<TPdfObjectRef> m_ContentVec;
    vector<TPdfObjectRef> m_AnnotVec;
    const CPageBuffers* m_PageBuffers;
    CRef<CPanelGrid> m_PanelGrid;

    // Parameters for panel-links
    TPdfUnit m_PanelLinkTriHeight;
    TPdfUnit m_PanelLinkMargin;
    TPdfUnit m_PanelLinkTextHeight;
};



END_NCBI_SCOPE


#endif // GUI_UTILS__PAGE_HANDLER_HPP
