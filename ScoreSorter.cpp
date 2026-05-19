#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <vector>
#include <string>
#include <sstream>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "comdlg32.lib")

/* ================= 工具 ================= */
BOOL mkdirw(const std::wstring& path) {
    return CreateDirectoryW(path.c_str(), nullptr);
}

BOOL existsw(const std::wstring& path) {
    return GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

/* ================= CSV ================= */
wchar_t detect_delimiter(const std::wstring& line) {
    if (line.find(L',') != std::wstring::npos) return L',';
    if (line.find(L'\t') != std::wstring::npos) return L'\t';
    return L' ';
}

std::vector<std::wstring> split(const std::wstring& s, wchar_t delim) {
    std::vector<std::wstring> r;
    if (delim == L' ') {
        std::wstringstream ss(s);
        std::wstring item;
        while (ss >> item) r.push_back(item);
    } else {
        size_t start = 0, pos;
        while ((pos = s.find(delim, start)) != std::wstring::npos) {
            r.push_back(s.substr(start, pos - start));
            start = pos + 1;
        }
        r.push_back(s.substr(start));
    }
    return r;
}

/* ================= 选择 ================= */
std::wstring selectFolder() {
    BROWSEINFOW bi{};
    wchar_t path[MAX_PATH]{};
    bi.lpszTitle = L"选择学生文件夹";
    PIDLIST_ABSOLUTE pidl = SHBrowseForFolderW(&bi);
    if (pidl && SHGetPathFromIDListW(pidl, path))
        return path;
    return {};
}

std::wstring selectFile() {
    OPENFILENAMEW ofn{};
    wchar_t file[MAX_PATH]{};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"CSV Files\0*.csv\0All Files\0*.*\0";
    ofn.Flags = OFN_FILEMUSTEXIST;
    GetOpenFileNameW(&ofn);
    return file;
}

/* ================= 读 CSV ================= */
std::vector<std::wstring> read_csv(const std::wstring& path) {
    std::vector<std::wstring> lines;
    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ,
                           nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return lines;

    DWORD size = GetFileSize(h, nullptr);
    std::vector<char> buf(size + 1, 0);
    ReadFile(h, buf.data(), size, nullptr, nullptr);
    CloseHandle(h);

    int len = MultiByteToWideChar(936, 0, buf.data(), -1, nullptr, 0);
    std::wstring wtext(len, 0);
    MultiByteToWideChar(936, 0, buf.data(), -1, &wtext[0], len);

    std::wstringstream ss(wtext);
    std::wstring line;
    while (std::getline(ss, line))
        lines.push_back(line);

    return lines;
}

/* ================= WinMain ================= */
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    std::wstring root = selectFolder();
    if (root.empty()) return 0;

    std::wstring csv = selectFile();
    if (csv.empty()) return 0;

    /* 成绩目录 */
    std::wstring base = root + L"\\课程设计成绩";
    mkdirw(base);

    std::wstring p[5] = {
        base + L"\\60分以下作业",
        base + L"\\60-69分作业",
        base + L"\\70-79分作业",
        base + L"\\80-89分作业",
        base + L"\\90-100分作业"
    };
    for (auto& x : p) mkdirw(x);
    mkdirw(root + L"\\其他");

    /* ================= 第一步：分类（不改名） ================= */
    auto lines = read_csv(csv);
    if (lines.size() < 2) {
        MessageBoxW(nullptr, L"CSV 为空或无法读取", L"错误", MB_OK);
        return 0;
    }

    wchar_t delim = detect_delimiter(lines[0]);
    int total = 0, processed = 0;

    for (size_t i = 1; i < lines.size(); ++i) {
        auto c = split(lines[i], delim);
        if (c.size() < 3) continue;
        total++;

        int score = std::stoi(c[2]);
        std::wstring stuDir = c[0] + L"-" + c[1];
        std::wstring src = root + L"\\" + stuDir;
        if (!existsw(src)) continue;

        std::wstring dst =
            score >= 90 ? p[4] :
            score >= 80 ? p[3] :
            score >= 70 ? p[2] :
            score >= 60 ? p[1] : p[0];

        std::wstring dstPath = dst + L"\\" + stuDir;
        if (MoveFileW(src.c_str(), dstPath.c_str()))
            processed++;
    }

    /* ================= 第二步：统一重命名（加分数字前缀） ================= */
    for (auto& dir : p) {
        WIN32_FIND_DATAW fd;
        HANDLE h = FindFirstFileW((dir + L"\\*").c_str(), &fd);
        if (h == INVALID_HANDLE_VALUE) continue;

        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
            if (wcscmp(fd.cFileName, L".") == 0 ||
                wcscmp(fd.cFileName, L"..") == 0) continue;

            std::wstring oldName = dir + L"\\" + fd.cFileName;

            /* 从 CSV 查成绩 */
            for (size_t i = 1; i < lines.size(); ++i) {
                auto c = split(lines[i], delim);
                if (c.size() < 3) continue;

                std::wstring stuDir = c[0] + L"-" + c[1];
                if (stuDir == fd.cFileName) {
                    int score = std::stoi(c[2]);
                    std::wstring newName =
                        dir + L"\\" + std::to_wstring(score) + L" " + stuDir;
                    MoveFileW(oldName.c_str(), newName.c_str());
                    break;
                }
            }
        } while (FindNextFileW(h, &fd));
        FindClose(h);
    }

    MessageBoxW(nullptr,
        (L"完成\n总人数：" + std::to_wstring(total) +
         L"\n成功：" + std::to_wstring(processed)).c_str(),
        L"结果", MB_OK);

    return 0;
}