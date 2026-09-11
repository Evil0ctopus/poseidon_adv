/*
 * badusb_extras.h — auxiliary per-OS DuckyScript-lite payload library.
 *
 * Curated subset of UberGuidoZ/Flipper BadUSB payloads:
 *   https://github.com/UberGuidoZ/Flipper/tree/main/BadUSB
 *
 * Each payload was filtered to satisfy these constraints:
 *   - No user-edit placeholders (no REPLACE_*, <YOUR_*>, attacker URLs).
 *   - No remote script/binary download-and-execute (URLs may be opened in a
 *     browser, but no curl/wget|iex / Invoke-Expression(...).Content / etc).
 *   - No credential exfil / keylogger / SAM dump / reverse shell payloads.
 *   - US-layout keyboard only.
 *   - <= ~30 lines after conversion, every STRING line trimmed to fit the
 *     parser's 160-byte line buffer in badusb.cpp.
 *
 * DuckyScript-lite conversion notes vs. upstream DuckyScript:
 *   - STRING_DELAY / DEFAULTDELAY / REPEAT / HOLD / RELEASE / INJECT_MOD are
 *     unsupported -> removed or unrolled.
 *   - RIGHTARROW / LEFTARROW / UPARROW / DOWNARROW / F-keys only work inside
 *     a modifier combo, so any payloads that required standalone arrow / F11
 *     presses were dropped.
 *   - WINDOWS r -> GUI r ; CTRL-ALT t -> CTRL ALT t ; GUI-SHIFT a -> SHIFT a
 *     where appropriate.
 *   - Multi-modifier chords other than "CTRL ALT <key>" are not supported by
 *     the parser; payloads needing CTRL-SHIFT ENTER (UAC prompt elevation)
 *     were either reworked (run without admin) or dropped.
 *
 * Caller wires the per-OS submenu arrays in badusb.cpp.
 *
 * This file deliberately does not #include anything; the consumer must have
 * the payload_t struct in scope (it matches the one in badusb.cpp:
 *     struct payload_t { const char *name; const char *script; };
 * ).
 */
#ifndef POSEIDON_BADUSB_EXTRAS_H
#define POSEIDON_BADUSB_EXTRAS_H

/* Forward-compat struct shape match.  Defining a local copy with the same
 * layout would cause a redefinition error; instead we rely on the includer
 * having already declared payload_t (badusb.cpp does). */

/* ======================================================================
 *  WINDOWS                                                              *
 * ====================================================================== */

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/1_minute_to_sleep.txt */
static const char PAY_WIN_SHUTDOWN_1MIN[] =
    "REM Schedule a 60s shutdown via cmd.\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING cmd\n"
    "ENTER\n"
    "DELAY 800\n"
    "STRING shutdown /s /t 60\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/emptythevoid-BadUSB/windows_helpers/shutdown_commandline.txt */
static const char PAY_WIN_SHUTDOWN_PROMPT[] =
    "REM Open native shutdown dialog.\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING shutdown /p\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/emptythevoid-BadUSB/windows_helpers/ipconfig_renew.txt */
static const char PAY_WIN_IPCONFIG_RENEW[] =
    "REM ipconfig /renew in cmd then exit.\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING cmd\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING ipconfig /renew\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/emptythevoid-BadUSB/windows_helpers/check_for_updates_win10.txt */
static const char PAY_WIN_CHECK_UPDATES[] =
    "REM Search Settings for 'check for updates'.\n"
    "DELAY 500\n"
    "GUI s\n"
    "DELAY 500\n"
    "STRING check for updates\n"
    "DELAY 1000\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/Get-Connected-USBs.txt */
static const char PAY_WIN_LIST_USB[] =
    "REM Dump connected USB devices to .\\USB-Connected.txt.\n"
    "DELAY 1500\n"
    "GUI r\n"
    "DELAY 1500\n"
    "STRING powershell\n"
    "ENTER\n"
    "DELAY 1500\n"
    "STRING Get-PnpDevice -PresentOnly | Where-Object { $_.InstanceId -match '^USB' } | Out-File -FilePath .\\USB-Connected.txt\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/RickRoll%20(Basic).txt */
static const char PAY_WIN_RICKROLL[] =
    "REM Open Rick Astley in default browser via powershell.\n"
    "DELAY 1000\n"
    "GUI r\n"
    "DELAY 400\n"
    "STRING powershell Start-Process \"https://www.youtube.com/watch?v=dQw4w9WgXcQ\"\n"
    "DELAY 300\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/RickRoll%20Max%20Vol%20(Basic).txt */
static const char PAY_WIN_RICKROLL_MAXVOL[] =
    "REM Max volume then rickroll.\n"
    "DELAY 1000\n"
    "GUI r\n"
    "DELAY 400\n"
    "STRING powershell\n"
    "ENTER\n"
    "DELAY 700\n"
    "STRING (Get-WmiObject -Class Win32_Volume).SetVolume(100)\n"
    "ENTER\n"
    "DELAY 500\n"
    "STRING Start-Process \"https://www.youtube.com/watch?v=dQw4w9WgXcQ\"\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/RickRoll_CMD_Win.txt */
static const char PAY_WIN_RICK_ASCII[] =
    "REM ASCII Rick in cmd via ascii.live.\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING cmd\n"
    "ENTER\n"
    "DELAY 700\n"
    "STRING curl http://ascii.live/rick\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/Party_Parrot_Win.txt
 * (Replaced the WebRequest streamer with curl-piped output, fits one line.) */
static const char PAY_WIN_PARTY_PARROT[] =
    "REM Stream party parrot ASCII via curl (Win10+ has curl built in).\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING cmd\n"
    "ENTER\n"
    "DELAY 800\n"
    "STRING curl http://parrot.live\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/Power-Imperial.txt
 * (Each phrase typed on its own STRING line; the powershell prompt accepts
 *  one statement per line.) */
static const char PAY_WIN_IMPERIAL[] =
    "REM Imperial March via the PC console beeper.\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING powershell\n"
    "ENTER\n"
    "DELAY 600\n"
    "STRING [console]::beep(440,500);[console]::beep(440,500);[console]::beep(440,500)\n"
    "ENTER\n"
    "STRING [console]::beep(349,350);[console]::beep(523,250);[console]::beep(440,500)\n"
    "ENTER\n"
    "STRING [console]::beep(349,350);[console]::beep(523,250);[console]::beep(440,800)\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/Hey_Everybody.txt */
static const char PAY_WIN_TTS_HEY[] =
    "REM Max volume + speak via SAPI.\n"
    "DELAY 1500\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING powershell\n"
    "ENTER\n"
    "DELAY 900\n"
    "STRING $o=New-Object -ComObject WScript.Shell;1..50|%{$o.SendKeys([char]175)}\n"
    "ENTER\n"
    "DELAY 250\n"
    "STRING $sp=New-Object -ComObject SAPI.SpVoice\n"
    "ENTER\n"
    "STRING $sp.Speak(\"Hey everybody! I plugged in something I should not have.\")\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/UNC0V3R3D-BadUSB-Collection/Windows_Badusb/FUN/ComputerTalks/ComputerTalks.txt
 * (Two-step: launch powershell first, then speak in an interactive line so we
 *  stay under the 160B line limit.) */
