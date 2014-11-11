#!python3

import random
import sys

Height = int(sys.argv[1])
Width = int(sys.argv[2])
OutFileName = sys.argv[3]

Min = 0
Max = 100

rand = random.SystemRandom()

with open(OutFileName, "w") as OutFile:
  OutFile.write("{\n")
  for x in range(0, Height):
    OutFile.write("  {")
    for y in range(0, Width):
      OutFile.write(str(rand.uniform(Min, Max)))
      if y < Width - 1:
        OutFile.write(", ")
    OutFile.write("}")
    if x < Height - 1:
      OutFile.write(",")
    OutFile.write("\n")
  OutFile.write("}\n")
