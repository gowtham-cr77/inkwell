# Inkwell — Linux Desktop Engineering & Development Guide

> A high-performance, distraction-free handwritten note-taking and PDF annotation application engineered for Fedora Linux (KDE Plasma), drawing tablets (Huion, Wacom), and OpenTabletDriver. Inspired by Noteastic for Windows.

---

## 1. System Overview & Technology Stack

| Layer | Component | Specification |
|---|---|---|
| **Language** | C++17 | Performance, low-latency inking, strong type safety |
| **GUI Framework** | Qt 6.4+ / QML | Hardware-accelerated Qt Quick with modern Controls |
| **Tablet Ingestion** | Native Qt 6 Tablet Event API | `QTabletEvent` reading pressure, position, subpixel coordinates |
| **Inking Smoothing** | 1€ Jitter Filter + Centripetal Catmull-Rom | Eliminates jitter without lag; variable ribbon width via cubic Bézier interpolation |
| **PDF Subsystem** | QPdfWriter + Poppler utilities (`pdftoppm`) | Lossless background rendering and high-resolution vector PDF export |
| **Local Storage** | SQLite 3 via Qt SQL | Offline-first, WAL journal mode, atomic transactions, no account required |
| **OCR Recognition** | Tesseract OCR CLI | Offline handwriting / text extraction |
| **Build & Packaging** | CMake 3.20+ & RPM Spec | Standard Fedora packaging (`.desktop`, scalable SVG icon, RPM spec) |

---

## 2. Milestone Implementation Status

### Milestone 1: Core Engine, Inking & Tablet Input (COMPLETED ✅)
- [x] Vector stroke representation (`Stroke.h`, `Stroke.cpp`)
- [x] InputPoint tracking: subpixel `(x, y)`, pressure `p ∈ [0.0, 1.0]`, hardware timestamp `t`
- [x] **1€ Filter** (`InputHandler.h`, `InputHandler.cpp`) for adaptive jitter reduction
- [x] **Centripetal Catmull-Rom spline** (`alpha = 0.5`) to eliminate self-intersections
- [x] Variable stroke width tessellation using normal offsets (`variableWidthPath()`)
- [x] Page data model with custom paper dimensions (`Page.h`, `Page.cpp`)
- [x] Notebook data model with multiple pages (`Notebook.h`, `Notebook.cpp`)
- [x] Full JSON serialization and deserialization for strokes, pages, and notebooks
- [x] Undo/Redo commands (`AddStrokeCommand`, `RemoveStrokeCommand`, `MoveStrokesCommand`)
- [x] Hardware tablet handling: `tabletEvent` intercept with `event->accept()` to prevent mouse emulation lag

### Milestone 2: Drawing Tools & Color Management (COMPLETED ✅)
- [x] **Pen Tool**: Pressure-sensitive variable width, custom colors, opacity
- [x] **Highlighter Tool**: Semi-transparent, layered behind text, smooth blending
- [x] **Eraser Tool**: Stroke-level and distance-tolerant point hit testing
- [x] **Presentation Laser Pointer**:
  - Live glowing trail and tip dot
  - Auto-fading timer loop (decaying alpha)
  - Zero permanent changes to notebook data
  - Presentation mode support
- [x] **Color Palette**:
  - 20-color curated palette (`ColorPicker.qml`)
  - Hex code input (`#RRGGBB`)
  - Noteastic-style flyout: tap-once to select, tap-again for settings

### Milestone 3: Geometry, Paper Templates & Page Management (COMPLETED ✅)
- [x] **Shapes Tool**:
  - Straight line, Arrow, Rectangle, Circle, Triangle, Ellipse
  - Angle snapping to 45° and 90° intervals
  - Shape fill options
- [x] **Precision Tools**:
  - Interactive on-screen Ruler with millimeter/centimeter measurements
  - Interactive Protractor with angular graduations (0° to 180°)
- [x] **Paper Templates**:
  - Blank paper
  - Ruled / Lined paper with configurable line spacing and pink margin rule
  - Grid paper with configurable grid sizes
  - Dotted paper (dot matrix)
  - Cornell Notes layout (Cue column, Notes column, Summary block)
  - Paper tint options (Pure White, Cream, Ivory, Sepia, Dark mode)
- [x] **Page Management**:
  - Add page, Duplicate page, Delete page, Next/Previous page controls
  - Real-time page counter

### Milestone 4: PDF Support (COMPLETED ✅)
- [x] **PDF Import**: Each PDF page is converted into a high-fidelity editable notebook page (`importPdf()`)
- [x] **PDF Background Rendering**: Rendered without lag or page shifting under handwriting
- [x] **PDF Export**: Full notebook vector PDF export via `QPdfWriter` with paper toggle (`exportPdf()`)
- [x] **High-Res Page Export**: PNG export for individual pages (`exportPageImage()`)

### Milestone 5: Library, Lasso, OCR & SQLite Storage (COMPLETED ✅)
- [x] **Lasso Tool**:
  - Freeform lasso path selection
  - Bounding box rendering with corner handles
  - Granular element filtering (toggle Pen strokes, Highlighter strokes, Shapes)
  - Move selection, Delete selection, Recolor selection, Copy and Paste
- [x] **Local OCR Recognition**:
  - Tesseract OCR integration for handwriting-to-text conversion (`performOcrOnSelection()`)
