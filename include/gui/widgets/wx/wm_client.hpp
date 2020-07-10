#ifndef GUI_WIDGETS_WORKSPACE___WM_CLIENT_HPP
#define GUI_WIDGETS_WORKSPACE___WM_CLIENT_HPP

/*  $Id: wm_client.hpp 31188 2014-09-10 16:14:04Z katargir $
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

#include <gui/utils/rgba_color.hpp>
#include <gui/widgets/wx/ui_command.hpp>

class wxWindow;
class wxMenu;
class wxEvtHandler;

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// IWClient - abstract Window Manager client
class IWMClient
    : public IMenuContributor
{
public:
    /// CFingerprint identifies an instance of IWMClient and is used for
    /// labeling layout positions.
    /// If Fingerprint is persistent it will be saved with the layout and when
    /// the layout is loaded a client will be automatically instantiated based
    /// on the saved fingerprint using a client factory.
    class NCBI_GUIWIDGETS_WX_EXPORT CFingerprint
    {
    public:
        CFingerprint();
        CFingerprint(const string& id, bool persistent);
        CFingerprint(const CFingerprint& orig);

        bool    operator==(const CFingerprint& orig) const;
        bool    IsEmpty() const;

        /// returns a string for serialization (if persistent) or ""
        string  AsPersistentString() const;
        void    FromPersistentString(const string& s);

        const string GetId() const;

    protected:
        string  m_Id;
        bool    m_Persistent;
    };

    /// Different flavors of label types for different GUI aspects
    enum ELabel
    {
        eContent,  ///< Short name with content 
        eTypeAndContent, ///< Short name with content and type
        eDetailed, ///< Detailed name
        eDefault = eDetailed
    };

public:
    /// returns a pointer to the wxWindow representing the client
    virtual wxWindow* GetWindow() = 0;

    /// returns a pointer to the command handler (for menu commands and updates)
    virtual wxEvtHandler* GetCommandHandler() = 0;

    /// returns the client label (name) to be displayed in UI
    virtual string  GetClientLabel(IWMClient::ELabel ltype = IWMClient::eDefault) const = 0;

    /// returns an icon alias that can be used to retrieve the client's icon
    virtual string GetIconAlias() const = 0;

    /// returns a color associated with the client, n/a if NULL
    virtual const CRgbaColor* GetColor() const = 0;

    /// returns a color associated with the client
    virtual void SetColor(const CRgbaColor& color) = 0;


    /// @name IWMClient implementation
    /// @{
    /// returns menu that can be merged with main menu of an application
    //  virtual const wxMenu* GetMenu();
    //  virtual const SwxMenuItemRec* GetMenuDef() const;
    //@}

    /// returns a fingerprint identifying the client
    virtual CFingerprint  GetFingerprint() const = 0;
    virtual void  SetFingerprint(const string& /*fp*/) {}

    /// returns true if client is a Form - wants to be sized to its client area (like a dialog)
    virtual bool IsAForm() const { return false; }

    /// returns true if client is a non-modal dioalog
    virtual bool IsADialog() const { return false; }


    virtual ~IWMClient()    {}
};


///////////////////////////////////////////////////////////////////////////////
/// IWMClientFactory - IWMClient factory.

class NCBI_GUIWIDGETS_WX_EXPORT IWMClientFactory
{
public:
    typedef IWMClient::CFingerprint TFingerprint;

    /// creates a client by fingerprint
    /// returns NULL if fingerprint is not recognized.
    virtual IWMClient*  CreateClient(const TFingerprint& fingerprint,
                                     wxWindow* parent) = 0;

    virtual ~IWMClientFactory() {};
};


END_NCBI_SCOPE


#endif  // GUI_WIDGETS_WORKSPACE___WM_CLIENT_HPP
