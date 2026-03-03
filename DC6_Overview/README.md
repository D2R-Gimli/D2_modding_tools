# DC6 Overview v1.0

**DC6 Overview v1.0**
(c) 2026 by Gimli

---

## 📌 Purpose

DC6 Overview is a small utility designed to create a **graphical overview of all frames inside a `.dc6` file**.

Instead of inspecting frames one by one, the program generates **one large image** that contains all frames arranged in a grid. This makes it much easier to review existing graphics at a glance.

The main purpose of this tool is to help with **AutoMap adjustments in Diablo II** when creating or modifying custom maps.

---

## 🎮 Background

In **Diablo II**, automap graphics are stored in `.dc6` files. These files contain multiple individual images (frames). When editing maps, it can be tedious to inspect every frame separately.

DC6 Overview solves this by:

* Extracting all frames
* Arranging them into a single large image
* Optionally drawing borders around frames
* Optionally adding frame numbers above each frame

This makes identifying specific frames significantly easier.

---

## 🎨 Color Palette

The tool uses the 256-color palette from:

* `act1.dat`

Details:

* 256-color indexed palette
* Color index 0 (black) is used as background
* Background becomes transparent later

---

## ⚙️ Usage

Run:

```
DC6_Overview_EXTRACT.bat
```

The batch file contains adjustable options.

### Available Options

| Option          | Description                                                     |
| --------------- | --------------------------------------------------------------- |
| `cols=<number>` | Number of columns in the output image (how many frames per row) |
| `-no_numbers`   | Do not draw frame numbers above each frame                      |
| `-every10`      | Draw only every 10th frame number                               |
| `-no_borders`   | Do not draw borders around frames                               |

You can edit these settings directly inside the `.bat` file before running it.

---

## 📂 Required Files

At least one `.dc6` file is required, for example:

* `MaxiMap.dc6`
* `MaxiMapS.dc6`
* Other `.dc6` files may also work

The palette file:

* `act1.dat`

Make sure the required files are located in the correct directory as expected by the batch file.

---

## 📤 Output

The program generates:

* A single large `.pcx` file
  → This image contains all frames arranged in a grid layout.

---

## 🛠 Typical Workflow

1. Place your `.dc6` file in the working directory.
2. Adjust options in `DC6_Overview_EXTRACT.bat` if needed.
3. Run the batch file.
4. Open the generated `.pcx` file.
5. Use the overview to identify frames for AutoMap corrections.

---

## 💡 Example Use Case

When creating new maps for **Diablo II**, you often need to:

* Identify which automap tile corresponds to which frame
* Verify alignment
* Adjust layout

Instead of manually checking each frame inside the `.dc6`, DC6 Overview provides a complete visual reference in one image.

---

## 📜 License

© 2026 Gimli
All rights reserved.

---

## 🔧 Version

**v1.0** – Initial release

---