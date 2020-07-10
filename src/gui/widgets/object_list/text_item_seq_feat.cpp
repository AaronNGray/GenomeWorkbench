/*  $Id: text_item_seq_feat.cpp 32647 2015-04-07 16:12:16Z katargir $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include "text_item_seq_feat.hpp"

#include <gui/objutils/label.hpp>
#include <objmgr/util/sequence.hpp>

#include <objects/general/Object_id.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>

#include <gui/objutils/utils.hpp>

#include <objtools/format/flat_file_config.hpp>
#include <objtools/format/flat_file_generator.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

ITextItem* CTextItemSeq_feat::CreateObject(SConstScopedObject& object, EMode mode)
{
    return new CTextItemSeq_feat(object, mode);
}

static void s_RenderAccList(
    CStyledTextOStream& ostream, const CUser_field::TData::TStrs& item_data, const string& title)
{
    string s(title);
    if ( !s.empty() ) {
        s += ": ";
    }

    list<string> refseq;
    list<string> non_refseq;
    ITERATE (CUser_field::TData::TStrs, it, item_data) {
        CSeq_id::EAccessionInfo info = CSeq_id::IdentifyAccession(*it);
        if ((info & CSeq_id::eAcc_type_mask) == CSeq_id::e_Other) {
            refseq.push_back(*it);
        } else {
            non_refseq.push_back(*it);
        }
    }

    string accs;
    if (refseq.size()) {
        accs += NStr::Join(refseq, ", ");
    }

    if (non_refseq.size()) {
        if ( !accs.empty() ) {
            accs += ", ";
        }
        accs += NStr::Join(non_refseq, ", ");
    }

    if ( !accs.empty() ) {
        s += accs;
    } else {
        s += "<unknown>";
    }

    list<string> lines;
    NStr::Wrap(s, 78, lines);
    if ( !lines.empty() ) {
        ITERATE (list<string>, iter, lines) {
            if (iter != lines.begin()) {
                ostream << "    " << *iter << NewLine();
            } else {
                ostream << *iter << NewLine();
            }
        }
    }
}

static void s_Render_Feat_Evidence(
    CStyledTextOStream& ostream, const CSeq_feat& feat, CScope& /*scope*/)
{
    if ( !feat.IsSetExt()  &&  !feat.IsSetExts()) {
        return;
    }

    list< CConstRef<CUser_object> > objs;
    if (feat.IsSetExt()) {
        CConstRef<CUser_object> o(&feat.GetExt());
        if (o->IsSetType()  &&  o->GetType().IsStr()  && o->GetType().GetStr() == "CombinedFeatureUserObjects") {
            ITERATE (CUser_object::TData, it, o->GetData()) {
                const CUser_field& field = **it;
                CConstRef<CUser_object> sub_o(&field.GetData().GetObject());
                objs.push_back(sub_o);
            }
        } else {
            objs.push_back(o);
        }
    }
    if (feat.IsSetExts()) {
        copy(feat.GetExts().begin(), feat.GetExts().end(), back_inserter(objs));
    }

    ITERATE (list< CConstRef<CUser_object> >, obj_it, objs) {
        const CUser_object& obj = **obj_it;
        if ( !obj.IsSetType()  ||
             !obj.GetType().IsStr()  ||
             obj.GetType().GetStr() != "ModelEvidence") {
            continue;
        }

        string s;
        CConstRef<CUser_field> f;

        ostream << NewLine();

        ///
        /// generation method
        string method;
        ostream << "Model Generation Method: ";
        f = obj.GetFieldRef("Method");
        if (f) {
            method = f->GetData().GetStr();
            ostream << method;
        } else {
            ostream << "<unknown>";
        }

        ostream << NewLine();

        ///
        /// list of support
        if (method == "Chainer"  ||  method == "Gnomon") {

            ///
            /// scan for multiple 'Core' entries
            f = obj.GetFieldRef("Support");
            if (f) {
                string core;
                ITERATE (CUser_field::TData::TFields, it, f->GetData().GetFields()) {
                    const CUser_field& this_f = **it;
                    if (this_f.GetLabel().GetStr() == "Core") {
                        if ( !core.empty() ) {
                            core += ", ";
                        }
                        core += this_f.GetData().GetStr();
                    }
                }
                if ( !core.empty() ) {
                    s = "Model Based On: ";
                    s += core;
                    ostream << s << NewLine();
                }
            }

            ///
            /// supporting proteins
            f = obj.GetFieldRef("Support.Proteins");
            if (f) {
                s_RenderAccList(ostream, f->GetData().GetStrs(), "Source Proteins");
            }

            ///
            /// supporting proteins
            f = obj.GetFieldRef("Support.mRNAs");
            if (f) {
                s_RenderAccList(ostream, f->GetData().GetStrs(), "Source mRNAs");
            }
        }
    }
}

