#ifndef TYPE_HPP
#define TYPE_HPP

/*  $Id: type.hpp 554977 2018-01-11 14:18:53Z gouriano $
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
* Author: Eugene Vasilchenko
*
* File Description:
*   Type definition
*
*/

#include <corelib/ncbistd.hpp>
#include <corelib/ncbistre.hpp>
#include <corelib/ncbiutil.hpp>
#include <serial/impl/typeref.hpp>
#include <serial/impl/objstrasnb.hpp>
#include "comments.hpp"
#include <list>
#include <set>
#include <functional>

BEGIN_NCBI_SCOPE

class CTypeInfo;
class CDataType;
class CDataTypeModule;
class CDataValue;
class CChoiceDataType;
class CUniSequenceDataType;
class CReferenceDataType;
class CTypeStrings;
class CFileCode;
class CClassTypeStrings;
class CNamespace;
class CDataMember;

/////////////////////////////////////////////////////////////////////////////
// ostream_iterator with corrected delimiter and inserter

template<typename TValue, typename TElem = char, typename TTraits = char_traits<TElem> >
class Dt_ostream_iterator
    : public iterator<output_iterator_tag, void, void, void, void>
{
public:
	typedef TElem char_type;
	typedef TTraits traits_type;
	typedef basic_ostream<TElem, TTraits> ostream_type;
    typedef function<void(ostream_type&, const TValue&)> TInserter;

	Dt_ostream_iterator(ostream_type& ostr, const TElem *delim = nullptr,
        TInserter fnIn = [](ostream_type& out, const TValue& v){ out << v;})
		: m_ostr(&ostr), m_delim(delim), m_fnIn(fnIn), m_first(true) {
	}
    Dt_ostream_iterator& operator=(const TValue& value) {
		if (!m_first && m_delim !=  nullptr) {
			*m_ostr << m_delim;
        }
		m_fnIn(*m_ostr, value);
		m_first = false;
        return *this;
    }
	Dt_ostream_iterator& operator*() {
		return *this;
	}
	Dt_ostream_iterator& operator++() {
		return *this;
	}
	Dt_ostream_iterator& operator++(int) {
		return *this;
	}
protected:
	ostream_type *m_ostr;
    const TElem *m_delim;
    TInserter    m_fnIn;
	bool m_first;
};

// mark Dt_ostream_iterator as checked
#ifdef NCBI_COMPILER_MSVC
template<typename TValue, typename TElem, class TTraits>
struct _Is_checked_helper<Dt_ostream_iterator<TValue, TElem, TTraits> >
		: public true_type {
};
#endif

template<typename TInputIt, typename TOutputIt, class TFnIf, class TFnTr>
TOutputIt Dt_transform_if(TInputIt first, TInputIt last, TOutputIt dest,
                          const TFnIf& funcIf, const TFnTr& funcTransform)
{
	for (; first != last; ++first, (void)++dest) {
        if (funcIf(*first)) {
		    *dest = funcTransform(*first);
        }
    }
	return (dest);
}
/////////////////////////////////////////////////////////////////////////////
class CMemberFacet
{
public:
    CMemberFacet(ESerialFacet type, const string& value)
        : m_Type(type), m_Value(value) {
    }
    ESerialFacet GetType(void) const {
        return m_Type;
    }
    const string& GetValue(void) const {
        return m_Value;
    }
private:
    ESerialFacet m_Type;
    string m_Value;
};


struct AnyType {
    union {
        bool booleanValue;
        Int8 integerValue;
        void* pointerValue;
    };
    AnyType(void)
        {
            pointerValue = 0;
        }
};

class CDataType {
public:
    typedef void* TObjectPtr;
    typedef list<const CReferenceDataType*> TReferences;

    CDataType(void);
    virtual ~CDataType(void);

    const CDataType* GetParentType(void) const
        {
            return m_ParentType;
        }
    const CDataTypeModule* GetModule(void) const
        {
            _ASSERT(m_Module != 0);
            return m_Module;
        }
    bool HaveModuleName(void) const
        {
            return m_ParentType == 0;
        }

    const string& GetSourceFileName(void) const;
    int GetSourceLine(void) const
        {
            return m_SourceLine;
        }
    void SetSourceLine(int line);
    string LocationString(void) const;

    string GetKeyPrefix(void) const;
    string IdName(void) const;
    string XmlTagName(void) const;
    const string& GlobalName(void) const; // name of type or empty
    bool Skipped(void) const;
    string DefClassMemberName(void) const;
    string ClassName(void) const;
    string FileName(void) const;
    const CNamespace& Namespace(void) const;
    string InheritFromClass(void) const;
    const CDataType* InheritFromType(void) const;
    const string GetVar(const string& value, int collect = 0) const;
    bool GetBoolVar(const string& value, bool default_value = false) const;
    void  ForbidVar(const string& var, const string& value);
    void  AllowVar(const string& var, const string& value);
    const string GetAndVerifyVar(const string& value) const;

    bool InChoice(void) const;