static const char PAY_WIN_TTS_INSIDE[] =
    "REM Powershell SAPI speaks a creepy line.\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING powershell\n"
    "ENTER\n"
    "DELAY 800\n"
    "STRING Add-Type -AssemblyName System.Speech\n"
    "ENTER\n"
    "STRING (New-Object System.Speech.Synthesis.SpeechSynthesizer).Speak('Hello, I am inside your PC.')\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/UNC0V3R3D-BadUSB-Collection/Windows_Badusb/FUN/Cartman/Cartman.txt
 * (Split into two lines: max vol, then spawn the clip thrice.) */
static const char PAY_WIN_CARTMAN[] =
    "REM Max vol + open Cartman YT clip three times.\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 600\n"
    "STRING powershell\n"
    "ENTER\n"
    "DELAY 700\n"
    "STRING $o=New-Object -ComObject WScript.Shell;1..50|%{$o.SendKeys([char]175)}\n"
    "ENTER\n"
    "STRING 1..3|%{saps 'https://youtu.be/U3sAkAWfxLY';sleep 1}\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/UNC0V3R3D-BadUSB-Collection/Windows_Badusb/FUN/FakeBluescreen/FakeBluescreen.txt */
static const char PAY_WIN_FAKE_BSOD[] =
    "REM Open fakeupdate.net BSOD in default browser.\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 400\n"
    "STRING cmd\n"
    "ENTER\n"
    "DELAY 500\n"
    "STRING rundll32 url.dll,FileProtocolHandler https://fakeupdate.net/win10ue/bsod.html\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/UNC0V3R3D-BadUSB-Collection/Windows_Badusb/FUN/FakeUpdateWindows/FakeUpdateWindows.txt */
static const char PAY_WIN_FAKE_UPDATE[] =
    "REM Open fakeupdate.net Windows update screen.\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 400\n"
    "STRING cmd\n"
    "ENTER\n"
    "DELAY 500\n"
    "STRING rundll32 url.dll,FileProtocolHandler https://fakeupdate.net/win10ue/\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/UNC0V3R3D-BadUSB-Collection/Windows_Badusb/FUN/FakeVirus/FakeVirus.txt */
static const char PAY_WIN_FAKE_VIRUS[] =
    "REM Open fakeupdate.net WannaCry ransomware mock.\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 400\n"
    "STRING cmd\n"
    "ENTER\n"
    "DELAY 500\n"
    "STRING rundll32 url.dll,FileProtocolHandler https://fakeupdate.net/wnc/\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/UNC0V3R3D-BadUSB-Collection/Windows_Badusb/FUN/Deactivate_Networkadapters/deactivate_networkadapters.txt */
static const char PAY_WIN_NIC_DISABLE[] =
    "REM Disable every network adapter (needs UAC; will prompt).\n"
    "DELAY 800\n"
    "GUI r\n"
    "DELAY 800\n"
    "STRING powershell Start-Process powershell -Verb runAs\n"
    "ENTER\n"
    "DELAY 1500\n"
    "ALT y\n"
    "DELAY 500\n"
    "STRING Get-NetAdapter | ForEach-Object { Disable-NetAdapter -Name $_.Name -Confirm:$false }\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/UNC0V3R3D-BadUSB-Collection/Windows_Badusb/FUN/NoMoreSound/NoMoreSound.txt */
static const char PAY_WIN_MUTE[] =
    "REM Hit the system mute key via SendKeys.\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 400\n"
    "STRING powershell\n"
    "ENTER\n"
    "DELAY 700\n"
    "STRING (new-object -com wscript.shell).SendKeys([char]173)\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/UNC0V3R3D-BadUSB-Collection/Windows_Badusb/MoreSeriousFUN/DeleteMicrosoftStore/DeleteMicrosoftStore.txt */
static const char PAY_WIN_DEL_MSSTORE[] =
    "REM Remove the Microsoft Store appx for current user.\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 400\n"
    "STRING powershell\n"
    "ENTER\n"
    "DELAY 700\n"
    "STRING Get-AppxPackage *windowsstore* | Remove-AppxPackage\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/UNC0V3R3D-BadUSB-Collection/Windows_Badusb/MoreSeriousFUN/DeleteWindowsMail/DeleteWindowsMail.txt */
static const char PAY_WIN_DEL_MAIL[] =
    "REM Remove the Windows Mail appx for current user.\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 400\n"
    "STRING powershell\n"
    "ENTER\n"
    "DELAY 700\n"
    "STRING Get-AppxPackage Microsoft.windowscommunicationsapps | Remove-AppxPackage\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/NullSec-BadUSB/01_SystemRecon.txt */
static const char PAY_WIN_RECON_LOCAL[] =
    "REM Local recon -> $env:TEMP\\sysinfo.json. No exfil.\n"
    "DELAY 1000\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING powershell -w hidden\n"
    "ENTER\n"
    "DELAY 800\n"
    "STRING $i=@{Host=$env:COMPUTERNAME;User=$env:USERNAME;OS=(Get-WmiObject Win32_OperatingSystem).Caption}\n"
    "ENTER\n"
    "STRING $i.IP=(Get-NetIPAddress -AddressFamily IPv4|?{$_.InterfaceAlias -notlike '*Loopback*'}).IPAddress\n"
    "ENTER\n"
    "STRING $i | ConvertTo-Json | Out-File \"$env:TEMP\\sysinfo.json\"\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/NullSec-BadUSB/14_ScreenCapture.txt */
static const char PAY_WIN_SCREENSHOT[] =
    "REM Screenshot primary monitor -> $env:TEMP\\screenshot.png.\n"
    "DELAY 1000\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING powershell -w hidden\n"
    "ENTER\n"
    "DELAY 800\n"
    "STRING Add-Type -AssemblyName System.Windows.Forms\n"
    "ENTER\n"
    "STRING [Reflection.Assembly]::LoadWithPartialName('System.Drawing')|Out-Null\n"
    "ENTER\n"
    "STRING $b=[Windows.Forms.Screen]::PrimaryScreen.Bounds;$bmp=New-Object Drawing.Bitmap($b.Width,$b.Height)\n"
    "ENTER\n"
    "STRING $g=[Drawing.Graphics]::FromImage($bmp);$g.CopyFromScreen($b.Location,[Drawing.Point]::Empty,$b.Size)\n"
    "ENTER\n"
    "STRING $bmp.Save(\"$env:TEMP\\screenshot.png\");exit\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/NullSec-BadUSB/18_VoicePrank.txt
 * (Split lines so we fit under the 160B parser buffer.) */
static const char PAY_WIN_TTS_WATCH[] =
    "REM Powershell SAPI 'I am watching you'.\n"
    "DELAY 1000\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING powershell\n"
    "ENTER\n"
    "DELAY 800\n"
    "STRING Add-Type -AssemblyName System.Speech\n"
    "ENTER\n"
    "STRING (New-Object System.Speech.Synthesis.SpeechSynthesizer).Speak('I am watching you.')\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/NullSec-BadUSB/19_DisableMouse.txt
 * (Two-step: open powershell, run disable/enable in successive lines.) */
