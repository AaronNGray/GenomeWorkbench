/*  $Id: page_handler.cpp 43850 2019-09-10 15:02:04Z evgeniev $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software / database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software / database is freely available
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


#include <ncbi_pch.hpp>
#include "page_handler.hpp"
#include <gui/print/print_options.hpp>
#include <gui/print/print_utils.hpp>
#include <gui/utils/id_generator.hpp>
#include <gui/print/pdf_object.hpp>
#include "pdf_object_writer.hpp"
#include <gui/print/pdf_font_handler.hpp>
#include <gui/print/pdf_element.hpp>
#include "panel_grid.hpp"


BEGIN_NCBI_SCOPE



CPageHandler::CPageHandler(const CPageBuffers& buffers,
                           CPdfObjectWriter& obj_writer,
                           CIdGenerator* objid_gen,
                           CPdfFontHandler* font_handler
                          )
    : m_ObjWriter(&obj_writer),
      m_ObjIdGenerator(objid_gen),
      m_FontHandler(font_handler),
      m_PageTreeRootObj(new CPdfObject(m_ObjIdGenerator->NextId())),
      m_PageBuffers(&buffers),        
      m_PanelLinkTriHeight(6),
      m_PanelLinkMargin(1),
      m_PanelLinkTextHeight(8)
{
}


CPageHandler::~CPageHandler()
{
}


CPageHandler::TPdfObjectRef CPageHandler::GetObject(void) const
{
    return m_PageTreeRootObj;
}


void CPageHandler::SetOptions(const CPrintOptions& options)
{
    m_Options = &options;

    const unsigned int pg_wide = m_Options->GetPagesWide();
    const unsigned int pg_tall = m_Options->GetPagesTall();
    m_PanelGrid.Reset(new CPanelGrid(m_ObjIdGenerator, pg_wide, pg_tall));
}


void CPageHandler::x_Update(const CRef<CPdfDictionary>& pagedict)
{
    x_UpdatePageTreeRoot(pagedict);
}


void CPageHandler::x_UpdatePageTreeRoot(const CRef<CPdfDictionary>& pagedict)
{
    CPdfObject& root = *m_PageTreeRootObj;

    root["Type"] = new CPdfName("Pages");
    root["Count"] = new CPdfNumber(int(m_Pages.size()));
    root["Rotate"] = new CPdfNumber(int(m_Options->GetPageOrientation()));

    CRef<CPdfArray> kids(new CPdfArray());
    ITERATE(vector<TPdfObjectRef>, it, m_Pages) {
        kids->GetArray().push_back(CRef<CPdfElement>(new CPdfIndirectObj(*it)));
    }
    root["Kids"] = kids;

    CRef<CPdfArray> mediabox(new CPdfArray());
    CPdfArray::TArray& mbarr = mediabox->GetArray();
    mbarr.push_back(CRef<CPdfElement>(new CPdfNumber(int(0))));
    mbarr.push_back(CRef<CPdfElement>(new CPdfNumber(int(0))));
    mbarr.push_back(CRef<CPdfElement>(new CPdfNumber(int(m_Options->GetMediaWidth()))));
    mbarr.push_back(CRef<CPdfElement>(new CPdfNumber(int(m_Options->GetMediaHeight()))));
    root["MediaBox"] = mediabox;

    /*
       root  << "     %/TrimBox [ "
       << xs.first << ' ' << ys.first << ' '
       << xs.second << ' ' << ys.second << " ]" << endl;
       */

    CRef<CPdfArray> procset(new CPdfArray());
    CPdfArray::TArray& pset = procset->GetArray();
    pset.push_back(CRef<CPdfElement>(new CPdfElement("/PDF")));
    pset.push_back(CRef<CPdfElement>(new CPdfElement("/Text")));
    //res["ProcSet"] = procset;

    CPdfDictionary& dict = root.GetDictionary();
    dict.Add(pagedict);
    CRef<CPdfElement>& res = dict["Resources"];
    if ( !res ) {
        CRef<CPdfDictionary> resources(new CPdfDictionary());
        (*resources)["ProcSet"] = procset;
        res = resources;
    } else {
        CRef<CPdfDictionary> _res(dynamic_cast < CPdfDictionary*>(res.GetPointer()));
        (*_res)["ProcSet"] = procset;
    }
}


