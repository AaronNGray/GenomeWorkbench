#ifndef GUI_WIDGETS_WX___PRINT_HANDLER_WX__HPP
#define GUI_WIDGETS_WX___PRINT_HANDLER_WX__HPP

/*  $Id: print_handler_wx.hpp 42756 2019-04-10 16:43:22Z katargir $
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
 * Authors: 
 *
 * File Description:
 *   
 */

#include <corelib/ncbistd.hpp>

#include <iostream>
#include "wx/wx.h"
#include "wx/print.h"
#include "wx/printdlg.h"
#include <cmath>



#include <corelib/ncbiobj.hpp>
#include <util/image/image.hpp>

#include <gui/widgets/wx/print_setup_dlg_wx.hpp>

BEGIN_NCBI_SCOPE

 
static const int brush_size = 3;
 
/**
  * Shows a basic example of how to print stuff in wx.
  */
class CPrintHanderWx : public wxPrintout
{
    wxPageSetupDialogData m_page_setup;
    
    /** the type of paper (letter, A4, etc...) */
    wxPaperSize m_paper_type;
    
    /** horizontal or vertical */
#if(wxMAJOR_VERSION==2 && wxMINOR_VERSION<9)
    int m_orient;
#else
    // Or, if you use wxWidgets 2.9+ :
    wxPrintOrientation m_orient;
#endif
 
    /** number of pages we want to print. here it's static, but in a real example you will often
      * want to calculate dynamically once you know the size of the printable area from page setup
      */
    int m_page_amount;
        
    /** margins, in millimeters */
    int m_margin_left, m_margin_right, m_margin_top, m_margin_bottom;
    
    /** we'll use this to determine the coordinate system; it describes the number of units per
      * centimeter (i.e. how fine the coordinate system is)
      */
    float m_units_per_cm;
    
    /** will contain the dimensions of the coordinate system, once it's calculated.
      * in the printing callback, you can then draw from (0, 0) to (m_coord_system_width, m_coord_system_height),
      * which will be the area covering the paper minus the margins
      */
    int m_coord_system_width, m_coord_system_height;
    int m_OffsetX, m_OffsetY;
    int m_TargetImageX;
    int m_TargetImageY;

    // Image(s) to print
    CRef<CImage> m_Img;
    
public:
    
    /**
      * @param page_amount    number of pages we want to print. Here it's static because it's just a test, but in
      *                       real code you will often want to calculate dynamically once you know the size of the
      *                       printable area from page setup
      * @param title          name of the print job / of the printed document
      * @param units_per_cem  we'll use this to determine the coordinate system; it describes the number of units
      *                       per centimeter (i.e. how fine the coordinate system is)
      */
    CPrintHanderWx(CRef<CImage> img,
                   int page_amount, 
                   wxString title, 
                   float units_per_cm) : wxPrintout( title )
    {
        m_page_amount = page_amount;
        m_Img = img;
        
        m_orient = wxPORTRAIT; // wxPORTRAIT, wxLANDSCAPE
        m_paper_type = wxPAPER_LETTER;
        m_margin_left   = 16;
        m_margin_right  = 16;
        m_margin_top    = 32;
        m_margin_bottom = 32;
        
        m_units_per_cm   = units_per_cm;
    }

    void SetImage(CRef<CImage> img)
    {
        m_Img = img;
    }    
    
