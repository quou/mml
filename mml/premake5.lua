project "mml"
	kind "StaticLib"
	language "C"
	cdialect "gnu89"

	architecture "x64"

	targetdir "../bin"
	objdir "obj"
	
	staticruntime "on"

	includedirs {
		"include"
	}

	files {
		"include/mml.h",
		"src/mml.c",
		"src/video.c",
		"src/keytable.c",
		"src/stb_image.h",
		"src/stb_rect_pack.h",
		"src/stb_truetype.h",
		"src/keytable.h",
	}

	filter "system:linux"
		files {
			"src/platform_x11.c",
			"src/platform_linux.c"
		}

	filter "system:windows"
		files {
			"src/platform_windows.c"
		}

		defines {
			"_CRT_SECURE_NO_WARNINGS"
		}

	filter "configurations:debug"
		runtime "debug"
		symbols "on"

	filter "configurations:release"
		runtime "release"
		optimize "on"
