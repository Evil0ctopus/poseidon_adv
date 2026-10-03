param(
    [string]$Port = "COM5",
    [string]$OutputDirectory = (Join-Path $PSScriptRoot "..\.pio\ui-smoke"),
    [switch]$SkipCapture
)

$ErrorActionPreference = "Stop"
$OutputDirectory = [IO.Path]::GetFullPath($OutputDirectory)
[IO.Directory]::CreateDirectory($OutputDirectory) | Out-Null
$Log = New-Object Text.StringBuilder
$Serial = New-Object IO.Ports.SerialPort $Port,115200
$Serial.DtrEnable = $true
$Serial.RtsEnable = $false
$Initial = $null
$CaptureAvailable = -not $SkipCapture

function Read-Device([int]$Milliseconds) {
    $End = [DateTime]::UtcNow.AddMilliseconds($Milliseconds)
    $Text = New-Object Text.StringBuilder
    while ([DateTime]::UtcNow -lt $End) {
        if ($Serial.BytesToRead -gt 0) {
            $Chunk = $Serial.ReadExisting()
            [void]$Text.Append($Chunk)
            [void]$Log.Append($Chunk)
        }
        Start-Sleep -Milliseconds 10
    }
    $Result = $Text.ToString()
    if ($Result -match "Guru Meditation|assert failed|Stack canary|Backtrace:") {
        throw "Device reported a panic; see the serial log."
    }
    return $Result
}

function Send-Key([int]$Code, [int]$Wait = 150) {
    $Serial.WriteLine(("K{0:X4}" -f $Code))
    [void](Read-Device $Wait)
}

function Get-State {
    for ($Attempt = 0; $Attempt -lt 3; $Attempt++) {
        $Serial.WriteLine("S")
        $Text = Read-Device 500
        $UI = [regex]::Matches($Text, "\[UI_STATE\] theme=(\d+) layout=(\d+) motion=(\d+) large=(\d+) ambient=(\d+)")
        $State = [regex]::Matches($Text, '\[STATE\] heap=(\d+) largest=(\d+).*feat="([^"]+)"')
        if ($UI.Count -gt 0 -and $State.Count -gt 0) { break }
        if ($Attempt -lt 2) { Write-Warning "Incomplete serial state response; retrying read-only query." }
    }
    if ($UI.Count -eq 0 -or $State.Count -eq 0) { throw "Missing Deepwater state response after three queries." }
    $U = $UI[$UI.Count - 1].Groups
    $S = $State[$State.Count - 1].Groups
    return @{
        Theme = [int]$U[1].Value; Layout = [int]$U[2].Value
        Motion = [int]$U[3].Value; Large = [int]$U[4].Value
        Ambient = [int]$U[5].Value; Heap = [int]$S[1].Value
        Largest = [int]$S[2].Value; Feature = $S[3].Value
    }
}

function Enter-Root {
    for ($I = 0; $I -lt 6; $I++) { Send-Key 0x1B 200 }
    [void](Read-Device 1000)
    $State = Get-State
    if ($State.Feature -ne "(menu)") { throw "Could not return to a menu safely." }
}

function Assert-Feature([string]$Name) {
    $State = Get-State
    if ($State.Feature -ne $Name) { throw "Expected $Name, got $($State.Feature)." }
}

