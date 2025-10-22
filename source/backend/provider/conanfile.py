import os
from conan import ConanFile
from conan.tools.files import get, copy, download
from conan.tools.cmake import cmake_layout, CMakeDeps, CMakeToolchain

class provider(ConanFile):
    name = "provider"
    version = "0.1.0"

    url = "url.com"
    description = ""
    settings = "os", "compiler", "build_type", "arch"
    #exports_sources = "CMakeLists.txt", "../include/CrashDump.h", "../source/CrashDump.cpp"
    #default_options = "toolchain:cmake=True"

    requires = "easyloggingpp/9.97.1", "gtest/1.16.0", "sentry-native/0.11.3"

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        deps = CMakeDeps(self)
        deps.generate()

