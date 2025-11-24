# Version bump script for OBS-WebRTC-Link (PowerShell)
# Usage: .\scripts\bump-version.ps1 [major|minor|patch]

param(
    [Parameter(Position=0)]
    [ValidateSet('major', 'minor', 'patch')]
    [string]$BumpType = 'patch'
)

# Get script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir
$VersionFile = Join-Path $ProjectRoot "VERSION"
$ChangelogFile = Join-Path $ProjectRoot "CHANGELOG.md"

# Check if VERSION file exists
if (-not (Test-Path $VersionFile)) {
    Write-Host "Error: VERSION file not found at $VersionFile" -ForegroundColor Red
    exit 1
}

# Read current version
$CurrentVersion = (Get-Content $VersionFile).Trim()
Write-Host "Current version: $CurrentVersion" -ForegroundColor Green

# Parse version components
$VersionParts = $CurrentVersion -split '\.'
$Major = [int]$VersionParts[0]
$Minor = [int]$VersionParts[1]
$Patch = [int]$VersionParts[2]

# Determine bump type
switch ($BumpType) {
    'major' {
        $Major++
        $Minor = 0
        $Patch = 0
    }
    'minor' {
        $Minor++
        $Patch = 0
    }
    'patch' {
        $Patch++
    }
}

$NewVersion = "$Major.$Minor.$Patch"
Write-Host "New version: $NewVersion" -ForegroundColor Green

# Confirm with user
$Confirmation = Read-Host "Bump version from $CurrentVersion to $NewVersion? (y/N)"
if ($Confirmation -notmatch '^[Yy]$') {
    Write-Host "Version bump cancelled" -ForegroundColor Yellow
    exit 0
}

# Update VERSION file
Set-Content -Path $VersionFile -Value $NewVersion -NoNewline
Write-Host "Updated VERSION file" -ForegroundColor Green

# Update CHANGELOG.md
$Today = Get-Date -Format "yyyy-MM-dd"
if (Test-Path $ChangelogFile) {
    # Create backup
    Copy-Item $ChangelogFile "$ChangelogFile.bak"

    # Read changelog content
    $ChangelogContent = Get-Content $ChangelogFile -Raw

    # Replace [Unreleased] with new version
    $ChangelogContent = $ChangelogContent -replace '## \[Unreleased\]', "## [Unreleased]`n`n## [$NewVersion] - $Today"

    # Update version links at bottom
    $ChangelogContent = $ChangelogContent -replace '\[Unreleased\]:.*', "[Unreleased]: https://github.com/m96-chan/OBS-WebRTC-Link/compare/v$NewVersion...HEAD`n[$NewVersion]: https://github.com/m96-chan/OBS-WebRTC-Link/compare/v$CurrentVersion...v$NewVersion"

    # Write updated content
    Set-Content -Path $ChangelogFile -Value $ChangelogContent -NoNewline

    # Remove backup
    Remove-Item "$ChangelogFile.bak"

    Write-Host "Updated CHANGELOG.md" -ForegroundColor Green
} else {
    Write-Host "Warning: CHANGELOG.md not found" -ForegroundColor Yellow
}

# Git operations
Write-Host "Creating git commit and tag..." -ForegroundColor Yellow

# Check if git is available
try {
    $GitStatus = git status --porcelain VERSION CHANGELOG.md 2>&1
    if ($GitStatus) {
        git add VERSION CHANGELOG.md
        git commit -m "chore: bump version to $NewVersion

- Updated VERSION file
- Updated CHANGELOG.md with release date

[skip ci]"

        # Create git tag
        git tag -a "v$NewVersion" -m "Release version $NewVersion"

        Write-Host "Created commit and tag v$NewVersion" -ForegroundColor Green
        Write-Host "To push changes, run:" -ForegroundColor Yellow
        Write-Host "  git push origin main" -ForegroundColor White
        Write-Host "  git push origin v$NewVersion" -ForegroundColor White
    } else {
        Write-Host "No changes to commit" -ForegroundColor Yellow
    }
} catch {
    Write-Host "Git operations skipped (git not available or not in a repository)" -ForegroundColor Yellow
}

Write-Host "Version bump complete!" -ForegroundColor Green
