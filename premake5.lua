project "termfmt"

kind "StaticLib"

files{ 
    "include/termfmt/**.h", 
    "src/**.cpp" 
}

includedirs "include"
