$ErrorActionPreference = "Stop"


function Get-ScriptDirectory {
    if ($psise) {
        Split-Path $psise.CurrentFile.FullPath
    }
    else {
        $PSScriptRoot
    }
}

# recursively delete files and folders in a folder
function Delete-FilesRecursively {
    param(
        [Parameter(Mandatory=$true)]
        [string]$Path
    )

    # Delete files in the current directory
    Get-ChildItem -Path $Path -Force | Where-Object { ! $_.PSIsContainer } | ForEach-Object {
        $file = $_
        try {
            Write-Host "Deleting file $($file.FullName)"            
            $file.Delete()
            
        }
        catch {
            Write-Host "Failed to delete file: $($file.FullName)"
        }
    }
    
    
    Get-ChildItem -Path $Path -Force | Where-Object { $_.PSIsContainer } | ForEach-Object {
        $folder = $_
        try {
            Delete-FilesRecursively -Path $folder.FullName
            Write-Host "Deleting folder $($folder.FullName)"
            $folder.Delete()
            
        }
        catch {
            Write-Host "Failed to delete folder: $($folder.FullName)"
        }
    }
}


$StartFolder = Get-ScriptDirectory
if (!$StartFolder.EndsWith("\\")) {$StartFolder += "\"}
Write-Host "Script started from folder: $StartFolder"

$location = Split-Path $StartFolder
$location = $location + "\FileHash-test-temp\"
Write-Host "Working directory is $location"

Set-Location -Path $StartFolder

Delete-FilesRecursively -Path $location

Write-Host "Done!"