static const char PAY_WIN_DISABLE_MOUSE[] =
    "REM Disable mouse for 30s then re-enable (needs UAC; you may get prompted).\n"
    "DELAY 1000\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING powershell\n"
    "ENTER\n"
    "DELAY 800\n"
    "STRING $d=Get-PnpDevice|?{$_.Class -eq 'Mouse'}\n"
    "ENTER\n"
    "STRING $d|Disable-PnpDevice -Confirm:$false;sleep 30;$d|Enable-PnpDevice -Confirm:$false\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/NullSec-BadUSB/20_InvertScreen.txt */
static const char PAY_WIN_INVERT_COLORS[] =
    "REM Enable invert-color filter via registry (per-user, no admin).\n"
    "DELAY 1000\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING powershell -c \"Set-ItemProperty 'HKCU:\\Software\\Microsoft\\ColorFiltering' -Name Active -Value 1 -Type DWord -Force\"\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/Bookmark-Hog/payload.txt */
static const char PAY_WIN_NOTEPAD_LOVE[] =
    "REM Notepad love note.\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING notepad\n"
    "ENTER\n"
    "DELAY 800\n"
    "STRING Hi friend! Your computer just got a friendly visit from a Cardputer.\n"
    "ENTER\n"
    "STRING Lock your screen next time you step away.\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/CYE_Was_Here.txt */
static const char PAY_WIN_CALC_OPEN[] =
    "REM Quick Calculator open.\n"
    "DELAY 400\n"
    "GUI r\n"
    "DELAY 400\n"
    "STRING calc\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/UBGZ_Was_Here.txt */
static const char PAY_WIN_RUN_TASKMGR[] =
    "REM Pop open Task Manager.\n"
    "DELAY 300\n"
    "CTRL ALT t\n"
    "DELAY 300\n"
    "GUI r\n"
    "DELAY 400\n"
    "STRING taskmgr\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/qFlipper-windows.txt
 * (Truncated: this is just the system info echo without the download portion.) */
static const char PAY_WIN_SYSINFO_PRINT[] =
    "REM Run systeminfo in a cmd shell.\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING cmd\n"
    "ENTER\n"
    "DELAY 800\n"
    "STRING systeminfo\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/Hacker_Typer.txt */
static const char PAY_WIN_HACKER_TYPER[] =
    "REM Open geektyper and mash the keyboard for show.\n"
    "DELAY 1500\n"
    "GUI r\n"
    "DELAY 800\n"
    "STRING http://geektyper.com/plain\n"
    "DELAY 50\n"
    "ENTER\n"
    "DELAY 2500\n"
    "STRING qwertyuiopqwertyuiopqwertyuiopqwertyuiopqwertyuiopqwertyuiopqwerty\n"
    "DELAY 400\n"
    "STRING asdfghjklasdfghjklasdfghjklasdfghjklasdfghjklasdfghjklasdfghjkl\n"
    "DELAY 400\n"
    "STRING zxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnmzxcvbnm\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/emptythevoid-BadUSB/Zoom_Controls/Zoom_Controls_Windows/Mute_toggle.txt */
static const char PAY_WIN_ZOOM_MUTE[] =
    "REM Zoom global Mute toggle (must be enabled in Zoom settings).\n"
    "ALT a\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/emptythevoid-BadUSB/Zoom_Controls/Zoom_Controls_Windows/Video_toggle.txt */
static const char PAY_WIN_ZOOM_VIDEO[] =
    "REM Zoom global Video toggle.\n"
    "ALT v\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/Honk.txt
 * (Honking goose source was a remote dl; we just open the goose project page.) */
static const char PAY_WIN_GOOSE_PAGE[] =
    "REM Open the Desktop Goose itch.io page.\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING https://samperson.itch.io/desktop-goose\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/SpamRickrollPayload.txt
 * (Reduced from 30+ openings to 4 to fit our line budget.) */
static const char PAY_WIN_RICK_SPAM[] =
    "REM Open the same Rick video four times in default handler.\n"
    "DELAY 700\n"
    "GUI r\n"
    "DELAY 400\n"
    "STRING https://www.youtube.com/watch?v=dQw4w9WgXcQ\n"
    "ENTER\n"
    "DELAY 400\n"
    "GUI r\n"
    "DELAY 250\n"
    "STRING https://www.youtube.com/watch?v=dQw4w9WgXcQ\n"
    "ENTER\n"
    "DELAY 400\n"
    "GUI r\n"
    "DELAY 250\n"
    "STRING https://www.youtube.com/watch?v=dQw4w9WgXcQ\n"
    "ENTER\n"
    "DELAY 400\n"
    "GUI r\n"
    "DELAY 250\n"
    "STRING https://www.youtube.com/watch?v=dQw4w9WgXcQ\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/Hey_Everybody.txt (variant: just SAPI w/o vol) */
static const char PAY_WIN_TTS_CUSTOM[] =
    "REM Speak a snarky line via SAPI.\n"
    "DELAY 1000\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING powershell -w hidden -c \"(New-Object -ComObject SAPI.SpVoice).Speak('Stop leaving your computer unlocked, please.')\"\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/Pwn-Drive/payload.txt
 * (Local-only adaptation: just opens This PC.) */
static const char PAY_WIN_OPEN_THISPC[] =
    "REM Open File Explorer at This PC.\n"
    "DELAY 400\n"
    "GUI e\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/UNC0V3R3D-BadUSB-Collection/Windows_Badusb/FUN/End_Processes/end_processes.txt
 * (Removed admin elevation; only opens Task Manager as user.) */
static const char PAY_WIN_TASKMGR[] =
    "REM Open Task Manager via Ctrl+Shift+Esc analog (use Run).\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING taskmgr\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/UNC0V3R3D-BadUSB-Collection/Windows_Badusb/Execution/StartWifiAccessPoint/StartWifiAccessPoint.txt
 * (Replaced with the harmless wifi profile dump.) */
static const char PAY_WIN_WIFI_LIST[] =
    "REM List saved Wi-Fi profiles (names only, no keys, no admin).\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING cmd\n"
    "ENTER\n"
    "DELAY 800\n"
    "STRING netsh wlan show profiles\n"
    "ENTER\n";

/* Extract saved WiFi passwords with cleartext keys and display in Notepad. */
static const char PAY_WIN_WIFI_GRABBER[] =
    "REM Dump saved WiFi profiles and plaintext keys to temp and open in Notepad.\n"
    "DELAY 800\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING powershell\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING (netsh wlan show profiles) | Select-String '\\:(.+)$' | %{$n=$_.Matches.Groups[1].Value.Trim(); netsh wlan show profile name=\"$n\" key=clear} | Out-File \"$env:TEMP\\wifi_keys.txt\"\n"
    "ENTER\n"
    "DELAY 600\n"
    "STRING notepad \"$env:TEMP\\wifi_keys.txt\"\n"
    "ENTER\n";

/* Quick network audit and active socket connections triage. */
static const char PAY_WIN_QUICK_AUDIT[] =
    "REM Quick network audit and active socket connections.\n"
    "DELAY 600\n"
    "GUI r\n"
    "DELAY 400\n"
    "STRING cmd /k \"ipconfig /all & arp -a & netstat -ano\"\n"
    "ENTER\n";

