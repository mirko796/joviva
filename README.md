<p align="center">
  <img src="./rsrc/app-icon.png" alt="Joviva icon" width="200" height="200">
</p>

# JovIva
A simple and lightweight image manipulation tool created with modest intention to simplify arranging and printing coloring pages so even a child can use it. \
This is a low-effort project that I created for my kids to have fun with. It does not strive to be a professional tool, but rather a simple and usable one. \
The app is written C++ with Qt6 framework which guarantees very good multiplatform support, yet I was really suprised how easy it was to make WASM version with only some minimal tweaks on the browser-side JavaScript code.
I haven't spent too much time on making the code super clean and well-structured, but it should be easy to understand and extend. Same goes for testing, there are some basic tests but don't expect any decent code coverage :) 

## Try it online
You can try it online [here](https://mrcode.dev/joviva/)

## Download
Head to the [releases](https://github.com/mirko796/joviva/releases) and pick binary for your system

## Goals and non-goals
Main goals from user perspective were:
- stable and multiplatform application (Windows, Linux, MacOS, WASM) \
My main motivation was frustration and innability to find a simple and stable app that would serve this purpose and work on all platforms (Linux is especially problematic in this regard).
- simple UI that even a child can use \
The most rewarding part is when you see a child using it and having fun with it without needing your help
- ability to add/resize/move images
- ability to add text and to some basic transformations on it

Non-goals:
- advanced image manipulation (adjusting levels, cropping images, drawing etc.)



## Build instructions
### Prerequisites
- C++11 compiler
- Qt6 (with minimal tweaks it can be compiled with Qt5 as well) - https://www.qt.io/download
- emscripten if you want to build WASM version 
- InnoSetup if you want to build Windows installer

### Building
1. Clone the repository
2. Open the `projects/JovIva/JovIva.pro` in Qt Creator
3. Build the project
4. Tadaa... enjoy your freshly baked JovIva

### Building Installers

#### Mac
Head to the `installers/mac` folder, create `config.sh` from `config.sh.template` and adjust it to your needs. \
Using terminal:
```bash
cd installers/mac
./make_installer.sh
```
The installer will be created in the `installers/mac/output` folder


#### Windows
Head to the `installers/windows` folder, create `config.bat` from `config.bat.template` and adjust it to your needs. \
Using git bash console (command prompt is also fine, just adjust the commands accordingly):
```bash
cd installers/windows
cmd //C ./make_installer.bat
```
The installer will be created in the `installers/windows/output` folder


#### Linux
Head to the `installers/linux` folder, create `config.sh` from `config.sh.template` and adjust it to your needs. 
```bash
cd installers/linux
./make_installer.sh
```
The installer will be created in the `installers/linux/output` folder

#### Wasm
Head to the `installers/wasm` folder, create `config.sh` from `config.sh.template` and adjust it to your needs. 
```bash
cd installers/wasm
./make_installer.sh
```
The archive with all files needed for hosting online version will be created in the `installers/wasm/output` folder. Just put these files on any static web server and you are good to go.