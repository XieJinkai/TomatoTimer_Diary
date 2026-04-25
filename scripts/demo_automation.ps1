param(
    [string]$ExePath = (Join-Path $PSScriptRoot "..\build\Debug\tomato_timer.exe"),
    [string]$DemoUser = "demo_presenter",
    [string]$DemoPassword = "123456",
    [switch]$CloseOnFinish
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

Add-Type -AssemblyName UIAutomationClient
Add-Type -AssemblyName UIAutomationTypes
Add-Type -AssemblyName System.Windows.Forms

Add-Type @"
using System;
using System.Runtime.InteropServices;
public static class DemoNative {
    [DllImport("user32.dll")]
    public static extern bool SetForegroundWindow(IntPtr hWnd);
}
"@

function Write-Step {
    param([string]$Message)
    Write-Host ("[DEMO] " + $Message) -ForegroundColor Cyan
}

function Wait-Until {
    param(
        [scriptblock]$Condition,
        [int]$TimeoutSeconds = 15,
        [int]$PollMilliseconds = 200,
        [string]$Description = "condition"
    )

    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    while((Get-Date) -lt $deadline){
        $result = & $Condition
        if($null -ne $result -and $result -ne $false){
            return $result
        }
        Start-Sleep -Milliseconds $PollMilliseconds
    }
    throw "Timed out waiting for $Description"
}

function Get-RootElement {
    [System.Windows.Automation.AutomationElement]::RootElement
}

function Get-WindowsForProcess {
    param([int]$ProcessId)
    $condition = New-Object System.Windows.Automation.PropertyCondition(
        [System.Windows.Automation.AutomationElement]::ProcessIdProperty,
        $ProcessId
    )
    $collection = (Get-RootElement).FindAll([System.Windows.Automation.TreeScope]::Children, $condition)
    $windows = @()
    for($i = 0; $i -lt $collection.Count; $i++){
        $windows += $collection.Item($i)
    }
    return ,$windows
}

function Find-ElementsByType {
    param(
        [System.Windows.Automation.AutomationElement]$Root,
        [System.Windows.Automation.ControlType]$ControlType
    )
    $condition = New-Object System.Windows.Automation.PropertyCondition(
        [System.Windows.Automation.AutomationElement]::ControlTypeProperty,
        $ControlType
    )
    $collection = $Root.FindAll([System.Windows.Automation.TreeScope]::Descendants, $condition)
    $items = @()
    for($i = 0; $i -lt $collection.Count; $i++){
        $items += $collection.Item($i)
    }
    return ,$items
}

function Get-VisibleElementsByType {
    param(
        [System.Windows.Automation.AutomationElement]$Root,
        [System.Windows.Automation.ControlType]$ControlType
    )

    $collection = Find-ElementsByType -Root $Root -ControlType $ControlType
    $items = @()
    for($i = 0; $i -lt $collection.Count; $i++){
        $item = $collection[$i]
        if(-not $item.Current.IsOffscreen){
            $items += $item
        }
    }
    return ,$items
}

function Get-ContentButtons {
    param([System.Windows.Automation.AutomationElement]$Window)
    $buttons = Get-VisibleElementsByType -Root $Window -ControlType ([System.Windows.Automation.ControlType]::Button)
    if($buttons.Count -le 3){
        return @()
    }

    $start = 3
    $end = $buttons.Count - 1
    if($buttons.Count -ge 7){
        $end = $buttons.Count - 4
    }

    $result = @()
    for($i = $start; $i -le $end; $i++){
        $result += $buttons[$i]
    }
    return ,$result
}

function Invoke-Element {
    param([System.Windows.Automation.AutomationElement]$Element)
    try {
        $pattern = $Element.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern)
        $pattern.Invoke()
        return
    } catch {}

    try {
        $pattern = $Element.GetCurrentPattern([System.Windows.Automation.SelectionItemPattern]::Pattern)
        $pattern.Select()
        return
    } catch {}

    $Element.SetFocus()
    Start-Sleep -Milliseconds 100
    [System.Windows.Forms.SendKeys]::SendWait(" ")
}

