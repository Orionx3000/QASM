#pragma once
#include <windows.h>
#include <stdexcept>
namespace EmbeddedUI {
    inline const char* get_html_content(size_t& size) {
        HRSRC hRes = FindResource(NULL, "HTML_PAYLOAD", RT_RCDATA);
        if (!hRes) throw std::runtime_error("Failed to find UI Resource");
        HGLOBAL hData = LoadResource(NULL, hRes);
        if (!hData) throw std::runtime_error("Failed to load UI Resource");
        size = SizeofResource(NULL, hRes);
        return static_cast<const char*>(LockResource(hData));
    }
}