    void PrintASNTypeComments(CNcbiOstream& out, int indent, int flags=0) const;
    void PrintDTDTypeComments(CNcbiOstream& out, int indent) const;
    virtual void PrintASN(CNcbiOstream& out, int indent) const = 0;
    virtual void PrintSpecDump(CNcbiOstream& out, int indent) const;
    virtual void PrintSpecDumpExtra(CNcbiOstream& out, int indent) const;
    virtual void PrintJSONSchema(CNcbiOstream& out, int indent, list<string>& required, bool contents_only=false) const = 0;
    virtual void PrintXMLSchema(CNcbiOstream& out, int indent, bool contents_only=false) const = 0;
    virtual const char* GetASNKeyword(void) const;
    virtual string      GetSpecKeyword(void) const;
    virtual string GetSchemaTypeString(void) const;
    void PrintDTD(CNcbiOstream& out) const;
    void PrintDTD(CNcbiOstream& out, const CComments& extra) const;
    virtual void PrintDTDElement(CNcbiOstream& out, bool contents_only=false) const = 0;
    virtual void PrintDTDExtra(CNcbiOstream& out) const;

    virtual CTypeRef GetTypeInfo(void);
    virtual const CTypeInfo* GetAnyTypeInfo(void);
    virtual bool NeedAutoPointer(const CTypeInfo* typeInfo) const;
    virtual const CTypeInfo* GetRealTypeInfo(void);
    virtual CTypeInfo* CreateTypeInfo(void);
    CTypeInfo* UpdateModuleName(CTypeInfo* typeInfo) const;

    void Warning(const string& mess, int err_subcode = 0) const;

    virtual AutoPtr<CTypeStrings> GenerateCode(void) const;
    void SetParentClassTo(CClassTypeStrings& code) const;

    virtual AutoPtr<CTypeStrings> GetRefCType(void) const;
    virtual AutoPtr<CTypeStrings> GetFullCType(void) const;
    virtual string GetDefaultString(const CDataValue& value) const;

    virtual const CDataType* Resolve(void) const;
    virtual CDataType* Resolve(void);

    // resolve type from global level
    CDataType* ResolveGlobal(const string& name) const;
    // resolve type from local level
    CDataType* ResolveLocal(const string& name) const;

    bool IsInSet(void) const
        {
            return m_Set != 0;
        }
    const CUniSequenceDataType* GetInSet(void) const
        {
            return m_Set;
        }
    void SetInSet(const CUniSequenceDataType* sequence);

    bool IsInChoice(void) const
        {
            return m_Choice != 0;
        }
    const CChoiceDataType* GetInChoice(void) const
        {
            return m_Choice;
        }
    void SetInChoice(const CChoiceDataType* choice);

    bool IsReferenced(void) const
        {
            return m_References;
        }
    void AddReference(const CReferenceDataType* reference);
    const TReferences& GetReferences(void) const
        {
            return *m_References;
        }
    bool IsInUniSeq(void) const;
    bool IsUniSeq(void) const;
    bool IsContainer(void) const;
    bool IsEnumType(void) const;

/*
    static string GetTemplateHeader(const string& tmpl);
    static bool IsSimplePointerTemplate(const string& tmpl);
    static string GetTemplateNamespace(const string& tmpl);
    static string GetTemplateMacro(const string& tmpl);
*/

    void SetParent(const CDataType* parent, const string& memberName, string xmlName=kEmptyStr);
    void SetParent(const CDataTypeModule* module, const string& typeName);
    virtual void FixTypeTree(void) const;

    bool Check(void);
    virtual bool CheckType(void) const;
    virtual bool CheckValue(const CDataValue& value) const = 0;
    virtual TObjectPtr CreateDefault(const CDataValue& value) const = 0;
    
    CComments& Comments(void)
        {
            return m_Comments;
        }
    const CComments& Comments(void) const
        {
            return m_Comments;
        }

    void SetDataMember(CDataMember* dm) {
        m_DataMember = dm;
    }

    const CDataMember* GetDataMember(void) const {
        return m_DataMember;
    }

    enum {
        eNoExplicitTag = -1
    };
    void SetTag(CAsnBinaryDefs::TLongTag tag) {
        m_Tag = tag;
    }

    CAsnBinaryDefs::TLongTag GetTag(void) const {
        return m_Tag;
    }

    bool HasTag(void) const {
        return m_Tag != eNoExplicitTag;
    }

    void SetTagClass(CAsnBinaryDefs::ETagClass tclass) {
        m_TagClass = tclass;
    }
    CAsnBinaryDefs::ETagClass GetTagClass(void) const {
        return m_TagClass;
    }
    void SetTagType(CAsnBinaryDefs::ETagType ttype) {
        m_TagType = ttype;
    }
    CAsnBinaryDefs::ETagType GetTagType(void) const {
        return m_TagType;
    }
    CNcbiOstream& PrintASNTag(CNcbiOstream& out) const;
    static string GetTagClassString(CAsnBinaryDefs::ETagClass tclass);
    static string GetTagTypeString(CAsnBinaryDefs::ETagType ttype);

