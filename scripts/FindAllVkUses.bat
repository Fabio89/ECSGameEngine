@echo off
setlocal enabledelayedexpansion

:: Set default paths if no arguments are provided
if "%~1"=="" (
    set "default_paths=..\GameEngine"
) else (
    set "default_paths="
)

:: Define the regex pattern to search for and to ignore files
set "regexPattern=\bVk\w*\b"
set "ignorePattern=Vulkan\.ixx"

:: Define the file extensions to search for
set "extensions=.h .cpp .inl .ixx"

:: Iterate over each provided path or default path
:next_path
if "%~1"=="" (
    if "!default_paths!"=="" goto end
    set "current_path=!default_paths!"
    set "default_paths="
) else (
    set "current_path=%~1"
    shift
)

:: Use PowerShell to process each file in the current directory and subdirectories with the specified extensions
for %%e in (%extensions%) do (
    for /r "%current_path%" %%f in (*%%e) do (
        :: Check if the file name matches the ignore pattern
        echo "%%~nxf" | findstr /r /i "%ignorePattern%" >nul && (
            echo Ignoring file: %%f
        ) || (
            :: Process the file content with PowerShell
            powershell -Command ^
            "Get-Content -Path '%%f' -Raw | ForEach-Object { if ($_ -match '%regexPattern%') { [regex]::Matches($_, '%regexPattern%') | ForEach-Object { $_.Value } } }"
        )
    )
)

goto next_path

:end
endlocal
pause
