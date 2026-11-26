# windows-install.ps1 -- the PowerShell inside windows-install.bat
# (build/mkinstall.py puts it there; the .bat runs it with -File, passing its
# own path and the files dropped on it). Windows PowerShell 5.1; ASCII
# only, as it reads a .ps1 without a byte-order mark as the ANSI code page.
#
# What linux-install.sh does: check that the folder is the release unpacked
# and nothing else (SHA256SUMS), find the two disc images of Tron 2.0 (USA)
# and the executable of its Update 2 (the arguments, or the files in the
# game folder or the one above it; a redump .bin, .cue or .zip converted to
# a temporary .iso first, an update in a .zip unpacked first), check
# their sizes and SHA-256, unpack the update's cabinet, then write each file
# of the list at the end of the .bat from the retail file it names -- read
# from byte ranges of the images, as they are or as InstallShield 6
# cabinet data (16-bit length, raw deflate stream; .NET's DeflateStream
# inflates them) -- by its records: C <length> its bytes, Z <length> zeros,
# L <base64> ours; check its size and SHA-256, and move it into place.
param(
    [Parameter(Mandatory = $true)][string]$Self,
    [Parameter(ValueFromRemainingArguments = $true)][string[]]$Images
)
$ErrorActionPreference = 'Stop'
$here = Split-Path -Parent $Self

# Converted images (.disc-<n>.iso.part), removed however the script ends.
$script:temps = @()
function Remove-Temps {
    foreach ($t in $script:temps) { Remove-Item -LiteralPath $t -Force -ErrorAction SilentlyContinue }
}

# Any other error: say it, and still remove the converted images.
trap {
    Write-Host ''
    Write-Host "windows-install: $_" -ForegroundColor Red
    if ($out) { $out.Close() }
    if ($streams) { foreach ($s in $streams.Values) { $s.Close() } }
    Remove-Temps
    exit 1
}

function Fail($msg) {
    Write-Host ''
    Write-Host "windows-install: $msg" -ForegroundColor Red
    Remove-Temps
    exit 1
}

