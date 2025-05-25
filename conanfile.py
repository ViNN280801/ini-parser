from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.scm import Git

INI_PARSER_USER = "ViNN280801"
INI_PARSER_CHANNEL = "stable"
INI_PARSER_LIB_NAME = "iniparser"
INI_PARSER_VERSION = "1.0.0"
INI_PARSER_AUTHOR = "Vladislav Semykin <vladislav.semykin@gmail.com>"
INI_PARSER_GIT_REPO_URL = "https://github.com/ViNN280801/ini-parser.git"
INI_PARSER_DESCRIPTION = "Lightweight INI parser library for C/C++"


class IniparserConan(ConanFile):
    name = INI_PARSER_LIB_NAME
    version = INI_PARSER_VERSION
    author = INI_PARSER_AUTHOR
    url = INI_PARSER_GIT_REPO_URL
    description = INI_PARSER_DESCRIPTION
    topics = ("ini", "parser", "configuration")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "build_tests": [True, False],
        "build_examples": [True, False],
    }
    default_options = {
        "shared": True,
        "fPIC": True,
        "build_tests": False,
        "build_examples": False,
    }
    generators = "CMakeToolchain", "CMakeDeps"

    def config_options(self):
        if getattr(self.settings, "os", None) == "Windows":
            del self.options.fPIC  # type: ignore

    def layout(self):
        cmake_layout(self)

    def source(self):
        git = Git(self)
        git.clone(url=INI_PARSER_GIT_REPO_URL, target=".")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["iniparser", "iniparser_cpp"]
        self.cpp_info.includedirs = ["include"]
