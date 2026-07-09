from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMakeDeps

class provider(ConanFile):
    name = "provider"
    version = "0.1.0"

    url = "url.com"
    description = ""
    settings = "os", "compiler", "build_type", "arch"

    requires = "easyloggingpp/9.97.1", "gtest/1.16.0", "sentry-native/0.11.3"
    generators = "CMakeToolchain"

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