function Save-Frame([string]$Name) {
    if (-not $script:CaptureAvailable) { return }
    $Serial.WriteLine("D")
    $Text = New-Object Text.StringBuilder
    $Deadline = [DateTime]::UtcNow.AddSeconds(20)
    while ([DateTime]::UtcNow -lt $Deadline) {
        [void]$Text.Append((Read-Device 150))
        if ($Text.ToString() -match "\[FRAME_END\]") { break }
    }
    $Raw = $Text.ToString()
    [IO.File]::WriteAllText((Join-Path $OutputDirectory "$Name.frame.txt"), $Raw)
    $Rows = [regex]::Matches($Raw, "\[FRAME_ROW\] (\d+) ([0-9A-F]{960})")
    $End = [regex]::Match($Raw, "\[FRAME_END\] ([0-9A-F]{8})")
    if ($Rows.Count -ne 135 -or -not $End.Success) { throw "Incomplete LCD readback for $Name." }
    Add-Type -AssemblyName System.Drawing
    $Image = New-Object Drawing.Bitmap 240,135
    $Colors = New-Object 'Collections.Generic.HashSet[uint16]'
    [uint32]$Checksum = 2166136261
    try {
        for ($Y = 0; $Y -lt 135; $Y++) {
            if ([int]$Rows[$Y].Groups[1].Value -ne $Y) { throw "Out-of-order LCD rows." }
            $Hex = $Rows[$Y].Groups[2].Value
            for ($X = 0; $X -lt 240; $X++) {
                [uint16]$Pixel = [Convert]::ToUInt16($Hex.Substring($X * 4,4),16)
                [void]$Colors.Add($Pixel)
                $Checksum = [uint32](([uint64]($Checksum -bxor $Pixel) * 16777619) -band 4294967295)
                $R = [int](($Pixel -shr 11) * 255 / 31)
                $G = [int]((($Pixel -shr 5) -band 63) * 255 / 63)
                $B = [int](($Pixel -band 31) * 255 / 31)
                $Image.SetPixel($X,$Y,[Drawing.Color]::FromArgb($R,$G,$B))
            }
        }
        if (("{0:X8}" -f $Checksum) -ne $End.Groups[1].Value) { throw "LCD checksum mismatch." }
        if ($Colors.Count -le 1) {
            Write-Warning "LCD returned a uniform frame; this panel may not support readback. UI smoke tests will continue."
            $script:CaptureAvailable = $false
            return
        }
        if ($Name -eq "root" -and $Initial.Theme -eq 0 -and -not $Colors.Contains([uint16]0x0864)) {
            throw "Deepwater navy is absent from readback; check pixel format and active palette."
        }
        $Image.Save((Join-Path $OutputDirectory "$Name.png"),[Drawing.Imaging.ImageFormat]::Png)
        Write-Host "Captured $Name ($($Colors.Count) colors)"
    } finally { $Image.Dispose() }
}

function Restore-Preferences {
    if (-not $Initial) { return }
    Enter-Root
    Send-Key ([int][char]'s')
    Send-Key ([int][char]'d')
    Assert-Feature "Display"
    $Current = Get-State
    if ($Current.Motion -ne $Initial.Motion) { Send-Key ([int][char]'m') }
    if ($Current.Large -ne $Initial.Large) { Send-Key ([int][char]'b') }
    if ($Current.Ambient -ne $Initial.Ambient) { Send-Key ([int][char]'a') }
    Enter-Root
    $Current = Get-State
    if ($Current.Layout -ne $Initial.Layout) {
        Send-Key ([int][char]'s')
        Send-Key ([int][char]'l') 1200
    }
    Enter-Root
    $Restored = Get-State
    foreach ($Field in @("Theme","Layout","Motion","Large","Ambient")) {
        if ($Restored[$Field] -ne $Initial[$Field]) { throw "Preference $Field was not restored." }
    }
}

