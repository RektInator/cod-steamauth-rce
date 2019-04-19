dependencies = {
	basePath = "./deps"
}

function dependencies.load()
	dir = path.join(dependencies.basePath, "premake/*.lua")
	deps = os.matchfiles(dir)

	for i, dep in pairs(deps) do
		dep = dep:gsub(".lua", "")
		require(dep)
	end
end

function dependencies.imports()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.import()
		end
	end
end

function dependencies.projects()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.project()
		end
	end
end

newoption {
	trigger = "copy-to",
	description = "Optional, copy the EXE to a custom folder after build, define the path here if wanted.",
	value = "PATH"
}

dependencies.load()

workspace "cod-exploit"
	location "./build"
	objdir "%{wks.location}/obj"
	targetdir "%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}"

	configurations {
		"Debug",
		"Release",
	}

	platforms { "x86" }
	architecture "x86"

	systemversion "latest"
	symbols "On"
	staticruntime "On"
	editandcontinue "Off"
	warnings "Extra"
	characterset "ASCII"

	flags {
		"NoIncrementalLink",
		"NoMinimalRebuild",
		"MultiProcessorCompile",
		"No64BitChecks"
	}

	filter "action:vs*"
		buildoptions "/std:c++17"
		defines {
			"_WINDOWS",
			"WIN32",
		}

	filter "action:gmake*"
		cppdialect "C++17"
		buildoptions "-std=c++17"
		defines {
			"_LINUX",
		}

	filter "Release"
		optimize "Full"

		defines {
			"NDEBUG",
		}

		flags {
			"FatalCompileWarnings",
		}

	filter "Debug"
		optimize "Debug"

		defines {
			"DEBUG",
			"_DEBUG",
		}

	filter {}

	startproject "cod-exploit"
	project "cod-exploit"
		kind "ConsoleApp"
		language "C++"

		pchheader "stdinc.hpp"
		pchsource "src/stdinc.cpp"

		files {
			"./src/**.hpp",
			"./src/**.cpp",
		}

		includedirs {
			"./src",
			"%{prj.location}/src",
		}

		dependencies.imports()

	group "Dependencies"
		dependencies.projects()

