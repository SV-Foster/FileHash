$ErrorActionPreference = "Stop"

$StartFolder = ""
$TempFolder = ""
$DemosFolder = ""
$TempRNDS = ""
$TempRND = ""


function Get-ScriptDirectory {
    if ($psise) {
        Split-Path $psise.CurrentFile.FullPath
    }
    else {
        $PSScriptRoot
    }
}

function ProcessPathToken($p, $tokenList){
    foreach( $token in $tokenList.GetEnumerator() )
    {
        $pattern = '#{0}#' -f $token.key
        $p = $p -replace $pattern, $token.Value
    }       
    return $p
}

class TTestTask
{
    [ValidateNotNullOrEmpty()][int]   $ID
                              [string]$Description
                              [string]$Args
    [ValidateNotNullOrEmpty()][int]   $ExitCodeExpected
    [ValidateNotNullOrEmpty()][bool]  $BinaryComp
                              [string]$BinaryExpected
                              [string]$BinarySample
    [ValidateNotNullOrEmpty()][bool]  $WorkingDirectorySet
                              [string]$WorkingDirectory

    TTestTask($x){
        $this.ID                  =  $x.ID
        $this.Description         =  $x.Description
        $this.Args                =  $x.Args
        $this.ExitCodeExpected    =  $x.ExitCodeExpected
        $this.BinaryComp          = ($x.BinaryComp -eq $true)
        $this.BinaryExpected      =  $x.BinaryExpected
        $this.BinarySample        =  $x.BinarySample
        $this.WorkingDirectorySet = ($x.WorkingDirectorySet -eq $true)
        $this.WorkingDirectory    =  $x.WorkingDirectory
    }
}

class TTestTaskExpendedPath : TTestTask
{
    [string]$ArgsExp
    [string]$BinaryExpectedExp
    [string]$BinarySampleExp
    [string]$WorkingDirectoryExp

    TTestTaskExpendedPath($x, $tl) : base($x) {
        $this.ArgsExp             = ProcessPathToken $this.Args $tl
        $this.BinaryExpectedExp   = ProcessPathToken $this.BinaryExpected $tl
        $this.BinarySampleExp     = ProcessPathToken $this.BinarySample $tl
        $this.WorkingDirectoryExp = ProcessPathToken $this.WorkingDirectory $tl
    }
}

function TestTaskProcess{
    param(
        [Parameter(Mandatory=$true)]
        [int]$tnum,

        [Parameter(Mandatory=$true)]
        [string]$EXE,

        [Parameter(Mandatory=$true)]
        [string]$SubFolderPlatform,

        [Parameter(Mandatory=$true)]
        [TTestTask]$test
    )

    $CmdLog    = $TempFolder + "log\test" + $tnum + "-cmd.txt"
	$EXERunLog = $TempFolder + "log\test" + $tnum + "-out.txt"
    $TempRND   = [System.IO.Path]::GetRandomFileName()

    #ExecutionContext ExpandString
    $tokenList = @{
        StartFolder = $StartFolder
        TempFolder = $TempFolder
        DemosFolder = $DemosFolder
        TempRNDS = $TempRNDS
        TempRND = $TempRND
        SubFolderPlatform = ($SubFolderPlatform + "\")
    }
    $testexp = [TTestTaskExpendedPath]::new($test, $tokenList)

    Write-Host ""
    Write-Host "Test $tnum, Task $($testexp.ID), $SubFolderPlatform, $($testexp.Description)"

    # Save parameters to log
    "EXE : $EXE"                              | Out-File -FilePath $CmdLog
    "ARGS: $($testexp.ArgsExp)"               | Out-File -FilePath $CmdLog -Append
    "BE  : $($testexp.BinaryExpectedExp)"     | Out-File -FilePath $CmdLog -Append
    "BS  : $($testexp.BinarySampleExp)"       | Out-File -FilePath $CmdLog -Append
    "WD  : $($testexp.WorkingDirectoryExp)"   | Out-File -FilePath $CmdLog -Append
    
    # Run the executable
    $SPParams = "Start-Process -FilePath $EXE -RedirectStandardOutput $EXERunLog -PassThru -Wait"
    if (![string]::IsNullOrEmpty($testexp.Args)){
        $SPParams = $SPParams + " -ArgumentList `"$($testexp.ArgsExp)`""
    }
    if ($testexp.WorkingDirectorySet){
        $SPParams = $SPParams + " -WorkingDirectory `"$($testexp.WorkingDirectoryExp)`""
    }
    $process = Invoke-Expression -Command $SPParams

    # Get the return value (error level)
    Write-Host "Return Value: $($process.ExitCode)"
    "RTRN: $($process.ExitCode)" | Out-File -FilePath $CmdLog -Append

    # Check if the return value is correct
    if ($process.ExitCode -ne $testexp.ExitCodeExpected) {
        Write-Host "FAILED WRONG EXIT CODE"
        exit
    }
    
	# Check file created is correct
    if ($testexp.BinaryComp){
        $result = (Get-FileHash $testexp.BinaryExpectedExp).hash -ne (Get-FileHash $testexp.BinarySampleExp).hash
        if ($result) {
            Write-Host "FAILED TEST FILES ARE DIFFERENT"
            exit
        } else {
            Write-Host "Files matched"
        }
    }

    Write-Host "Task OK!"
}


Write-Host " "

$StartFolder = Get-ScriptDirectory
if (!$StartFolder.EndsWith("\\")) {$StartFolder += "\"}
Write-Host "Script started from the folder: $StartFolder"

$TempFolder = Split-Path $StartFolder
$TempFolder = $TempFolder + "\FileHash-test-temp\"
Write-Host "Temp folder is $TempFolder"

$DemosFolder = Split-Path $StartFolder
$DemosFolder = $DemosFolder + "\FileHash-test-structures\"
Write-Host "Demo files folder is $DemosFolder"

$EXE32 = Split-Path $StartFolder
$EXE64 = Split-Path $StartFolder
$EXE32 = $EXE32 + "\FileHash-exe-IA-32\filehash.exe"
$EXE64 = $EXE64 + "\FileHash-exe-x86-64\filehash.exe"
Write-Host "Executable IA-32 is $EXE32"
Write-Host "Executable x86-64 is $EXE64"

Set-Location -Path $StartFolder

Write-Host " "
Write-Host "Cleaning old mess..."
& "$StartFolder\clean.ps1"
New-Item -ItemType Directory -Path ($TempFolder + "log\")

Write-Host " "
Write-Host "Loading test tasks..."

[xml]$xml = Get-Content ($StartFolder + "TestTasksList.xml")
$TestTaskssArray = @()
foreach ($x in $xml.TestTasksList.Task) {
    $TestTaskssArray += [TTestTask]::new($x)
}

$TestTaskssArray | ForEach-Object {
    Write-host "ID $($_.ID) -- $($_.Description)"
}


Write-Host " "
Write-Host "Testing..."

$i = 1
$TempRNDS = [System.IO.Path]::GetRandomFileName()
foreach ($t in $TestTaskssArray)
{
    TestTaskProcess $i $EXE32 "IA-32" $t
    $i++
}
$TempRNDS = [System.IO.Path]::GetRandomFileName()
foreach ($t in $TestTaskssArray)
{
    TestTaskProcess $i $EXE64 "x86-64" $t
    $i++
}


Write-Host ""
Write-Host "Done!"