function Get-Sha256($path) {
    return (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
}

# The list: every line after the marker line.
$text = [IO.File]::ReadAllText($Self)
$marker = '__TRON20_INSTALL' + '_DATA__'
$at = $text.IndexOf("`n" + $marker)
if ($at -lt 0) { Fail 'the file list is missing from the .bat' }
$lines = $text.Substring($at + 1 + $marker.Length) -split "`r?`n" | Where-Object { $_ -ne '' }

# This folder: the release unpacked, and nothing else? (linux-install.sh's
# check_dir.) SHA256SUMS lists every file the archive holds and every file
# this script writes. Each of the archive's must be here as it was shipped,
# and nothing may be here that is neither, nor what the game writes as it
# runs (saves, profiles, server configurations, settings, logs, screenshots,
# the launcher's command file, mods) nor a disc image or the update. Names
# compare without case, as Windows does (a hashtable's keys and -like do).
$sumsPath = Join-Path $here 'SHA256SUMS'
if (-not (Test-Path -LiteralPath $sumsPath -PathType Leaf)) {
    Fail 'SHA256SUMS is missing: run windows-install.bat in the folder the release unpacked to'
}
$sums = @{}
foreach ($l in [IO.File]::ReadAllLines($sumsPath)) {
    if ($l.Length -gt 66) { $sums[$l.Substring(66)] = $l.Substring(0, 64) }
}
$writes = @{}
foreach ($l in $lines) {
    if ($l.StartsWith('F ')) { $writes[$l.Split(' ', 6)[5]] = $true }   # (the name may have spaces)
}
Write-Host 'checking this folder ...'
$bad = @()
foreach ($rel in $sums.Keys) {
    if ($writes.ContainsKey($rel)) { continue }          # this script writes it
    $path = Join-Path $here ($rel.Replace('/', [IO.Path]::DirectorySeparatorChar))
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { $bad += "missing: $rel" }
    elseif ((Get-Sha256 $path) -ne $sums[$rel]) { $bad += "changed: $rel" }
}
$extra = @()
$top = @('SHA256SUMS', 'autoexec.cfg', 'display.cfg', 'tron20.ini', 'launchcmds.txt',
         'run-cmds.txt', 'BanList.txt', 'tron-update_v1x2_complete.exe')
$anywhere = @('Save/*', 'Profiles/*', 'ServerData/*', 'custom/Mods/*', 'custom/Resources/*', '*.part')
$topPatterns = @('*.log', '*.bmp', '*.tga', '*.iso', '*.bin', '*.cue', '*.zip')
$hereLength = $here.TrimEnd('\', '/').Length + 1
foreach ($f in @(Get-ChildItem -LiteralPath $here -Recurse -File -Force)) {
    $rel = $f.FullName.Substring($hereLength).Replace('\', '/')
    if ($sums.ContainsKey($rel) -or $writes.ContainsKey($rel)) { continue }
    if ($top -contains $rel) { continue }
    if (@($anywhere | Where-Object { $rel -like $_ }).Count) { continue }
    if (@('Thumbs.db', 'desktop.ini', '.DS_Store') -contains $f.Name) { continue }
    if (-not $rel.Contains('/') -and @($topPatterns | Where-Object { $rel -like $_ }).Count) { continue }
    $extra += $rel
}
if ($bad.Count -or $extra.Count) {
    Write-Host ''
    Write-Host 'windows-install: this folder is not just this release, unpacked:' -ForegroundColor Red
    foreach ($b in $bad) { Write-Host "  $b" }
    foreach ($e in ($extra | Select-Object -First 20)) { Write-Host "  not part of it: $e" }
    if ($extra.Count -gt 20) { Write-Host "  ... and $($extra.Count - 20) more files not part of it" }
    Write-Host ''
    Write-Host 'The release is the whole game, not a patch: do not copy it over another'
    Write-Host 'installation, or over an older release. Unpack it into a new, empty'
    Write-Host 'folder and run windows-install.bat there. (Your saved games and profiles'
    Write-Host 'are in the old folder''s Save and Profiles folders: copy them across'
    Write-Host 'afterwards.)'
    exit 1
}

# The images (D: a disc's .iso, U: the update), their redump .bin sizes (R)
# and the sets the game installs from (S <name> <id>...).
$imgs = @{}
$sets = @()
foreach ($l in $lines) {
    $p = $l.Split(' ')
    if ($p[0] -eq 'D' -or $p[0] -eq 'U') {
        $imgs[$p[1]] = @{ Size = [int64]$p[2]; Sha = $p[3]; Name = $l.Split(' ', 5)[4]; Raw = [int64]-1; Path = $null }
    } elseif ($p[0] -eq 'R') {
        $imgs[$p[1]].Raw = [int64]$p[2]
    } elseif ($p[0] -eq 'S') {
        $sets += , @($p[1..($p.Count - 1)])
    }
}
function Test-Complete {
    foreach ($s in $sets) {
        $ok = $true
        foreach ($id in $s[1..($s.Count - 1)]) { if (-not $imgs[$id].Path) { $ok = $false } }
        if ($ok) { return $true }
    }
    return $false
}

# A redump image: a .bin, or the one .bin of a .zip, of 2352-byte MODE1 or
# MODE2 sectors. Open-Raw gives @(size, stream) (stream $null with -SizeOnly);
# a .zip without exactly one .bin has size 0, so it matches no disc.
Add-Type -AssemblyName System.IO.Compression.FileSystem
function Open-Raw($path, [switch]$SizeOnly) {
    if ($path.ToLowerInvariant().EndsWith('.zip')) {
        try { $zip = [IO.Compression.ZipFile]::OpenRead($path) } catch { return @([int64]0, $null) }
        $bins = @($zip.Entries | Where-Object { $_.FullName.ToLowerInvariant().EndsWith('.bin') })
        if ($bins.Count -ne 1) { $zip.Dispose(); return @([int64]0, $null) }
        if ($SizeOnly) { $size = $bins[0].Length; $zip.Dispose(); return @($size, $null) }
        return @($bins[0].Length, $bins[0].Open())
    }
    $size = ([IO.FileInfo]$path).Length
    if ($SizeOnly) { return @($size, $null) }
    return @($size, [IO.File]::OpenRead($path))
}

# The .iso of a redump image: the 2048 bytes of user data of each sector,
# after its 16-byte sync and header -- and, on a MODE2 track (the USA
# discs; the European ones are MODE1), its 8-byte subheader.
function Convert-Raw($path, $out) {
    $r = Open-Raw $path
    $src = $r[1]
    $dst = [IO.File]::Create($out)
    try {
        $S = 2352
        $buf = New-Object byte[] ($S * 512)
        $k = 0
        while ($true) {
            $have = 0
            while ($have -lt $buf.Length) {       # a zip stream can read short
                $n = $src.Read($buf, $have, $buf.Length - $have)
                if ($n -le 0) { break }
                $have += $n
            }
            if ($have -eq 0) { break }
            if ($have % $S) { Fail "$(Split-Path -Leaf $path): not whole 2352-byte sectors" }
            # the track's mode, as its first sector's header gives it
            if ($k -eq 0) { if ($buf[15] -eq 2) { $k = 24 } else { $k = 16 } }
            for ($i = 0; $i -lt $have; $i += $S) { $dst.Write($buf, $i + $k, 2048) }
            if ($have -lt $buf.Length) { break }
        }
    } finally {
        $dst.Close()
        $src.Close()
    }
}

# Which image a file is, if any: by size, then SHA-256 (computed once).
function Use-Image($path, $label) {
    $size = ([IO.FileInfo]$path).Length
    $sum = $null
    foreach ($id in @($imgs.Keys)) {
        $d = $imgs[$id]
        if ($d.Path -or $size -ne $d.Size) { continue }
        if (-not $sum) {
            Write-Host "checking $label ..."
            $sum = Get-Sha256 $path
        }
        if ($sum -eq $d.Sha) {
            $d.Path = $path
            Write-Host "  $($d.Name)"
            return $true
        }
    }
    if ($sum) { Write-Host '  the size of an image, but not its contents (a bad copy?)' }
    return $false
}

if (-not $Images -or $Images.Count -eq 0) {
    # .iso first: one is used as it is, the others need converting.
    $Images = @()
    $up = Split-Path -Parent $here
    foreach ($pat in '*.iso', '*.exe', '*.bin', '*.zip') {
        foreach ($dir in @($here, $up)) {
            if ($dir) { $Images += @(Get-ChildItem -LiteralPath $dir -Filter $pat -File | ForEach-Object { $_.FullName }) }
        }
    }
}
foreach ($img in $Images) {
    if (Test-Complete) { break }
    if (-not (Test-Path -LiteralPath $img -PathType Leaf)) { Fail "$img`: no such file" }
    $label = Split-Path -Leaf $img
    $lower = $img.ToLowerInvariant()
    if ($lower.EndsWith('.cue')) {
        $cue = [IO.File]::ReadAllText($img)
        $m = [regex]::Match($cue, '(?m)^\s*FILE\s+"([^"]+)"')
        if (-not $m.Success -or $cue -notmatch '(?i)TRACK\s+01\s+MODE[12]/2352') {
            Write-Host "  $label`: not a one-track MODE1/2352 or MODE2/2352 cue sheet; skipped"
            continue
        }
        $img = Join-Path (Split-Path -Parent $img) $m.Groups[1].Value
        if (-not (Test-Path -LiteralPath $img -PathType Leaf)) { Fail "$label names $($m.Groups[1].Value), which is not beside it" }
        $lower = '.bin'
    }
    if ($lower.EndsWith('.zip') -and -not $imgs['update'].Path) {
        # the update's .exe in a .zip: unpacked to a temporary file first
        $exe = $null
        try {
            $zip = [IO.Compression.ZipFile]::OpenRead($img)
            $exes = @($zip.Entries | Where-Object { $_.FullName.ToLowerInvariant().EndsWith('.exe') -and $_.Length -eq $imgs['update'].Size })
            if ($exes.Count -eq 1) {
                $exe = Join-Path $here ".update-$($script:temps.Count).exe.part"
                $script:temps += $exe
                [IO.Compression.ZipFileExtensions]::ExtractToFile($exes[0], $exe, $true)
            }
            $zip.Dispose()
        } catch { $exe = $null }
        if ($exe) {
            if (-not (Use-Image $exe "$label (unpacked)")) { Remove-Item -LiteralPath $exe -Force }
            continue
        }
    }
    if ($lower.EndsWith('.bin') -or $lower.EndsWith('.zip')) {
        $size = (Open-Raw $img -SizeOnly)[0]
        foreach ($id in @($imgs.Keys)) {
            $d = $imgs[$id]
            if ($d.Path -or $size -ne $d.Raw) { continue }
            $tmp = Join-Path $here ".disc-$($script:temps.Count).iso.part"
            $script:temps += $tmp
            Write-Host "converting $label to an .iso ..."
            Convert-Raw $img $tmp
            if (-not (Use-Image $tmp "$label (converted)")) { Remove-Item -LiteralPath $tmp -Force }
            break
        }
    } else {
        [void](Use-Image $img $label)
    }
}
if (-not (Test-Complete)) {
    Write-Host 'These are needed, and not all were found:' -ForegroundColor Red
    foreach ($s in $sets) {
        Write-Host "  $($s[0]):"
        foreach ($id in $s[1..($s.Count - 1)]) {
            $d = $imgs[$id]
            if ($d.Path) { Write-Host "    found:   $($d.Name)" }
            else { Write-Host "    missing: $($d.Name) ($($d.Size) bytes, SHA-256 $($d.Sha))" }
        }
    }
    Write-Host '  (a disc as an .iso, or redump''s .bin/.cue or .zip of it; the update as'
    Write-Host '  its .exe, or a .zip that holds it)'
    Fail 'drop both disc images and the update on windows-install.bat, or put them in this folder'
}

# Raw deflate data, inflated: $pre (a stored block holding a dictionary,
# or $null) and then $count bytes of $buf from $off, into $dst from $at.
# Returns how many bytes it wrote.
function Expand-Deflate($pre, $buf, $off, $count, $dst, $at) {
    $ms = New-Object IO.MemoryStream
    if ($pre) { $ms.Write($pre, 0, $pre.Length) }
    $ms.Write($buf, $off, $count)
    $ms.Position = 0
    $z = New-Object IO.Compression.DeflateStream($ms, [IO.Compression.CompressionMode]::Decompress)
    $total = 0
    while ($true) {
        $n = $z.Read($dst, $at + $total, $dst.Length - $at - $total)
        if ($n -le 0) { break }
        $total += $n
    }
    $z.Close()
    return $total
}

# The update's payload (M <id> <of> <offset> <size> <sha>): the one folder of
# the Microsoft cabinet in it, unpacked. MSZIP blocks are "CK" and a deflate
# stream whose history is the previous block's last 32 KB: DeflateStream
# takes no dictionary, so each gets it as a stored block first, and that
# much of the output is dropped.
$memory = @{}
foreach ($l in $lines) {
    $p = $l.Split(' ')
    if ($p[0] -ne 'M' -or -not $imgs[$p[2]].Path) { continue }
    $exe = [IO.File]::ReadAllBytes($imgs[$p[2]].Path)
    $base = [int]$p[3]
    $size = [int]$p[4]
    $off = [BitConverter]::ToInt32($exe, $base + 36)
    $nblocks = [int][BitConverter]::ToUInt16($exe, $base + 40)
    $kind = [int][BitConverter]::ToUInt16($exe, $base + 42) -band 15
    $payload = New-Object byte[] ($size + 65536)
    $tmp = New-Object byte[] (32768 + 65536)
    $len = 0
    $pos = $base + $off
    for ($b = 0; $b -lt $nblocks; $b++) {
        $cb = [int][BitConverter]::ToUInt16($exe, $pos + 4)
        $cu = [int][BitConverter]::ToUInt16($exe, $pos + 6)
        if ($kind -eq 0) {
            [Array]::Copy($exe, $pos + 8, $payload, $len, $cb)
            $n = $cb
        } else {
            $w = [Math]::Min($len, 32768)
            $pre = $null
            if ($w -gt 0) {
                $pre = New-Object byte[] (5 + $w)
                $pre[1] = $w -band 255; $pre[2] = $w -shr 8
                $pre[3] = (-bnot $w) -band 255; $pre[4] = ((-bnot $w) -shr 8) -band 255
                [Array]::Copy($payload, $len - $w, $pre, 5, $w)
            }
            $n = (Expand-Deflate $pre $exe ($pos + 10) ($cb - 2) $tmp 0) - $w
            [Array]::Copy($tmp, $w, $payload, $len, $n)
        }
        if ($n -ne $cu) { Fail "$(Split-Path -Leaf $imgs[$p[2]].Path): its cabinet did not unpack right" }
        $len += $n
        $pos += 8 + $cb
    }
    $sha = [Security.Cryptography.SHA256]::Create()
    $sum = ([BitConverter]::ToString($sha.ComputeHash($payload, 0, $len)) -replace '-', '').ToLowerInvariant()
    if ($len -ne $size -or $sum -ne $p[5]) { Fail "$(Split-Path -Leaf $imgs[$p[2]].Path): its cabinet did not unpack right" }
    $memory[$p[1]] = New-Object IO.MemoryStream (, $payload)
}

# Each retail file (V <n> ...): the first way (W copy|is <id>:<offset>:<length>...)
# whose images are all here.
$ways = @{}
$vid = $null
foreach ($l in $lines) {
    $p = $l.Split(' ')
    if ($p[0] -eq 'V') { $vid = $p[1]; continue }
    if ($p[0] -ne 'W' -or $ways.ContainsKey($vid)) { continue }
    $ranges = @()
    $ok = $true
    foreach ($r in $p[2..($p.Count - 1)]) {
        $q = $r.Split(':')
        if (-not ($memory.ContainsKey($q[0]) -or ($imgs.ContainsKey($q[0]) -and $imgs[$q[0]].Path))) { $ok = $false }
        $ranges += , @($q[0], [int64]$q[1], [int64]$q[2])
    }
    if ($ok) { $ways[$vid] = @{ Kind = $p[1]; Ranges = $ranges } }
}

$streams = @{}
foreach ($id in $imgs.Keys) { if ($imgs[$id].Path) { $streams[$id] = [IO.File]::OpenRead($imgs[$id].Path) } }
foreach ($id in $memory.Keys) { $streams[$id] = $memory[$id] }

# A retail file being read, in order: its ranges, and (is) the chunk
# inflated last and how much of it is left.
function New-Reader($way) {
    return @{ Kind = $way.Kind; Ranges = $way.Ranges; Next = 0; Left = [int64]0; Src = $null
              Chunk = (New-Object byte[] 65536); At = 0; Have = 0 }
}
$raw = New-Object byte[] (65536)
# $count bytes of the ranges, joined, into $buf from $at.
function Read-Ranges($r, $buf, $at, $count) {
    while ($count -gt 0) {
        if ($r.Left -eq 0) {
            if ($r.Next -ge $r.Ranges.Count) { Fail 'a retail file ends early' }
            $q = $r.Ranges[$r.Next]
            $r.Next = $r.Next + 1
            $r.Src = $streams[$q[0]]
            $r.Src.Position = $q[1]
            $r.Left = $q[2]
        }
        $n = $r.Src.Read($buf, $at, [int][Math]::Min([int64]$count, [int64]$r.Left))
        if ($n -le 0) { Fail 'an image ended early' }
        $at += $n
        $count -= $n
        $r.Left -= $n
    }
}
# The next $count bytes of the retail file: to $dst (or dropped, $null).
function Read-File($r, $count, $dst) {
    while ($count -gt 0) {
        if ($r.Kind -eq 'copy') {
            $n = [int][Math]::Min([int64]$count, [int64]$raw.Length)
            Read-Ranges $r $raw 0 $n
            if ($dst) { $dst.Write($raw, 0, $n) }
            $count -= $n
            continue
        }
        if ($r.At -ge $r.Have) {
            Read-Ranges $r $raw 0 2
            $n = [int][BitConverter]::ToUInt16($raw, 0)
            Read-Ranges $r $raw 0 $n
            $r.Have = Expand-Deflate $null $raw 0 $n $r.Chunk 0
            $r.At = 0
        }
        $n = [int][Math]::Min([int64]$count, [int64]($r.Have - $r.At))
        if ($dst) { $dst.Write($r.Chunk, $r.At, $n) }
        $r.At += $n
        $count -= $n
    }
}

function Test-Done($path, $size, $sha) {
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { return $false }
    if (([IO.FileInfo]$path).Length -ne $size) { return $false }
    return (Get-Sha256 $path) -eq $sha
}

$zero = New-Object byte[] (1MB)
$out = $null
$written = 0
$kept = 0
try {
    foreach ($l in $lines) {
        $p = $l.Split(' ')
        switch ($p[0]) {
            'F' {
                $p = $l.Split(' ', 6)              # F <size> <sha> <flag> <n> <name, may have spaces>
                $name = $p[5]
                $vid = $p[4]
                $dest = Join-Path $here ($name.Replace('/', [IO.Path]::DirectorySeparatorChar))
                $size = [int64]$p[1]
                $sha = $p[2]
                $part = "$dest.part"
                $skip = (($p[3] -eq 'once') -and (Test-Path -LiteralPath $dest)) -or (Test-Done $dest $size $sha)
                if ($skip) { $kept++ } else {
                    Write-Host $name
                    if ($vid -ne '-' -and -not $ways.ContainsKey($vid)) { Fail "$name`: none of the images found has it" }
                    $dir = Split-Path -Parent $dest
                    if (-not (Test-Path -LiteralPath $dir)) { New-Item -ItemType Directory -Path $dir | Out-Null }
                    $out = [IO.File]::Create($part)
                    # (source '-': a file of ours, only L records)
                    $src = $null
                    if ($vid -ne '-') { $src = New-Reader $ways[$vid] }
                }
            }
            'C' {
                if ($skip) { continue }
                Read-File $src ([int64]$p[1]) $out
            }
            'Z' {
                if ($skip) { continue }
                Read-File $src ([int64]$p[1]) $null
                $left = [int64]$p[1]
                while ($left -gt 0) {
                    $n = [int][Math]::Min($left, [int64]$zero.Length)
                    $out.Write($zero, 0, $n)
                    $left -= $n
                }
            }
            'L' {
                if ($skip) { continue }
                $b = [Convert]::FromBase64String($p[1])
                if ($src) { Read-File $src $b.Length $null }
                $out.Write($b, 0, $b.Length)
            }
            'E' {
                if ($skip) { continue }
                $out.Close()
                $out = $null
                if (-not (Test-Done $part $size $sha)) {
                    Remove-Item -LiteralPath $part -Force
                    Fail "$name came out wrong (a disk error?); the files before it are in place"
                }
                Move-Item -LiteralPath $part -Destination $dest -Force
                $written++
            }
        }
    }
} finally {
    if ($out) { $out.Close() }
    foreach ($s in $streams.Values) { $s.Close() }
    Remove-Temps                    # once nothing has them open
}
$save = Join-Path $here 'Save'
if (-not (Test-Path -LiteralPath $save)) { New-Item -ItemType Directory -Path $save | Out-Null }

# The registry, in the legacy build only (the modern build's launcher,
# TronLauncher.exe, and Tron.exe play from their own folder with no
# registry at all: docs/MODERN.md APPS-3). The legacy build's launcher,
# Tron.exe, reads the values that the retail setup and Update 2 wrote, in
# HKLM\SOFTWARE\Monolith Productions\Tron 2.0\1.0 (build/launchertest.sh
# writes the same ones):
#   * InstallDir: this folder (the launcher changes to it).
#   * NetRegionCode: EN.
#   * UpdateNum, UpdateCommandLine0 and 1: the archives of Update 2,
#     Gamep3.rez and Gamep4.rez, which the launcher adds last. (Their
#     retail game DLLs are zeroed and hidden: the game DLLs built from
#     source in custom\ are the ones that the engine finds.)
# Where: Tron.exe is a 32-bit program, so it reads HKLM\SOFTWARE through
# the 32-bit view (SOFTWARE\WOW6432Node on 64-bit Windows). Writing HKLM
# needs an administrator; run without one, that is skipped. Tron.exe has no
# manifest, so UAC runs it with registry virtualization: its reads of that
# key see HKCU\Software\Classes\VirtualStore\MACHINE\SOFTWARE\[WOW6432Node\]
# ... first, and that per-user copy needs no administrator. Both get the
# same values, so whichever the launcher reads agrees. Like the retail
# launcher, the legacy one also wants disc 1 in a drive; without it, start
# the game with Lithtech.exe -cmdfile tron20cmds.txt.
$modern = Test-Path -LiteralPath (Join-Path $here 'TronLauncher.exe') -PathType Leaf
$machine = $false
$user = $false
if (-not $modern) {
    $regKey = 'Monolith Productions\Tron 2.0\1.0'
    function Set-GameKey($base, $path) {
        $k = $base.CreateSubKey($path)
        if (-not $k) { throw "cannot create $path" }
        try {
            $k.SetValue('InstallDir', $here, [Microsoft.Win32.RegistryValueKind]::String)
            $k.SetValue('NetRegionCode', 'EN', [Microsoft.Win32.RegistryValueKind]::String)
            $k.SetValue('UpdateNum', 2, [Microsoft.Win32.RegistryValueKind]::DWord)
            $k.SetValue('UpdateCommandLine0', '-rez gamep3.rez', [Microsoft.Win32.RegistryValueKind]::String)
            $k.SetValue('UpdateCommandLine1', '-rez gamep4.rez', [Microsoft.Win32.RegistryValueKind]::String)
        } finally {
            $k.Close()
        }
    }
    try {
        $hklm = [Microsoft.Win32.RegistryKey]::OpenBaseKey([Microsoft.Win32.RegistryHive]::LocalMachine,
                                                           [Microsoft.Win32.RegistryView]::Registry32)
        try { Set-GameKey $hklm "SOFTWARE\$regKey"; $machine = $true } finally { $hklm.Close() }
    } catch {
        $machine = $false            # not an administrator: the per-user copy only
    }
    $virtual = 'Software\Classes\VirtualStore\MACHINE\SOFTWARE\'
    $view = [Microsoft.Win32.RegistryView]::Default
    if ([Environment]::Is64BitOperatingSystem) {
        $virtual += 'WOW6432Node\'
        $view = [Microsoft.Win32.RegistryView]::Registry64   # as it is, from a 32-bit PowerShell too
    }
    try {
        $hkcu = [Microsoft.Win32.RegistryKey]::OpenBaseKey([Microsoft.Win32.RegistryHive]::CurrentUser, $view)
        try { Set-GameKey $hkcu "$virtual$regKey"; $user = $true } finally { $hkcu.Close() }
    } catch {
        $user = $false
    }
}

Write-Host ''
Write-Host "done: $written files written, $kept already in place." -ForegroundColor Green
if ($modern) {
    Write-Host 'Start the game with Tron.exe (or the launcher, TronLauncher.exe).'
} else {
    Write-Host 'Start the game with Tron.exe (the launcher: it wants disc 1 in a drive),'
    Write-Host 'or with Lithtech.exe -cmdfile tron20cmds.txt.'
    if ($machine) {
        Write-Host 'The game is registered for all users of this PC.'
    } elseif ($user) {
        Write-Host 'The game is registered for this Windows user (to register it for all users,'
        Write-Host 'run windows-install.bat again as an administrator).'
    } else {
        Write-Host 'Could not write the game into the registry: Tron.exe may not start it.' -ForegroundColor Yellow
    }
}
exit 0
