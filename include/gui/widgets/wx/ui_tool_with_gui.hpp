#ifndef GUI_WIDGETS_WX___UI_TOOL_WITH_GUI__HPP
#define GUI_WIDGETS_WX___UI_TOOL_WITH_GUI__HPP

/*  $Id: ui_tool_with_gui.hpp 25706 2012-04-25 00:21:04Z voronov $
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
 *  File Description:
 *
 */

#include <corelib/ncbistd.hpp>

class wxWindow;

BEGIN_NCBI_SCOPE

class CMenuItem;

////////////////////////////////////////////////////////////////////////////////
/// IUIToolWithGUI - represents a method that can be integrated in GUI.
class   NCBI_GUIWIDGETS_WX_EXPORT IUIToolWithGUI
{
public:
    virtual ~IUIToolWithGUI() {}

    /// Returns a pointer to the submenu. If the pointer is not NULL, then this
    /// submenu is integrated into the application's menu. If method is derived
    /// from CCommandTarget it will recieve command events.
    virtual const CMenuItem* GetMenu() = 0;

    /// returns true if the method supports properties dialog
    virtual bool HasPropertiesPanel() const = 0;

    /// This a factory method that shall produce a CPropertiesPanel
    /// representing properties of the method; the panel Apply() must take care
    /// of appying changes to the method.
    virtual wxWindow* CreatePropertiesPanel(wxWindow* parent) = 0;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_WX___UI_TOOL_WITH_GUI__HPP