    /** shows the page setup dialog, OR sets up defaults */
    bool performPageSetup(const bool showPageSetupDialog)
    {
        // don't show page setup dialog, use default values
        wxPrintData printdata;
        printdata.SetPrintMode( wxPRINT_MODE_PRINTER );
        printdata.SetOrientation( m_orient );
        printdata.SetNoCopies(1);
        printdata.SetPaperId( m_paper_type ); 
        
        m_page_setup = wxPageSetupDialogData(printdata);
        m_page_setup.SetMarginTopLeft(wxPoint(m_margin_left,  m_margin_top));
        m_page_setup.SetMarginBottomRight(wxPoint(m_margin_right, m_margin_bottom)); 
        
        if (showPageSetupDialog)
        {
           // wxGenericPageSetupDialog dlg(NULL, &m_page_setup );
            CPrintSetupDlg dlg(m_Img, NULL, &m_page_setup);
           // dlg.ShowModal();

           // wxPageSetupDialog dialog( NULL, &m_page_setup );
           //if (dialog.ShowModal() == wxID_OK)
            if (dlg.ShowModal() == wxID_OK)
            {
                
                m_page_setup = dlg.GetPageSetupDialogData();
                m_orient = m_page_setup.GetPrintData().GetOrientation();
                m_paper_type = m_page_setup.GetPrintData().GetPaperId();

                //??print to file??
                
                wxPoint marginTopLeft = m_page_setup.GetMarginTopLeft();
                wxPoint marginBottomRight = m_page_setup.GetMarginBottomRight();
                m_margin_left   = marginTopLeft.x;
                m_margin_right  = marginBottomRight.x;
                m_margin_top    = marginTopLeft.y;
                m_margin_bottom = marginBottomRight.y;

                m_coord_system_width = dlg.GetDrawAreaWidth();
                m_coord_system_height = dlg.GetDrawAreaHeight();
                m_OffsetX = dlg.GetPrintOffsetX();
                m_OffsetY = dlg.GetPrintOffsetY();              

                m_TargetImageX = dlg.GetTargetImageWidth();
                m_TargetImageY = dlg.GetTargetImageHeight();
            }
            else
            {
                std::cout << "user canceled at first dialog" << std::endl;
                return false;
            }
        }
        return true;
    }
    
    /** Called when printing starts */
    void OnBeginPrinting()
    {
        /*
        // set-up coordinate system however we want, we'll use it when drawing
        
        // take paper size and margin sizes into account when setting up coordinate system
        // so that units are "square" (1 unit x is a wide as 1 unit y is high)
        // (actually, if we don't make it square, on some platforms wx will even resize your
        //  margins to make it so)
        wxSize paperSize = m_page_setup.GetPaperSize();  // in millimeters
        
        // still in millimeters
        float large_side = std::max(paperSize.GetWidth(), paperSize.GetHeight());
        float small_side = std::min(paperSize.GetWidth(), paperSize.GetHeight());
        
        float large_side_cm = large_side / 10.0f;  // in centimeters
        float small_side_cm = small_side / 10.0f;  // in centimeters
        
        if (m_orient == wxPORTRAIT)
        {
            float ratio = float(large_side - m_margin_top  - m_margin_bottom) /
                          float(small_side - m_margin_left - m_margin_right);
            
            m_coord_system_width  = (int)((small_side_cm - m_margin_left/10.f -
                                           m_margin_right/10.0f)*m_units_per_cm);
            m_coord_system_height = m_coord_system_width*ratio;
        }
        else
        {
            float ratio = float(large_side - m_margin_left - m_margin_right) /
                          float(small_side - m_margin_top  - m_margin_bottom);
            
            m_coord_system_height = (int)((small_side_cm - m_margin_top/10.0f -
                                           m_margin_bottom/10.0f)* m_units_per_cm);
            m_coord_system_width  = m_coord_system_height*ratio;
            
        }
        */
 
    }
    
    /** returns the data obtained from the page setup dialog (or the defaults,
     * if dialog was not shown) */
    wxPrintData getPrintData()
    {
        return m_page_setup.GetPrintData();
    }
    
    /** Called when starting to print a document */
    bool OnBeginDocument(int startPage, int endPage)
    {
        std::cout << "beginning to print document, from page " << startPage
                  << " to " << endPage << std::endl;
        return wxPrintout::OnBeginDocument(startPage, endPage);
    }
    
