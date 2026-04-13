param(
    [string]$TokenFile = "C:\Users\gadin\Desktop\temporal\logosnodehostinger\Foryouenv.txt",
    [string]$RepoName = "logos-node-inspector",
    [string]$Description = "Read-only Basecamp module for inspecting a single Logos VPS.",
    [string]$RepoUrl = "",
    [switch]$ForceRemoteOverwrite,
    [switch]$Private
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
if ($null -ne (Get-Variable -Name PSNativeCommandUseErrorActionPreference -ErrorAction SilentlyContinue)) {
    $PSNativeCommandUseErrorActionPreference = $false
}

function Get-GitHubContext {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        throw "Token file not found: $Path"
    }

    $lines = Get-Content -Raw -LiteralPath $Path |
        Select-String -Pattern ".*" -AllMatches |
        ForEach-Object { $_.Matches.Value.Trim() } |
        Where-Object { $_ -ne "" }

    $token = $null
    $user = $null
    $repoUrl = $null

    foreach ($line in $lines) {
        if (-not $token -and ($line -like "GitHubToken:*" -or $line -like "githubtoken:*")) {
            $token = ($line -split ":", 2)[1].Trim()
            continue
        }

        if (-not $user -and ($line -like "GitHubUser:*" -or $line -like "githubuser:*")) {
            $user = ($line -split ":", 2)[1].Trim()
            continue
        }

        if (-not $repoUrl -and ($line -like "GitHubRepoUrl:*" -or $line -like "githubrepourl:*")) {
            $repoUrl = ($line -split ":", 2)[1].Trim()
            continue
        }
    }

    if (-not $token) {
        $tokenIndex = -1
        for ($i = 0; $i -lt $lines.Count; $i++) {
            if ($lines[$i] -like "github_pat_*" -or $lines[$i] -like "ghp_*") {
                $tokenIndex = $i
                break
            }
        }

        if ($tokenIndex -lt 0) {
            throw "GitHub token not found in $Path"
        }

        $token = $lines[$tokenIndex]

        if (-not $user) {
            if ($tokenIndex + 1 -ge $lines.Count) {
                throw "GitHub username not found immediately after the token in $Path"
            }

            $user = $lines[$tokenIndex + 1]
        }
    }

    if (-not $user) {
        throw "GitHub username not found in $Path"
    }

    return @{
        Token = $token
        User = $user
        RepoUrl = $repoUrl
    }
}

function Invoke-GitHub {
    param(
        [string]$Method,
        [string]$Uri,
        [hashtable]$Headers,
        [object]$Body = $null
    )

    if ($null -eq $Body) {
        return Invoke-RestMethod -Method $Method -Uri $Uri -Headers $Headers
    }

    return Invoke-RestMethod -Method $Method -Uri $Uri -Headers $Headers -Body ($Body | ConvertTo-Json) -ContentType "application/json"
}

function Get-StatusCode {
    param([System.Management.Automation.ErrorRecord]$ErrorRecord)

    if ($ErrorRecord.Exception.Response) {
        try {
            return [int]$ErrorRecord.Exception.Response.StatusCode
        } catch {
            return $null
        }
    }

    return $null
}

function Write-TokenGuidance {
    param(
        [string]$User,
        [string]$RepoName,
        [string]$TokenFilePath
    )

    $repoWebUrl = "https://github.com/$User/$RepoName"
    $repoCloneUrl = "$repoWebUrl.git"

    Write-Host ""
    Write-Host "GitHub refused to create the repository with this token." -ForegroundColor Yellow
    Write-Host "The POST /user/repos endpoint requires either:" -ForegroundColor Yellow
    Write-Host "  - a fine-grained token with repository Administration: write" -ForegroundColor Yellow
    Write-Host "  - or a classic PAT with public_repo (public repo) or repo (private repo)" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "If you are using a fine-grained token limited to selected repositories," -ForegroundColor Yellow
    Write-Host "it usually cannot create a brand-new repository. In that case:" -ForegroundColor Yellow
    Write-Host "  1. Create the empty repo in the browser: $repoWebUrl" -ForegroundColor Yellow
    Write-Host "  2. Add this line to $TokenFilePath" -ForegroundColor Yellow
    Write-Host "     GitHubRepoUrl: $repoCloneUrl" -ForegroundColor Yellow
    Write-Host "  3. Re-run this script" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "If you want the script to create the repo for you, generate a token with the required permissions and try again." -ForegroundColor Yellow
}

function Test-PlaceholderRepoUrl {
    param([string]$Value)

    if (-not $Value) {
        return $false
    }

    return $Value -match "YOUR-USERNAME"
}

function Assert-LastExitCode {
    param([string]$Context)

    if ($LASTEXITCODE -ne 0) {
        throw "$Context failed with exit code $LASTEXITCODE"
    }
}

