workspace "termfmt"
configurations { "Debug", "Release" }
cppdialect "C++20"

project "test"
kind "ConsoleApp"
externalincludedirs "include"
files { 
    "test/**.h",
    "test/**.cpp"
}
links "termfmt"

include "lib.lua"