#ifndef GUI_GUI___EXTENSION__HPP
#define GUI_GUI___EXTENSION__HPP

/*  $Id: extension.hpp 18397 2008-11-28 19:45:17Z dicuccio $
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
 *    Application Extension API
 */


/** @addtogroup GUI_UTILS
 *
 * @{
 */

#include <gui/gui_export.h>

#include <corelib/ncbiobj.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// IExtension
/// IExtension interface represents an abstract pluggable component. Software
/// components can contribute Extensions to various Extension Points in order
/// to add new functionality to the application.
/// IExtension is managed by CIRef and so classes implementing IExtension
/// interface need to be CRef-compatible (for instance, derived from CObject)

class NCBI_GUIUTILS_EXPORT  IExtension
{
public:
    /// returns the unique human-readable identifier for the extension
    /// the id should use lowercase letters separated by underscores - "like_this"
    virtual string  GetExtensionIdentifier() const = 0;

    /// returns a displayable label for this extension
    /// ( please capitalize the key words - "My Extension" )
    virtual string  GetExtensionLabel() const = 0;

    virtual ~IExtension()   {};
};


///////////////////////////////////////////////////////////////////////////////
/// IExtensionPoint
///
/// IExtensionPoint interface represents an aspect of an application
/// functionality that can be extended by pluggable components.
/// Each Extension Point has a unique ID and represents an aspect of the
/// application that can be abstracted. For instance an Extension Point can
/// represent a place in a popup menu where arbitrary commands can be inserted.
/// Any software component in an application can declare its own Extension Points
/// to allow other components to add new functionality. Usually an Extension Point
/// is associated with a specific interface and all Extensions contributed to
/// this point must implement this interface.

class NCBI_GUIUTILS_EXPORT  IExtensionPoint
{
public:
    typedef CIRef<IExtension> TExtRef;
    typedef vector<TExtRef>   TExtVec;

public:
    // returns the unique identifier of this extension
    virtual string  GetIdentifier() const = 0;

    // returns a displayable label for this extension
    virtual string  GetLabel() const = 0;

    // return true if extension is succesfully added
    virtual bool    AddExtension(IExtension& extension) = 0;

    // retuns an extension with the given id (or empty reference)
    virtual TExtRef GetExtension(const string& ext_id) = 0;

    // fills the container with references to extensions
    virtual void    GetExtensions(TExtVec& extensions) = 0;

    // removes the given extension, if extension is not found returns false
    virtual bool    RemoveExtension(IExtension& extension) = 0;

    // removes the extension with the given id, if extension is not found returns false
    virtual bool    RemoveExtension(const string& ext_id) = 0;

    virtual ~IExtensionPoint()  {};
};


///////////////////////////////////////////////////////////////////////////////
/// IExtensionRegistry
/// IExtensionRegistry is an interface representing a Registry where Extension
/// Points and Extensions are registered. IExtensionRegistry provides a more
/// convenient higher-level API for working with Points and Extensions.
class NCBI_GUIUTILS_EXPORT  IExtensionRegistry
{
public:
    typedef CIRef<IExtension> TExtRef;
    typedef vector<TExtRef>   TExtVec;
    typedef CIRef<IExtensionPoint>  TExtPointRef;
    typedef vector<TExtPointRef> TExtPointVec;

public:
    /// adds a new Extension Point, returns true if successful
    virtual bool    AddExtensionPoint(IExtensionPoint& ext_point) = 0;

    /// adds a new Extension Point, returns true if successful
    virtual bool    AddExtensionPoint(const string& ext_point_id,
                                      const string& ext_point_label) = 0;

    /// adds a new Extension to an Extension Point, if the Point does not exists
    /// creates a new one, returns true if successful
    virtual bool    AddExtension(const string& ext_point_id,
                                 IExtension& extension) = 0;

    /// finds and returns a reference to an Extension with the specified Id,
    /// search is performed across all Points in the Registry
    /// returns null if there the Id is not found
    virtual TExtRef GetExtension(const string& extension_id) = 0;

    /// finds and returns a reference to an Extension with the specified Id,
    /// search is performed only in the Extension Point with the given Id,
    /// returns null if there the Id is not found
    virtual TExtRef GetExtension(const string& ext_point_id,
                                 const string& extension_id) = 0;

    /// returns a reference to an Extension Point with the given identifier,
    /// returns null if there the Id is not found
    virtual TExtPointRef    GetExtensionPoint(const string& ext_point_id) = 0;

    /// finds an Extension Point with the given Id and fills the given container
    /// with references to all Extensions registered in the Point.
    /// returns false if a Point with the given id is not registered
    virtual bool    GetExtensions(const string& ext_point_id,
                                  TExtVec& extensions) = 0;

    /// fills the given container with all Extension Points registered
    virtual void    GetExtensionPoints(TExtPointVec& ext_points) = 0;

    /// removes the given Extension from all Extension Points in the Registry
    /// returns false if the Extension is not registered
    virtual bool    RemoveExtension(IExtension& extension) = 0;

    /// removes the Extension with the given Id from all Extension Point in the
    /// Registry, returns  false if the Extension Id is not found
    virtual bool    RemoveExtension(const string& ext_id) = 0;

    /// removes the given Extension from the Extension Point with the given id
    /// returns false if the Point or Extension is not found
    virtual bool    RemoveExtension(const string& ext_point_id,
                                    IExtension& extension) = 0;

    /// removes the Extension with the given Id from the Extension Point with
    /// the specified Id.
    /// returns false if the Point or Extension is not found
    virtual bool    RemoveExtension(const string& ext_point_id,
                                    const string& ext_id) = 0;

    /// removes the specified Extension Point from the Registry.
    virtual bool    RemoveExtensionPoint(IExtensionPoint& ext_point) = 0;

    virtual ~IExtensionRegistry()   {}
};


END_NCBI_SCOPE

/* @} */

#endif // GUI_GUI___EXTENSION__HPP