function Write-NonFastForwardGuidance {
    param([string]$CloneUrl)

    Write-Host ""
    Write-Host "The remote repository already contains commits that are not in your local branch." -ForegroundColor Yellow
    Write-Host "This usually happens when the GitHub repo was initialized with a README, license, or .gitignore." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "If that remote history is disposable, re-run the script with:" -ForegroundColor Yellow
    Write-Host "  -ForceRemoteOverwrite" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "That will use 'git push --force-with-lease' against:" -ForegroundColor Yellow
    Write-Host "  $CloneUrl" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "If you want to keep the remote commit instead, fetch/rebase manually before pushing." -ForegroundColor Yellow
}

function Invoke-GitProcess {
    param(
        [string[]]$Arguments
    )

    $stdoutPath = [System.IO.Path]::GetTempFileName()
    $stderrPath = [System.IO.Path]::GetTempFileName()

    try {
        $process = Start-Process -FilePath "git" -ArgumentList $Arguments -NoNewWindow -Wait -PassThru -RedirectStandardOutput $stdoutPath -RedirectStandardError $stderrPath
        $stdout = if (Test-Path -LiteralPath $stdoutPath) { Get-Content -LiteralPath $stdoutPath } else { @() }
        $stderr = if (Test-Path -LiteralPath $stderrPath) { Get-Content -LiteralPath $stderrPath } else { @() }

        return @{
            ExitCode = $process.ExitCode
            StdOut = @($stdout)
            StdErr = @($stderr)
        }
    } finally {
        Remove-Item -LiteralPath $stdoutPath, $stderrPath -Force -ErrorAction SilentlyContinue
    }
}

$context = Get-GitHubContext -Path $TokenFile
$headers = @{
    Authorization = "Bearer $($context.Token)"
    Accept = "application/vnd.github+json"
    "X-GitHub-Api-Version" = "2022-11-28"
    "User-Agent" = "logos-node-inspector-bootstrap"
}

$repoApiUrl = "https://api.github.com/repos/$($context.User)/$RepoName"
$cloneUrl = $null

if ($RepoUrl) {
    $cloneUrl = $RepoUrl
    Write-Host "Using repo URL from parameter: $cloneUrl"
} elseif ($context.RepoUrl) {
    $cloneUrl = $context.RepoUrl
    Write-Host "Using repo URL from token file: $cloneUrl"
} else {
    try {
        $existing = Invoke-GitHub -Method Get -Uri $repoApiUrl -Headers $headers
        $cloneUrl = $existing.clone_url
        Write-Host "Repository already exists: $cloneUrl"
    } catch {
        $status = Get-StatusCode -ErrorRecord $_

        if ($status -ne 404) {
            throw
        }

        try {
            $created = Invoke-GitHub -Method Post -Uri "https://api.github.com/user/repos" -Headers $headers -Body @{
                name = $RepoName
                description = $Description
                private = [bool]$Private
                has_issues = $true
                has_projects = $false
                has_wiki = $false
            }

            $cloneUrl = $created.clone_url
            Write-Host "Created repository: $cloneUrl"
        } catch {
            $createStatus = Get-StatusCode -ErrorRecord $_
            if ($createStatus -eq 403) {
                Write-TokenGuidance -User $context.User -RepoName $RepoName -TokenFilePath $TokenFile
            }

            throw
        }
    }
}

if (Test-PlaceholderRepoUrl -Value $cloneUrl) {
    throw "The repository URL still contains the placeholder 'YOUR-USERNAME'. Replace it with your real GitHub username or pass -RepoUrl explicitly."
}

git config --global --add safe.directory $repoRoot 2>$null

cmd /c "git -C ""$repoRoot"" remote remove origin 1>nul 2>nul"
Assert-LastExitCode -Context "git remote remove origin"
git -C $repoRoot remote add origin $cloneUrl
Assert-LastExitCode -Context "git remote add origin"

$basic = [Convert]::ToBase64String([Text.Encoding]::ASCII.GetBytes("x-access-token:$($context.Token)"))
$pushArgs = @(
    "-C", $repoRoot,
    "-c", "http.https://github.com/.extraheader=AUTHORIZATION: basic $basic",
    "push"
)

if ($ForceRemoteOverwrite) {
    $pushArgs += "--force-with-lease"
}

$pushArgs += @("-u", "origin", "main")

$pushResult = Invoke-GitProcess -Arguments $pushArgs
$pushOutput = @($pushResult.StdOut + $pushResult.StdErr)
$pushExitCode = $pushResult.ExitCode
$pushOutput | ForEach-Object { Write-Host $_ }

if ($pushExitCode -ne 0) {
    $pushText = ($pushOutput | Out-String)
    if (-not $ForceRemoteOverwrite -and ($pushText -match "fetch first" -or $pushText -match "non-fast-forward" -or $pushText -match "remote contains work")) {
        Write-NonFastForwardGuidance -CloneUrl $cloneUrl
    }

    throw "git push failed with exit code $pushExitCode"
}

Write-Host "Pushed main to $cloneUrl"
