#!/bin/bash
################################################################################
### Packages all necessary files for a release.                              ###
################################################################################

packageName="ExamSubmission"
files=(
  # General Files
  # =============
  "README.md"

  # Data
  # ====
  "Data/Fonts/arial.ttf"
  "Data/fern.png"
  "Data/Readme.txt"

  # Kernels
  # =======
  "Kernels/Blend.cl"
  "Kernels/PrefixSum.cl"
  "Kernels/PrefixSum256.cl"
  "Kernels/RadixSort.cl"

  # Built Binaries
  # ==============
  "Output/Bin/x64Release/mpWrapper.dll"
  "Output/Bin/x64Release/Texture.exe"
  "Output/Bin/x64Release/PrefixSum.exe"
  "Output/Bin/x64Release/RadixSort.exe"
  "Output/Bin/x64Release/sfml-graphics-2.dll"
  "Output/Bin/x64Release/sfml-system-2.dll"
  "Output/Bin/x64Release/sfml-window-2.dll"

  # Generated Files For a Release
  # =============================
  "RunExamTask1.bat"
  "RunExamTask2.bat"
  "RunExamTask3.bat"
)

# Generate the helper scripts.
echo "start Output/Bin/x64Release/Texture.exe"   > RunExamTask1.bat
echo "start Output/Bin/x64Release/PrefixSum.exe" > RunExamTask2.bat
echo "start Output/Bin/x64Release/RadixSort.exe" > RunExamTask3.bat

# Build the archive.
7z a -mx9 -t7z "$packageName" ${files[@]}

# Clean up generated files.
rm RunExamTask1.bat RunExamTask2.bat RunExamTask3.bat