function Set-WindowForeground {
    param([System.Windows.Automation.AutomationElement]$Window)
    $handle = [IntPtr]$Window.Current.NativeWindowHandle
    [void][DemoNative]::SetForegroundWindow($handle)
    Start-Sleep -Milliseconds 250
}

function Set-ElementValue {
    param(
        [System.Windows.Automation.AutomationElement]$Element,
        [string]$Value
    )

    try {
        $pattern = $Element.GetCurrentPattern([System.Windows.Automation.ValuePattern]::Pattern)
        $pattern.SetValue($Value)
        return
    } catch {}

    [System.Windows.Forms.Clipboard]::SetText($Value)
    $Element.SetFocus()
    Start-Sleep -Milliseconds 120
    [System.Windows.Forms.SendKeys]::SendWait("^a")
    Start-Sleep -Milliseconds 120
    [System.Windows.Forms.SendKeys]::SendWait("^v")
}

function Select-ComboAbsoluteIndex {
    param(
        [System.Windows.Automation.AutomationElement]$Combo,
        [int]$ItemIndex
    )

    $Combo.SetFocus()
    Start-Sleep -Milliseconds 150
    [System.Windows.Forms.SendKeys]::SendWait("%{DOWN}")
    Start-Sleep -Milliseconds 250
    [System.Windows.Forms.SendKeys]::SendWait("{HOME}")
    Start-Sleep -Milliseconds 120
    for($i = 0; $i -lt $ItemIndex; $i++){
        [System.Windows.Forms.SendKeys]::SendWait("{DOWN}")
        Start-Sleep -Milliseconds 80
    }
    [System.Windows.Forms.SendKeys]::SendWait("{ENTER}")
    Start-Sleep -Milliseconds 250
}

function Get-LoginWindow {
    param([System.Diagnostics.Process]$Process)
    Wait-Until -Description "login window" -Condition {
        $windows = Get-WindowsForProcess -ProcessId $Process.Id
        for($i = 0; $i -lt $windows.Count; $i++){
            $window = $windows[$i]
            $edits = Get-VisibleElementsByType -Root $window -ControlType ([System.Windows.Automation.ControlType]::Edit)
            $buttons = Get-ContentButtons -Window $window
            if($edits.Count -ge 2 -and $buttons.Count -ge 2){
                return $window
            }
        }
        return $null
    }
}

function Get-MainWindow {
    param([System.Diagnostics.Process]$Process)
    Wait-Until -Description "main window" -Condition {
        $windows = Get-WindowsForProcess -ProcessId $Process.Id
        for($i = 0; $i -lt $windows.Count; $i++){
            $window = $windows[$i]
            $tabs = Get-VisibleElementsByType -Root $window -ControlType ([System.Windows.Automation.ControlType]::TabItem)
            if($tabs.Count -ge 7){
                return $window
            }
        }
        return $null
    }
}

function Select-TabByIndex {
    param(
        [System.Windows.Automation.AutomationElement]$Window,
        [int]$Index
    )
    $tabs = Get-VisibleElementsByType -Root $Window -ControlType ([System.Windows.Automation.ControlType]::TabItem)
    if($tabs.Count -le $Index){
        throw "Tab index $Index not found"
    }
    Invoke-Element $tabs[$Index]
    Start-Sleep -Milliseconds 700
}

function Get-VisibleEdits {
    param([System.Windows.Automation.AutomationElement]$Window)
    Get-VisibleElementsByType -Root $Window -ControlType ([System.Windows.Automation.ControlType]::Edit)
}

function Get-VisibleCombos {
    param([System.Windows.Automation.AutomationElement]$Window)
    Get-VisibleElementsByType -Root $Window -ControlType ([System.Windows.Automation.ControlType]::ComboBox)
}

