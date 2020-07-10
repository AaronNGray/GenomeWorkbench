#ifndef GUI_WIDGET_SEQ_GRAPHICS___ALIGNMENT_SORTER_BY_FIELD__HPP
#define GUI_WIDGET_SEQ_GRAPHICS___ALIGNMENT_SORTER_BY_FIELD__HPP

/*  $Id:
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
 * Authors:  Vladislav Evgeniev
 *
 * File Description: Defines a template class, that sorts alignments by Tracebacks' field value
 *
 */


#include <corelib/ncbistd.hpp>
#include <sstream>  
#include <map>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <gui/widgets/seq_graphic/feature_sorter.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CAlignSorterByField is a sorter used to sort alignments based on user fields
///
template <typename T>
class CAlignSorterByField
    : public CObject
    , public IAlignSorter
{
public:
    const int NotSet = 0xffff;

    CAlignSorterByField(std::string const& field, std::string const& id, const std::string& group_name, std::string const& description) : m_Field(field), m_Id(id), m_GroupName(group_name), m_Description(description) {}

    virtual std::string GroupIdxToName(int idx) const
    {
        std::stringstream name;
        name << m_GroupName << ' ';
        if (NotSet != idx) {
            for (auto const& group : m_SortGroups) {
                if (group.second != idx)
                    continue;
                name << group.first;
                break;
            }
        }
        else {
            name << "not set";
        }
        return name.str();
    }

    virtual int GetGroupIdx(const objects::CSeq_align& align) const
    {
        do {
            try {
                CConstRef<objects::CUser_object> tracebacks = align.FindExt("Tracebacks");
                if (!tracebacks)
                    break;

                if (!x_HasData(*tracebacks))
                    break;

                T data = x_GetData(*tracebacks);
                auto it = m_SortGroups.find(data);
                if (m_SortGroups.end() != it)
                    return it->second;

                int idx = (int)m_SortGroups.size();
                m_SortGroups[data] = idx;
                return idx;
            }
            catch (const CException&) {
                break;
            }
        } while (false);
        return NotSet;
    }

    virtual SSorterDescriptor GetThisDescr() const
    {
        return SSorterDescriptor(m_Id, m_GroupName, m_Description);
    }

protected:
    bool x_HasData(objects::CUser_object const& objects) const;
    T x_GetData(objects::CUser_object const& objects) const;

private:
    /// Curently intended sorting groups.
    /// Any feature that doesn't have the field set will be assigned to an invalid group index
    mutable std::map<T, int> m_SortGroups;
    const std::string      m_Field;
    const std::string      m_Id;
    const std::string      m_GroupName;
    const std::string      m_Description;
};

template <>
bool CAlignSorterByField<std::string>::x_HasData(objects::CUser_object const& objects) const
{
    return (objects.HasField(m_Field) && objects.GetField(m_Field).GetData().IsStr());
}

template <>
bool CAlignSorterByField<int>::x_HasData(objects::CUser_object const& objects) const
{
    return (objects.HasField(m_Field) && objects.GetField(m_Field).GetData().IsInt());
}

template <>
bool CAlignSorterByField<bool>::x_HasData(objects::CUser_object const& objects) const
{
    return (objects.HasField(m_Field) && objects.GetField(m_Field).GetData().IsBool());
}

template <>
std::string CAlignSorterByField<std::string>::x_GetData(objects::CUser_object const& objects) const
{
    return objects.GetField(m_Field).GetData().GetStr();
}

template <>
int CAlignSorterByField<int>::x_GetData(objects::CUser_object const& objects) const
{
    return objects.GetField(m_Field).GetData().GetInt();
}

template <>
bool CAlignSorterByField<bool>::x_GetData(objects::CUser_object const& objects) const
{
    return objects.GetField(m_Field).GetData().GetBool();
}

END_NCBI_SCOPE

#endif  // GUI_WIDGET_SEQ_GRAPHICS___ALIGNMENT_SORTER_BY_FIELD__HPP
