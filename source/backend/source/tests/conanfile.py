import os
from conan import ConanFile
from conan.tools.files import get, copy, download
from conan.errors import ConanInvalidConfiguration
from conan.tools.scm import Version
from conan.tools.cmake import CMake, CMakeDeps, cmake_layout

class GlobalTestProject(ConanFile):
    name = "BackendTestProject"
    version = "0.1.0"
    
    url = "url.com"
    description = ""
    settings = "os", "compiler", "build_type", "arch"
 
    requires = (
    "gtest/1.16.0"
    )

    generators = "CMakeToolchain"

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

    def layout(self):
       cmake_layout(self)
