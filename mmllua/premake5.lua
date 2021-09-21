include "lua54"

project "mmllua"
	kind "ConsoleApp"
	language "C"
	cdialect "C99"
	warnings "Extra"

	architecture "x64"

	staticruntime "on"

	targetdir "../bin"
	objdir "obj"

	debugdir "../"

	files {
		"src/**.h",
		"src/**.c"
	}

	includedirs {
		"../mml/include",
		"lua54/include"
	}

	links {
		"mml",
		"lua54"
	}

	filter "configurations:debug"
		runtime "debug"
		symbols "on"

	filter "configurations:release"
		runtime "release"
		optimize "on"

	filter "system:linux"
		links {
			"X11",
			"m",
			"GL",
			"pthread"
		}

	filter "system:windows"
		links {
			"opengl32",
			"gdi32",
			"user32",
			"kernel32",
			"winmm",
		}

		defines {
			"_CRT_SECURE_NO_WARNINGS"
		}

	filter { "system:linux", "configurations:release" }
		postbuildcommands { "upx ../bin/mmllua" }

	filter { "system:windows", "configurations:release" }
		postbuildcommands { "upx ..\\bin\\mmllua.exe" }