void CPageHandler::x_PrintText(CPdfObject& /*obj*/, const CPrintOptions::SText& /*txt*/, TPdfUnit /*yoffset*/)
{
    /*
    obj << "BT" << pdfbrk;
    const TPdfUnit font_size = (TPdfUnit)txt.font_size;
    const string fontname = m_FontHandler->GetFontName(txt.font);
    obj << CPdfName(fontname) << " " << font_size << " Tf" << pdfbrk;
    const TPdfUnit str_w = font_size * txt.text.length();
    obj << (m_Options->GetMediaWidth() - str_w) / 2 << ' '
        << yoffset << " Td" << pdfbrk;
    obj << CPdfString(txt.text) << "Tj" << pdfbrk;
    obj << "ET" << pdfbrk;
    */
}


CPageHandler::TPdfObjectRef CPageHandler::x_CreateHeaderFooter(void)
{
    TPdfObjectRef hf_obj;
    const CPrintOptions::SText& header = m_Options->GetHeaderDetails();
    const CPrintOptions::SText& footer = m_Options->GetFooterDetails();
    const bool has_header = header.text.length() > 0;
    const bool has_footer = footer.text.length() > 0;

    if (has_header || has_footer) {
        hf_obj.Reset(new CPdfObject(m_ObjIdGenerator->NextId()));
        CPdfObject& obj = *hf_obj;
        obj << "0 0 m n" << pdfbrk;
        obj << "0 0 0 rg" << pdfbrk;

        if (has_header) {
            x_PrintText(obj, header, TPdfUnit(m_Options->GetMediaHeight() - m_Options->GetHeaderOffset() - header.font_size));
        }

        if (has_footer) {
            x_PrintText(obj, footer, m_Options->GetFooterOffset());
        }
    }

    return hf_obj;
}


CPageHandler::TPdfObjectRef CPageHandler::x_CreatePrintersMarks(const CRef<CPanel>& panel)
{
    TPdfObjectRef pmarks_obj(new CPdfObject(m_ObjIdGenerator->NextId()));
    CPdfObject& pmarks = *pmarks_obj;

    // create printer's marks, page alignment etc
    pmarks << "0 0 m n" << pdfbrk;
    pmarks << "0 0 0 rg" << pdfbrk;
    pmarks << "BT" << pdfbrk;
    const string fontname = m_FontHandler->GetFontName(CGlTextureFont::eFontFace_Helvetica);
    pmarks << CPdfName(fontname) << ' ' << CPdfNumber(8) << " Tf" << pdfbrk;

    //
    // output current panel number
    //
    const string pstr("Panel " + NStr::IntToString(panel->m_PanelNum)
                      + " of " + NStr::IntToString(m_Options->GetNumPages())
                     );
    const string::size_type pstr_len(pstr.length() * 8);
    pmarks << CPdfNumber(m_Options->GetMediaWidth() - pstr_len - 30) << ' '
           << CPdfNumber(m_Options->GetMediaHeight() - 20) << " Td" << pdfbrk;
    pmarks << CPdfString(pstr) << "Tj" << pdfbrk;
    pmarks << "ET" << pdfbrk;

    //
    // corner crop marks
    //
    x_DrawCornerCropMarks(pmarks, 8);

    //
    // print neighbour pointers (links)
    //
    const CPanelGrid::SNeighbors n = m_PanelGrid->GetNeighbours(panel->m_Col, panel->m_Row);
    TPdfObjectRef panelref = panel->m_Panel;
    CPdfObject& panelobj = *panelref;

    CRef<CPdfArray> annots(new CPdfArray());
    
    TPdfUnit link_height = m_PanelLinkTriHeight + m_PanelLinkMargin*2 + m_PanelLinkTextHeight;
    const TPdfUnit link_margin = link_height + 2;

    // draw top
    if (n.top) {
        const TPdfUnit x = m_Options->GetMediaWidth() / 2;
        const TPdfUnit y = m_Options->GetMediaHeight() - link_margin;
        annots->Add(x_DrawPanelLink(pmarks, n.top->m_PanelNum, x, y));
    }

    // draw bottom
    if (n.bottom) {
        const TPdfUnit x = m_Options->GetMediaWidth() / 2;
        const TPdfUnit y = link_margin - 10;
        annots->Add(x_DrawPanelLink(pmarks, n.bottom->m_PanelNum, x, y, eBottom));
    }

    // draw left
    if (n.left) {
        const TPdfUnit x = link_margin;
        const TPdfUnit y = m_Options->GetMediaHeight() / 2;
        annots->Add(x_DrawPanelLink(pmarks, n.left->m_PanelNum, x, y, eLeft));
    }

    // draw right
    if (n.right) {
        const TPdfUnit x = m_Options->GetMediaWidth() - link_margin;
        const TPdfUnit y = m_Options->GetMediaHeight() / 2;
        annots->Add(x_DrawPanelLink(pmarks, n.right->m_PanelNum, x, y, eRight));
    }

    if (annots->GetArray().size() > 0) {
        panelobj["Annots"] = annots;
    }

    return pmarks_obj;
}


