param(
    [string]$TokenFile = "C:\Users\gadin\Desktop\temporal\logosnodehostinger\Foryouenv.txt",
    [string]$RepoName = "logos-node-inspector",
    [string]$Description = "Read-only Basecamp module for inspecting a single Logos VPS.",
    [switch]$Private
)

$ErrorActionPreference = "Stop"

function Get-GitHubContext {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        throw "Token file not found: $Path"
    }

    $lines = Get-Content -Raw -LiteralPath $Path |
        Select-String -Pattern ".*" -AllMatches |
        ForEach-Object { $_.Matches.Value.Trim() } |
        Where-Object { $_ -ne "" }

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

    if ($tokenIndex + 1 -ge $lines.Count) {
        throw "GitHub username not found immediately after the token in $Path"
    }

    return @{
        Token = $lines[$tokenIndex]
        User = $lines[$tokenIndex + 1]
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

$context = Get-GitHubContext -Path $TokenFile
$headers = @{
    Authorization = "Bearer $($context.Token)"
    Accept = "application/vnd.github+json"
    "X-GitHub-Api-Version" = "2022-11-28"
    "User-Agent" = "logos-node-inspector-bootstrap"
}

$repoApiUrl = "https://api.github.com/repos/$($context.User)/$RepoName"
$cloneUrl = $null

try {
    $existing = Invoke-GitHub -Method Get -Uri $repoApiUrl -Headers $headers
    $cloneUrl = $existing.clone_url
    Write-Host "Repository already exists: $cloneUrl"
} catch {
    $status = $null
    if ($_.Exception.Response) {
        $status = [int]$_.Exception.Response.StatusCode
    }

    if ($status -ne 404) {
        throw
    }

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
}

git remote remove origin 2>$null
git remote add origin $cloneUrl

$basic = [Convert]::ToBase64String([Text.Encoding]::ASCII.GetBytes("x-access-token:$($context.Token)"))
git -c "http.https://github.com/.extraheader=AUTHORIZATION: basic $basic" push -u origin main

Write-Host "Pushed main to $cloneUrl"
