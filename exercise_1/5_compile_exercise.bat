REM Change the following line to where you want to install the examples

REM cd /d sources
mkdir build & cd build


set CMAKE=%VCPKG_PATH%\downloads\tools\cmake-3.14.0-windows\cmake-3.14.0-win32-x86\bin
%CMAKE%\cmake.exe -G "Visual Studio 15 2017 Win64" ..\ -DCMAKE_TOOLCHAIN_FILE=%VCPKG_PATH%/scripts/buildsystems/vcpkg.cmake & %CMAKE%\cmake.exe --build . --config Release

pause