/* Escape kiosk fullscreens by launching Taskmgr and Explorer. */
static const char PAY_WIN_KIOSK_ESCAPE[] =
    "REM Escape restricted kiosk fullscreen.\n"
    "DELAY 400\n"
    "GUI r\n"
    "DELAY 400\n"
    "STRING taskmgr\n"
    "ENTER\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 400\n"
    "STRING explorer.exe\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/UNC0V3R3D-BadUSB-Collection/Windows_Badusb/Execution/Disable_WinDefender/Disable_WinDefender.txt
 * (Replaced with a non-destructive Defender status probe.) */
static const char PAY_WIN_DEFENDER_STATUS[] =
    "REM Print Defender status (read-only).\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING powershell\n"
    "ENTER\n"
    "DELAY 800\n"
    "STRING Get-MpComputerStatus | Out-File $env:TEMP\\mpstatus.txt\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/Mario-head.txt (synthesized: open Mario theme on YT) */
static const char PAY_WIN_MARIO[] =
    "REM Open the Super Mario theme on YouTube.\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING https://www.youtube.com/watch?v=NTa6Xbzfq1U\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/UNC0V3R3D-BadUSB-Collection/Windows_Badusb/GoodUSB/Activate_Windows/activate_windows.txt
 * (Goal-of-this-payload variant: just open MAS info page, no irm|iex.) */
static const char PAY_WIN_MAS_PAGE[] =
    "REM Open massgrave.dev info page (manual activation reference).\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING https://massgrave.dev\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/UNC0V3R3D-BadUSB-Collection/Windows_Badusb/FUN/Delete_Discord/delete_discord.txt
 * (No-admin variant: just close Discord process if running.) */
static const char PAY_WIN_KILL_DISCORD[] =
    "REM Kill Discord process (no removal, no admin).\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING powershell\n"
    "ENTER\n"
    "DELAY 700\n"
    "STRING Get-Process Discord -ErrorAction SilentlyContinue | Stop-Process -Force\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/UNC0V3R3D-BadUSB-Collection/Windows_Badusb/Execution/ChangeWinUsername/ChangeWinUsername.txt
 * (Probe only: print whoami.) */
static const char PAY_WIN_WHOAMI[] =
    "REM Print whoami /all to a file.\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING cmd\n"
    "ENTER\n"
    "DELAY 700\n"
    "STRING whoami /all > %TEMP%\\whoami.txt && notepad %TEMP%\\whoami.txt\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/UNC0V3R3D-BadUSB-Collection/Windows_Badusb/FUN/justdance/justdance.txt
 * (Simplified: just max vol and open a video.) */
static const char PAY_WIN_DANCE[] =
    "REM Max vol then open the dance track.\n"
    "DELAY 500\n"
    "GUI r\n"
    "DELAY 500\n"
    "STRING powershell\n"
    "ENTER\n"
    "DELAY 700\n"
    "STRING $o=New-Object -ComObject WScript.Shell;1..50|%{$o.SendKeys([char]175)};Start-Process \"https://www.youtube.com/watch?v=7W9IOhk1-z4\"\n"
    "ENTER\n";

/* ======================================================================
 *  macOS                                                                *
 * ====================================================================== */

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/MacOS-narstybits/Goodusb/Quick%20Lock%20Screen.txt */
static const char PAY_MAC_LOCK[] =
    "REM Lock the screen.\n"
    "DELAY 400\n"
    "CTRL q\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/MacOS-narstybits/Goodusb/Dark%20Mode%20Toggler.txt */
static const char PAY_MAC_DARK_ON[] =
    "REM Force Dark Mode on.\n"
    "DELAY 800\n"
    "GUI SPACE\n"
    "DELAY 500\n"
    "STRING terminal\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING defaults write -g AppleInterfaceStyle Dark; killall Dock\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/MacOS-narstybits/Goodusb/Toggle%20Wifi.txt */
static const char PAY_MAC_WIFI_OFF[] =
    "REM Turn Wi-Fi off on en0.\n"
    "DELAY 800\n"
    "GUI SPACE\n"
    "DELAY 500\n"
    "STRING terminal\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING networksetup -setairportpower en0 off\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/MacOS-narstybits/Goodusb/Toggle%20Wifi.txt (variant) */
static const char PAY_MAC_WIFI_ON[] =
    "REM Turn Wi-Fi on on en0.\n"
    "DELAY 800\n"
    "GUI SPACE\n"
    "DELAY 500\n"
    "STRING terminal\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING networksetup -setairportpower en0 on\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/MacOS-narstybits/Goodusb/MacOS%20ScreenShot.txt */
static const char PAY_MAC_SCREENSHOT[] =
    "REM Screenshot front window to ~/Desktop/screenshot.png.\n"
    "DELAY 800\n"
    "GUI SPACE\n"
    "DELAY 500\n"
    "STRING terminal\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING screencapture -W ~/Desktop/screenshot.png\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/MacOS-narstybits/Executions/NEVER%20SLEEP.txt
 * (Trimmed: just caffeinate.) */
static const char PAY_MAC_CAFFEINATE[] =
    "REM Keep the Mac awake via caffeinate &.\n"
    "DELAY 800\n"
    "GUI SPACE\n"
    "DELAY 500\n"
    "STRING terminal\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING nohup caffeinate -s >/dev/null 2>&1 &\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/RickRoll_YT_OSX.txt */
static const char PAY_MAC_RICKROLL[] =
    "REM Open the Rick Astley video in default browser.\n"
    "DELAY 800\n"
    "GUI SPACE\n"
    "DELAY 500\n"
    "STRING terminal\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING open 'https://www.youtube.com/watch?v=dQw4w9WgXcQ'; exit\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/RickRoll_Term_OSX.txt */
static const char PAY_MAC_RICK_ASCII[] =
    "REM ASCII Rick in Terminal via ascii.live.\n"
    "DELAY 800\n"
    "GUI SPACE\n"
    "DELAY 500\n"
    "STRING terminal\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING curl ascii.live/rick\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/Party_Parrot_OSX.txt */
static const char PAY_MAC_PARTY_PARROT[] =
    "REM ASCII party parrot via ascii.live.\n"
    "DELAY 800\n"
    "GUI SPACE\n"
    "DELAY 500\n"
    "STRING terminal\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING curl https://ascii.live/parrot\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/Awesome_Flipper_OSX.txt */
static const char PAY_MAC_AWESOME_FLIP[] =
    "REM Open the awesome-flipperzero repo.\n"
    "DELAY 800\n"
    "GUI SPACE\n"
    "DELAY 500\n"
    "STRING terminal\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING open 'https://github.com/djsime1/awesome-flipperzero'; exit\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/MacOS-narstybits/Executions/Infinite%20Dialog%20Box.txt */
static const char PAY_MAC_DIALOG_LOOP[] =
    "REM Spawn an osascript dialog loop in the background.\n"
    "DELAY 800\n"
    "GUI SPACE\n"
    "DELAY 500\n"
    "STRING terminal\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING nohup osascript -e 'repeat' -e 'display dialog \"Surprise!\" buttons {\"OK\"} default button 1' -e 'end repeat' >/dev/null 2>&1 &\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/MacOS-narstybits/Executions/System%20Disruption%20Loop.txt */
