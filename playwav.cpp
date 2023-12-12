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

using namespace std;

//------------------------------------------------------------

int main(int /*argc*/, char** /*argv*/)
{
    try {
        locale::global(locale{locale{}, "", locale::ctype});
        string wavPath ="C:\\WINDOWS\\Media\\Windows Ding.wav";
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
