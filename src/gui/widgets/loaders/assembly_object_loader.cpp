/*  $Id: assembly_object_loader.cpp 41353 2018-07-12 18:47:59Z katargir $
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
*  and reliability of the software and data,  the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties,  express or implied,  including
*  warranties of performance,  merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Authors:  Roman Katargin
*/


#include <ncbi_pch.hpp>

#include <gui/widgets/loaders/assembly_object_loader.hpp>

#include <util/icanceled.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/label.hpp>

#include <objmgr/object_manager.hpp>

#include <objects/genomecoll/GC_AssemblySet.hpp>
#include <objects/genomecoll/GC_AssemblyDesc.hpp>
#include <objects/genomecoll/GC_Assemblies.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seq/Seq_descr.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CAssemblyObjectLoader::CAssemblyObjectLoader(const vector<string>& accessions)
 : m_Accessions(accessions)
{
    m_Descr = "Retrieving Gencoll assemblies: ";
    for (size_t i = 0; i < m_Accessions.size(); ++i) {
        if (i > 0) {
            m_Descr += ", ";
        }
        m_Descr += m_Accessions[i];
    }
}

IObjectLoader::TObjects& CAssemblyObjectLoader::GetObjects()
{
    return m_Objects;
}

string CAssemblyObjectLoader::GetDescription() const
{
    return m_Descr;
}

bool CAssemblyObjectLoader::PreExecute()
{
    return true;
}

bool CAssemblyObjectLoader::Execute(ICanceled& canceled)
{
    CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();
    CRef<CScope> scope(new CScope(*obj_mgr));
    scope->AddDefaults();

    string currAcc;
    CGencollSvc::TAssmAccs AssmAccs(m_Accessions.begin(), m_Accessions.end());
    CGC_Assemblies Assemblies;
    CGencollSvc::GetAssmsInfo(AssmAccs, "Gbench", Assemblies.SetAssemblies(), this);

    for(auto iAssemblies: Assemblies.GetAssemblies()) {
        if (canceled.IsCanceled()) {
            return false;
        }
        CRef<CGC_Assembly>& gc_assm(iAssemblies);
        currAcc = gc_assm->GetAccession();

		try {
			string label;
			CLabel::GetLabel(*gc_assm, &label, CLabel::eDefault, scope);
            if (label.empty())
                label = currAcc;

            string descr = "Name:  " + label;
            if (gc_assm->IsAssembly_set()) {
                CGC_AssemblySet::TSet_type type = gc_assm->GetAssembly_set().GetSet_type();
                if (type == CGC_AssemblySet::eSet_type_full_assembly) {
                    descr += " (full assembly)";
                } else if (type == CGC_AssemblySet::eSet_type_assembly_set) {
                    descr += " (assembly set)";
                }
            }

            list< CRef<CDbtag> > ids;
            CConstRef<CGC_AssemblyDesc> desc;

            if (gc_assm->IsAssembly_set()) {
                desc.Reset(&gc_assm->GetAssembly_set().GetDesc());
                ids = gc_assm->GetAssembly_set().GetId();
            } else if (gc_assm->IsUnit()) {
                desc.Reset(&gc_assm->GetUnit().GetDesc());
                ids = gc_assm->GetUnit().GetId();
            }

            ITERATE (list< CRef<CDbtag> >, iter, ids) {
                if ((*iter)->GetDb() == "GenColl"  &&  (*iter)->GetTag().IsStr()) {
                    descr += "\nAccession:  " + (*iter)->GetTag().GetStr();
                }
                if ((*iter)->GetDb() == "UCSC_name"  &&  (*iter)->GetTag().IsStr()) {
                    descr += "\nUCSC name:  " + (*iter)->GetTag().GetStr();
                }
            }
            descr += "\nRelease type:  ";
            descr += gc_assm->IsRefSeq() ? "RefSeq" : "Genbank";
            if (desc) {
                if (desc->IsSetLong_name()) {
                    descr += "\nDescription:  " + desc->GetLong_name();
                }
                if (desc->IsSetDescr()) {
                    ITERATE (CGC_AssemblyDesc::TDescr::Tdata, it, desc->GetDescr().Get()) {
                        if ((*it)->IsSource()) {
                            descr += "\nOrganism:  ";
                            if ((*it)->GetSource().IsSetTaxname()) {
                                descr += (*it)->GetSource().GetTaxname();
                            }
                            if ((*it)->GetSource().IsSetCommon()) {
                                descr += " (" + (*it)->GetSource().GetCommon() + ")";
                            }
                        }
                    }
                }
            }

            if (label != currAcc) {
                label += " (";
                label += currAcc;
                label += ')';
            }
            m_Objects.push_back(SObject(*gc_assm, label, descr));
		}
        catch (const CException& e) {
            x_UpdateHTMLResults(currAcc, 0, e.GetMsg(), "", wxT("Accession:"));
        }
        catch (const exception& e) {
            x_UpdateHTMLResults(currAcc, 0, e.what(), "", wxT("Accession:"));
        }
	}

    return true;
}

bool CAssemblyObjectLoader::PostExecute()
{
    x_ShowErrorsDlg(wxT("Assembly import errors"));
    return true;
}

void CAssemblyObjectLoader::ReportError(const string &accession, const string &error)
{
    x_UpdateHTMLResults(accession, 0, error, "", wxT("Accession:"));
}

END_NCBI_SCOPE
