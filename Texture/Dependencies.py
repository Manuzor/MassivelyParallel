import os

# Some info about the ezEngine
Sfml2Config = {
    "Root": os.getenv("SFML_INSTALL_DIR"),
    "SupportedConfigurations": [ "Debug", "Release" ],
    "SupportedArchitectures": [ "32", "64" ],
    "ConfigDependendSuffix": {
        "Debug": "-d",
        "Release": "",
    },
}

#####################################################################

# Generates ezEngine module dependency generator functions.
def Sfml2Module(Name):
    def GenerateFilePattern(FileList, Config, Arch):
        assert Config in Sfml2Config["SupportedConfigurations"], "Unsupported configuration '{0}'".format(Config)
        assert Arch   in Sfml2Config["SupportedArchitectures"], "Unsupported architecture '{0}'".format(Arch)
        # The following produces something like "sfml-window-2-d.dll" or "sfml-graphics-2.dll"
        FileName = os.path.join(Sfml2Config["Root"], "bin", "{0}{1}-2.dll".format(Name, Sfml2Config["ConfigDependendSuffix"][Config]))
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
