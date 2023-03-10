# UnrealOodleWrapper
Visual Studio project for linking Unreal Engine Oodle plugin with command tool

Release nor library are included since Oodle is a licensed property of Epic Games Inc. that cannot be shared publicly.

Download Unreal Engine 4.27+ from Epic Launcher (not tested with 5),
copy "include" and "lib" folders from Oodle plugin's sdk to root folder of project. 

Default Oodle sdk path: 
```
X:\Program Files\Epic Games\UE_4.27\Engine\Plugins\Compression\OodleData\Sdks\2.9.0
```
and compile in Visual Studio.

Used setup:
- Visual Studio 2022 (v143)
- Windows SDK 10.0.19041.0

For additional info while decompressing,
like what type of compression method was used, you must run tool with debugger.

# Help
```
.exe [option] file_path/stdin=%d output_file_path/stdout
Option:
-c %d %d:                       compress file
                                1st %d: compression level (from -4 to 9)
                                2nd %d: compression method (from -1 to 5)
-d %d:                          decompress file
                                %d: exact decompressed file size
                                (0 is not accepted, wrong value will return error)

Other:
file_path/stdin=%d:             you can either provide input file path,
                                or use stdin by writing "stdin=%d",
                                where %d is size of stdin binary data in bytes
output_file_path/stdout:        you can either provide output file path,
                                or use stdout by writing "stdout"

Compression Levels:
<0 = compression ratio < speed
=0 = No compression
>0 = compression ratio > speed

Compression Methods:
-1 = LZB16 (DEPRECATED but still supported)
0 = None (memcpy, pass through uncompressed bytes)
1 = Kraken (Fast decompression and high compression ratios)
2 = Leviathan (Higher compression than Kraken, slightly slower decompression.)
3 = Mermaid (between Kraken & Selkie - crazy fast, still decent compression.)
4 = Selkie (Selkie is a super-fast relative of Mermaid. For maximum decode speed.)
5 = Hydra
```

# Examples

- decompress file `test.oodle` that decompressed has size `5578` B and save it as `test.unc`
```
UnrealOodleWrapper -d 5578 test.oodle test.unc
```
- compress file `test.temp` using `Mermaid method` with `compression level 9` and save it as `test.oodle`
```
UnrealOodleWrapper -c 9 3 test.temp test.oodle
```
- With python 3 script compress file `test.temp` using `Mermaid method` at `compression level 9` through `stdin` and get result to `stdout`, then save it as `test.oodle`
```py
import subprocess

file = open("test.temp", "rb")
buffer = file.read()
file.close()
catch = subprocess.run(["UnrealOodleWrapper.exe", "-c", "9", "3", "stdin=%d" % len(buffer), "stdout"], input=buffer, capture_output=True, text=False)
if (catch.stderr != b""):
    print("Error while compressing file!")
    return 1
new_file = open("test.oodle", "wb")
new_file.write(catch.stdout)
new_file.close()
```
