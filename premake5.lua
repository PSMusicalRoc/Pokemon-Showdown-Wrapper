workspace "PShowdownWrapper"  
    configurations { "Debug", "Release" }

project "WrapperApp"  
    kind "ConsoleApp"   
    language "C++"   

targetdir "bin/%{cfg.buildcfg}" 
files { "include/**.h", "include/**.hpp", "src/**.cpp" }
removefiles { "include/Windows/**.h", "include/UNIX/**.h",
              "src/Windows/**.cpp", "src/UNIX/**.cpp" }
includedirs {"include"}

filter "system:Windows"
    defines {"ROC_WINDOWS"}
    files {
        "src/Windows/WINDOWS_PID_HANDLER.cpp",
        "include/Windows/WINDOWS_PID_HANDLER.h"
    }

filter "system:Linux"
    defines {"ROC_NIX"}
    files {
        "src/UNIX/UNIX_PID_HANDLER.cpp",
        "include/UNIX/UNIX_PID_HANDLER.h"
    }

filter "configurations:Debug"
    defines { "ROC_DEBUG" }  
    symbols "On" 

filter "configurations:Release" 
    optimize "On" 