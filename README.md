# Polygon Editor

A Qt Widgets application that displays a five-sided polygon with labeled points
in a graphics view. The properties panel stays synchronized with the canvas.

## Requirements

- Qt 6 with the Widgets module and CMake package
- CMake 3.27 or newer
- A C++17 compiler
- Ninja when using the command-line preset

## Build with Qt Creator

1. Install Qt with the **Qt Widgets** component and a compatible desktop kit.
2. Start Qt Creator and select **File > Open File or Project**.
3. Select the project's `CMakeLists.txt` file.
4. In the **Configure Project** dialog, select the desktop kit that matches your
	Qt installation and compiler, then select **Configure Project**.
5. Select the `QtProject` target in the project pane.
6. Select the **Build** button, or choose **Build > Build Project
	"QtProject"**.
7. Select the green **Run** button to launch the application.

Qt Creator configures the CMake build directory and supplies the Qt package,
compiler, and generator through the selected kit.

## Build from the command line

1. Set the variables to the Qt locations on your computer if Qt is not installed in the default directory. 
2. Use these commands to a terminal, change the examples below according to your directory:

```
$env:QT_ROOT = "C:/Qt/6.*.*/mingw_64" 
$env:MINGW_ROOT = "C:/Qt/Tools/mingw1310_64"
$env:NINJA_ROOT = "C:/Qt/Tools/Ninja"
$env:PATH = "$env:QT_ROOT\bin;$env:PATH"
```

2. From the repository directory, run:

```
cmake --preset default
cmake --build out/build/default
.\out\build\default\QtProject.exe
```