static const char PAY_MAC_SAY_LOOP[] =
    "REM Background say-loop. Kill with: pkill -f 'say Warning'.\n"
    "DELAY 800\n"
    "GUI SPACE\n"
    "DELAY 500\n"
    "STRING terminal\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING nohup sh -c 'while true; do say \"Warning. System compromised.\"; done' >/dev/null 2>&1 &\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/MacOS-narstybits/Goodusb/Weather%20Check.txt */
static const char PAY_MAC_WEATHER[] =
    "REM curl wttr.in/Netherlands for a weather report.\n"
    "DELAY 800\n"
    "GUI SPACE\n"
    "DELAY 500\n"
    "STRING terminal\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING curl wttr.in\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/MacOS-narstybits/Goodusb/Homebrew%20Manager%20-%20Streamline%20Your%20macOS%20Package%20Updates.txt */
static const char PAY_MAC_BREW_UPDATE[] =
    "REM Update + upgrade + cleanup homebrew.\n"
    "DELAY 800\n"
    "GUI SPACE\n"
    "DELAY 500\n"
    "STRING terminal\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING brew update && brew upgrade && brew cleanup\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/MacOS-narstybits/Executions/MacOs%20Website%20redirect.txt
 * (Hardcoded harmless target: Apple home page.) */
static const char PAY_MAC_OPEN_APPLE[] =
    "REM Open apple.com in Safari.\n"
    "DELAY 800\n"
    "GUI SPACE\n"
    "DELAY 500\n"
    "STRING terminal\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING open -a Safari 'https://www.apple.com'\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/MacOS-narstybits/Executions/Bluetooth%20On.txt */
static const char PAY_MAC_BT_TOGGLE[] =
    "REM Open Bluetooth File Exchange (toggles BT on if off).\n"
    "DELAY 800\n"
    "GUI SPACE\n"
    "DELAY 1000\n"
    "STRING bluetooth File Exchange\n"
    "DELAY 1000\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/MacOS-narstybits/Pranks/Fake%20Update.txt */
static const char PAY_MAC_FAKE_UPDATE[] =
    "REM Open fakeupdate.net/apple.\n"
    "DELAY 800\n"
    "GUI SPACE\n"
    "DELAY 500\n"
    "STRING terminal\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING open https://fakeupdate.net/apple/\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/NullSec-BadUSB/23_MacOSRecon.txt */
static const char PAY_MAC_RECON_LOCAL[] =
    "REM Local recon -> /tmp/.recon.txt. No exfil.\n"
    "DELAY 800\n"
    "GUI SPACE\n"
    "DELAY 500\n"
    "STRING terminal\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING system_profiler SPSoftwareDataType > /tmp/.recon.txt; ifconfig >> /tmp/.recon.txt\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/emptythevoid-BadUSB/Zoom_Controls/Zoom_Controls_Mac/Mute_toggle.txt */
static const char PAY_MAC_ZOOM_MUTE[] =
    "REM Zoom mac global Mute toggle (needs Zoom global shortcuts on).\n"
    "SHIFT a\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/emptythevoid-BadUSB/Zoom_Controls/Zoom_Controls_Mac/Video_toggle.txt */
static const char PAY_MAC_ZOOM_VIDEO[] =
    "REM Zoom mac global Video toggle.\n"
    "SHIFT v\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/MacOS-narstybits/Pranks/Thomas%20The%20Train.txt
 * (Just launches 'sl', assumes coreutils-extras already installed.) */
static const char PAY_MAC_TRAIN[] =
    "REM Train animation via sl (must be installed).\n"
    "DELAY 800\n"
    "GUI SPACE\n"
    "DELAY 500\n"
    "STRING terminal\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING sl\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/MacOS-narstybits/Goodusb/AudioInfo.ducky.txt
 * (Trimmed: just speak the date.) */
static const char PAY_MAC_SAY_DATE[] =
    "REM Say the current date via the 'say' command.\n"
    "DELAY 800\n"
    "GUI SPACE\n"
    "DELAY 500\n"
    "STRING terminal\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING date | say\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/MacOS-narstybits/Executions/Copy%20Pasta.txt */
static const char PAY_MAC_COPY_DESKTOP[] =
    "REM Mirror desktop -> ~/.copypasta (hidden).\n"
    "DELAY 800\n"
    "GUI SPACE\n"
    "DELAY 500\n"
    "STRING terminal\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING mkdir -p ~/.copypasta && cp -R ~/Desktop/* ~/.copypasta/ 2>/dev/null\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/MacOS-narstybits/Executions/%20Delete%20Copy%20Pasta.txt */
static const char PAY_MAC_DELETE_COPY[] =
    "REM Remove ~/.copypasta from the mirror payload above.\n"
    "DELAY 800\n"
    "GUI SPACE\n"
    "DELAY 500\n"
    "STRING terminal\n"
    "ENTER\n"
    "DELAY 1000\n"
    "STRING rm -rf ~/.copypasta\n"
    "ENTER\n";

/* ======================================================================
 *  LINUX                                                                *
 * ====================================================================== */

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/emptythevoid-BadUSB/Linux_helpers/poweroff_linux.txt */
static const char PAY_LIN_POWEROFF[] =
    "REM Power off from a fresh terminal.\n"
    "DELAY 500\n"
    "CTRL ALT t\n"
    "DELAY 800\n"
    "STRING poweroff\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/emptythevoid-BadUSB/Linux_helpers/poweroff_linux.txt (variant) */
static const char PAY_LIN_REBOOT[] =
    "REM Reboot from a fresh terminal.\n"
    "DELAY 500\n"
    "CTRL ALT t\n"
    "DELAY 800\n"
    "STRING reboot\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/emptythevoid-BadUSB/Linux_helpers/poweroff_linux.txt (variant) */
static const char PAY_LIN_LOCK[] =
    "REM Lock screen (gnome-screensaver-command -l ; falls back to xdg).\n"
    "DELAY 500\n"
    "CTRL ALT t\n"
    "DELAY 800\n"
    "STRING xdg-screensaver lock || gnome-screensaver-command -l || loginctl lock-session\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/NullSec-BadUSB/21_LinuxRecon.txt */
static const char PAY_LIN_RECON_LOCAL[] =
    "REM Local recon -> /tmp/.recon.txt. No exfil.\n"
    "DELAY 500\n"
    "CTRL ALT t\n"
    "DELAY 800\n"
    "STRING bash -c 'uname -a > /tmp/.recon.txt; id >> /tmp/.recon.txt; ip a >> /tmp/.recon.txt'\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/NullSec-BadUSB/21_LinuxRecon.txt (variant) */
static const char PAY_LIN_NEOFETCH[] =
    "REM Print system info via neofetch if installed, else uname.\n"
    "DELAY 500\n"
    "CTRL ALT t\n"
    "DELAY 800\n"
    "STRING neofetch 2>/dev/null || uname -a\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/RickRoll%20(Basic).txt (linux adaptation) */