void CPageHandler::x_DrawCornerCropMarks(CPdfObject& obj, TPdfUnit margin) const
{
    const TPdfUnit cmark_len = 20;
    const TPdfUnit cm_l = m_Options->GetMarginLeft();
    const TPdfUnit cm_lm = cm_l - margin;
    const TPdfUnit cm_t = m_Options->GetMediaHeight() - m_Options->GetMarginTop();
    const TPdfUnit cm_tm = cm_t + margin;
    const TPdfUnit cm_r = m_Options->GetMediaWidth() - m_Options->GetMarginRight();
    const TPdfUnit cm_rm = cm_r + margin;
    const TPdfUnit cm_b = m_Options->GetMarginBottom();
    const TPdfUnit cm_bm = cm_b - margin;

    // top left
    obj << cm_lm - cmark_len << ' ' << cm_t << " m "
        << cm_lm << ' ' << cm_t << " l s" << pdfbrk;

    obj << cm_l << ' ' << cm_tm + cmark_len << " m "
        << cm_l << ' ' << cm_tm << " l s" << pdfbrk;

    // top right
    obj << cm_rm + cmark_len << ' ' << cm_t << " m "
        << cm_rm << ' ' << cm_t << " l s" << pdfbrk;

    obj << cm_r << ' ' << cm_tm + cmark_len << " m "
        << cm_r << ' ' << cm_tm << " l s" << pdfbrk;

    // bottom left
    obj << cm_lm - cmark_len << ' ' << cm_b << " m "
        << cm_lm << ' ' << cm_b << " l s" << pdfbrk;

    obj << cm_l << ' ' << cm_bm - cmark_len << " m "
        << cm_l << ' ' << cm_bm << " l s" << pdfbrk;

    // bottom right
    obj << cm_rm + cmark_len << ' ' << cm_b << " m "
        << cm_rm << ' ' << cm_b << " l s" << pdfbrk;

    obj << cm_r << ' ' << cm_bm - cmark_len << " m "
        << cm_r << ' ' << cm_bm << " l s" << pdfbrk;
}


