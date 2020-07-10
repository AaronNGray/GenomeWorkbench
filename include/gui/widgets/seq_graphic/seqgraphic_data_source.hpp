#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_DATA_SOURCE__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_DATA_SOURCE__HPP

/* $Id: seqgraphic_data_source.hpp 25703 2012-04-24 19:57:50Z wuliangs $
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
 * Author:  Liangshou Wu
 *
 */

 /**
 * File Description:
 */

#include <corelib/ncbistr.hpp>
#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>
#include <gui/objutils/objects.hpp>

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// ISGDataSource - represents a relatively independent visual conponent disaplayed
/// in Graphical view.
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT ISGDataSource
{
public:
    /// List of annotations with the corresponding titles
    typedef map<string, string> TAnnotNameTitleMap;

    virtual ~ISGDataSource() {}
};

/// Default seqgraphic data source implementation.

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSGDataSource :
    public CObjectEx,
    public ISGDataSource
{
public:
    static void SetBackgroundJob(bool flag) { m_Background = flag; }
    static bool IsBackgroundJob() { return m_Background; }

protected:
    static bool m_Background;
};

///////////////////////////////////////////////////////////////////////////////
/// ISGDataSourceFactory
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT  ISGDataSourceType
{
public:
    virtual ~ISGDataSourceType() {};

    /// create an instance of the layout track type using default settings.
    virtual ISGDataSource*  CreateDS(SConstScopedObject& object) const = 0;

    /// check if the data source can be shared.
    virtual bool IsSharable() const = 0;
};


END_NCBI_SCOPE

#endif // GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_DATA_SOURCE__HPP
