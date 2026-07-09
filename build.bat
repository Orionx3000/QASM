@echo off
echo [BUILD] Compiling Tinaten OS (Phase 17)...

REM Try MSVC first
where cl >nul 2>nul
if %ERRORLEVEL% == 0 (
    echo [BUILD] Microsoft Visual C++ Compiler detected.
    cl /EHsc /std:c++17 src\main.cpp src\AdaptiveCompressionEngine.cpp src\Base138.cpp src\QasmParser.cpp /I src /Fe:TinatenOS.exe
    if %ERRORLEVEL% == 0 (
        echo [SUCCESS] Compiled successfully to TinatenOS.exe
        exit /b 0
    )
)

REM Try g++ if MSVC isn't present
where g++ >nul 2>nul
if %ERRORLEVEL% == 0 (
    echo [BUILD] MinGW g++ compiler detected.
    g++ -std=c++17 -I src src\main.cpp src\AdaptiveCompressionEngine.cpp src\Base138.cpp src\QasmParser.cpp -o TinatenOS.exe -lws2_32 -lwinhttp -lwininet
    if %ERRORLEVEL% == 0 (
        echo [SUCCESS] Compiled successfully to TinatenOS.exe
        exit /b 0
    )
)

echo [ERROR] No compatible C++ compiler found (MSVC or g++), or compilation failed.
exit /b 1
