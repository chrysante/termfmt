project "termfmt"

kind "StaticLib"
cppdialect "C++20"

files{ 
    "include/termfmt/**.h", 
    "src/**.cpp" 
}

includedirs "include"
