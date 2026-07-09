from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMakeDeps

class FrontendTestProject(ConanFile):
    name = "FrontendTestProject"
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
