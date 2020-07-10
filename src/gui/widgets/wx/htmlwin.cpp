/*  $Id: htmlwin.cpp 24859 2011-12-14 00:21:38Z voronov $
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
 *    
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/htmlwin.hpp>


BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CwxHtmlWindow, wxHtmlWindow )

BEGIN_EVENT_TABLE( CwxHtmlWindow, wxHtmlWindow )
	EVT_LEFT_DOWN( CwxHtmlWindow::OnMouse )
	EVT_LEFT_UP( CwxHtmlWindow::OnMouse )
	EVT_RIGHT_UP( CwxHtmlWindow::OnMouse )
END_EVENT_TABLE()

void CwxHtmlWindow::OnMouse( wxMouseEvent& event )
{
	if( event.LeftDown() ){
		mf_LeftIsDown = true;
	
	} else if( event.LeftUp() ){
		if( !mf_LeftIsDown ){
			return;
		}

		mf_LeftIsDown = false;

	} else if( event.RightUp() ){
		return;
	}
	
	event.Skip();
}

END_NCBI_SCOPE