    void SetTypeStr(CClassTypeStrings* TypeStr) const {
        m_TypeStr = TypeStr;
    }
    CClassTypeStrings* GetTypeStr(void) const {
        return m_TypeStr;
    }

    bool IsPrimitive(void) const;
    bool IsStdType(void) const;
    bool IsReference(void) const;

    void SetIsAlias(bool value) {
        m_IsAlias = value;
    }
    bool IsAlias(void) const {
        return m_IsAlias;
    }
    // used when generating code, to provide backward compatibility
    void SetIsTypeAlias(bool value) const {
        m_IsTypeAlias = value;
    }
    bool IsTypeAlias(void) const {
        return m_IsTypeAlias;
    }

    static void EnableDTDEntities(bool enable = true) {
        sm_EnableDTDEntities = enable;
    }
    static bool DTDEntitiesEnabled(void) {
        return sm_EnableDTDEntities;
    }
    static void SetEnforcedStdXml(bool set = true) {
        sm_EnforcedStdXml = set;
    }
    static bool GetEnforcedStdXml(void) {
        return sm_EnforcedStdXml;
    }
    static void SetSourceDataSpec(EDataSpec spec) {
        sm_SourceDataSpec = spec;
    }
    static EDataSpec GetSourceDataSpec(void) {
        return sm_SourceDataSpec;
    }
    static string GetSourceDataSpecString(void);
    static bool IsASNDataSpec(void) {
        return sm_SourceDataSpec == EDataSpec::eASN;
    }
    static bool IsXMLDataSpec(void) {
        return sm_SourceDataSpec == EDataSpec::eDTD || sm_SourceDataSpec == EDataSpec::eXSD;
    }

    virtual const char* GetDEFKeyword(void) const;
    const string& GetMemberName(void) const
    {
        return m_MemberName;
    }

    void SetNamespaceName(const string& name)
    {
        m_Namespace = name;
    }
    const string& GetNamespaceName(void) const
    {
        return m_Namespace;
    }
    void SetNsQualified(bool qualified)
    {
        m_NsQualified = qualified ? eNSQualified : eNSUnqualified;
    }
    ENsQualifiedMode IsNsQualified(void) const
    {
        return m_NsQualified;
    }
    void SetNillable(void)
    {
        m_IsNillable = true;
    }
    bool IsNillable(void) const
    {
        return m_IsNillable;
    }
    enum EGlobalType {
        eElement = 0,
        eType,
        eGroup
    };
    void SetGlobalType(EGlobalType type) {
        m_GlobalType = type;
    }
    EGlobalType GetGlobalType(void) const {
        return m_GlobalType;
    }
    void SetEmptyExternalName(bool set) {
        m_EmptyExternalName = set;
    }
    bool HasExternalName(void) const {
        return !m_EmptyExternalName;
    }
    string GetFullName(void) const;
    void SetRestrictions(const list<CMemberFacet>& c) {
        m_Restrictions = c;
    }
    const list<CMemberFacet>& GetRestrictions(void) const {
        return m_Restrictions;
    }

protected:
    static bool x_IsSavedName(const string& name);
    static void x_AddSavedName(const string& name);
    void x_SetMemberAndClassName(const string& memberName);

private:
    const CDataType* m_ParentType;       // parent type
    const CDataTypeModule* m_Module;
    string m_MemberName;
    string m_ClassName;
    string m_XmlName;
    int m_SourceLine;
    CComments m_Comments;
    CDataMember* m_DataMember;
    mutable CClassTypeStrings* m_TypeStr;

    // tree info
    const CUniSequenceDataType* m_Set;
    const CChoiceDataType* m_Choice;
    AutoPtr<TReferences> m_References;

    bool m_Checked;

    CTypeRef m_TypeRef;
    AutoPtr<CTypeInfo> m_AnyTypeInfo;
    AutoPtr<CTypeInfo> m_RealTypeInfo;
    static set<string, PNocase> sm_AllFileNames;
    mutable string m_CachedFileName;
    mutable unique_ptr<CNamespace> m_CachedNamespace;
    CAsnBinaryDefs::TLongTag  m_Tag;
    CAsnBinaryDefs::ETagClass m_TagClass;
    CAsnBinaryDefs::ETagType  m_TagType;
    bool m_IsAlias;
    mutable bool m_IsTypeAlias;
    multimap<string,string> m_ForbidVar;
    string m_Namespace;
    ENsQualifiedMode m_NsQualified;
    bool m_IsNillable;
    EGlobalType m_GlobalType;
    bool m_EmptyExternalName;
    list<CMemberFacet> m_Restrictions;

    CDataType(const CDataType&);
    CDataType& operator=(const CDataType&);
    static bool sm_EnableDTDEntities;
    static bool sm_EnforcedStdXml;
    static EDataSpec sm_SourceDataSpec;
    static set<string> sm_SavedNames;
    static map<string,string> sm_ClassToMember;
};

#define CheckValueType(value, type, name) do{ \
if ( dynamic_cast<const type*>(&(value)) == 0 ) { \
    (value).Warning(name " value expected", 1); return false; \
} } while(0)

END_NCBI_SCOPE

#endif
