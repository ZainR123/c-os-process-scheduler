# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\JetBrains\CLion 2020.3\bin\cmake\win\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\JetBrains\CLion 2020.3\bin\cmake\win\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\Administrator\Documents\Personal\Y2\OSC\psyzr2_coursework\Project

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\Administrator\Documents\Personal\Y2\OSC\psyzr2_coursework\Project\cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/Project.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/Project.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Project.dir/flags.make

CMakeFiles/Project.dir/src/processSimulator.obj: CMakeFiles/Project.dir/flags.make
CMakeFiles/Project.dir/src/processSimulator.obj: ../src/processSimulator.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Administrator\Documents\Personal\Y2\OSC\psyzr2_coursework\Project\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/Project.dir/src/processSimulator.obj"
	C:\TDM-GCC-64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles\Project.dir\src\processSimulator.obj   -c C:\Users\Administrator\Documents\Personal\Y2\OSC\psyzr2_coursework\Project\src\processSimulator.c

CMakeFiles/Project.dir/src/processSimulator.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/Project.dir/src/processSimulator.i"
	C:\TDM-GCC-64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E C:\Users\Administrator\Documents\Personal\Y2\OSC\psyzr2_coursework\Project\src\processSimulator.c > CMakeFiles\Project.dir\src\processSimulator.i

CMakeFiles/Project.dir/src/processSimulator.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/Project.dir/src/processSimulator.s"
	C:\TDM-GCC-64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S C:\Users\Administrator\Documents\Personal\Y2\OSC\psyzr2_coursework\Project\src\processSimulator.c -o CMakeFiles\Project.dir\src\processSimulator.s

# Object files for target Project
Project_OBJECTS = \
"CMakeFiles/Project.dir/src/processSimulator.obj"

# External object files for target Project
Project_EXTERNAL_OBJECTS =

Project.exe: CMakeFiles/Project.dir/src/processSimulator.obj
Project.exe: CMakeFiles/Project.dir/build.make
Project.exe: CMakeFiles/Project.dir/linklibs.rsp
Project.exe: CMakeFiles/Project.dir/objects1.rsp
Project.exe: CMakeFiles/Project.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\Users\Administrator\Documents\Personal\Y2\OSC\psyzr2_coursework\Project\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable Project.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\Project.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Project.dir/build: Project.exe

.PHONY : CMakeFiles/Project.dir/build

CMakeFiles/Project.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\Project.dir\cmake_clean.cmake
.PHONY : CMakeFiles/Project.dir/clean

CMakeFiles/Project.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\Administrator\Documents\Personal\Y2\OSC\psyzr2_coursework\Project C:\Users\Administrator\Documents\Personal\Y2\OSC\psyzr2_coursework\Project C:\Users\Administrator\Documents\Personal\Y2\OSC\psyzr2_coursework\Project\cmake-build-debug C:\Users\Administrator\Documents\Personal\Y2\OSC\psyzr2_coursework\Project\cmake-build-debug C:\Users\Administrator\Documents\Personal\Y2\OSC\psyzr2_coursework\Project\cmake-build-debug\CMakeFiles\Project.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Project.dir/depend

