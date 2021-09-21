project "lua54"
	kind "StaticLib"
	language "C"
	cdialect "C99"

	architecture "x64"

	targetdir "../../bin"
	objdir "obj"

	staticruntime "on"

	includedirs {
		"include/lua"
	}

	files {
		"include/**.h",
		"src/**.c"
	}

	filter "configurations:debug"
		runtime "debug"
		symbols "on"

	filter "configurations:release"
		runtime "release"
		optimize "on"
