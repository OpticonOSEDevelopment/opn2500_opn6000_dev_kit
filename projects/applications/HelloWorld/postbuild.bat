call dir

@echo off
setlocal enabledelayedexpansion

echo %1 %2 %3 %4 %5

set BOARD_VERSION=%1_%5

for /f "delims=" %%i in ('dir /b /a-d /o-n "..\..\..\..\..\..\SDK\firmware\bootloader\%4.HEX"') do (
  set "BOOTLOADER=%%~ni"
  goto :next
)
:next

set "version_str=%3"

rem Extract the last character
for /l %%i in (0, 1, 1) do (
    set "version_char=!version_str:~-1!"
)

REM Initialize the value variable
set APP_VERSION_NR=-1

REM Check if the character is a digit (0-9)
for /L %%I in (0,1,9) do (
    if "%version_char%"=="%%I" set APP_VERSION_NR=%%I
)

REM Check if the character is a letter (A-Z)
if %APP_VERSION_NR%==-1 (
    for %%L in (A B C D E F G H I J K L M N O P Q R S T U V W X Y Z) do (
        set /A counter+=1
        if "%version_char%"=="%%L" (
            set /A APP_VERSION_NR=counter+9
            goto :next2
        )
    )
)

:next2

set CPU_VERSION=%5

set SOFTDEVICE=s113_nrf52_7.3.0_softdevice

echo CPU = %CPU_VERSION%
echo SD = %SOFTDEVICE%

if "%BOOTLOADER%" == "" (
	echo "Bootloader %4 not Found"
	goto :error;
)

rem extract the last 4 digits from the string
set "BOOT_VERSION=!BOOTLOADER:~-4!"

:: Trim all leading zeros from BOOT_VERSION
set "BOOT_VERSION=%BOOT_VERSION: =%"
if "%BOOT_VERSION:~0,1%"=="0" (
  set "BOOT_VERSION=%BOOT_VERSION:~1%"
  goto :trim_zeros
)
:trim_zeros
if "%BOOT_VERSION:~0,1%"=="0" (
  set "BOOT_VERSION=%BOOT_VERSION:~1%"
  goto :trim_zeros
)
set APP_VERSION=%3

if "%APP_VERSION_NR%" == "" (
	set APP_VERSION_NR=%APP_VERSION:~-1%"
)

:: Get the last character
set "APP_VERSION=%APP_VERSION:~-1%"

echo BOOT_VERSION = %BOOT_VERSION%
echo APP_VERSION = %APP_VERSION%
echo APP_VERSION_NR = %APP_VERSION_NR%

set SOFTDEVICE_FILE="..\..\..\..\..\..\SDK\softdevice\s113/hex\%SOFTDEVICE%.hex"
set BOOTLOADER_FILE="..\..\..\..\..\..\SDK\firmware\bootloader\%BOOTLOADER%.hex"
set SETTINGS_FILE=%1_%BOOTLOADER%_%2_%3_settings.hex
set OS_FILE=%1_%2_%3.hex
set KIT_FOLDER=..\..\..\Output

if "%CPU_VERSION%"=="nRF52840" (
    call "..\..\..\..\..\..\SDK\nrfutil\nrfutil.exe" settings generate --family NRF52840 --application "%OS_FILE%" --application-version %APP_VERSION_NR% --bootloader-version %BOOT_VERSION% --bl-settings-version 1 "%SETTINGS_FILE%"
) else (
	call "..\..\..\..\..\..\SDK\nrfutil\nrfutil.exe" settings generate --family NRF52 --application "%OS_FILE%" --application-version %APP_VERSION_NR% --bootloader-version %BOOT_VERSION% --bl-settings-version 1 "%SETTINGS_FILE%"
)

call "..\..\..\..\..\..\SDK\nrfutil\nrfutil.exe" pkg generate --hw-version 52 --application-version %APP_VERSION_NR% --application "%OS_FILE%" --sd-req 0x102,0x125 --softdevice %SOFTDEVICE_FILE% --sd-id 0x125 --bootloader %BOOTLOADER_FILE% --bootloader-version %BOOT_VERSION% --key-file "../../../private.key" "%1_%BOOTLOADER%_%2_%3.bin"

copy "%1_%BOOTLOADER%_%2_%3.bin" "%KIT_FOLDER%/%1_%BOOTLOADER%_%2_%3.bin"

goto :end

:error

:end