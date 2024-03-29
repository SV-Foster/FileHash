# File Hashing and Verification Tool

FileHash is an innovative application designed to ensure the integrity and security of your valuable files. With its advanced file hashing capabilities and seamless verification process, FileHash offers unrivaled protection against unauthorized modifications.

Featuring a comprehensive array of hashing algorithms, FileHash enables you to hash files using industry standard algorithms such as MD5, SHA-1, SHA-256 and many more. Whether you need to hash a single file or an entire directory, FileHash allows you to safeguard your data with ease.

But that's not all. FileHash also empowers you to conveniently save the generated hashes into a file, providing a reliable reference point for future verification. This means you can effortlessly compare the original hashes with the current state of your files to determine if any unauthorized changes have occurred.

Thanks to FileHash's intuitive commands, users of all skill levels can effortlessly navigate and perform the desired actions. So whether you're a beginner seeking an extra layer of protection or a seasoned professional conducting rigorous security audits, FileHash is the ideal tool to meet your needs.

Don't compromise your data's integrity — trust FileHash to keep your files secure and unchanged. Download now and experience the peace of mind that comes with solid file integrity protection!


## Usage

FileHash is the command line tool. Open command prompt and start the FileHash executable with following parameters:

```filehash <command> /<switch 1> /<switch N> <checkfile> <path to folder> <path to file> <pa?h with?wild*cards>```

### Commands

```
c             Calculate hashes of files and folders provided and save them into a checkfile
v             Compare checkfile contents with a path provided
l             List checkfile contents to the console
```

### Switches

```
/Help          Display this help message, if no arguments provided, set by default
/NoLogo        Don't print copyright logo
/Recursive     Scan all subfolders in the provided folder
/Overwrite     Overwrites the checkfile, if already exists
/IncludeHidden Hash hidden files and scan hidden folders. By default files and folders with
               hidden attribute are ignored
/MD2           Hash files with MD2 algorithm
/MD4           Hash files with MD4 algorithm
/MD5           Hash files with MD5 algorithm
/SHA1          Hash files with SHA1 algorithm
/SHA256        Hash files with SHA256 algorithm. If no other algorithms provided, set by default
/SHA384        Hash files with SHA384 algorithm
/SHA512        Hash files with SHA512 algorithm
/AlgoALL       Hash files with ALL algorithms above
```

### Path limitations

With `c command`, maximum 1024 paths to files and/or folders can be provided. Wildcards are supported

With `v command`, only one folder can be provided

With `l command`, only one checkfile can be provided

You should use double quotation marks `"` if the path has spaces in it

### Exit codes

An exit code is a number that is returned by a program when it finishes running. This code is used to indicate the success or failure of the program's execution.

The exit code is often used by other programs or scripts to determine how to proceed after running a particular program. For example, a batch script may check the exit code of a program and take different actions based on the result.

```
0             Requested operation completed successfully
1             No files were found to hash, checkfile was not created
2             The user pressed CTRL+C or CTRL+BREAK to terminate the program
3             Check completed, but file(s) with wrong hash(es) found during the check
4             Initialization error occurred. There is not enough memory or disk space,
              an invalid path to a checkfile entered or invalid syntax of the command line arguments
5             Disruption or inability to read, write, transfer or analyze data from/to the disk
```

### Usage examples

hash all files and folders recursively inside the D:\backups\ folder with SHA1 and MD5 algorithms and save hashes into the backup.checkfile:

```
filehash c /Recursive /SHA1 /MD5 backup.checkfile D:\backups\
```

<br/>
hash all files inside the D:\My Projects\Slam\ folder with the default SHA256 algorithm and save them to the project-slam.checkfile. don't hash files in subfolders:

```
filehash c project-slam.checkfile "D:\My Projects\Slam\"
```

<br/>
hash files with .cab and .zip extensions inside the D:\reports\ folder with MD5 algorithm and save them to the reports-2024.checkfile. if the file exists, overwrite it:

```
filehash c /Overwrite /MD5 reports-2024.checkfile D:\reports\*.zip D:\reports\*.cab
```

<br/>
list contents of the "books about art.checkfile" from the C:\Users\Public\Documents\ folder to the console:

```
filehash l "C:\Users\Public\Documents\books about art.checkfile"
```
  
<br/>
compare contents of usb-flash-copy.checkfile with files on the E:\ drive:

```
filehash v E:\usb-flash-copy.checkfile E:\ 
```


## .checkfile structure

![Screenshot](Documents/screenshot%20001.PNG)

[Template for WinHEX](Documents/SV%20Foster's%20FileHash%20.checkfile.tpl) is available for download


## What's new

### Version 1.1

* Added /IncludeHidden option. Now files and folders with hidden attribute are ignored by default
* Read error checks improved
* Potential problems w/non-unicode file names and paths inside the .checkfile fixed
* Improved .checkfile managment code


## Building

FileHash uses the `Microsoft Visual Studio 2022` for its builds.

To build FileHash from source files with Microsoft Visual Studio, you can use either the graphical or the command-line mode. Here are the instructions for both methods:

### Graphical mode
1. Open Microsoft Visual Studio and select `Open a project or solution` from the start page or the `File` menu
2. Browse to the folder where the `filehash.sln` file is located and select it. This will load the project in Microsoft Visual Studio
3. Select the `configuration` and `platform` for the project by using the drop-down menus on the toolbar. For example, you can choose Debug or Release for the configuration, and x86 or x64 for the platform
4. Build the project by clicking on the `Build` menu and selecting `Build Solution`. You can also use the keyboard shortcut `Ctrl+Shift+B`
5. Run the project by clicking on the `Debug` menu and selecting `Start Debugging`. You can also use the keyboard shortcut `F5`

### Command-line mode
1. Open a `Developer Command Prompt` for Microsoft Visual Studio. You can find it in the Start menu under Microsoft Visual Studio Tools
2. Navigate to the folder where the `filehash.sln` file is located by using the `cd` command
3. Invoke the MSBuild tool to build the project. You can specify various options and flags for the tool. For example, the following command builds the project with the Release configuration and the x64 platform:
```
msbuild filehash.sln /p:Configuration=Release /p:Platform=x64
```
4. Run your executable by typing its name and path in the command prompt. For example:
```
filehash-exe-x86-64\filehash /?
```


## Testing

To test both 32 and 64 bit versions of the FileHash start testing script `FileHash-test-scripts\runall.ps1` and wait for it to finish.


## Authors

This program was written and is maintained by SV Foster.


## License

This program is available under EULA, see [EULA text file](EULA.txt) for the complete text of the license. This program is free for personal, educational and/or non-profit usage.
