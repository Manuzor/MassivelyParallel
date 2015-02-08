#!/bin/bash
################################################################################
### Packages all necessary files for a release.                              ###
################################################################################

packageName="MassivelyParallel_Build"
files=(
  # General Files
  # =============
  "README.md"
  "MassivelyParallel.sln"
  "CopyDependencies.py"
  "Common.props"
  "CUDA_CL.props"
  "Wrapper.props"
  ".editorconfig"

  # Data
  # ====
  $(find Data -type f)

  # Kernels
  # =======
  $(find Kernels -type f)

  # Third Party dependencies
  # ========================
  $(find ThirdParty -type f -follow)

  # Solution Projects
  # =================
  $(find CL_Wrapper -type f)
  $(find Texture    -type f)
  $(find PrefixSum  -type f)
  $(find RadixSort  -type f)
)

# Build the archive.
#7z a -mx9 -t7z "$packageName" ${files[@]}
echo "PackageName: $packageName"
echo "Files: ${files[@]}"
