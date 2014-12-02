import os

# Some info about the ezEngine
Sfml2Config = {
    "Root": os.getenv("SFML_INSTALL_DIR"),
    "SupportedArchitectures": [ "32", "64" ],
}

#####################################################################

# Generates ezEngine module dependency generator functions.
def Sfml2Module(Name):
    def GenerateFilePattern(FileList, Config, Arch):
        assert Arch   in Sfml2Config["SupportedArchitectures"], "Unsupported architecture '{0}'".format(Arch)
        FileName = os.path.join(Sfml2Config["Root"], "bin", Name + "-2.dll")
        FileList.append(FileName)
    return GenerateFilePattern

#####################################################################

Dependencies = [
    Sfml2Module("sfml-system"),
    Sfml2Module("sfml-window"),
    Sfml2Module("sfml-graphics"),
]

# Required by the API.
def GetDependencies(Configuration, PlatformArchitecture):
    FileList = []
    for fileNameGenerator in Dependencies:
        fileNameGenerator(FileList, Configuration, PlatformArchitecture)
    return FileList
