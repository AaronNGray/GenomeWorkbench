#ifndef GUI_UTILS___LABEL__HPP
#define GUI_UTILS___LABEL__HPP

/*  $Id: label.hpp 36346 2016-09-15 17:56:22Z evgeniev $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <serial/serialdef.hpp>
#include <objmgr/scope.hpp>
//#include <objects/seqfeat/Seq_feat.hpp>
#include <gui/gui.hpp>

/** @addtogroup GUI_UTILS
 *
 * @{
 */


BEGIN_NCBI_SCOPE


class ILabelHandler;

BEGIN_SCOPE(objects)
class CProjectItem;
END_SCOPE(objects)


class NCBI_GUIOBJUTILS_EXPORT CLabel
{
public:
    enum ELabelType {
        eInvalid = -1,
        eContent,
        eUserType,
        eUserSubtype,
        eType,
        eUserTypeAndContent,
        eDescriptionBrief,
        eDescription,
        eSymbolAlias,
		eGi,
        eContentAndProduct,

        eDefault = eContent
    };

    // retrieve a label for an object
    static void GetLabel(
		const CObject& obj, 
		string* label,
        ELabelType type = eDefault
    );

	static void GetLabel(
		const CObject& obj, 
		string* label,
		ELabelType type,
		objects::CScope* scope 
	);

	// register handler classes that will be called when this interface
    // is presented with an object for which a label is requested
    static void RegisterLabelHandler(const string& type,
                                     ILabelHandler& handler);
    static void RegisterLabelHandler(const CTypeInfo& type,
                                     ILabelHandler& handler);

    // Query to see whether a handler exists for a specific type
    static bool HasHandler(const CTypeInfo& type);
    static bool HasHandler(const string& type);

    static void RegisterTypeIcon(const string& user_type,
                                 const string& user_subtype,
                                 const string& icon_alias);
    static string   GetTypeIconAlias(const string& user_type,
                                     const string& user_subtype);
    static string   GetTypeIconAlias(const string& user_type);

    static void     GetIconAlias(const CObject& obj, string* alias,
                                 objects::CScope* scope = NULL);

    /// truncate the label to some specific length with a desired leven of precision
    ///  algorithm tries to analyse the string, find punctuation and truncate on that
    ///
    /// @param label - string to truncate
    /// @param length_limit - desired target length
    /// @param length_tolerance - tolerance of error in number of characters, 0- no tolerance.
    ///
    static void TruncateLabel(string* label, 
                              unsigned length_limit, 
                              unsigned length_tolerance);

	static void SetLabelByData(objects::CProjectItem& item, objects::CScope* scope = NULL);

private:
    typedef map<string, CRef<ILabelHandler> > TLabelMap;
    typedef map<string, string> TAliasMap;

    static TLabelMap    sm_LabelMap;
    static TAliasMap    sm_AliasMap;
};


///////////////////////////////////////////////////////////////////////////////
///  ILabelHandler interface
class ILabelHandler: public CObject
{
public:
    virtual void GetLabel(const CObject& obj, string* str,
                          CLabel::ELabelType type,
                          objects::CScope* scope) const = 0;
};


/// Class to control runtime cache for description labels
///   label generation can be slow if we need to retrieve molecules, this
///   class controls session only runtime cache for the labels
///
class NCBI_GUIOBJUTILS_EXPORT CLabelDescriptionCache
{
public:
    /// Initialize cache instance (call once at gBench start)
    static void InitCache();

    /// Stop cache instance (call once at gBench stop)
    static void StopCache();

    /// Generate and cache description label for the specified gi
    /// 
    static void CacheSeqId(const objects::CSeq_id& id, objects::CScope* scope);

    static void AddCaches(const map<TGi, string>& caches);
};


END_NCBI_SCOPE


/* @} */

#endif  // GUI_UTILS___LABEL__HPP
