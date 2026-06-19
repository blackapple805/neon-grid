# quickshot.ps1 — countdown, then capture whatever window is focused
param([string]$Out = "docs\screenshot.png")

Add-Type -AssemblyName System.Drawing
Add-Type @"
using System; using System.Runtime.InteropServices;
public class W {
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out R r);
  [StructLayout(LayoutKind.Sequential)] public struct R { public int L,T,Ri,B; }
}
"@

Write-Host "Click the terminal running NEONGRID now..."
5..1 | ForEach-Object { Write-Host "  capturing in $_"; Start-Sleep 1 }

$h  = [W]::GetForegroundWindow()
$rc = New-Object W+R
[W]::GetWindowRect($h, [ref]$rc) | Out-Null

$w  = $rc.Ri - $rc.L
$ht = $rc.B  - $rc.T

if ($w -le 0 -or $ht -le 0) {
    Write-Host "Could not read window size (got ${w}x${ht}). Make sure a normal window is focused." -ForegroundColor Red
    return
}

$bmp = New-Object System.Drawing.Bitmap $w, $ht
$g   = [System.Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($rc.L, $rc.T, 0, 0, $bmp.Size)

$dir = Split-Path $Out -Parent
if ($dir -and -not (Test-Path $dir)) { New-Item -ItemType Directory -Path $dir | Out-Null }

$bmp.Save((Join-Path (Get-Location) $Out), [System.Drawing.Imaging.ImageFormat]::Png)
$g.Dispose(); $bmp.Dispose()
Write-Host "Saved $Out ($w x $ht)" -ForegroundColor Green