CRef<CPdfArray> CPageHandler::x_DrawPanelLink(CPdfObject& obj,
                                              unsigned int panel_num,
                                              TPdfUnit x,
                                              TPdfUnit y,
                                              enum Side side
                                             ) const
{
    const TPdfUnit link_width = 44;
    const TPdfUnit link_width2 = link_width / 2;
    const TPdfUnit fnt_h = 8;

    TPdfUnit xlate_x = 0;
    TPdfUnit xlate_y = 0;
    int rot = 0;

    switch (side) {
    case (eTop):
    case (eBottom):
        rot = (side == eTop) ? 0 : 180;
        xlate_x = x - link_width2;
        xlate_y = y;
        break;
    case (eLeft):
        rot = 90;
        xlate_x = x;
        xlate_y = y - link_width2;
        break;
    case (eRight):
        rot = 270;
        xlate_x = x;
        xlate_y = y + link_width2;
        break;
    }

    obj << CPdfElement("q") << pdfbrk;
    obj << CPdfTranslate(xlate_x, xlate_y) << pdfbrk;

    if (side == eLeft || side == eRight) {
        obj << CPdfRotate(rot) << pdfbrk;
        rot = 0; // don't rotate again
    }

    obj << "BT" << pdfbrk;
    const string txt("Panel " + NStr::IntToString(panel_num));
    obj << "4 1 Td" << pdfbrk;
    obj << CPdfString(txt) << "Tj" << pdfbrk;
    obj << "ET" << pdfbrk;

    if (rot != 0) {
        obj << CPdfRotate(rot) << pdfbrk;
    }

    if (side == eBottom) {
        // if upside down, then offset the triangles according to the text
        obj << CPdfTranslate(-int(link_width), -int(fnt_h)) << pdfbrk;
    }

    obj << CPdfElement("0.416 0.549 0.792 rg") << pdfbrk;
    const TPdfUnit _margin = m_PanelLinkMargin;
    const TPdfUnit tri_w = m_PanelLinkTriHeight*2;
    const TPdfUnit tri_h = m_PanelLinkTriHeight;
    x_DrawTriangle(obj, link_width / 2, fnt_h + _margin, tri_w, tri_h);

    CRef<CPdfArray> annots(new CPdfArray());
    CRef<CPdfDictionary> link_annot(new CPdfDictionary());
    CPdfDictionary& link = *link_annot;
    link["Type"] = new CPdfName("Annot");
    link["Subtype"] = new CPdfName("Link");

    string _x1, _x2, _y1, _y2;
    switch (side) {
    case (eTop):
    case (eBottom):
        _x1 = NStr::IntToString(int(x - link_width2 - _margin));
        _y1 = NStr::IntToString(int(y - _margin));
        _x2 = NStr::IntToString(int(x + link_width2 + _margin));
        _y2 = NStr::IntToString(int(y + fnt_h + _margin));
        break;
    case (eLeft):
    case (eRight):
        {
            const bool left = (side == eLeft);
            _x1 = NStr::IntToString(int(x + (left ? 1 : -1) * _margin));
            _y1 = NStr::IntToString(int(y + link_width2 + _margin));
            _x2 = NStr::IntToString(int(x + (left ? -1 : 1) * (fnt_h + _margin)));
            _y2 = NStr::IntToString(int(y - link_width2 - _margin));
        }
        break;
    }
    link["Rect"] = new CPdfElement("[" + _x1 + " " + _y1 + " " + _x2 + " " + _y2 + "]");
    link["Border"] = new CPdfElement("[3 3 2]");
    link["C"] = new CPdfElement("[0.416 0.549 0.792]");
    link["H"] = new CPdfName("P");
    CRef<CPdfArray> dest(new CPdfArray());
    const CRef<CPanel> p = m_PanelGrid->GetPanel(panel_num);
    dest->GetArray().push_back(CRef<CPdfElement>(new CPdfIndirectObj(p->m_Panel)));
    dest->GetArray().push_back(CRef<CPdfElement>(new CPdfName("Fit")));
    link["Dest"] = dest;
    annots->GetArray().push_back(CRef<CPdfElement>(link_annot));

    obj << CPdfElement("Q") << pdfbrk;

    return annots;
}