    /** wx will call this to know how many pages can be printed */
    void GetPageInfo(int *minPage, int *maxPage, int *pageSelFrom, int *pageSelTo)
    {
        *minPage = 1;
        *maxPage = m_page_amount;
        
        *pageSelFrom = 1;
        *pageSelTo = m_page_amount;
    }
    
    /** called by wx to know what pages this document has */
    bool HasPage(int pageNum)
    {
        // wx will call this to know how many pages can be printed
        if (pageNum >= 1 && pageNum <= m_page_amount)
            return true;
        else
            return false;
    }
    
    
    /** called by wx everytime it's time to render a specific page onto the
     * printing device context */
    bool OnPrintPage(int pageNum)
    {
        std::cout << "printing page " << pageNum << std::endl;

        
        // ---- setup DC with coordinate system ----
        FitThisSizeToPageMargins(wxSize(m_coord_system_width, m_coord_system_height), m_page_setup);
        
        wxRect fitRect = GetLogicalPageMarginsRect(m_page_setup);
        
        wxCoord xoff = (fitRect.width - m_coord_system_width) / 2;
        wxCoord yoff = (fitRect.height - m_coord_system_height) / 2;
        OffsetLogicalOrigin(xoff, yoff);
        
        wxDC* ptr = GetDC();
        if (ptr==NULL || !ptr->IsOk())
        {
            std::cout << "DC is not Ok, interrupting printing" << std::endl;
            return false;
        }
        wxDC& dc = *ptr;
        
        // ---- A couple helper variables to help us during draw within paper area -----
        const int x0 = 0;
        const int y0 = 0;
        const int width = m_coord_system_width;
        const int height = m_coord_system_height;
        const int x1 = x0 + width;
        const int y1 = y0 + height;
        
        //const int center_x = x0 + width/2;
        //const int center_y = y0 + height/2;
        
        std::cout << "printable area : (" << x0 << ", " << y0 << ") to ("
        << x1 << ", " << y1 << ")" << std::endl;
        
        wxImage img(m_Img->GetWidth(), m_Img->GetHeight(), (unsigned char*)m_Img->GetData(), true);
        //img.SaveFile(wxT("c:\\temp\\image_save1.bmp"),wxBITMAP_TYPE_BMP);

        img.Rescale(m_TargetImageX, m_TargetImageY, wxIMAGE_QUALITY_HIGH);
        //img.SaveFile(wxT("c:\\temp\\image_save2.bmp"),wxBITMAP_TYPE_BMP);
        wxBitmap wbm(img);

        //wbm.SaveFile(wxT("c:\\temp\\test_bmp.bmp"), wxBITMAP_TYPE_BMP);
        
        // ---- Draw to the print DC ----
        dc.Clear();
        
        dc.DrawBitmap(wbm, m_OffsetX, m_OffsetY, false);
        /*
        dc.SetPen(  wxPen( wxColour(0,0,0), brush_size ) );
        dc.SetBrush( *wxTRANSPARENT_BRUSH );

        
        // draw a rectangle to show its bounds.
        dc.DrawRectangle(x0, y0, width, height);
        
        // draw wxWidgets logo
        dc.SetBrush( *wxRED_BRUSH );
        dc.DrawRectangle(center_x-45-38, center_y, 76, 76);
        dc.SetBrush( *wxBLUE_BRUSH );
        dc.DrawRectangle(center_x-38, center_y-45, 76, 76);
        dc.SetBrush( wxBrush( wxColor(255,244,0) ) );
        dc.DrawRectangle(center_x+45-38, center_y-10, 76, 76);
        */
        
        // draw page number label
        wxString label( wxT("This is page #") );
        label << pageNum;
        dc.SetTextBackground( wxColour(255,255,0) );
        dc.SetTextForeground( wxColour(0,0,0) );
        dc.DrawText( label, x0 + width/5, y0 + height - 50 );
        
        return true;
    }  
    
    /** Called when printing is done. I have nothing to do in this case
     * in this example. */
    void OnEndPrinting()
    {
    }
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_WX___PRINT_HANDLER_WX__HPP
