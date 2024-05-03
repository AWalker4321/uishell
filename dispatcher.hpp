
#pragma once

/**
 * @file    dispatcher.hpp
 * @author  Austin Walker
 * @date    2/23/2024
 * @version 0.1
 *
 * @brief Contains the declaration and documentation for the dispatcher class 
 */

class dispatcher_c {
   //---Custom Types---
   /**
    * @breif Declares a structure used to represnet processes
    */
   struct process {
      //custom types
      enum state_c{UINIT, RUNNING, READY, TERM}; //constants for the current state of the processes. Running means the process is running, ready means the process is ready, and term means the process has been terminated
      //members
      state_c     state;     //the current state of the process
      bool background_f;     //flag set when the process runs in the background
      char **args;           //arguments of the process
      pid_t       pid;       //pid of the process from the OS
   };
   //---Members---
   int numRunning;                    //Number of running processes
   process foregroundP;               //foreground process
   std::vector<process> backgroundPs; //storage for information on background processes
   //---Private Methods---
   /**
    * @param s    The string to tokenize
    * @param args used to return the memory address of a dynamically allocated array of c strings
    *
    * @return int representing the number of arguments in argv; on error returns -1
    */
   //int makearg(const char str[], char **argv[]);
   /**
    *
    */
   void addProcess(char *argv[], bool bf);
   public:
   //---constructors---
   /**
    * @brief does some necessary basic setup, such as changing the sigchld handler
    */
   dispatcher_c();
   //---Methods---
   /**
    * @brief Dispatches a new process
    */
   //void dispatch(char *argv[], std::string PATH, bool bf);
   void dispatch(char *argv[], bool bf);
   /**
    * @brief Acknowledges children's deaths
    */
   void clearChildren();
   /**
    * @brief Kills all children
    */
   void killChildren();
};

