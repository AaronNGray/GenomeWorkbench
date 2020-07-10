#ifndef GUI_WIDGETS_WX___HTML_WINDOW__HPP
#define GUI_WIDGETS_WX___HTML_WINDOW__HPP

/*  $Id: htmlwin.hpp 24859 2011-12-14 00:21:38Z voronov $
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
 * Authors: Yury Voronov
 *
 * File Description:
 */

#include <gui/gui.hpp>

#include <wx/html/htmlwin.h>

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_WX_EXPORT CwxHtmlWindow 
	: public wxHtmlWindow
{
	DECLARE_DYNAMIC_CLASS(CwxHtmlWindow)
	DECLARE_NO_COPY_CLASS(CwxHtmlWindow)
	DECLARE_EVENT_TABLE()

public:
	CwxHtmlWindow() : wxHtmlWindow(), mf_LeftIsDown(false) {}
	CwxHtmlWindow( 
		wxWindow *parent, wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxHW_DEFAULT_STYLE,
		const wxString& name = wxT("CwxHtmlWindow")
	)
		: wxHtmlWindow( parent, id, pos, size, style, name )
		, mf_LeftIsDown(false)
	{
	}

protected:
	void OnMouse( wxMouseEvent& event );

private:
	bool mf_LeftIsDown;
};

END_NCBI_SCOPE


#endif  // GUI_WIDGETS_WX___HTML_WINDOW__HPP
