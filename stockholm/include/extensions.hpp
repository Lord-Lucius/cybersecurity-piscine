#pragma once

#include <unordered_set>
#include <string>

inline std::unordered_set<std::string> wannacry_extensions() {
    return {
        ".doc", ".docx", ".docb", ".docm", ".dot", ".dotm", ".dotx",
        ".xls", ".xlsx", ".xlsm", ".xlsb", ".xlw", ".xlt", ".xlm",
        ".xlc", ".xltx", ".xltm", ".ppt", ".pptx", ".pptm", ".pot",
        ".pps", ".ppsm", ".ppsx", ".ppam", ".potx", ".potm",
        ".pdf", ".rtf", ".txt", ".csv", ".123", ".wks", ".wk1",
        ".pst", ".ost", ".msg", ".eml",
        ".vsd", ".vsdx",
        ".jpg", ".jpeg", ".bmp", ".png", ".gif", ".raw", ".tif",
        ".tiff", ".nef", ".psd", ".ai", ".svg", ".djvu", ".cgm",
        ".mp3", ".wav", ".wma", ".mid", ".m3u", ".m4u", ".flv",
        ".mp4", ".mov", ".avi", ".mkv", ".mpeg", ".mpg", ".wmv",
        ".vob", ".asf", ".3gp", ".3g2", ".fla", ".swf",
        ".zip", ".rar", ".7z", ".tar", ".gz", ".tgz", ".bz2",
        ".backup", ".bak", ".tbk", ".iso", ".vcd", ".arc",
        ".sql", ".sqlite3", ".sqlitedb", ".mdb", ".accdb", ".db",
        ".dbf", ".odb", ".frm", ".myd", ".myi", ".ibd", ".mdf", ".ldf",
        ".cpp", ".c", ".h", ".cs", ".java", ".class", ".jar",
        ".js", ".php", ".asp", ".jsp", ".rb", ".pl", ".sh",
        ".vb", ".vbs", ".ps1", ".bat", ".cmd", ".asm", ".pas",
        ".sln", ".suo",
        ".pem", ".csr", ".crt", ".key", ".pfx", ".p12", ".der", ".asc",
        ".gpg", ".aes", ".PAQ",
        ".vdi", ".vmdk", ".vmx",
        ".odt", ".ott", ".sxw", ".stw", ".uot",
        ".ods", ".ots", ".sxc", ".stc", ".wb2", ".slk", ".dif",
        ".odp", ".otp", ".sxi", ".sti", ".uop",
        ".odg", ".otg", ".sxd", ".std",
        ".onetoc2", ".snt", ".hwp", ".602",
        ".lay", ".lay6", ".mml", ".sxm",
        ".3ds", ".3dm", ".max",
        ".dwg",
        ".edb"
    };
}