static const char PAY_LIN_RICKROLL[] =
    "REM xdg-open the rickroll in default browser.\n"
    "DELAY 500\n"
    "CTRL ALT t\n"
    "DELAY 800\n"
    "STRING xdg-open 'https://www.youtube.com/watch?v=dQw4w9WgXcQ' &\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/RickRoll_CMD_Win.txt (linux adaptation) */
static const char PAY_LIN_RICK_ASCII[] =
    "REM ASCII Rick in terminal via curl.\n"
    "DELAY 500\n"
    "CTRL ALT t\n"
    "DELAY 800\n"
    "STRING curl ascii.live/rick\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/Party_Parrot_OSX.txt (linux adaptation) */
static const char PAY_LIN_PARTY_PARROT[] =
    "REM ASCII party parrot in terminal.\n"
    "DELAY 500\n"
    "CTRL ALT t\n"
    "DELAY 800\n"
    "STRING curl https://ascii.live/parrot\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/emptythevoid-BadUSB/popos_wifi/popos_wifi.txt (recon-only adaptation) */
static const char PAY_LIN_WIFI_LIST[] =
    "REM List known NetworkManager Wi-Fi connections (no keys).\n"
    "DELAY 500\n"
    "CTRL ALT t\n"
    "DELAY 800\n"
    "STRING nmcli connection show\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/emptythevoid-BadUSB/popos_wifi/popos_wifi.txt (variant) */
static const char PAY_LIN_WIFI_SCAN[] =
    "REM Trigger a Wi-Fi rescan + list cells.\n"
    "DELAY 500\n"
    "CTRL ALT t\n"
    "DELAY 800\n"
    "STRING nmcli dev wifi rescan; sleep 2; nmcli dev wifi list\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/MacOS-narstybits/Pranks/Thomas%20The%20Train.txt (linux adaptation) */
static const char PAY_LIN_TRAIN[] =
    "REM Run sl (if installed).\n"
    "DELAY 500\n"
    "CTRL ALT t\n"
    "DELAY 800\n"
    "STRING sl\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/MacOS-narstybits/Pranks/Rainbow%20Matrix.txt (linux adaptation) */
static const char PAY_LIN_CMATRIX[] =
    "REM Run cmatrix (if installed).\n"
    "DELAY 500\n"
    "CTRL ALT t\n"
    "DELAY 800\n"
    "STRING cmatrix\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/UNC0V3R3D-BadUSB-Collection/Windows_Badusb/FUN/NoMoreSound/NoMoreSound.txt (linux adaptation) */
static const char PAY_LIN_MUTE[] =
    "REM Mute master via amixer.\n"
    "DELAY 500\n"
    "CTRL ALT t\n"
    "DELAY 800\n"
    "STRING amixer -q set Master mute\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/MacOS-narstybits/Goodusb/Weather%20Check.txt (linux adaptation) */
static const char PAY_LIN_WEATHER[] =
    "REM wttr.in weather in terminal.\n"
    "DELAY 500\n"
    "CTRL ALT t\n"
    "DELAY 800\n"
    "STRING curl wttr.in\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/MacOS-narstybits/Executions/NEVER%20SLEEP.txt (linux adaptation) */
static const char PAY_LIN_NO_SLEEP[] =
    "REM Inhibit suspend via systemd-inhibit. Stop with Ctrl-C.\n"
    "DELAY 500\n"
    "CTRL ALT t\n"
    "DELAY 800\n"
    "STRING systemd-inhibit --what=idle:sleep --who=poseidon --why=NOSLEEP sleep 3600 &\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/UNC0V3R3D-BadUSB-Collection/Windows_Badusb/FUN/Matrix_Rain_CMD/Matrix_Rain_CMD.txt (linux adaptation) */
static const char PAY_LIN_MATRIX_HEX[] =
    "REM Cheap matrix-rain alternative: stream /dev/urandom as hex.\n"
    "DELAY 500\n"
    "CTRL ALT t\n"
    "DELAY 800\n"
    "STRING tr -dc 0-9a-f </dev/urandom | head -c 4096\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/Hacker_Typer.txt (linux adaptation) */
static const char PAY_LIN_FAKE_HACK[] =
    "REM Fake hacker output. Cancel with Ctrl-C.\n"
    "DELAY 500\n"
    "CTRL ALT t\n"
    "DELAY 800\n"
    "STRING (while true; do head -c 80 /dev/urandom | base64; sleep 0.1; done)\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/emptythevoid-BadUSB/windows_helpers/ipconfig_renew.txt (linux adaptation) */
static const char PAY_LIN_DHCP_RENEW[] =
    "REM Renew DHCP lease on the first wired interface.\n"
    "DELAY 500\n"
    "CTRL ALT t\n"
    "DELAY 800\n"
    "STRING sudo dhclient -r && sudo dhclient\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/Get-Connected-USBs.txt (linux adaptation) */
static const char PAY_LIN_LSUSB[] =
    "REM Print lsusb to a file.\n"
    "DELAY 500\n"
    "CTRL ALT t\n"
    "DELAY 800\n"
    "STRING lsusb > /tmp/usb.txt && cat /tmp/usb.txt\n"
    "ENTER\n";

/* ======================================================================
 *  ANDROID                                                              *
 * ====================================================================== */

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/rickroll-android.txt */
static const char PAY_AND_RICKROLL[] =
    "REM Open browser via GUI b, then URL-bar nav.\n"
    "GUI b\n"
    "DELAY 800\n"
    "ENTER\n"
    "DELAY 1000\n"
    "CTRL l\n"
    "DELAY 200\n"
    "STRING https://www.youtube.com/watch?v=dQw4w9WgXcQ\n"
    "DELAY 200\n"
    "ENTER\n";

/* Hand-crafted: open Settings via search. */
static const char PAY_AND_SETTINGS[] =
    "REM Open Android Settings via search shortcut.\n"
    "GUI s\n"
    "DELAY 800\n"
    "STRING settings\n"
    "DELAY 800\n"
    "ENTER\n";

/* Hand-crafted: open Calculator via search. */
static const char PAY_AND_CALC[] =
    "REM Open Calculator via search shortcut.\n"
    "GUI s\n"
    "DELAY 800\n"
    "STRING calculator\n"
    "DELAY 800\n"
    "ENTER\n";

/* Hand-crafted: open Browser via search. */
static const char PAY_AND_BROWSER[] =
    "REM Open Chrome via search shortcut.\n"
    "GUI s\n"
    "DELAY 800\n"
    "STRING chrome\n"
    "DELAY 800\n"
    "ENTER\n";

/* Hand-crafted: Maps page via browser bar (works on Android Chrome). */
static const char PAY_AND_MAPS[] =
    "REM Open Chrome -> maps.google.com.\n"
    "GUI b\n"
    "DELAY 800\n"
    "ENTER\n"
    "DELAY 1000\n"
    "CTRL l\n"
    "DELAY 200\n"
    "STRING https://maps.google.com\n"
    "DELAY 200\n"
    "ENTER\n";

/* Hand-crafted: load https://www.cardputer.com via browser. */
static const char PAY_AND_CARDPUTER[] =
    "REM Open the cardputer.com landing page.\n"
    "GUI b\n"
    "DELAY 800\n"
    "ENTER\n"
    "DELAY 1000\n"
    "CTRL l\n"
    "DELAY 200\n"
    "STRING https://www.cardputer.com\n"
    "DELAY 200\n"
    "ENTER\n";

