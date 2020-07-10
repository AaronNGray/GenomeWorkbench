/*  $Id: dump_sysinfo.cpp 44798 2020-03-18 19:44:26Z evgeniev $
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
 *    OpenGL utility functions
 */


#include <ncbi_pch.hpp>
#include <corelib/ncbifile.hpp>
#include <corelib/ncbi_system.hpp>
#include <gui/opengl.h>
#include <gui/widgets/feedback/dump_sysinfo.hpp>

#ifdef NCBI_OS_UNIX
#include <connect/ncbi_pipe.hpp>
#endif

#ifdef NCBI_OS_DARWIN
#include <CoreServices/CoreServices.h>
#import <Foundation/Foundation.h>
#import <Foundation/NSProcessInfo.h>

size_t GetPhysicalMemory(void)
{
    return [NSProcessInfo processInfo].physicalMemory;
}
#endif

#if defined(__WXGTK20__) || defined(__WXGTK__)
extern "C"
{
#include <gdk/gdkx.h>
#include <GL/glx.h>
}
#endif

#include <wx/gdicmn.h>
#include <wx/utils.h>

#include <connect/ncbi_socket.hpp>

BEGIN_NCBI_SCOPE

void CDumpSysInfo::DumpSystemInfo(CNcbiOstream& ostr)
{
    ///
    /// build our system info
    ///

    ostr << "System Info" << endl << string(72, '-') << endl << endl;
    {{

#ifdef NCBI_OS_UNIX
        ostr << "Host info: ";
        /// system info
        CPipe pipe;
        vector<string> args;
        args.push_back("-a");
        pipe.Open("uname", args);

        char buf[128];
        size_t bytes;
        while( pipe.Read(buf, 128, &bytes) == eIO_Success ){
            ostr << string( buf, bytes );
        }
#endif

        ostr << "Number of CPUs: ";
        ostr << CSystemInfo::GetCpuCount();
        ostr << endl;

#ifdef NCBI_OS_DARWIN
        size_t ram_size = GetPhysicalMemory();

        ostr << "RAM size: "  << ram_size / 1048576 << "MB" << endl;

        ostr << "OS Version: " << wxGetOsDescription().ToUTF8() << endl;
#endif


#ifdef NCBI_OS_MSWIN
        SYSTEM_INFO sysinfo;
        memset(&sysinfo, 0, sizeof(sysinfo));

        typedef void (WINAPI *PGetSysInfo)(SYSTEM_INFO*);

        PGetSysInfo pGetSysInfo = (PGetSysInfo)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
        if( pGetSysInfo != NULL ){
            pGetSysInfo( &sysinfo );
        } else {
            GetSystemInfo( &sysinfo );
        }

        ostr << "Processor type: ";
        switch (sysinfo.wProcessorArchitecture) {
        default:
        case PROCESSOR_ARCHITECTURE_UNKNOWN:
            ostr << "unknown" << endl;
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            ostr << "Intel x32" << endl;
            break;
        case PROCESSOR_ARCHITECTURE_AMD64:
            ostr << "Intel x64" << endl;
            break;
        case PROCESSOR_ARCHITECTURE_IA64:
            ostr << "Itanium" << endl;
            break;
        }
        ostr << "Processor level: "
            << sysinfo.wProcessorLevel << endl
        ;
        ostr << "Processor revision: "
             << hex << sysinfo.wProcessorRevision << dec << endl
        ;

        ostr << "Page size: " << sysinfo.dwPageSize << endl;

        DWORDLONG ramSize = 0;
        typedef BOOL (WINAPI *PGMSE)(LPMEMORYSTATUSEX);
        PGMSE pGMSE = (PGMSE)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GlobalMemoryStatusEx");
        if (pGMSE) {
            MEMORYSTATUSEX mi;
            memset(&mi, 0, sizeof(MEMORYSTATUSEX));
            mi.dwLength = sizeof(MEMORYSTATUSEX);
            if (pGMSE(&mi))
                ramSize = mi.ullTotalPhys;
        }
        if (ramSize == 0) {
            MEMORYSTATUS mi;
            memset(&mi, 0, sizeof(MEMORYSTATUS));
            mi.dwLength = sizeof(MEMORYSTATUS);
            GlobalMemoryStatus(&mi);
            ramSize = mi.dwTotalPhys;
        }
        ostr << "RAM size: " << (ramSize / 1024 / 1024) << "MB" << endl;

        ostr << "OS Version: " << wxGetOsDescription().ToUTF8() << endl;

        ostr << "Number of monitors: " << GetSystemMetrics(SM_CMONITORS) << endl;
        ostr << "Base Monitor Resolution: "
            << GetSystemMetrics(SM_CXSCREEN) << "x"
            << GetSystemMetrics(SM_CYSCREEN) << endl
        ;
        ostr << "Virtual Monitor Resolution: "
            << GetSystemMetrics(SM_CXVIRTUALSCREEN) << "x"
            << GetSystemMetrics(SM_CYVIRTUALSCREEN) << endl
        ;
        ostr << "Network detected: "
            << (GetSystemMetrics(SM_NETWORK) & 0x01 ? "yes" : "no") << endl
        ;
        ostr << "Remote Session: "
            << (GetSystemMetrics(SM_REMOTESESSION) ? "yes" : "no") << endl
        ;
#endif

#ifdef NCBI_OS_LINUX
        ostr << "Linux Distribution: " 
            << wxGetLinuxDistributionInfo().Description.ToUTF8() << endl << endl;

        typedef pair<const char*, const char*> TPair;
        static TPair s_ProcEntries[] = {
            TPair("CPU Information",    "/proc/cpuinfo"),
            TPair("Memory Information", "/proc/meminfo"),
        };

        for( size_t i = 0;  i < sizeof(s_ProcEntries) / sizeof (TPair);  ++i ){
            CNcbiIfstream istr(s_ProcEntries[i].second);
            if (istr.good()) {
                ostr << s_ProcEntries[i].first << ":" << endl;
                string line;
                while (NcbiGetlineEOL(istr, line)) {
                    ostr << "     " << line << endl;
                }
            }
        }
#endif
        ostr << "IP address: ";
        
        unsigned int host = CSocketAPI::GetLocalHostAddress();
        ostr << CSocketAPI::ntoa( host );
        ostr << endl;

        ostr << endl << endl;
    }}

    ostr << "Graphics Subsystem Info" << endl << string(72, '-') << endl << endl;
    int x, y, w, h;
    wxClientDisplayRect(&x, &y, &w, &h);
    ostr << "Root window size: " << w << " x " << h << endl;

    {{
         const GLubyte* na_text = (const GLubyte*)"NOT AVAILABLE";
         const GLubyte* ptr = glGetString(GL_VENDOR);
         if ( !ptr ) {
             ptr = na_text;
         }
         ostr << "GL_VENDOR:   " << ptr << endl;

         ptr = glGetString(GL_RENDERER);
         if ( !ptr ) {
             ptr = na_text;
         }
         ostr << "GL_RENDERER: " << ptr << endl;

         ptr = glGetString(GL_VERSION);
         if ( !ptr ) {
             ptr = na_text;
         }
         ostr << "GL_VERSION:  " << ptr << endl;

         ptr = glGetString(GLU_VERSION);
         if ( !ptr ) {
             ptr = na_text;
         }
         ostr << "GLU_VERSION: " << ptr << endl;

#if defined(__WXGTK20__) || defined(__WXGTK__)
        int glxMajorVer = 1, glxMinorVer = 0;
        glXQueryVersion(GDK_DISPLAY(), &glxMajorVer, &glxMinorVer);
        ostr << "GLX Version: " << glxMajorVer << "." << glxMinorVer << endl;
#endif
         ptr = glGetString(GL_EXTENSIONS);
         if (ptr) {
             string s(reinterpret_cast<const char*>(ptr));
             list<string> toks;
             NStr::Split(s, " ", toks, NStr::fSplit_Tokenize);
             ostr << "GL_EXTENSIONS:" << endl;
             ITERATE (list<string>, iter, toks) {
                 ostr << "     " << *iter << endl;
             }
         }

         ptr = gluGetString(GLU_EXTENSIONS);
         if (ptr) {
             string s(reinterpret_cast<const char*>(ptr));
             list<string> toks;
             NStr::Split(s, " ", toks, NStr::fSplit_Tokenize);
             ostr << "GLU_EXTENSIONS:" << endl;
             ITERATE (list<string>, iter, toks) {
                 ostr << "     " << *iter << endl;
             }
         }
     }}

    {{
         typedef pair<const char*, GLenum> TPair;
         static const TPair s_EnumPairs[] = {
             TPair("GL_DOUBLEBUFFER", GL_DOUBLEBUFFER),
             TPair("GL_RED_BITS", GL_RED_BITS),
             TPair("GL_GREEN_BITS", GL_GREEN_BITS),
             TPair("GL_BLUE_BITS", GL_BLUE_BITS),
             TPair("GL_ALPHA_BITS", GL_ALPHA_BITS),
             TPair("GL_DEPTH_BITS", GL_DEPTH_BITS),
             TPair("GL_MAX_ATTRIB_STACK_DEPTH", GL_MAX_ATTRIB_STACK_DEPTH),
             TPair("GL_MAX_LIGHTS", GL_MAX_LIGHTS),
             TPair("GL_MAX_LIST_NESTING", GL_MAX_LIST_NESTING),
             TPair("GL_MAX_PROJECTION_STACK_DEPTH", GL_MAX_PROJECTION_STACK_DEPTH),
             TPair("GL_MAX_TEXTURE_SIZE", GL_MAX_TEXTURE_SIZE),
             TPair("GL_MAX_TEXTURE_STACK_DEPTH", GL_MAX_TEXTURE_STACK_DEPTH),
             TPair("GL_MAX_MODELVIEW_STACK_DEPTH", GL_MAX_MODELVIEW_STACK_DEPTH),
             TPair("GL_MAX_VIEWPORT_DIMS", GL_MAX_VIEWPORT_DIMS),
             TPair("GL_MAX_NAME_STACK_DEPTH", GL_MAX_NAME_STACK_DEPTH),
         };

         size_t max_len = 0;
         for (size_t i = 0;  i < sizeof(s_EnumPairs) / sizeof(TPair);  ++i) {
             max_len = max(max_len, strlen(s_EnumPairs[i].first));
         }

         for (size_t i = 0;  i < sizeof(s_EnumPairs) / sizeof(TPair);  ++i) {
             GLint vals[4];
             glGetIntegerv(s_EnumPairs[i].second, vals);
             ostr << s_EnumPairs[i].first << ":"
                 << string(max_len - strlen(s_EnumPairs[i].first) + 3, ' ')
                 << vals[0] << endl;
         }
     }}

}



END_NCBI_SCOPE
