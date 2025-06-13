# Approval Testing Helper Script
# Helps developers review and approve changes to approval test baselines

param(
    [string]$TestName = "",
    [switch]$ListChanges,
    [switch]$ApproveAll,
    [switch]$ViewImages
)

Write-Host "Approval Testing Helper" -ForegroundColor Green
Write-Host "=======================" -ForegroundColor Green

$ArtifactsDir = "tests\approval_artifacts"

if (!(Test-Path $ArtifactsDir)) {
    Write-Host "ERROR: Approval artifacts directory not found" -ForegroundColor Red
    Write-Host "Make sure you're running this from the ImageAnalysis directory" -ForegroundColor Red
    exit 1
}

# Find received files (indicating changes/failures)
$receivedFiles = Get-ChildItem "$ArtifactsDir\*.received.*" -ErrorAction SilentlyContinue

if ($receivedFiles.Count -eq 0) {
    Write-Host "✅ No pending approval changes found" -ForegroundColor Green
    Write-Host "All tests are passing with current baselines" -ForegroundColor Green
    exit 0
}

if ($ListChanges) {
    Write-Host "📋 Pending approval changes:" -ForegroundColor Yellow
    foreach ($file in $receivedFiles) {
        $testName = $file.BaseName -replace '\.received$', ''
        $fileType = $file.Extension
        Write-Host "  • $testName ($fileType)" -ForegroundColor Cyan
    }
    exit 0
}

if ($ViewImages) {
    Write-Host "🖼️  Opening received images for review..." -ForegroundColor Cyan
    $imageFiles = $receivedFiles | Where-Object { $_.Extension -eq ".png" }
    foreach ($img in $imageFiles) {
        Write-Host "Opening: $($img.Name)" -ForegroundColor Gray
        & code $img.FullName
    }
    exit 0
}

if ($ApproveAll) {
    Write-Host "⚠️  Approving ALL changes..." -ForegroundColor Yellow
    foreach ($file in $receivedFiles) {
        $approved = $file.FullName -replace '\.received\.', '.approved.'
        Copy-Item $file.FullName $approved -Force
        Write-Host "✅ Approved: $($file.BaseName)" -ForegroundColor Green
    }
    
    # Clean up received files
    $receivedFiles | Remove-Item -Force
    Write-Host "🧹 Cleaned up received files" -ForegroundColor Cyan
    Write-Host "✅ All changes approved!" -ForegroundColor Green
    exit 0
}

if ($TestName) {
    # Approve specific test
    $testFiles = $receivedFiles | Where-Object { $_.BaseName -like "*$TestName*" }
    if ($testFiles.Count -eq 0) {
        Write-Host "❌ No received files found for test: $TestName" -ForegroundColor Red
        exit 1
    }
    
    Write-Host "📝 Approving changes for: $TestName" -ForegroundColor Yellow
    foreach ($file in $testFiles) {
        $approved = $file.FullName -replace '\.received\.', '.approved.'
        Copy-Item $file.FullName $approved -Force
        Remove-Item $file.FullName -Force
        Write-Host "✅ Approved: $($file.Name)" -ForegroundColor Green
    }
    exit 0
}

# Default behavior - show help and pending changes
Write-Host "📋 Found $($receivedFiles.Count) pending approval change(s):" -ForegroundColor Yellow

$groupedFiles = $receivedFiles | Group-Object { ($_.BaseName -split '\.received')[0] }
foreach ($group in $groupedFiles) {
    $testName = $group.Name
    $files = $group.Group
    Write-Host ""
    Write-Host "🔍 Test: $testName" -ForegroundColor Cyan
    
    foreach ($file in $files) {
        $fileType = if ($file.Extension -eq ".txt") { "📄 Text" } else { "🖼️  Image" }
        Write-Host "   $fileType : $($file.Name)" -ForegroundColor Gray
        
        # Show diff command for text files
        if ($file.Extension -eq ".txt") {
            $approved = $file.FullName -replace '\.received\.', '.approved.'
            if (Test-Path $approved) {
                Write-Host "   📋 Diff : code --diff `"$approved`" `"$($file.FullName)`"" -ForegroundColor DarkGray
            }
        }
    }
}

Write-Host ""
Write-Host "Usage Examples:" -ForegroundColor Yellow
Write-Host "  .\approve_changes.ps1 -ListChanges              # List all pending changes" -ForegroundColor Gray
Write-Host "  .\approve_changes.ps1 -ViewImages               # Open all received images" -ForegroundColor Gray
Write-Host "  .\approve_changes.ps1 -TestName 'spin_ball_1'   # Approve specific test" -ForegroundColor Gray
Write-Host "  .\approve_changes.ps1 -ApproveAll               # Approve all changes" -ForegroundColor Gray
Write-Host ""
Write-Host "⚠️  Review changes carefully before approving!" -ForegroundColor Yellow
