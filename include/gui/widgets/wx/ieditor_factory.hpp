#ifndef GUI_WIDGETS_WX__IEDITOR_FACTORY__HPP
#define GUI_WIDGETS_WX__IEDITOR_FACTORY__HPP

/*  $Id: ieditor_factory.hpp 31173 2014-09-08 21:02:57Z katargir $
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

#include <corelib/ncbistd.hpp>

class wxFileArtProvider;

BEGIN_NCBI_SCOPE

class CUICommandRegistry;
class ICommandProccessor;
class IGuiWidgetHost;

#define EXT_POINT__EDITOR_FACTORY         "editor_factory"

/////////////////////////////////////////////////////////////////////////////
/// IEditFactory
class IEditorFactory
{
public:
    virtual ~IEditorFactory() {}

    virtual void RegisterIconAliases(wxFileArtProvider& provider) = 0;
    virtual void RegisterCommands(CUICommandRegistry&, wxFileArtProvider&) = 0;

    virtual CObject* CreateEditor(const string& interface_name, ICommandProccessor& cmdProccessor, IGuiWidgetHost* guiWidgetHost) = 0;
};


END_NCBI_SCOPE

#endif //GUI_WIDGETS_WX__IEDITOR_FACTORY__HPP
