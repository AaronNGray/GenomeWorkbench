#ifndef GUI_WIDGETS_WX___UI_TOOL__HPP
#define GUI_WIDGETS_WX___UI_TOOL__HPP

/*  $Id: ui_tool.hpp 25706 2012-04-25 00:21:04Z voronov $
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
 *  File Description: Defines base interfaces and classes 
 *                    (i.e. for Alignment Scoring Methods).
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

class wxWindow;

BEGIN_NCBI_SCOPE

class CDialog;
class CMenuItem;
class CNcbiRegistry;


////////////////////////////////////////////////////////////////////////////////
/// IUITool represents an abstract algorithm that is bound to a UI component.
/// IUITool has a unique Name that identifies the algorithm and Description that
/// can contain provide detailed information. Both Name and Description are
/// supposed to be human-readable so that they can be used in UI.
/// IUITool serves as a base class for the hierarchy of interfaces representing
/// various types of algorithms such as alignment scoring methods, functions
/// calculated along the sequence, search methods etc.

class NCBI_GUIWIDGETS_WX_EXPORT IUITool
{
public:
    virtual ~IUITool()   {};

    // creates a copy
    virtual IUITool* Clone() const = 0;

    /// returns unique name of the method that is used in UI to identify it
    virtual string  GetName() const = 0;

    /// returns a detailed description of the method that is used in UI
    virtual string  GetDescription() const = 0;
};

////////////////////////////////////////////////////////////////////////////////
/// ITemplateUITool
class   NCBI_GUIWIDGETS_WX_EXPORT ITemplateUITool
{
public:
    virtual ~ITemplateUITool();

    /// loads only descriptive information
    virtual bool LoadInfo(CNcbiRegistry& reg) = 0;

    // loads all information
    virtual bool Load(CNcbiRegistry& reg);

    /// @name Edit interface
    /// @{
    virtual void SetName(const string& s) = 0;
    virtual void SetDescription(const string& s) = 0;

    /// saves only descriptive information
    virtual bool SaveInfo(CNcbiRegistry& reg) = 0;

    // saves all information
    virtual bool Save(CNcbiRegistry& reg);
    /// @}
};

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

#endif  // GUI_WIDGETS_WX___UI_TOOL__HPP
