workspace "PShowdownWrapper"  
    configurations { "Debug", "Release" }

project "WrapperApp"  
    kind "ConsoleApp"   
    language "C++"   

targetdir "bin/%{cfg.buildcfg}" 
files { "include/**.h", "include/**.hpp", "src/**.cpp" }
includedirs {"include"}

filter "system:Windows"
    defines {"ROC_WINDOWS"}

filter "system:Unix"
    defines {"ROC_NIX"}

filter "configurations:Debug"
    defines { "DEBUG" }  
    symbols "On" 

filter "configurations:Release"  
    defines { "NDEBUG" }    
    optimize "On" 