function Initialize-DemoData {
    param(
        [string]$Username,
        [string]$Password
    )

    Write-Step "Preparing demo account and seed data"

    $appRoot = Join-Path $env:APPDATA "Trae\TomatoTimerQt"
    $userDir = Join-Path $appRoot $Username
    New-Item -ItemType Directory -Force -Path $userDir | Out-Null
    Set-Content -Path (Join-Path $userDir "credentials.txt") -Value $Password -Encoding UTF8

    $today = Get-Date
    $records = @(
        @{
            time = $today.AddDays(-3).ToString("s")
            item = "Lunch"
            amount = 28.50
            type = [string]::Concat([char]39184, [char]39278)
            note = "seed data"
        },
        @{
            time = $today.AddDays(-2).ToString("s")
            item = "Metro"
            amount = 4.00
            type = [string]::Concat([char]20132, [char]36890)
            note = "seed data"
        },
        @{
            time = $today.AddDays(-1).ToString("s")
            item = "Books"
            amount = 56.00
            type = [string]::Concat([char]23398, [char]20064)
            note = "seed data"
        }
    )
    $records | ConvertTo-Json | Set-Content -Path (Join-Path $userDir "accounting.json") -Encoding UTF8

    $diaryText = @"
Today is the prepared demo session.
The script will continue editing this page during the live walkthrough.
"@
    Set-Content -Path (Join-Path $userDir ($today.ToString("yyyy-MM-dd") + ".txt")) -Value $diaryText -Encoding UTF8
}

function Login-AndOpenMainWindow {
    param(
        [System.Diagnostics.Process]$Process,
        [string]$Username,
        [string]$Password
    )

    $loginWindow = Get-LoginWindow -Process $Process
    Set-WindowForeground $loginWindow
    Write-Step "Logging in"

    $edits = Get-VisibleEdits -Window $loginWindow
    Set-ElementValue -Element $edits[0] -Value $Username
    Set-ElementValue -Element $edits[1] -Value $Password

    $buttons = Get-ContentButtons -Window $loginWindow
    Invoke-Element $buttons[1]
    Start-Sleep -Seconds 1

    $mainWindow = Get-MainWindow -Process $Process
    Set-WindowForeground $mainWindow
    return $mainWindow
}

function Demo-Pomodoro {
    param([System.Windows.Automation.AutomationElement]$Window)
    Write-Step "Showing pomodoro"
    Select-TabByIndex -Window $Window -Index 0
    $edits = Get-VisibleEdits -Window $Window
    if($edits.Count -ge 1){
        Set-ElementValue -Element $edits[0] -Value "Defense prep"
    }
    $buttons = Get-ContentButtons -Window $Window
    Invoke-Element $buttons[0]
    Start-Sleep -Seconds 2
    Invoke-Element $buttons[3]
    Start-Sleep -Seconds 1
}

function Demo-Stopwatch {
    param([System.Windows.Automation.AutomationElement]$Window)
    Write-Step "Showing stopwatch"
    Select-TabByIndex -Window $Window -Index 1
    $edits = Get-VisibleEdits -Window $Window
    if($edits.Count -ge 1){
        Set-ElementValue -Element $edits[0] -Value "Demo narration rehearsal"
    }
    $buttons = Get-ContentButtons -Window $Window
    Invoke-Element $buttons[0]
    Start-Sleep -Seconds 3
    Invoke-Element $buttons[1]
    Start-Sleep -Seconds 1
}

function Add-AccountingRecord {
    param(
        [System.Windows.Automation.AutomationElement]$Window,
        [string]$Item,
        [string]$Amount,
        [int]$TypeIndex,
        [string]$Note
    )

    $edits = Get-VisibleEdits -Window $Window
    $combos = Get-VisibleCombos -Window $Window
    if($edits.Count -lt 4 -or $combos.Count -lt 1){
        throw "Accounting controls not found"
    }

    Set-ElementValue -Element $edits[0] -Value $Item
    Set-ElementValue -Element $edits[1] -Value $Amount
    Select-ComboAbsoluteIndex -Combo $combos[0] -ItemIndex $TypeIndex
    Set-ElementValue -Element $edits[2] -Value $Note

    $buttons = Get-ContentButtons -Window $Window
    Invoke-Element $buttons[0]
    Start-Sleep -Milliseconds 700
}

