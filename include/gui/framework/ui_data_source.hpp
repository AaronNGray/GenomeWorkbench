#ifndef GUI_FRAMEWORK___UI_DATA_SOURCE__HPP
#define GUI_FRAMEWORK___UI_DATA_SOURCE__HPP

/*  $Id: ui_data_source.hpp 38460 2017-05-11 18:58:47Z evgeniev $
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

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbistl.hpp>

#include <gui/gui_export.h>

class wxEvtHandler;

BEGIN_NCBI_SCOPE

class IUIObject;
class IUIToolManager;
class IUIDataSourceType;

BEGIN_SCOPE(objects)
    class CUser_object;
END_SCOPE(objects)


///////////////////////////////////////////////////////////////////////////////
/// IUIDataSource - an interface representing a Data Source.
/// Data Source is an object that provides access to data available in external
/// databases and services. Data Source can support functions such as loading,
/// saving, searching, exploring and editing. Data Source is supposed
/// to represent a user concept and be exposed in the application UI.
/// Users will be able to interact with Data Source objects creating new sources,
/// changing their properties and accessing functions provided by the objects.
/// Technically Data Source may work as façade for a collection of several
/// different databases and services, but in the user world it shall correspond
/// to a single concept understandable by the end users (such as GenBank).

class  NCBI_GUIFRAMEWORK_EXPORT  IUIDataSource
{
public:
    virtual ~IUIDataSource()    {};


    virtual IUIDataSourceType&  GetType() const = 0;

    /// returns UI description of the object (label, icon etc.)
    virtual const IUIObject&    GetDescr() = 0;

    /// a Data source needs to be open before use and closed after.
    virtual bool    IsOpen() = 0;

    /// Prepares Data Source for use, any initialization that can be potentially
    /// time-consuming or may require error handling shall be done in this
    /// function, not in a constructor. Is initialization takes more than 1-2 sec
    /// it shall be done on a background thread (see App Job Dispatcher).
    virtual bool    Open() = 0;

    virtual bool    Close() = 0;

    virtual IUIToolManager* GetLoadManager() = 0;

    virtual int GetDefaultCommand() { return 0; }
    virtual wxEvtHandler* CreateEvtHandler() { return 0; }
};


///////////////////////////////////////////////////////////////////////////////
/// IUIDataSourceType - defines a type of a Data Source, can serve as a
/// Data Source factory.

class  NCBI_GUIFRAMEWORK_EXPORT  IUIDataSourceType
{
public:
    /// returns UI description of the type (label, icon etc.)
    virtual const IUIObject&    GetDescr() = 0;

    /// factory method; create an Data Source instance
    virtual IUIDataSource*  CreateDataSource() = 0;

    /// returns "true" if this type needs to create a default instance of the
    /// Data Source at start-up
    virtual bool    AutoCreateDefaultDataSource() = 0;

    virtual ~IUIDataSourceType()    {};
};


///////////////////////////////////////////////////////////////////////////////
/// IDataLoaderProvider - an interface representing a Data Source.
/// Data Source is an object that provides access to data available in external
/// databases and services. IDataLoaderProvider is a data source that provides
/// data loader to other consumers and is responsible to register data loader
/// in object manager.

class  NCBI_GUIFRAMEWORK_EXPORT  IDataLoaderProvider
{
public:
    virtual ~IDataLoaderProvider()    {};

    /// Add a new data loader to object manager.
    /// If the data loader exists, return the existing one.
    /// @return added data loader's name.
    virtual string AddDataLoader(const objects::CUser_object& obj) = 0;

    /// Get the data loader's name based on input object.
    virtual string GetLoaderName(const objects::CUser_object& obj) const = 0;
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___UI_DATA_SOURCE__HPP

