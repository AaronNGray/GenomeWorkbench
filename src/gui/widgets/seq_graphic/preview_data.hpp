const char * s_PreviewData[] = {
    "Seq-entry ::= set { seq-set { seq { id {",
    " local str \"fake_nuc\" }, inst { repr raw, mol dna,",
    " length 200,",
    " seq-data ncbi2na '67C6697351FF4AEC29CDBAABF2FBE3467CC254F81BE8E78D765A",
    "2E63339FC99A66320DB73158A35A255D051758E95ED4ABB2'H } },",
    " seq { id { local str \"fake_prot\" }, inst { repr raw,",
    " mol aa, length 150,",
    " seq-data iupacaa \"ARNDCEQGHILKMFPSTWYVARNDCEQGHILKMFPSTWYVARNDCEQGHILK",
    "MFPSTWYVARNDCEQGHILKMFPSTWYVARNDCEQGHILKMFPSTWYVARNDCEQGHILKMFPSTWYVARNDCEQGHI",
    "LKMFPSTWYVARNDCEQGHI\" } } }, annot { { data ftable { {",
    " data gene { locus \"My Gene\" }, location int { from 25,",
    " to 175, id local str \"fake_nuc\" } }, { data rna {",
    " type mRNA, ext name \"Undefined transcription factor\" },",
    " location mix { int { from 25, to 75,",
    " id local str \"fake_nuc\" }, int { from 100, to 175,",
    " id local str \"fake_nuc\" } } }, { data cdregion {",
    " frame one, code { id 1 } },",
    " product whole local str \"fake_prot\", location mix { int {",
    " from 25, to 75, id local str \"fake_nuc\" }, int {",
    " from 100, to 175, id local str \"fake_nuc\" } } }, {",
    " data imp { key \"STS\" }, comment \"Fake STS\",",
    " location int { from 30, to 120, strand plus,",
    " id local str \"fake_nuc\" } }, { data imp {",
    " key \"misc_feature\" }, comment \"Fake misc feature\",",
    " location int { from 50, to 160, strand plus,",
    " id local str \"fake_nuc\" } } } } }}",
    0
};
/*
 * The preceding array initializer was generated by the script text2cstr.pl
 * with the following options:
 *   -s -l 60 -v s_PreviewData preview_data.asn
 */
