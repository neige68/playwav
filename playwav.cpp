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

#include <Mmsystem.h>           // PlaySound
#include <shlobj_core.h>        // SHGetKnownFolderPath

#include <boost/program_options.hpp> // boost::program_options

#include <filesystem>           // std::filesystem

using namespace std;

//------------------------------------------------------------

/// バージョン
const char* str_version = "0.00";

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
#if !defined(NDEBUG)
    cout << " Debug";
#endif
    cout << " Version " << str_version << endl;
}

/// ヘルプメッセージ出力
void help(const boost::program_options::options_description& opt)
{
    version();
    cout << endl;
    cout << "書式: playwav {オプション} wavファイル ..." << endl << endl;
    cout << "wavファイルは絶対パス指定がなければ Windows Meida フォルダを使用します" << endl << endl;
    cout << "wavファイルは複数指定しても存在する最初のファイルのみを再生します" << endl << endl;
    ostringstream oss;
    oss << opt;
    cout << oss.str() << endl;
}

/// Windows Media フォルダパス
filesystem::path GetWindowsMediaPath()
{
    return GetKnownFolderPath(FOLDERID_Windows) / "Media";
}

/// 文字列を区切り文字 c で分解
vector<string> split(const string& str, char c)
{
    vector<string> result;
    size_t start = 0;
    for (;;) {
        size_t pos = str.find(c, start);
        if (pos == string::npos) break;
        result.push_back(str.substr(start, pos - 1));
        start = pos + 1;
    }
    return result;
}

/// ターミナルの桁数
int GetTerminalCols()
{
    CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ConsoleScreenBufferInfo))
        return ConsoleScreenBufferInfo.dwSize.X;
    else {
        // Emacs の shell ではエラー: ハンドルが無効です。
#pragma warning(disable:4996) // getenv の結果は string にすぐコピーするので安全だと思う
        if (string(getenv("TERM")) == "emacs") { // Windows の Emacs の shell では環境変数 TERM=emacs になっている
            string cap = getenv("TERMCAP"); // "emacs:co#115:tc=unknown:" のように起動時の幅が記憶されている
            vector<string> scap = split(cap, ':');
            auto iscap = find_if(scap.begin(), scap.end(),
                                 [] (const string& s) { return s.substr(0, 3) == "co#"; });
            if (iscap != scap.end())
                return atoi(iscap->substr(3).c_str());
        }
#pragma warning(default:4996)
    }
    return 80;
}

/// リスト表示
void show_list()
{
    vector<string> v;
    size_t max_width = 0;
    for (const auto& ent : filesystem::directory_iterator(GetWindowsMediaPath())) {
        if (ent.is_regular_file()) {
            auto ext = ent.path().extension();
            if (ext == ".wav") {
                string fname = ent.path().filename().string();
                v.push_back(fname);
                if (max_width < fname.size())
                    max_width = fname.size();
            }
        }
    }
    int c = 1 + (GetTerminalCols() - max_width) / (max_width + 2);
    int r = (v.size() + c - 1) / c;
    for (int ir = 0; ir < r; ++ir) {
        for (int ic = 0; ic < c; ++ic) {
            size_t s = ir + ic * r;
            if (s < v.size()) {
                cout << v[s];
                if (ic + 1 < c)
                    cout << string(max_width + 2 - v[s].size(), ' ');
            }
        }
        cout << endl;
    }
}

/// wav ファイルを再生
/// \result true = ファイルが存在し再生した
///         false = ファイルが存在しないなどのエラー
bool process(filesystem::path wavPath, const boost::program_options::variables_map& vm, bool last)
{
    if (wavPath.is_relative())
        wavPath = GetWindowsMediaPath() / wavPath;
    if (!filesystem::exists(wavPath)) {
        // .wav を追加して試す
        filesystem::path wavPathExtAdded = wavPath;
        wavPathExtAdded += L".wav";
        if (filesystem::exists(wavPathExtAdded))
            wavPath = wavPathExtAdded;
    }
    if (!filesystem::exists(wavPath)) {
        if (last)
            cerr << "ERROR: File " << wavPath << " not found." << endl;
        else if (vm.count("verbose"))
            cerr << "WARN: File " << wavPath << " not found." << endl;
        return false;
    }
    if (vm.count("verbose"))
        cout << "INFO: Play File: " << wavPath << endl;
    //
    int timeOut = vm["timeout"].as<int>();
    bool result = PlaySound(wavPath.string().c_str(), NULL, SND_FILENAME | SND_ASYNC);
    Sleep(timeOut);
    PlaySound(NULL, NULL, SND_FILENAME | SND_ASYNC);
    return result;
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
            ("timeout,T", po::value<int>()->default_value(1000), "タイムアウト[ミリ秒]")
            ("list,L", "ファイルリスト表示")
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
        if (vm.count("list")) {
            show_list();
            return 0;
        }
        //
        if (vm.count("wav-file")) {
            size_t count = vm["wav-file"].as<vector<string>>().size();
            size_t i = 0;
            for (const auto& str : vm["wav-file"].as<vector<string>>()) {
                if (process(str, vm, ++i == count))
                    break;
            }
        }
        else {
            process("Ding.wav", vm, true);
        }
    }
    catch (const exception& x) {
        cerr << "ERROR: " << x.what() << endl;
    }
    return 0;
}

//------------------------------------------------------------

// end of <playwav.cpp>
