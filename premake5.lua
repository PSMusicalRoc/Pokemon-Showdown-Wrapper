workspace "PShowdownWrapper"  
    configurations { "Debug", "Release" } 

project "WrapperApp"  
    kind "ConsoleApp"   
    language "C++"   

targetdir "bin/%{cfg.buildcfg}" 
files { "include/**.h", "include/**.hpp", "src/**.cpp" }
includedirs {"include"}

filter "configurations:Debug"
    defines { "DEBUG" }  
    symbols "On" 

filter "configurations:Release"  
    defines { "NDEBUG" }    
    optimize "On" 