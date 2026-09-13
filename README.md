# Polygon Canvas

A Qt Widgets application that displays a five-sided polygon with labeled points in a graphics view. The properties panel stays synchronized with the canvas.

## Requirements

- Qt 6 (Widgets module)
- CMake 3.16 or newer
- A C++17 compiler

## Build and run

From the repository directory:

```sh
cmake -S . -B build
cmake --build build
```

Run the resulting executable from `build/` (on Windows it is normally `build/QtProject.exe`, depending on the selected generator).

## Using the application

- Drag inside the polygon to move it.
- Drag any circular point handle to adjust that vertex.
- Edit any point label on the canvas or in its properties field.
- Use the properties panel to edit the fill color or any point coordinate. The area is calculated automatically.