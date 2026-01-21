<#
.SYNOPSIS
    Test script for Agent-Sentinel.
    Verifies that the agent responds correctly to the "my name is antigravity" prompt
    and checks for the repetition loop or empty response issues.
#>

$AgentPath = ".\build\agentic.exe"
$LogFile = "log.txt"

if (!(Test-Path $AgentPath)) {
    Write-Host "Error: AgentSentinel.exe not found at $AgentPath" -ForegroundColor Red
    exit 1
}

Write-Host "Starting AgentSentinel test..." -ForegroundColor Cyan

# Prepare the input process
$ProcessInfo = New-Object System.Diagnostics.ProcessStartInfo
$ProcessInfo.FileName = $AgentPath
$ProcessInfo.RedirectStandardInput = $true
$ProcessInfo.RedirectStandardOutput = $true
$ProcessInfo.RedirectStandardError = $true # Captured in log.txt per main.cpp, but checking here recursively
$ProcessInfo.UseShellExecute = $false
$ProcessInfo.CreateNoWindow = $true

$Process = New-Object System.Diagnostics.Process
$Process.StartInfo = $ProcessInfo

# Start the process
$Process.Start() | Out-Null

# Wait a bit for initialization (model loading)
Write-Host "Waiting for model to load..."
Start-Sleep -Seconds 5

# Send the first prompt
$Prompt1 = "my name is antigravity"
Write-Host "Sending prompt: '$Prompt1'"
$Process.StandardInput.WriteLine($Prompt1)

# Wait for response generation
Write-Host "Waiting for first response..."
Start-Sleep -Seconds 12

# Send the second prompt to test memory
$Prompt2 = "what is my name?"
Write-Host "Sending prompt: '$Prompt2' (Testing memory...)"
$Process.StandardInput.WriteLine($Prompt2)

# Wait for second response
Write-Host "Waiting for second response..."
Start-Sleep -Seconds 12

# Send exit command
$Process.StandardInput.WriteLine("exit")

# Capture output
$Output = $Process.StandardOutput.ReadToEnd()
$Process.WaitForExit()

Write-Host "`n--- Output ---"
Write-Host $Output
Write-Host "--------------"

# Verification Logic
if ($Output -match "(?s)Assistant:.*(Nice to meet you|Hello|Hi).*Antigravity") {
    Write-Host "`n[PASS] Response contains expected greeting." -ForegroundColor Green
} else {
    Write-Host "`n[WARN] Response might be empty or unexpected. Check log.txt. Regex check failed." -ForegroundColor Yellow
}

if ($Output -match "User:.*User:.*User:") {
     Write-Host "[FAIL] Repetition loop detected!" -ForegroundColor Red
} else {
    Write-Host "[PASS] No repetition loop detected." -ForegroundColor Green
}

Write-Host "`nTest complete."