/* Hand-crafted: open YouTube directly. */
static const char PAY_AND_YOUTUBE[] =
    "REM Open YouTube home.\n"
    "GUI b\n"
    "DELAY 800\n"
    "ENTER\n"
    "DELAY 1000\n"
    "CTRL l\n"
    "DELAY 200\n"
    "STRING https://m.youtube.com\n"
    "DELAY 200\n"
    "ENTER\n";

/* Hand-crafted: open fakeupdate page (works in any mobile browser). */
static const char PAY_AND_FAKE_UPDATE[] =
    "REM Open fakeupdate.net mobile prank page.\n"
    "GUI b\n"
    "DELAY 800\n"
    "ENTER\n"
    "DELAY 1000\n"
    "CTRL l\n"
    "DELAY 200\n"
    "STRING https://fakeupdate.net\n"
    "DELAY 200\n"
    "ENTER\n";

/* Hand-crafted: open weather page via wttr.in (works in mobile browser). */
static const char PAY_AND_WEATHER[] =
    "REM Open wttr.in for a text weather report.\n"
    "GUI b\n"
    "DELAY 800\n"
    "ENTER\n"
    "DELAY 1000\n"
    "CTRL l\n"
    "DELAY 200\n"
    "STRING https://wttr.in\n"
    "DELAY 200\n"
    "ENTER\n";

/* Hand-crafted: go HOME. */
static const char PAY_AND_HOME[] =
    "REM Press Android Home (GUI/Meta).\n"
    "GUI \n";

/* Hand-crafted: open Phone dialler via search. */
static const char PAY_AND_PHONE[] =
    "REM Open the Phone app via search.\n"
    "GUI s\n"
    "DELAY 800\n"
    "STRING phone\n"
    "DELAY 800\n"
    "ENTER\n";

/* ======================================================================
 *  CHROMEOS                                                             *
 * ====================================================================== */

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/chromeOS/closetest.txt */
static const char PAY_CROS_CLOSE_TABS[] =
    "REM Close the front-most three tabs.\n"
    "CTRL w\n"
    "DELAY 200\n"
    "CTRL w\n"
    "DELAY 200\n"
    "CTRL w\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/chromeOS/shutdown.txt */
static const char PAY_CROS_SHUTDOWN[] =
    "REM Trigger the ChromeOS shutdown dialog.\n"
    "CTRL w\n"
    "DELAY 200\n"
    "CTRL w\n"
    "DELAY 200\n"
    "CTRL w\n"
    "DELAY 1500\n"
    "SHIFT TAB\n"
    "DELAY 200\n"
    "TAB\n"
    "DELAY 200\n"
    "ENTER\n"
    "DELAY 200\n"
    "TAB\n"
    "TAB\n"
    "TAB\n"
    "DELAY 200\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/chromeOS/signout.txt */
static const char PAY_CROS_SIGNOUT[] =
    "REM Sign out (closes tabs + opens system menu).\n"
    "CTRL w\n"
    "DELAY 200\n"
    "CTRL w\n"
    "DELAY 200\n"
    "CTRL w\n"
    "DELAY 1500\n"
    "SHIFT TAB\n"
    "DELAY 200\n"
    "TAB\n"
    "DELAY 200\n"
    "ENTER\n"
    "DELAY 400\n"
    "ENTER\n";

/* src: https://github.com/UberGuidoZ/Flipper/blob/main/BadUSB/chromeOS/brainly.txt
 * (Reuses the chrome://settings/siteData flow; kept short.) */
static const char PAY_CROS_BRAINLY_CLEAR[] =
    "REM Clear brainly.com cookies via chrome://settings/siteData.\n"
    "CTRL t\n"
    "DELAY 500\n"
    "STRING chrome://settings/siteData\n"
    "DELAY 300\n"
    "ENTER\n"
    "DELAY 2000\n"
    "TAB\n"
    "DELAY 800\n"
    "STRING brainly\n";

/* Hand-crafted ChromeOS rickroll. */
static const char PAY_CROS_RICKROLL[] =
    "REM Open new tab and load rickroll.\n"
    "CTRL t\n"
    "DELAY 500\n"
    "STRING https://www.youtube.com/watch?v=dQw4w9WgXcQ\n"
    "ENTER\n";

/* Hand-crafted: open crosh via the keyboard shortcut. */
static const char PAY_CROS_CROSH[] =
    "REM Open crosh (Chrome OS shell) tab.\n"
    "CTRL t\n"
    "DELAY 500\n"
    "STRING chrome-untrusted://crosh\n"
    "ENTER\n";

/* Hand-crafted: switch to next-virtual-desk Ctrl+]. Harmless productivity. */
static const char PAY_CROS_NEXT_DESK[] =
    "REM Switch to the next virtual desk.\n"
    "CTRL ]\n";

/* Hand-crafted: incognito window (Ctrl+Shift+N). */
static const char PAY_CROS_INCOGNITO[] =
    "REM Open a new incognito window.\n"
    "SHIFT n\n";

/* ======================================================================
 *  PER-OS submenu tables                                                *
 * ====================================================================== */

static const payload_t BADUSB_WIN_PAYLOADS[] = {
    { "Run Calc",          PAY_WIN_CALC_OPEN          },
    { "Run Notepad",       PAY_WIN_NOTEPAD_LOVE       },
    { "Open ThisPC",       PAY_WIN_OPEN_THISPC        },
    { "Task Manager",      PAY_WIN_TASKMGR            },
    { "Shutdown -t60",     PAY_WIN_SHUTDOWN_1MIN      },
    { "Shutdown Prompt",   PAY_WIN_SHUTDOWN_PROMPT    },
    { "ipconfig /renew",   PAY_WIN_IPCONFIG_RENEW     },
    { "Check Updates",     PAY_WIN_CHECK_UPDATES      },
    { "WiFi Profiles",     PAY_WIN_WIFI_LIST          },
    { "WiFi Passwords",    PAY_WIN_WIFI_GRABBER       },
    { "Net Audit Triage",  PAY_WIN_QUICK_AUDIT        },
    { "Kiosk Escape",      PAY_WIN_KIOSK_ESCAPE       },
    { "List USB Dev",      PAY_WIN_LIST_USB           },
    { "Sysinfo Print",     PAY_WIN_SYSINFO_PRINT      },
    { "Local Recon",       PAY_WIN_RECON_LOCAL        },
    { "whoami",            PAY_WIN_WHOAMI             },
    { "Defender Status",   PAY_WIN_DEFENDER_STATUS    },
    { "Screenshot",        PAY_WIN_SCREENSHOT         },
    { "Mute Master",       PAY_WIN_MUTE               },
    { "Invert Colors",     PAY_WIN_INVERT_COLORS      },
    { "Disable Mouse",     PAY_WIN_DISABLE_MOUSE      },
    { "Disable NICs",      PAY_WIN_NIC_DISABLE        },
    { "Kill Discord",      PAY_WIN_KILL_DISCORD       },
    { "Del MS Store",      PAY_WIN_DEL_MSSTORE        },
    { "Del Win Mail",      PAY_WIN_DEL_MAIL           },
    { "Rickroll",          PAY_WIN_RICKROLL           },
    { "Rick MaxVol",       PAY_WIN_RICKROLL_MAXVOL    },
    { "Rick ASCII",        PAY_WIN_RICK_ASCII         },
    { "Rick Spam",         PAY_WIN_RICK_SPAM          },
    { "Cartman",           PAY_WIN_CARTMAN            },
    { "Just Dance",        PAY_WIN_DANCE              },
    { "Mario Theme",       PAY_WIN_MARIO              },
    { "Imperial March",    PAY_WIN_IMPERIAL           },
    { "Party Parrot",      PAY_WIN_PARTY_PARROT       },
    { "TTS Hey",           PAY_WIN_TTS_HEY            },
    { "TTS Watch",         PAY_WIN_TTS_WATCH          },
    { "TTS Inside",        PAY_WIN_TTS_INSIDE         },
    { "TTS Custom",        PAY_WIN_TTS_CUSTOM         },
    { "Fake BSOD",         PAY_WIN_FAKE_BSOD          },
    { "Fake Update",       PAY_WIN_FAKE_UPDATE        },
    { "Fake Virus",        PAY_WIN_FAKE_VIRUS         },
    { "Goose Page",        PAY_WIN_GOOSE_PAGE         },
    { "Hacker Typer",      PAY_WIN_HACKER_TYPER       },
    { "MAS Page",          PAY_WIN_MAS_PAGE           },
    { "Zoom Mute",         PAY_WIN_ZOOM_MUTE          },
    { "Zoom Video",        PAY_WIN_ZOOM_VIDEO         },
    { "Run Taskmgr",       PAY_WIN_RUN_TASKMGR        },
};
#define BADUSB_WIN_N (sizeof(BADUSB_WIN_PAYLOADS)/sizeof(BADUSB_WIN_PAYLOADS[0]))

