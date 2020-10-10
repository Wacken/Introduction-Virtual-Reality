
cd /d %VCPKG_PATH%

set VCPKG_DEFAULT_TRIPLET=x64-windows

cmd /C vcpkg install opencv[core,jasper,nonfree,contrib,dnn,sfm,opengl]

cmd /C vcpkg install dlib 



pause