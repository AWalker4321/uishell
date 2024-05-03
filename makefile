#############################################################################################
# file: makefile
# author: Austin Walker
# last updated: 2/1/2024
# 
# description: Make file used to build and run assignment 4 for CS240 
# usage:
# 	make                   - builds and runs
# 	make buid              - builds the target
# 	make run               - runs target
# 	make clear_swp         - removes swap files in current directory (got tired of removing swps due to a bad internet conection)
# 	make edit              - opens all editable source files as tabs in vim
#############################################################################################

#---Variables---
cxx       =g++#C++ compiler to use
target    =project4#The target of this makefile
editables =$(target).cpp dispatcher.cpp dispatcher.hpp#eitable files
#dirs
binDir  =bin#Binaries directory
#Arugments
exeArgs =#Arugments used when compiling executable
runArgs =#Default args used when running the executable

#---Phonies---
.PHONY: default edit build run clean

#default: compiles, cleans, then runs with the default arguments
default:
	@make build --no-print-directory
	@make run --no-print-directory
	@echo "Hello World"

#opens all editable files in vim with tabs
edit:
	@vim -p $(editables)

#compiles the project
build:
	@echo [MAKE] Building...
	@make $(binDir)/$(target).out --no-print-directory
	@echo [MAKE] Done building.

#runs the executable
run:
	@echo [MAKE] running with args: $(runArgs)
	@echo -------------------------
	@./$(binDir)/$(target).out $(runArgs)
	@echo -------------------------
	@echo [MAKE] done.

#cleans out object files
clear_swp:
	@echo [MAKE] Clearing swaps...
	@rm .*.*.swp
	@echo [MAKE] Done clearing swaps

#---Targets---

#compiles the project
$(binDir)/$(target).out: $(target).cpp dispatcher.cpp
	@echo [MAKE] compiling $<...
	@mkdir -p $(binDir)
	$(cxx) $^ $(exeArgs) -o $@
	@echo [MAKE] Done compiling $<.

$(target).cpp: dispatcher.hpp

dispatcher.cpp: dispatcher.hpp