function Demo-Accounting {
    param([System.Windows.Automation.AutomationElement]$Window)
    Write-Step "Showing accounting"
    Select-TabByIndex -Window $Window -Index 3

    Add-AccountingRecord -Window $Window -Item "Coffee" -Amount "18.00" -TypeIndex 0 -Note "Afternoon"
    Add-AccountingRecord -Window $Window -Item "Stationery" -Amount "12.50" -TypeIndex 4 -Note "Pens"
    Add-AccountingRecord -Window $Window -Item "Supplies" -Amount "23.90" -TypeIndex 2 -Note "Daily use"

    $combos = Get-VisibleCombos -Window $Window
    Select-ComboAbsoluteIndex -Combo $combos[2] -ItemIndex 1
    Start-Sleep -Seconds 1
    Select-ComboAbsoluteIndex -Combo $combos[2] -ItemIndex 0
    Start-Sleep -Milliseconds 500

    $edits = Get-VisibleEdits -Window $Window
    Set-ElementValue -Element $edits[3] -Value "Coffee"
    Start-Sleep -Seconds 1
    Set-ElementValue -Element $edits[3] -Value ""
    Start-Sleep -Seconds 1
}

function Demo-Diary {
    param([System.Windows.Automation.AutomationElement]$Window)
    Write-Step "Showing diary"
    Select-TabByIndex -Window $Window -Index 4
    $edits = Get-VisibleEdits -Window $Window
    if($edits.Count -ge 1){
        $text = @"
Today the live demo covered pomodoro, stopwatch, accounting, diary, and statistics.
The goal is to show an integrated desktop workflow for personal management.
"@
        Set-ElementValue -Element $edits[0] -Value $text.Trim()
    }
    $buttons = Get-ContentButtons -Window $Window
    Invoke-Element $buttons[2]
    Start-Sleep -Seconds 1
}

function Demo-Stats {
    param([System.Windows.Automation.AutomationElement]$Window)
    Write-Step "Showing statistics"
    Select-TabByIndex -Window $Window -Index 2
    Start-Sleep -Seconds 1
    $buttons = Get-ContentButtons -Window $Window
    foreach($button in $buttons){
        Invoke-Element $button
        Start-Sleep -Milliseconds 700
    }
    Start-Sleep -Seconds 1
}

function Demo-Settings {
    param([System.Windows.Automation.AutomationElement]$Window)
    Write-Step "Showing settings and sync"
    Select-TabByIndex -Window $Window -Index 6
    Start-Sleep -Seconds 2
}

if(-not (Test-Path $ExePath)){
    throw "Executable not found: $ExePath"
}

Initialize-DemoData -Username $DemoUser -Password $DemoPassword

Write-Step "Starting application"
$process = Start-Process -FilePath $ExePath -PassThru

try {
    $mainWindow = Login-AndOpenMainWindow -Process $process -Username $DemoUser -Password $DemoPassword
    Demo-Pomodoro -Window $mainWindow
    Demo-Stopwatch -Window $mainWindow
    Demo-Accounting -Window $mainWindow
    Demo-Diary -Window $mainWindow
    Demo-Stats -Window $mainWindow
    Demo-Settings -Window $mainWindow

    Write-Step "Demo flow completed"
    if($CloseOnFinish){
        Stop-Process -Id $process.Id -Force
    } else {
        Write-Host "Application remains open for recording." -ForegroundColor Yellow
    }
}
catch {
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host $_.ScriptStackTrace -ForegroundColor DarkYellow
    if($process -and -not $process.HasExited){
        Stop-Process -Id $process.Id -Force
    }
    throw
}