void CPageHandler::x_DrawTriangle(CPdfObject& obj,
                                  TPdfUnit x,
                                  TPdfUnit y,
                                  TPdfUnit width,
                                  TPdfUnit height
                                 ) const
{
    const TPdfUnit w = width / 2;
    obj << CPdfNumber(x - w) << ' ' << CPdfNumber(y) << " m ";
    obj << CPdfNumber(x) << ' ' << CPdfNumber(y + int(height)) << " l ";
    obj << CPdfNumber(x + w) << ' ' << CPdfNumber(y) << " l h f" << pdfbrk;
}


CPageHandler::TPdfObjectRef CPageHandler::x_CreatePageClipObject(void)
{
    TPdfObjectRef clip_obj(new CPdfObject(m_ObjIdGenerator->NextId()));
    CPdfObject& clip = *clip_obj;

    // margin between actual margin and printing marks
    const TPdfUnit pmm = 1;

    const TPdfUnit L = m_Options->GetMarginLeft() - pmm;
    const TPdfUnit B = m_Options->GetMarginBottom() - pmm;
    const TPdfUnit R = m_Options->GetMediaWidth() - m_Options->GetMarginLeft() + pmm;
    const TPdfUnit T = m_Options->GetMediaHeight() - m_Options->GetMarginTop() + pmm;
    clip << L << ' ' << B << " m ";
    clip << R << ' ' << B << " l ";
    clip << R << ' ' << T << " l ";
    clip << L << ' ' << T << " l W n" << pdfbrk;

    return clip_obj;
}


CPageHandler::TPdfObjectRef CPageHandler::x_CreateContentXForm(void)
{
    TPdfObjectRef xform_obj(new CPdfObject(m_ObjIdGenerator->NextId()));
    CPdfObject& xform = *xform_obj;

    //
    // scale and translate the coordinate system based on the size of
    // the page content
    //
    const CPageBuffers::TOffset docsize = m_PageBuffers->GetPageSize();
    const pair<float, float> scale = x_GetPageScaling((TPdfUnit)docsize.first, 
                                                      (TPdfUnit)docsize.second);
    xform << scale.first << " 0 0 " << scale.second << " 0 0 cm" << pdfbrk;

    const CBBox<3>& bbox = m_PageBuffers->GetBoundingBox();
    const float x_off = bbox.GetNthRange(0).first;
    const float y_off = bbox.GetNthRange(1).first;
    xform << "1 0 0 1 " << -x_off << ' ' << -y_off << " cm" << pdfbrk;

    return xform_obj;
}


CPageHandler::TPdfObjectRef CPageHandler::x_CreatePageXForm(const CRef<CPanel>& panel)
{
    // translate page according to offset
    TPdfObjectRef xform_obj(new CPdfObject(m_ObjIdGenerator->NextId()));
    CPdfObject& xform = *xform_obj;

    const TOffset offset = x_GetPanelOffset(panel);
    xform << "1 0 0 1 " << -offset.first << ' ' << -offset.second << " cm" << pdfbrk;

    return xform_obj;
}


