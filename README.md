# Inkwell

Inkwell is a high-performance, distraction-free handwritten note-taking and PDF annotation application built with C++17 and Qt 6 / QML. It is designed for Linux (Fedora, KDE Plasma / GNOME) and provides smooth, low-latency inking with full pressure sensitivity for drawing tablets (such as Huion and Wacom) via OpenTabletDriver.

---

## Features

- **Low-Latency Inking Engine**: 1€ adaptive jitter filter and Centripetal Catmull-Rom splines for smooth vector strokes.
- **Drawing Tools**: Pressure-sensitive fountain pen, highlighter with background blending, eraser, and presentation laser pointer.
- **Geometry & Precision**: Straight lines, rectangles, ellipses, triangles, snap-to-angle modes, plus interactive on-screen ruler and protractor.
- **Paper Templates**: Blank, ruled, grid, dotted, and Cornell notes with custom tints (cream, sepia, dark mode).
- **PDF Annotation**: High-fidelity PDF import, multi-page vector PDF export, and PNG export.
- **Lasso & Selection**: Freeform lasso to move, delete, copy, recolor, or extract text via offline Tesseract OCR.
- **Local Storage**: Offline-first storage powered by SQLite with WAL mode.

---

## Fedora Linux Installation Guide

Follow these steps on your Fedora machine to build and run Inkwell.

### Step 1: Install Required Dependencies

Open a terminal and run:

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

---

### Step 2: Clone the Repository

```bash
git clone https://github.com/gowtham-cr77/inkwell.git
cd inkwell
```

---

### Step 3: Build the Application

Use CMake and Ninja to compile the project:

```bash
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja
```

---

### Step 4: Run or Install

#### Run Directly:
```bash
./inkwell
```

#### Install System-Wide (Optional):
To add Inkwell to your application launcher menu and system paths:

```bash
sudo ninja install
```

After installation, you can launch Inkwell from your application launcher or by running `inkwell` in any terminal.

---

### Alternative: Build and Install via RPM

If you prefer installing via Fedora's package manager:

```bash
sudo dnf install -y rpmdevtools rpmlint
rpmdev-setuptree
sudo dnf builddep packaging/inkwell.spec

git archive --format=tar.gz --prefix=inkwell-0.1.0/ -o ~/rpmbuild/SOURCES/inkwell-0.1.0.tar.gz HEAD
cp packaging/inkwell.spec ~/rpmbuild/SPECS/
rpmbuild -ba ~/rpmbuild/SPECS/inkwell.spec

sudo dnf install ~/rpmbuild/RPMS/x86_64/inkwell-0.1.0-1.fc*.x86_64.rpm
```

---

## Drawing Tablet Setup (OpenTabletDriver / Huion / Wacom)

To ensure low latency and pressure sensitivity:

1. **Verify OpenTabletDriver Service**:
   ```bash
   systemctl --user status opentabletdriver.service
   ```

2. **OpenTabletDriver Settings**:
   - Open the OpenTabletDriver GUI.
   - Set Output Mode to **Artist Mode** (Absolute mode).
   - In Pen Settings, ensure the tip is assigned to **Tip (Pressure)**.

3. **Wayland Compatibility**:
   Inkwell communicates natively over Wayland through `qt6-qtwayland` (`zwp_tablet_v2`).

---

## License

This project is licensed under the MIT License.
