#ifndef GUI_CORE___FILE_AUTO_LOADER__HPP
#define GUI_CORE___FILE_AUTO_LOADER__HPP

/*  $Id: file_auto_loader.hpp 38477 2017-05-15 21:10:59Z evgeniev $
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

#include <gui/core/ui_file_load_manager.hpp>
#include <gui/utils/extension.hpp>


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CFileAutoLoader
class  CFileAutoLoader :
    public CObject,
    public IFileFormatLoaderManager,
    public IExtension
{
public:
    CFileAutoLoader();

    /// @name IUILoadManager interface implementation
    /// @{
    virtual void    SetServiceLocator(IServiceLocator* srv_locator);
    virtual void    SetParentWindow(wxWindow* parent);
    virtual const IUIObject&  GetDescriptor() const;
    virtual void    InitUI();
    virtual void    CleanUI();
    virtual wxPanel*    GetCurrentPanel();
    virtual bool    CanDo(EAction action);
    virtual bool    IsFinalState();
    virtual bool    IsCompletedState();
    virtual bool    DoTransition(EAction action);
    virtual IAppTask*      GetTask();
    virtual IExecuteUnit*  GetExecuteUnit();
    /// @}

    /// @{ IFileFormatLoaderManager - additional members
    /// @{
    virtual string   GetFileLoaderId() const { return "file_loader_auto"; }
    virtual wxString GetFormatWildcard();
    virtual bool     ValidateFilenames(const vector<wxString>& filenames);
    virtual void     SetFilenames(const vector<wxString>& filenames);
    virtual void     GetFilenames(vector<wxString>& filenames) const;
    virtual bool     IsInitialState();
    virtual bool     RecognizeFormat(const wxString& filename);
    virtual bool     RecognizeFormat(CFormatGuess::EFormat) { return false; }
    virtual bool     SingleFileLoader() const { return false; }
    /// @}

    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

    IFileFormatLoaderManager* GetCurrentManager() { return m_FormatManager.GetPointerOrNull(); }
    void ResentCurrentManager() { m_FormatManager.Reset(); }

    CFormatGuess::EFormat SetFormatManager(const vector<wxString>& filenames);

protected:
    CUIObject m_Descr;
    CIRef<IFileFormatLoaderManager> m_FormatManager;
};

END_NCBI_SCOPE


#endif // GUI_CORE___FILE_AUTO_LOADER__HPP