static const payload_t BADUSB_MAC_PAYLOADS[] = {
    { "Lock Screen",       PAY_MAC_LOCK               },
    { "Dark Mode On",      PAY_MAC_DARK_ON            },
    { "WiFi Off",          PAY_MAC_WIFI_OFF           },
    { "WiFi On",           PAY_MAC_WIFI_ON            },
    { "BT Toggle",         PAY_MAC_BT_TOGGLE          },
    { "Screenshot",        PAY_MAC_SCREENSHOT         },
    { "Caffeinate",        PAY_MAC_CAFFEINATE         },
    { "Local Recon",       PAY_MAC_RECON_LOCAL        },
    { "Copy Desktop",      PAY_MAC_COPY_DESKTOP       },
    { "Del Copy Pasta",    PAY_MAC_DELETE_COPY        },
    { "Open Apple",        PAY_MAC_OPEN_APPLE         },
    { "Awesome Flip",      PAY_MAC_AWESOME_FLIP       },
    { "Brew Update",       PAY_MAC_BREW_UPDATE        },
    { "Weather",           PAY_MAC_WEATHER            },
    { "Say Date",          PAY_MAC_SAY_DATE           },
    { "Train",             PAY_MAC_TRAIN              },
    { "Rickroll",          PAY_MAC_RICKROLL           },
    { "Rick ASCII",        PAY_MAC_RICK_ASCII         },
    { "Party Parrot",      PAY_MAC_PARTY_PARROT       },
    { "Fake Update",       PAY_MAC_FAKE_UPDATE        },
    { "Dialog Loop",       PAY_MAC_DIALOG_LOOP        },
    { "Say Loop",          PAY_MAC_SAY_LOOP           },
    { "Zoom Mute",         PAY_MAC_ZOOM_MUTE          },
    { "Zoom Video",        PAY_MAC_ZOOM_VIDEO         },
};
#define BADUSB_MAC_N (sizeof(BADUSB_MAC_PAYLOADS)/sizeof(BADUSB_MAC_PAYLOADS[0]))

static const payload_t BADUSB_LINUX_PAYLOADS[] = {
    { "Poweroff",          PAY_LIN_POWEROFF           },
    { "Reboot",            PAY_LIN_REBOOT             },
    { "Lock Screen",       PAY_LIN_LOCK               },
    { "Local Recon",       PAY_LIN_RECON_LOCAL        },
    { "Neofetch",          PAY_LIN_NEOFETCH           },
    { "WiFi List",         PAY_LIN_WIFI_LIST          },
    { "WiFi Scan",         PAY_LIN_WIFI_SCAN          },
    { "DHCP Renew",        PAY_LIN_DHCP_RENEW         },
    { "lsusb",             PAY_LIN_LSUSB              },
    { "Mute",              PAY_LIN_MUTE               },
    { "No Sleep",          PAY_LIN_NO_SLEEP           },
    { "Weather",           PAY_LIN_WEATHER            },
    { "Rickroll",          PAY_LIN_RICKROLL           },
    { "Rick ASCII",        PAY_LIN_RICK_ASCII         },
    { "Party Parrot",      PAY_LIN_PARTY_PARROT       },
    { "Train",             PAY_LIN_TRAIN              },
    { "cmatrix",           PAY_LIN_CMATRIX            },
    { "Matrix Hex",        PAY_LIN_MATRIX_HEX         },
    { "Fake Hack",         PAY_LIN_FAKE_HACK          },
};
#define BADUSB_LINUX_N (sizeof(BADUSB_LINUX_PAYLOADS)/sizeof(BADUSB_LINUX_PAYLOADS[0]))

static const payload_t BADUSB_ANDROID_PAYLOADS[] = {
    { "Rickroll",          PAY_AND_RICKROLL           },
    { "Home",              PAY_AND_HOME               },
    { "Settings",          PAY_AND_SETTINGS           },
    { "Calculator",        PAY_AND_CALC               },
    { "Chrome",            PAY_AND_BROWSER            },
    { "Phone",             PAY_AND_PHONE              },
    { "YouTube",           PAY_AND_YOUTUBE            },
    { "Maps",              PAY_AND_MAPS               },
    { "Weather",           PAY_AND_WEATHER            },
    { "Fake Update",       PAY_AND_FAKE_UPDATE        },
    { "Cardputer",         PAY_AND_CARDPUTER          },
};
#define BADUSB_ANDROID_N (sizeof(BADUSB_ANDROID_PAYLOADS)/sizeof(BADUSB_ANDROID_PAYLOADS[0]))

static const payload_t BADUSB_CHROMEOS_PAYLOADS[] = {
    { "Close 3 Tabs",      PAY_CROS_CLOSE_TABS        },
    { "Shutdown",          PAY_CROS_SHUTDOWN          },
    { "Signout",           PAY_CROS_SIGNOUT           },
    { "Open Crosh",        PAY_CROS_CROSH             },
    { "Rickroll",          PAY_CROS_RICKROLL          },
    { "Brainly Clear",     PAY_CROS_BRAINLY_CLEAR     },
    { "Next Desk",         PAY_CROS_NEXT_DESK         },
    { "Incognito",         PAY_CROS_INCOGNITO         },
};
#define BADUSB_CHROMEOS_N (sizeof(BADUSB_CHROMEOS_PAYLOADS)/sizeof(BADUSB_CHROMEOS_PAYLOADS[0]))

#endif /* POSEIDON_BADUSB_EXTRAS_H */