try {
    $Serial.Open()
    [void](Read-Device 3000)
    Enter-Root
    $Initial = Get-State
    Write-Host "Starting UI smoke: heap=$($Initial.Heap) largest=$($Initial.Largest)"
    Save-Frame "root"

    Send-Key 0x105
    Save-Frame "footer-page-2"
    Send-Key ([int][char]'=')
    for ($I = 0; $I -lt 12; $I++) { Send-Key ([int][char]'.') 40 }
    Save-Frame "help-scrolled"
    Send-Key 0x1B

    Enter-Root
    Send-Key ([int][char]'s')
    Save-Frame "system"
    Send-Key ([int][char]'t')
    Assert-Feature "Theme"
    for ($I = 0; $I -lt 11; $I++) {
        $Preview = Get-State
        if ($Preview.Theme -ne (($Initial.Theme + $I) % 11)) { throw "Theme preview order mismatch." }
        if ($Preview.Theme -eq 0 -or $Preview.Theme -eq 2) { Save-Frame ("theme-" + $Preview.Theme) }
        Send-Key ([int][char]'.')
    }
    Send-Key 0x1B
    if ((Get-State).Theme -ne $Initial.Theme) { throw "Theme cancel did not restore saved choice." }

    Send-Key ([int][char]'d')
    Assert-Feature "Display"
    Save-Frame "display"
    Send-Key ([int][char]'m')
    if ((Get-State).Motion -eq $Initial.Motion) { throw "Motion toggle failed." }
    Send-Key ([int][char]'a')
    if ((Get-State).Ambient -eq $Initial.Ambient) { throw "Ambient toggle failed." }
    Send-Key ([int][char]'b')
    if ((Get-State).Large -eq $Initial.Large) { throw "Large-text toggle failed." }
    if ((Get-State).Large -ne 1) { Send-Key ([int][char]'b') }
    Send-Key 0x1B
    Save-Frame "system-large"

    Enter-Root
    Send-Key ([int][char]'w')
    $HelpStart = $Log.Length
    Send-Key ([int][char]'=')
    for ($I = 0; $I -lt 30; $I++) { Send-Key ([int][char]'.') 40 }
    $Pages = [regex]::Matches($Log.ToString().Substring($HelpStart),
        "\[UI_TEXT\] first=(\d+) visible=(\d+) total=(\d+)")
    if ($Pages.Count -eq 0) { throw "Missing help scrolling evidence." }
    $LastPage = $Pages[$Pages.Count - 1].Groups
    if ([int]$LastPage[1].Value -le 0 -or
        [int]$LastPage[1].Value -ne ([int]$LastPage[3].Value - [int]$LastPage[2].Value)) {
        throw "Long help did not reach its final page."
    }
    Save-Frame "large-help-final-page"
    Send-Key 0x1B
    Enter-Root
    Send-Key ([int][char]'s')
    Send-Key ([int][char]'s')
    Assert-Feature "Settings"
    Send-Key ([int][char]'f')
    Save-Frame "confirm-cancel"
    Send-Key 0x1B
    Send-Key 0x1B
    Send-Key ([int][char]'l') 1200
    if ((Get-State).Layout -eq $Initial.Layout) { throw "Layout toggle failed." }
    Save-Frame "alternate-layout"

    Enter-Root
    for ($I = 0; $I -lt 24; $I++) { Send-Key ([int][char]'.') 40 }
    Save-Frame "navigation-wrap"
    Enter-Root
    Send-Key ([int][char]'j')
    Assert-Feature "SaltyJack"
    Save-Frame "saltyjack-list"
    Send-Key ([int][char]'v')
    Save-Frame "saltyjack-grid"
    $GridStart = $Log.Length
    for ($I = 0; $I -lt 6; $I++) { Send-Key ([int][char]'.') 80 }
    if ($Log.ToString().Substring($GridStart) -notmatch "\[SJ_UI\] view=1 first=6 cursor=6") {
        throw "SaltyJack grid did not expose the final tools."
    }
    Save-Frame "saltyjack-grid-page-2"
    Send-Key ([int][char]'v')
    Save-Frame "saltyjack-card"
    Send-Key ([int][char]'i')
    Save-Frame "saltyjack-help"
    Send-Key 0x1B
    Send-Key 0x1B
    Restore-Preferences
    $Final = Get-State
    if ($Final.Heap -lt $Initial.Heap - 4096) { throw "UI sweep retained more than 4 KB of heap." }
    $Paints = [regex]::Matches($Log.ToString(), "\[UI_PAINT\] (?:list|card) (\d+)ms")
    if ($Paints.Count -eq 0) { throw "Missing UI paint timing evidence." }
    $Maximum = ($Paints | ForEach-Object { [int]$_.Groups[1].Value } | Measure-Object -Maximum).Maximum
    if ($Maximum -gt 100) { throw "UI paint exceeded the 100 ms target ($Maximum ms)." }
    Write-Host "PASS: themes, footer pages, scrolling help, display settings, cancel-only confirmation, both layouts, wrap navigation, SaltyJack views and preference restoration."
    Write-Host "Paint maximum=$Maximum ms; heap=$($Final.Heap); largest=$($Final.Largest)"
} finally {
    try {
        if ($Serial.IsOpen -and $Initial) { Restore-Preferences }
    } finally {
        [IO.File]::WriteAllText((Join-Path $OutputDirectory "serial.log"),$Log.ToString())
        if ($Serial.IsOpen) { $Serial.Close() }
        $Serial.Dispose()
    }
}