//
// GetPanelOffset() - given dimensions in PDF (default user space)
//                    coordinates, return page offsets in that coordinate
//                    system given horizontal and vertical alignments.
//
// The horizontal/vertical factors specify the number of pages
// by which to offset this panel
//
CPageHandler::TOffset CPageHandler::x_GetPanelOffset(const CRef<CPanel>& panel) const
{
//    const TAlignment& halign = panel->m_HAlign;
//    const TAlignment& valign = panel->m_VAlign;
//    const unsigned int pg_wide = m_Options->GetPagesWide();
//    const unsigned int pg_tall = m_Options->GetPagesTall();
    const TPdfUnit page_w = m_Options->GetPageTrimWidth();
    const TPdfUnit page_h = m_Options->GetPageTrimHeight();

    TPdfUnit h_offset = page_w * panel->m_HPageOffset;   
    TPdfUnit v_offset = page_h * panel->m_VPageOffset;

    // Doesn't take alighment into account or take advantage of blank space...
    return make_pair(h_offset, v_offset);

    /*

    TSize docsize = x_GetScaledPageSize();

    const TPdfUnit fullpg_wide = pg_wide < 3 ? 0.0f : (TPdfUnit)(pg_wide - 2);
    const TPdfUnit fullpg_w = fullpg_wide * page_w;
    if (halign == CPrintOptions::eCenter) {
        docsize.first = page_w;
    }
    else {
        docsize.first = (docsize.first - fullpg_w) / 2;
    }

    const TPdfUnit fullpg_tall = pg_tall < 3 ? 0.0f : (TPdfUnit)(pg_tall - 2);
    const TPdfUnit fullpg_h = fullpg_tall * page_h;
    if (valign == CPrintOptions::eMiddle) {
        docsize.second = page_h;
    }
    else {
        docsize.second = (docsize.second - fullpg_h) / 2;
    }

    //int h_offset = page_w * -panel->m_HPageOffset + m_Options->GetMarginLeft();
    //int v_offset = page_h * -panel->m_VPageOffset + m_Options->GetMarginBottom();
    TPdfUnit h_offset = -fullpg_w + m_Options->GetMarginLeft();
    TPdfUnit v_offset = -fullpg_h + m_Options->GetMarginBottom();

    switch (halign) {
    case CPrintOptions::eLeft:
        //h_offset += docsize.first - page_w;
        break;

    case CPrintOptions::eCenter:
        h_offset += (page_w - docsize.first) / 2;
        break;

    case CPrintOptions::eRight:
        h_offset += page_w - docsize.first;
        break;

    default:
        break;
    }

    switch (valign) {
    case CPrintOptions::eTop:
        v_offset += page_h - docsize.second;
        break;

    case CPrintOptions::eMiddle:
        v_offset += (page_h - docsize.second) / 2;
        break;

    case CPrintOptions::eBottom:
        //v_offset += docsize.second - page_h;
        break;

    default:
        break;
    }

    return make_pair(h_offset, v_offset);
    */
}


//
// x_GetPageScaling() - scale the dimensions in the user's coordinate system to
//                      dimensions in PDF (default user space)
//
pair<float, float> CPageHandler::x_GetPageScaling(TPdfUnit w,
                                                  TPdfUnit h,
                                                  bool keepAspect
                                                 ) const
{
    const TPdfUnit page_w = m_Options->GetPageTrimWidth();
    const TPdfUnit page_h = m_Options->GetPageTrimHeight();

    const TPdfUnit total_w = page_w * m_Options->GetPagesWide();
    const TPdfUnit total_h = page_h * m_Options->GetPagesTall();

    const float scale_w = float(total_w) / w;
    const float scale_h = float(total_h) / h;

    if (keepAspect) {
        const float scale = scale_w < scale_h ? scale_w : scale_h;
        return make_pair(scale, scale);
    }

    return make_pair(scale_w, scale_h);
}


void CPageHandler::AddContent(const TPdfObjectRef& content)
{
    m_ContentVec.push_back(content);
}

void CPageHandler::AddAnnot(const TPdfObjectRef& content)
{
    m_AnnotVec.push_back(content);
}

void CPageHandler::Clear()
{
    m_ContentVec.clear();
    m_AnnotVec.clear();
}

void CPageHandler::WritePageTree(const CRef<CPdfDictionary>& pagedict)
{
    const_cast<CPageHandler*>(this)->x_Update(pagedict);
    m_ObjWriter->WriteObject(m_PageTreeRootObj);
}


const CPageHandler::TSize CPageHandler::x_GetScaledPageSize(void) const
{
    const CPageBuffers::TOffset sz = m_PageBuffers->GetPageSize();
    const pair<float, float> scale(x_GetPageScaling((TPdfUnit)sz.first, 
                                                    (TPdfUnit)sz.second));
    return TSize(scale.first * sz.first,
                 scale.second * sz.second
                );
}