- [x] **Library & Organization**:
  - Folder and notebook hierarchy
  - SQLite database (`inkwell.db`) with WAL mode
  - Auto-save timer to prevent data loss on unexpected closure
  - Search and filter by folder

### Milestone 6: UI/UX, Themes, Shortcuts & Fedora Packaging (COMPLETED ✅)
- [x] Noteastic-inspired clean, distraction-free interface
- [x] Dynamic Dark & Light theme switching (`ThemeManager`)
- [x] Complete keyboard shortcuts (`Ctrl+Z`, `Ctrl+Y`, `Ctrl+S`, `Ctrl+N`, `Ctrl+=`, `Ctrl+-`, `F11`)
- [x] Fedora `.desktop` file with SVG application icon
- [x] Fedora RPM packaging specification (`inkwell.spec`)
- [x] Unit test suite (`tests/test_core.cpp`)

---

## 3. Fedora Linux Build Instructions

### Prerequisites & Dependencies
Run the following single command to install all necessary development packages on Fedora:

```bash
sudo dnf install -y \
    cmake gcc-c++ ninja-build \
    qt6-qtbase-devel qt6-qtdeclarative-devel \
    qt6-qtsvg-devel qt6-qtshadertools-devel \
    qt6-qtwayland-devel \
    poppler-qt6-devel poppler-utils \
    sqlite-devel \
    tesseract tesseract-devel leptonica-devel \
    desktop-file-utils
```

### Building with CMake
```bash
cd inkwell
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja
```

### Running the Application
```bash
./inkwell
```

---

## 4. Huion Tablet & OpenTabletDriver Setup

To ensure pressure sensitivity and low latency on Fedora with the Huion H640P:

1. **Verify OpenTabletDriver Daemon**:
   ```bash
   systemctl --user status opentabletdriver.service
   ```
2. **OpenTabletDriver Configuration**:
   - In OTD GUI, set Output Mode to **Artist Mode** (Absolute mode).
   - Verify in Pen Settings that the tip is set to **Tip** (Pressure), not mapped to a simple mouse click.
   - Run OTD's Tablet Debugger to ensure raw pressure values (0 to 8192) are streaming.
3. **Wayland & KDE Plasma Settings**:
   - On KDE Plasma Wayland, Inkwell communicates natively via `zwp_tablet_v2`.
   - If running under X11, verify stylus recognition using:
     ```bash
     xinput list
     ```
     Ensure `OpenTabletDriver Virtual Artist Tablet Pen` appears as a pointer device.

---

## 5. File Structure Reference

```
inkwell/
├── CMakeLists.txt                 # Project build script
├── resources.qrc                  # Root Qt resource manifest
├── DEVELOPMENT.md                 # Technical & development documentation
├── packaging/
│   ├── inkwell.desktop            # FreeDesktop desktop launcher
│   └── inkwell.spec               # Fedora RPM packaging spec
├── resources/
│   └── icons/
│       └── inkwell.svg            # Scalable application icon
├── src/
│   ├── main.cpp                   # App entry point & QML registration
│   ├── core/
│   │   ├── Types.h                # InputPoint, PageDimensions, PaperConfig
│   │   ├── Stroke.h / .cpp        # Vector stroke & Catmull-Rom ribbon
│   │   ├── Page.h / .cpp          # Page model with paper & PDF background
│   │   ├── Notebook.h / .cpp      # Notebook document structure
│   │   └── UndoCommands.h / .cpp  # QUndoCommand subclasses
│   ├── canvas/
│   │   ├── InputHandler.h / .cpp  # 1€ filter & tablet event ingestion
│   │   ├── StrokeRenderer.h / .cpp# Background, stroke & laser rendering
│   │   └── InkCanvas.h / .cpp     # QQuickPaintedItem main canvas
│   ├── storage/
│   │   └── Database.h / .cpp      # SQLite database wrapper
│   ├── library/
│   │   ├── NotebookManager.h/.cpp # Document persistence manager
│   │   └── LibraryModel.h / .cpp  # QAbstractListModel for QML GridView
│   └── ui/
│       └── ThemeManager.h / .cpp  # Dark & Light theme provider
├── qml/
│   ├── main.qml                   # Main application window & shortcuts
│   ├── pages/
│   │   ├── LibraryPage.qml        # Home screen & notebook browser
│   │   └── EditorPage.qml         # Full notebook canvas workspace
│   ├── components/
│   │   ├── Toolbar.qml            # Inking toolbar with tool flyouts
│   │   ├── PageNavigator.qml      # Bottom page indicator & controls
│   │   ├── ZoomControls.qml       # Zoom in/out/fit widget
│   │   ├── ColorPicker.qml        # 20-color palette + hex selector
│   │   ├── PenSettingsPopup.qml   # Thickness slider & nib preview
│   │   ├── ShapePickerPopup.qml   # Geometry picker & angle snapping
│   │   ├── LaserSettingsPopup.qml # Presentation laser mode & colors
│   │   ├── PaperSettingsPopup.qml # Ruled, grid, dotted, Cornell selector
│   │   └── LassoSettingsPopup.qml # Lasso element filtering & actions
│   └── dialogs/
│       ├── SettingsDialog.qml     # App preferences dialog
│       └── PdfExportDialog.qml    # PDF export options
└── tests/
    └── test_core.cpp              # Unit tests for core engine
```
