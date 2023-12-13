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

#include <boost/program_options.hpp> // boost::program_options

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

/// バージョン出力
void version()
{
    cout << "playwav";
#if defined(_WIN64)
    cout << " x64";
#else        
    cout << " x86";
#endif
#if defined(NDEBUG)
    cout << " Release";
#else
    cout << " Debug";
#endif
    cout << " Version";
}

/// ヘルプメッセージ出力
void help(const boost::program_options::options_description& opt)
{
    version();
    cout << endl;
    cout << "書式: playwav {オプション} wavファイル ..." << endl << endl;
    ostringstream oss;
    oss << opt;
    cout << oss.str() << endl;
}

/// wav ファイルを再生
void process(filesystem::path wavPath, const boost::program_options::variables_map& vm)
{
    if (wavPath.is_relative())
        wavPath = GetKnownFolderPath(FOLDERID_Windows) / "Media" / wavPath;
    if (vm.count("verbose"))
        cout << "INFO: wavPath: " << wavPath << endl;
    int timeOut = 1000;
    ostringstream cmdLine;
    cmdLine << "mshta \"about:playing... "
            << "<OBJECT CLASSID='CLSID:22D6F312-B0F6-11D0-94AB-0080C74C7E95' WIDTH=1 HEIGHT=1>"
            << "  <PARAM NAME='src' VALUE='" << wavPath.string() << "'>"
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

int main(int argc, char** argv)
{
    try {
        locale::global(locale{locale{}, "", locale::ctype});
        namespace po = boost::program_options;
        po::positional_options_description p;
        p.add("wav-file", -1);
        po::options_description hidden("hidden options");
        hidden.add_options()
            ("wav-file", po::value<vector<string>>(), "wav file")
            ;
        po::options_description visible("オプション");
        visible.add_options()
            ("help,H", "ヘルプ")
            ("version,V", "バージョン表示")
            ("verbose,v", "冗長表示")
             ;
        po::options_description opt("オプション");
        opt.add(visible).add(hidden);
        po::variables_map vm;
        store(po::basic_command_line_parser<char>(argc, argv).options(opt).positional(p).run(), vm);
        po::notify(vm);
        if (vm.count("help")) {
            help(visible);
            return 0;
        }
        if (vm.count("version")) {
            version();
            return 0;
        }
        //
        if (vm.count("wav-file")) {
            for (const auto& str : vm["wav-file"].as<vector<string>>()) {
                process(str, vm);
            }
        }
        else {
            process("Ding.wav", vm);
        }
    }
    catch (const exception& x) {
        cerr << "ERROR: " << x.what() << endl;
    }
    return 0;
}

//------------------------------------------------------------

// end of <playwav.cpp>
