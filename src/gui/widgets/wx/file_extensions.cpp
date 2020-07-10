/*  $Id: file_extensions.cpp 44436 2019-12-18 17:13:59Z katargir $
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

#include <gui/widgets/wx/file_extensions.hpp>

BEGIN_NCBI_SCOPE

void sGetExtensions(CFileExtensions::EFileType fileType, vector<wxString>& exts)
{
    switch (fileType) {
    case CFileExtensions::kASN :
        exts.push_back(wxT("asn"));
        exts.push_back(wxT("asnb"));
        exts.push_back(wxT("asnt"));
		exts.push_back(wxT("asn1"));
        exts.push_back(wxT("aso"));
        exts.push_back(wxT("ent"));
        exts.push_back(wxT("sqn"));
        exts.push_back(wxT("bss"));
        break;
    case CFileExtensions::kFASTA :
        exts.push_back(wxT("fa"));
        exts.push_back(wxT("mpfa"));
        exts.push_back(wxT("fna"));
        exts.push_back(wxT("fsa"));
        exts.push_back(wxT("fas"));
		exts.push_back(wxT("faa"));
		exts.push_back(wxT("fnn"));
        exts.push_back(wxT("fasta"));
        break;
    case CFileExtensions::kGenBank :
        exts.push_back(wxT("gb"));
        exts.push_back(wxT("gen"));
        exts.push_back(wxT("GEN"));
        break;
    case CFileExtensions::kAGP :
        exts.push_back(wxT("agp"));
        exts.push_back(wxT("AGP"));
        break;
    case CFileExtensions::kNewick :
        exts.push_back(wxT("phy"));
		exts.push_back(wxT("tre"));
		exts.push_back(wxT("nwk"));
        break;
    case CFileExtensions::kNexus :
        exts.push_back(wxT("nex"));
		exts.push_back(wxT("nxs"));
        break;
    case CFileExtensions::kTree :
        sGetExtensions(CFileExtensions::kNewick, exts);
        sGetExtensions(CFileExtensions::kNexus, exts);
        break;
    case CFileExtensions::kGFF :
        exts.push_back(wxT("gff"));
        exts.push_back(wxT("gvf"));
        break;
    case CFileExtensions::kGTF :
        exts.push_back(wxT("gtf"));
        break;
    case CFileExtensions::kVCF :
        exts.push_back(wxT("vcf"));
        break;
    case CFileExtensions::kBED :
        exts.push_back(wxT("bed"));
        break;
    case CFileExtensions::kWIG :
        exts.push_back(wxT("wig"));
        break;
    case CFileExtensions::kTxt :
        exts.push_back(wxT("txt"));
        break;
    case CFileExtensions::kTable :
        exts.push_back(wxT("txt"));
        exts.push_back(wxT("csv"));
        break;
    case CFileExtensions::k5Column :
        exts.push_back(wxT("tbl"));
        break;
    case CFileExtensions::kTextAlign :
        exts.push_back(wxT("txt"));
        exts.push_back(wxT("aln"));
        exts.push_back(wxT("afa"));
        exts.push_back(wxT("mfa"));
        break;
    case CFileExtensions::kPSL :
        exts.push_back(wxT("psl"));
        break;
    case CFileExtensions::kAllFiles :
        exts.push_back("*");
        break;
    default:
        break;
    }
}

static wxString sBuildFilter(const vector<wxString>& exts)
{
    wxString filter;
    vector<wxString>::const_iterator it;
    for (it = exts.begin(); it != exts.end(); it++) { 
        if (it != exts.begin())
            filter += wxT(";");

#ifdef NCBI_OS_MSWIN
        filter += wxT("*.") + *it;
#else
        if (*it == "*")
            filter += *it;
        else
            filter += wxT("*.") + *it;
#endif
    }
    return filter;
}

wxString CFileExtensions::GetLabel(EFileType fileType)
{
    switch (fileType) {
    case kASN :
        return wxT("ASN.1 files");
    case kFASTA :
        return wxT("FASTA files");
    case kGenBank :
        return wxT("GenBank files");
    case kAGP :
        return wxT("AGP assembly files");
    case kNewick :
        return wxT("Newick Tree files");
    case kNexus :
        return wxT("Nexus Tree files");
    case kTree :
        return wxT("Newick/Nexus Tree files");
    case CFileExtensions::kGFF :
        return wxT("GFF/GVF files");
    case CFileExtensions::kGTF :
        return wxT("GTF files");
    case CFileExtensions::kVCF :
        return wxT("VCF (Variant Call Format) files");
    case CFileExtensions::kBED :
        return wxT("BED files");
    case CFileExtensions::kWIG :
        return wxT("WIG files");
    case kTxt :
        return wxT("Text files");
    case kTable :
        return wxT("Table files");
    case k5Column :
        return wxT("5 Column Feature files");
    case kTextAlign :
        return wxT("FASTA Alignment files");
    case kPSL :
        return wxT("PSL Alignment files");
    case kAllFiles :
    default:
        return wxT("All files");
    }
}

wxString CFileExtensions::GetExtensions(EFileType fileType)
{
    vector<wxString> exts;
    sGetExtensions(fileType, exts);
    return sBuildFilter(exts);
}

wxString CFileExtensions::GetDefaultExtension(EFileType fileType)
{
    vector<wxString> exts;
    sGetExtensions(fileType, exts);
    if (!exts.empty())
        return exts[0];
    else
        return wxString();
}

void CFileExtensions::AppendDefaultExtension(EFileType fileType, wxString &path)
{
    size_t extension_idx = path.find_last_of(wxT("."));
    if (extension_idx != std::string::npos)
        return;

    wxString extension = GetDefaultExtension(fileType);
    if (extension.empty())
        return;

    path += '.';
    path += extension;
}


wxString CFileExtensions::GetDialogFilter(EFileType fileType)
{
    wxString extensions = GetExtensions(fileType);
    return GetLabel(fileType) + wxT(" (") + extensions + wxT(")|") + extensions;
}

bool CFileExtensions::RecognizeExtension(EFileType fileType, const wxString& ext)
{
    if (fileType == kAllFiles)
        return true;

    vector<wxString> exts;
    sGetExtensions(fileType, exts);
    return find(exts.begin(), exts.end(), ext) != exts.end();
}

END_NCBI_SCOPE
