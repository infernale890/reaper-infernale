@echo off

set builddir=build
set buildarch=x86
set buildgen=Visual Studio 10
set buildtype=OFF
call :do_setup
set builddir=build_cpu
set buildtype=ON
call :do_setup

set builddir=build64
set buildarch=x86_64
set buildgen=Visual Studio 10 Win64
set buildtype=OFF
call :do_setup
set builddir=build64_cpu
set buildtype=ON
call :do_setup

goto :end

:do_setup
echo Setting up %builddir% for architecture %buildarch%...
echo.
rmdir /S /Q %builddir%
mkdir %builddir%
mkdir %builddir%\Release
xcopy *.cl %builddir%\Release
xcopy *.conf %builddir%\Release
xcopy windows\lib\%buildarch%\*.dll %builddir%\Release\
cd %builddir%
cmake -G "%buildgen%" -D CMAKE_LIBRARY_ARCHITECTURE=%buildarch% -D "CMAKE_PREFIX_PATH=windows" -D CPU_MINING_ONLY=%buildtype% ..
cd ..
echo.
goto :eof

:end
echo All done.
