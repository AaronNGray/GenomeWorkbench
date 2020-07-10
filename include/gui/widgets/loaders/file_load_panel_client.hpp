#ifndef GUI_WIDGETS___LOADERS___FILE_LOAD_PANEL_CLIENT__HPP
#define GUI_WIDGETS___LOADERS___FILE_LOAD_PANEL_CLIENT__HPP

/*  $Id: file_load_panel_client.hpp 28982 2013-09-20 14:40:24Z katargir $
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

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <wx/string.h>

#include <util/format_guess.hpp>
#include <gui/utils/extension.hpp>

BEGIN_NCBI_SCOPE

class IFileLoadPanelClient;

class IFileLoadPanelClientFactory
{
public:
	virtual ~IFileLoadPanelClientFactory() {}

    /// create and instance of the IFileLoadPanelClient
    virtual IFileLoadPanelClient*  CreateInstance() const = 0;

    // return file loader id string to be used in MRU file list instead of label.
    virtual string   GetFileLoaderId() const = 0;
};

template <class T> class CFileLoadPanelClientFactory :
    public CObject,
    public IExtension,
    public IFileLoadPanelClientFactory
{
public:
	virtual string  GetExtensionIdentifier() const { return T::Id() + "_factory"; }
	virtual string  GetExtensionLabel() const { return T::Label() + " Format Loader Factory"; }

	virtual IFileLoadPanelClient* CreateInstance() const { return new T(); }
	virtual string   GetFileLoaderId() const { return T::Id(); }
};

class IFileLoadPanelClient
{
public:
	virtual ~IFileLoadPanelClient() {}

	virtual string   GetLabel() const = 0;
    virtual IExecuteUnit* GetExecuteUnit() { return 0; }

    // return file loader id string to be used in MRU file list instead of label.
    virtual string   GetFileLoaderId() const = 0;

    /// return file wildcard for the format (example - "*.txt;*.tab" )
    virtual wxString GetFormatWildcard() const = 0;

    /// checks given filenames and returns true if the manager can handle them,
    /// otherwise - shows an error message
    virtual bool    ValidateFilenames(const vector<wxString>& filenames) = 0;

    /// initilize the manager with the given filenames
    virtual void    SetFilenames(const vector<wxString>& filenames) = 0;
    /// get actual filenames that will be opened
    virtual void    GetFilenames(vector<wxString>& filenames) const = 0;

    /// return true if the given file format is supported by this manager
    virtual bool    RecognizeFormat(const wxString& filename) = 0;

    /// return true if the given file format is supported by this manager
    virtual bool    RecognizeFormat(CFormatGuess::EFormat fmt) = 0;

    /// returns true if loader can handle only one file
    virtual bool    SingleFileLoader() const { return false; }
};


END_NCBI_SCOPE


#endif  // GUI_WIDGETS___LOADERS___FILE_LOAD_PANEL_CLIENT__HPP

