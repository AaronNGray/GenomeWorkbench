#ifndef GUI_WIDGETS_FL___DDX_DDV__HPP
#define GUI_WIDGETS_FL___DDX_DDV__HPP

/*  $Id: ddx_ddv.hpp 18080 2008-10-14 22:33:27Z yazhuk $
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

/** @addtogroup GUI_FltkWidgets
* @{
*/

#include <gui/gui.hpp>


class Fl_Widget;

BEGIN_NCBI_SCOPE


enum EDDXDirection {
    eDDX_Set,
    eDDX_Get
};


///////////////////////////////////////////////////////////////////////////////
/// CDDV_DDX_Exception
class NCBI_GUIUTILS_EXPORT CDDV_DDX_Exception
            : EXCEPTION_VIRTUAL_BASE public CException
{
public:
    CDDV_DDX_Exception(const CDiagCompileInfo &info,
                       Fl_Widget* w,
                       const string& message);

    /// constructor for widgets that can select invalid range of data
    CDDV_DDX_Exception(const CDiagCompileInfo &info,
                       Fl_Widget* w,
                       const string& message,
                       int sel_start,
                       int sel_end);

    Fl_Widget*  GetWidget() {   return m_Widget;    }
    int     GetSelStart() const {   return m_SelStart;  }
    int     GetSelEnd() const   {   return m_SelEnd;  }

    NCBI_EXCEPTION_DEFAULT(CDDV_DDX_Exception, CException);
protected:
    Fl_Widget*  m_Widget; /// widget that failed
    int     m_SelStart; /// selection start
    int     m_SelEnd; /// selection end
};


/// show message box reporting the problem and move the foxus
NCBI_GUIUTILS_EXPORT
    void HandleDDXDDVError(CDDV_DDX_Exception& e,
                           const string& dlg_title = "Error");

END_NCBI_SCOPE


/* @} */

#endif  // GUI_WIDGETS_FL___DDX_DDV__HPP
