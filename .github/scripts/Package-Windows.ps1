[CmdletBinding()]
param(
    [ValidateSet('x64')]
    [string] $Target = 'x64',
    [ValidateSet('Debug', 'RelWithDebInfo', 'Release', 'MinSizeRel')]
    [string] $Configuration = 'RelWithDebInfo'
)

$ErrorActionPreference = 'Stop'

if ( $DebugPreference -eq 'Continue' ) {
    $VerbosePreference = 'Continue'
    $InformationPreference = 'Continue'
}

if ( $env:CI -eq $null ) {
    throw "Package-Windows.ps1 requires CI environment"
}

if ( ! ( [System.Environment]::Is64BitOperatingSystem ) ) {
    throw "Packaging script requires a 64-bit system to build and run."
}

if ( $PSVersionTable.PSVersion -lt '7.2.0' ) {
    Write-Warning 'The packaging script requires PowerShell Core 7. Install or upgrade your PowerShell version: https://aka.ms/pscore6'
    exit 2
}

function Package {
    trap {
        Write-Error $_
        exit 2
    }

    $ScriptHome = $PSScriptRoot
    $ProjectRoot = Resolve-Path -Path "$PSScriptRoot/../.."
    $BuildSpecFile = "${ProjectRoot}/buildspec.json"

    $UtilityFunctions = Get-ChildItem -Path $PSScriptRoot/utils.pwsh/*.ps1 -Recurse

    foreach( $Utility in $UtilityFunctions ) {
        Write-Debug "Loading $($Utility.FullName)"
        . $Utility.FullName
    }

    $BuildSpec = Get-Content -Path ${BuildSpecFile} -Raw | ConvertFrom-Json
    $ProductName = $BuildSpec.name
    $ProductVersion = $BuildSpec.version

    $OutputName = "${ProductName}-${ProductVersion}-windows-${Target}"

    $RemoveArgs = @{
        ErrorAction = 'SilentlyContinue'
        Path = @(
            "${ProjectRoot}/release/${ProductName}-*-windows-*.zip"
        )
    }

    Remove-Item @RemoveArgs

    Log-Group "Archiving ${ProductName}..."
    $CompressArgs = @{
        Path = (Get-ChildItem -Path "${ProjectRoot}/release/${Configuration}" -Exclude "${OutputName}*.*")
        CompressionLevel = 'Optimal'
        DestinationPath = "${ProjectRoot}/release/${OutputName}.zip"
        Verbose = ($Env:CI -ne $null)
    }
    Compress-Archive -Force @CompressArgs
    Log-Group

    # This comes straight from the obs-plugintemplate docs

    # Declare the location of the InnoSetup setup file
    $IsccFile = "${ProjectRoot}/build_${Target}/installer-Windows.generated.iss"

    # Throw an error if the provided path is invalid
    if ( ! ( Test-Path -Path $IsccFile ) ) {
        throw 'InnoSetup install script not found. Run the build script or the CMake build and install procedures first.'
    }

    Log-Group 'Creating InnoSetup installer...'

    # Push the current location on the "BuildTemp" directory stack for easier return later
    Push-Location -Stack BuildTemp

    # Change to "release" sub-directory of the project root directory
    Ensure-Location -Path "${ProjectRoot}/release"

    # Copy the directory for the specified configuration (e.g. "Release") to a new directory named "Package"
    Copy-Item -Path ${Configuration} -Destination Package -Recurse

    # Invoke the InnoSetup iscc compiler with the specified setup file and the sub-directory "release" in 
    # the project root directory as the output directory
    Invoke-External iscc /O"${ProjectRoot}/release" /F"${OutputName}-Installer" ${IsccFile}

    # Remove the copied "Package" directory and its contents
    Remove-Item -Path Package -Recurse

    # Pop the location stored in the "BuildTemp" directory stack earlier
    Pop-Location -Stack BuildTemp

    Log-Group
}

Package
