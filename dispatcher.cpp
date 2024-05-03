
/**
 * @file    dispatcher.cpp
 * @author  Austin Walker
 * @date    2/23/2024
 * @version 0.1
 * 
 * @brief Contains the defintions for the dispatcher class
 * @note  Documentation can be found in the header dipatcher header file
 */

//---Libraries---
#include <unistd.h>
#include <string>
#include <cstring>
#include <vector>
#include <sstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <climits>
#include <cstdlib>
//---Headers---
#include "dispatcher.hpp"

using namespace std;

//---Private Methods---
/*
int dispatcher_c::makearg(const char str[], char **argv[]) {
   stringstream tokenStream = stringstream(str);     //storage for the line containing the arguments
   string temp_s = "";                                    //temp storage for tokens
   vector<string> args = vector<string>(); //temp storage for the arguments before they are passed to argv
   int numArgs = 0;                                            //number of arugments found
   //read tokens into the vector tokens
   while(!tokenStream.eof()) {
      static bool quotes_f = false; //flag that is set when reading a multi-token argument (such as a string with spaces)
      //move a token to temp_s
      tokenStream >> temp_s;
      //move from token from temp storage to the vector
      args.push_back(temp_s);
      //check for errors
      if(!tokenStream) {
         //throw runtime_error("tokenstream: " + tokenStream.rdstate());
         return -1;
      }
   }
   //allocate an array of char pointers by the number of arguments
   numArgs = args.size();
   *argv = new char*[numArgs + 1];
   //populate argv
   for(size_t arg = 0; arg < numArgs; arg ++) {       //go through each argument entry
      (*argv)[arg] = new char[args[arg].size() + 1];  //+1 for the null terminator
      for(size_t i = 0; i < args[arg].size(); i ++) { //add all characters from the string in the vector to corresponding string in argv
         (*argv)[arg][i] = args[arg][i];
      }
      (*argv)[arg][args[arg].size()] = '\0';          //null terminate the string in argv
   }
   //make the last argument NULL
   (*argv)[numArgs] = NULL;

   return numArgs;
}
*/

//---Constuctors---
dispatcher_c::dispatcher_c() {
   numRunning = 0;
   backgroundPs = vector<process>();
   foregroundP.pid = 0;
   foregroundP.state = process::UINIT;
}

//---Methods---
void dispatcher_c::addProcess(char *argv[], bool bf) {
   //setup new process
   process newP;
   newP.state = process::READY;
   newP.background_f = bf;
   newP.pid = 0;
   newP.args = argv;
   //add process to correct location
   if(bf) {
      backgroundPs.push_back(newP);
   } else {
      foregroundP = newP;
   }
}

//void dispatcher_c::dispatch(char *argv[], string PATH, bool bf) {
void dispatcher_c::dispatch(char *argv[], bool bf) {
   int exitStatus; //exit status of a dying child
   pid_t newPid;   //stores the pid returned by fork
   string PATH = (string)getenv("PATH");

   addProcess(argv, bf);
   //do the fork
   newPid = fork();
   if(newPid == 0) { 
      //The child
      //check for a path
      if(strchr(argv[0], '/') != NULL) {
         char* commandPath = realpath(argv[0], NULL);
         if(commandPath == NULL) {
            cerr << "[WARNING] Couldn't expand the path: " << argv[0] << '\n';
            exit(1);
         } else
         if(execv(commandPath, argv) < 0) {
            cerr << "[WARNING] Process " << getpid() << ": execv failed on command: ";
            for(int i = 0; argv[i] != NULL; i++) {
               cerr << argv[i];
            }
            cerr << '\n';
            exit(1);
         }
      } else /*
      //check for cd
      if(strcmp(argv[0], "cd") == 0) {
         if(argv[1] != NULL) {
            if(chdir(argv[1]) < 0) {
               throw runtime_error("Couldn't change directories");
            }
          } else {
            throw invalid_argument("cd needs a path\n   try: cd {path}");
         }
         return;
      } else */{
         //search using PATH
         string commandPath = "";
         size_t start = 0;
         for(int i = 0; i < PATH.length(); i++) {
            if(PATH[i] == ':') {
               commandPath = PATH.substr(start, (i - start));
               if(commandPath.back() != '/') {
                  commandPath.append(1, '/');
               }
               commandPath.append((string)(argv[0]));
               start = i + 1;
               execv(commandPath.c_str(), argv);
            }
         }
         if(start < PATH.length()) {
            commandPath = PATH.substr(start, string::npos);
            if(commandPath.back() != '/') {
               commandPath.append(1, '/');
            }
            commandPath.append((string)(argv[0]));
            execv(commandPath.c_str(), argv);
         }
      }
      cerr << "[WARNING] Couldn't find the command: " << argv[0] << '\n';
      exit(1);
   } else
   if(newPid < 0) { 
      //fork error
      throw runtime_error("Couldn't fork to service command");
   } else {
      //the parent
      numRunning ++;
      //do some house keeping
      if(bf) {
         backgroundPs.back().pid = newPid;
         backgroundPs.back().state = process::RUNNING;
         cout << "PID: " << newPid << "; Num Background Tasks: " << backgroundPs.size() << endl;
      } else {
         bool wait_f = true; //flag that is set when needing to wait for the process
         foregroundP.pid = newPid;
         foregroundP.state = process::RUNNING;
         //wait for foreground to exit
         while(wait_f) {
            if(waitpid(foregroundP.pid, &exitStatus, 0) < (pid_t)0) {
               if(errno == ECHILD) {
                  cerr << "[WARNING] Tried to wait for foreground task, and there were no tasks to wait for\n";
                  wait_f = false;
               } else
               if(errno == EINVAL) {
                  cerr << "[FATAL] waitpid's option argument is invalid\n";
                  killChildren();
                  exit(1);
               } else
               if(errno == EINTR) {
                  cerr << "[INFO] Something interupted waiting for foreground task\n";
               }
            } else {
               wait_f = false;
            }
         }
         numRunning--;
         foregroundP.state == process::TERM;
         if(exitStatus != 0) {
            cerr << "[WARNING] One of the children had troubles\n";
         }
      }
   }
}

void dispatcher_c::clearChildren() {
   pid_t waitR;
   int status;
   bool wait_f = true;
   while(wait_f) {
      while((waitR = waitpid(0, &status, WNOHANG)) > 0) {
         numRunning--;
         for(int i = 0; i < backgroundPs.size(); i++) {
            if(backgroundPs[i].pid == waitR) {
               backgroundPs.erase(backgroundPs.begin() + i);
               cout << '[' << waitR << ']' << " Done" << "; Num Background Tasks: " << backgroundPs.size() << endl;
            }
         }
      }
      if(waitR == 0) {
         wait_f = false;
      } else
      if(errno == ECHILD) {
         wait_f = false;
      } else
      if(errno == EINTR) {
      } else
      if(errno == EINVAL) {
         cerr << "[FATAL] dispatcher_c::clearChildren(): Invalid option argument was given to waitpid()\n";
         killChildren();
         exit(1);
      }
   }
}

void dispatcher_c::killChildren() {
   int exitStatus;
   if(foregroundP.state == process::RUNNING) {
      kill(foregroundP.pid, SIGTERM);
      waitpid(foregroundP.pid, &exitStatus, 0);
      foregroundP.state = process::TERM;
      numRunning--;
   }
   while(backgroundPs.size() > 0) {
      process current = backgroundPs.back();
      kill(current.pid, SIGTERM);
      waitpid(current.pid, &exitStatus, 0);
      backgroundPs.pop_back();
      numRunning--;
   }
}