unsigned int CPageHandler::WritePages(void)
{
    //
    // write the content object
    //
    //m_ObjWriter->WriteObject(m_Content);
    ITERATE(vector<TPdfObjectRef>, it, m_ContentVec) {
        m_ObjWriter->WriteObject(*it);
    }

    ITERATE(vector<TPdfObjectRef>, it, m_AnnotVec) {
        m_ObjWriter->WriteObject(*it);
    }

    //
    // write the header/footer object
    //
    TPdfObjectRef headfoot_obj = x_CreateHeaderFooter();
    if (headfoot_obj.GetPointer()) {
        m_StdContents.push_back(headfoot_obj);
        m_ObjWriter->WriteObject(headfoot_obj);
    }

    //
    // create the clip/crop object
    //
    m_Clip = x_CreatePageClipObject();
    m_ObjWriter->WriteObject(m_Clip);

    //
    // create the content transform object
    //
    //m_ContentPagesXForm = x_CreateContentXForm();
    //m_ObjWriter->WriteObject(m_ContentPagesXForm);

    //
    // generate and output the content pages
    //
    unsigned int panel_num = 1;
    const unsigned int pg_wide = m_Options->GetPagesWide();
    const unsigned int pg_tall = m_Options->GetPagesTall();

    for (unsigned int row = 0; row < pg_tall; ++row) {
        //const TAlignment valign = m_PanelGrid->GetVAlign(row);

        for (unsigned int col = 0; col < pg_wide; ++col, ++panel_num) {
            //const TAlignment halign = m_PanelGrid->GetHAlign(col);

            const CRef<CPanel> panel = m_PanelGrid->GetPanel(col, row);

            //
            // create printer's marks
            //
            //TPdfObjectRef printers_marks = x_CreatePrintersMarks(panel);
            //m_ObjWriter->WriteObject(printers_marks);

            //
            // offset the page (alignment)
            //
            //TPdfObjectRef page_xform = x_CreatePageXForm(panel);
            //m_ObjWriter->WriteObject(page_xform);

            //
            // set the page contents
            //
            CRef<CPdfArray> contents(new CPdfArray());
            CPdfArray::TArray& carr = contents->GetArray();
            ITERATE(vector<TPdfObjectRef>, it, m_StdContents) {
                carr.push_back(CRef<CPdfElement>(new CPdfIndirectObj(*it)));
            }
            //carr.push_back(CRef<CPdfElement>(new CPdfIndirectObj(printers_marks)));
            carr.push_back(CRef<CPdfElement>(new CPdfIndirectObj(m_Clip)));
            //carr.push_back(CRef<CPdfElement>(new CPdfIndirectObj(page_xform)));
            //carr.push_back(CRef<CPdfElement>(new CPdfIndirectObj(m_ContentPagesXForm)));
            ITERATE(vector<TPdfObjectRef>, it, m_ContentVec) {               
                carr.push_back(CRef<CPdfElement>(new CPdfIndirectObj(*it)));
            }
            //carr.push_back(CRef<CPdfElement>(new CPdfIndirectObj(m_Content)));
            
            CRef<CPdfArray> annotations(new CPdfArray());
            CPdfArray::TArray& aarr = annotations->GetArray();
            ITERATE(vector<TPdfObjectRef>, it, m_AnnotVec) {               
                aarr.push_back(CRef<CPdfElement>(new CPdfIndirectObj(*it)));
            }
            
            x_CreatePage(panel->m_Panel, contents, annotations);
        }
    }

    return m_PanelGrid->GetNumPanels();
}


void CPageHandler::x_CreatePage(TPdfObjectRef page, 
                                CRef<CPdfArray>& contents,
                                CRef<CPdfArray>& annots)
{
    // create page object
    CPdfObject& pageobj = *page;
    m_Pages.push_back(page);

    pageobj["UserUnit"] = new CPdfNumber(m_Options->GetUserUnit());
    pageobj["Type"] = new CPdfName("Page");
    pageobj["Parent"] = new CPdfIndirectObj(m_PageTreeRootObj);
    pageobj["Contents"] = contents;

    if (annots->GetArray().size() > 0)
        pageobj["Annots"] = annots;

    //
    // output the page
    //
    m_ObjWriter->WriteObject(page);
}


END_NCBI_SCOPE
