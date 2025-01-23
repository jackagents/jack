@echo off
setlocal EnableDelayedExpansion

where /Q cl || (
    echo [ERROR] msvc's cl must be available on the path
    goto :eof
)

where /Q findstr || (
    echo [ERROR] Window's findstr utility must be available on the path
    goto :eof
)

where /Q sort || (
    echo [ERROR] Window's sort utility must be available on the path
    goto :eof
)

set script_dir=%~dp0
if not exist %script_dir%\build\tools (
    mkdir %script_dir%\build\tools
)

pushd %script_dir%\build\tools
    cl /I %script_dir%\..\src ^
       /I %script_dir%\..\3rd\flatbuffers\include ^
       /I %script_dir%\..\event-protocol\src ^
       /I %script_dir%\..\event-protocol\3rd\concurrentqueue ^
       /I %script_dir%\..\3rd\aos-log\src ^
       /P /nologo ^
       %script_dir%\generate-header-list.cpp || (
           echo [ERROR] Failed to build JACK super header program
       )

    set file=header-list.txt
    set sorted_file=sorted-header-list.txt
    if exist %file% del %file%
    for /f delims^=^"^ tokens^=2 %%a IN ('findstr /R ^#line.*jack-core generate-header-list.i') do @echo %%a>>%file%
    call C:\Windows\System32\sort /uniq %file%>%sorted_file%
    call findstr /V "\\tools\\" %sorted_file%
popd


