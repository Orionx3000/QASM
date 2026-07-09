import os
import sys

def bake_ui():
    ui_dist = 'ui/dist/index.html'
    if not os.path.exists(ui_dist):
        print(f"Warning: {ui_dist} not found. Skipping UI bake.")
        return

    # Just create a resource file that points to the UI HTML
    # so MSVC's resource compiler (rc.exe) can pack it instantly.
    print(f"Baking monolithic React UI into Windows Resource...")
    
    with open('src/app_ui.rc', 'w', encoding='utf-8') as f:
        # We need absolute path or relative from the build directory, 
        # but the RC compiler runs from the build directory.
        # Actually, CMake runs rc.exe from where the .rc file is, or the build dir.
        # We can just write the absolute path to be safe.
        abs_path = os.path.abspath(ui_dist).replace('\\', '/')
        f.write(f'HTML_PAYLOAD RCDATA "{abs_path}"\n')

    # Also generate a tiny EmbeddedUI.h header to declare the resource loading function
    with open('src/EmbeddedUI.h', 'w', encoding='utf-8') as f:
        f.write('#pragma once\n')
        f.write('#include <windows.h>\n')
        f.write('#include <stdexcept>\n')
        f.write('namespace EmbeddedUI {\n')
        f.write('    inline const char* get_html_content(size_t& size) {\n')
        f.write('        HRSRC hRes = FindResource(NULL, "HTML_PAYLOAD", RT_RCDATA);\n')
        f.write('        if (!hRes) throw std::runtime_error("Failed to find UI Resource");\n')
        f.write('        HGLOBAL hData = LoadResource(NULL, hRes);\n')
        f.write('        if (!hData) throw std::runtime_error("Failed to load UI Resource");\n')
        f.write('        size = SizeofResource(NULL, hRes);\n')
        f.write('        return static_cast<const char*>(LockResource(hData));\n')
        f.write('    }\n')
        f.write('}\n')

if __name__ == '__main__':
    bake_ui()
