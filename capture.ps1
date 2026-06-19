# capture.ps1 — launch the app, let it render, screenshot the window to docs/screenshot.png
# Usage:  .\capture.ps1 -Args "--blood" -Delay 3

param(
    [string]$RunArgs = "--blood",   # args passed to dotnet run
    [int]$Delay      = 3,           # seconds to wait before capturing
    [string]$Out     = "docs\screenshot.png"
)

Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName System.Windows.Forms

# Win32 calls to find the new console window and get its bounds.
Add-Type @"
using System;
using System.Runtime.InteropServices;
public class Win {
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT r);
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L,T,R,B; }
}
"@

# Make sure the output folder exists.
$dir = Split-Path $Out -Parent
if ($dir -and -not (Test-Path $dir)) { New-Item -ItemType Directory -Path $dir | Out-Null }

# Launch the program in a brand-new window so we can grab its handle.
$proc = Start-Process -FilePath "dotnet" -ArgumentList "run -- $RunArgs" -PassThru
Start-Sleep -Seconds $Delay   # let the rain fill the screen

# Bring it to the front and read its window rectangle.
[Win]::SetForegroundWindow($proc.MainWindowHandle) | Out-Null
Start-Sleep -Milliseconds 400
$r = New-Object Win+RECT
[Win]::GetWindowRect($proc.MainWindowHandle, [ref]$r) | Out-Null

$w = $r.R - $r.L
$h = $r.B - $r.T
$bmp = New-Object System.Drawing.Bitmap $w, $h
$g   = [System.Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($r.L, $r.T, 0, 0, $bmp.Size)
$bmp.Save((Resolve-Path -LiteralPath $dir).Path + "\" + (Split-Path $Out -Leaf), [System.Drawing.Imaging.ImageFormat]::Png)

$g.Dispose(); $bmp.Dispose()

# Close the program.
Stop-Process -Id $proc.Id -ErrorAction SilentlyContinue
Write-Host "Saved screenshot to $Out"