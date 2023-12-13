// <playwav.cpp>
//
// Project playwav
// Copyright (C) 2023 neige68
//
/// \file
/// \brief playwav
//
// Compiler: VC14.2
//

#include "pch.h"
#pragma hdrstop

#include "shlobj_core.h"

#include <filesystem>           // std::filesystem

using namespace std;

//------------------------------------------------------------

/// 既知のフォルダーの完全パスを取得
filesystem::path GetKnownFolderPath(REFKNOWNFOLDERID rfid, DWORD dwFlags = 0, HANDLE hToken = 0)
{
    PWSTR pszPath = 0;
    if (SHGetKnownFolderPath(rfid, dwFlags, hToken, &pszPath) != S_OK) {
        ::CoTaskMemFree(pszPath);
        throw runtime_error("SHGetKnownFolderPath failure.");
    }
    wstring result{pszPath};
    ::CoTaskMemFree(pszPath);
    return result;
}

//------------------------------------------------------------

int main(int /*argc*/, char** /*argv*/)
{
    try {
        locale::global(locale{locale{}, "", locale::ctype});
        auto p = GetKnownFolderPath(FOLDERID_Windows) / "Media" / "Windows Ding.wav";
        string wavPath = p.string();
        int timeOut = 1000;
        ostringstream cmdLine;
        cmdLine << "mshta \"about:playing... "
                << "<OBJECT CLASSID='CLSID:22D6F312-B0F6-11D0-94AB-0080C74C7E95' WIDTH=1 HEIGHT=1>"
                << "  <PARAM NAME='src' VALUE='" << wavPath << "'>"
                << "  <PARAM NAME='PlayCount' VALUE='1'>"
                << "  <PARAM NAME='AutoStart' VALUE='true'>"
                << "</OBJECT>"
                << "<SCRIPT>"
                << "  window.resizeTo(10,10);"
                << "  window.moveTo(-32000,-32000);"
                << "  setTimeout(function(){window.close()}," << timeOut << ");"
                << "</SCRIPT>\"";
        system(cmdLine.str().c_str());
    }
    catch (const exception& x) {
        cerr << "ERROR: " << x.what() << endl;
    }
    return 0;
}

//------------------------------------------------------------

// end of <playwav.cpp>