static void s_Render_SeqFeat(
    CStyledTextOStream& ostream, const CSeq_feat& feat, CScope& scope)
{
    try {
        string text;
        /// start with the description text
        CLabel::GetLabel( feat, &text, CLabel::eDescription, &scope );
        ostream.WriteMultiLineText(text);

        /// add information about the feature's location
        TSeqRange range = feat.GetLocation().GetTotalRange();
        ostream << "Total Range:  ";
        switch( sequence::GetStrand(feat.GetLocation(), &scope) ){
        case eNa_strand_plus:
            ostream << "(+) ";
            break;
        case eNa_strand_minus:
            ostream << "(-) ";
            break;
        default:
            break;
        }

        ostream << NStr::IntToString(range.GetFrom() + 1, NStr::fWithCommas)
                << " - "
                << NStr::IntToString(range.GetTo() + 1, NStr::fWithCommas)
                << NewLine();

        ostream << "Total Length:  "
                << NStr::IntToString(range.GetLength(), NStr::fWithCommas)
                << NewLine();

        ostream << "Processed Length:  "
                << NStr::IntToString(sequence::GetLength(feat.GetLocation(), &scope),
                                  NStr::fWithCommas)
                << NewLine();

        if (feat.IsSetProduct()) {
            try {
                string prod_len_str;
                CBioseq_Handle h = scope.GetBioseqHandle(feat.GetProduct());
                if (h) {
                    ostream << "Product Length: "
                            << NStr::IntToString(h.GetBioseqLength(),
                                              NStr::fWithCommas)
                            << NewLine();
                }
            }
            catch (CException&) {
            }
        }
        ostream << NewLine();

        /// next, include the flat file text for this feature
        CFlatFileConfig cfg;
        cfg.SetNeverTranslateCDS();

        /// hack: search for this feature using CFeat_CI
        /// it'd be better to use CMappedFeat instead of CSeq_feat
        SAnnotSelector sel = CSeqUtils::GetAnnotSelector();
        sel.IncludeFeatSubtype(feat.GetData().GetSubtype());

        CFeat_CI feat_it(scope, feat.GetLocation(), sel);
        for (; feat_it ; ++feat_it) {
            if (&feat_it->GetOriginalFeature() != &feat){
                continue;
            }
            string s =
                CFlatFileGenerator::GetSeqFeatText(*feat_it, scope, cfg);
            string::size_type pos = s.find_first_not_of(" \n\r\t");
            if( pos != string::npos ){
                s.erase(0, pos);
            }

            string::size_type lpos = string::npos;
            do {
                pos = lpos +1;
                lpos = s.find_first_of("\n\r", pos );

                if( lpos == string::npos ){
                    text = s.substr( pos );
                } else {
                    text = s.substr( pos, lpos - pos );
                }
                ostream << text << NewLine();

            } while( lpos != string::npos );
        } // for

        /// lastly, add items relating to model evidence
        s_Render_Feat_Evidence(ostream, feat, scope);
    }
    catch (CException&) {
    }
}

void CTextItemSeq_feat::x_RenderText(CStyledTextOStream& ostream, CTextPanelContext*) const
{
    x_Indent(ostream);

    if (m_Mode == kBrief) {
        string text;
        CLabel::GetLabel(*m_Object, &text, CLabel::eDescription, m_Scope);
        ostream.WriteMultiLineText(text);
        return;
    }

    if (m_Mode != kFull)
        return;

    const objects::CSeq_feat* seq_feat = 
        dynamic_cast<const objects::CSeq_feat*>(m_Object.GetPointer());
    if (!seq_feat) {
        ostream 
            << CTextStyle(255,0,0,255,255,255,wxTRANSPARENT,false,false)
            << "Invalid object: should be Seq-feat";
        return;
    }

    s_Render_SeqFeat(ostream, *seq_feat, *m_Scope);
}

END_NCBI_SCOPE
