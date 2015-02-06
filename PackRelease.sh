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

  # Built Binaries
  # ==============
  "Output/Bin/x64Release/mpWrapper.dll"
  "Output/Bin/x64Release/Texture.exe"
  "Output/Bin/x64Release/PrefixSum.exe"
  "Output/Bin/x64Release/RadixSort.exe"
  "Output/Bin/x64Release/sfml-graphics-2.dll"
  "Output/Bin/x64Release/sfml-system-2.dll"
  "Output/Bin/x64Release/sfml-window-2.dll"
)

7z a -mx9 -t7z "$packageName" ${files[@]}
