project "sample"
	kind "ConsoleApp"
	language "C"
	cdialect "C89"
	warnings "Extra"

	architecture "x64"

	staticruntime "on"

	targetdir "../bin"
	objdir "obj"

	debugdir "../"

	files {
		"src/**.c"
	}

	includedirs {
		"../mml/include"
	}

	links {
		"mml"
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
		postbuildcommands { "upx ../bin/sample" }

	filter { "system:windows", "configurations:release" }
		postbuildcommands { "upx ..\\bin\\sample.exe" }
