
//---Libraries---
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

//Headers
#include "dispatcher.hpp"

using namespace std;

//---Functions---
int makearg(string str, char **argv[]);

//---Custom Data Types---
/*
//-Structs
struct shellVar {
   string name;
   string data;

   shellVar() {
      name = "";
      data = "";
   }
};
*/

//---Globals---
//vector<shellVar> shellVars = vector<shellVar>(); //storage for shell variables

int main() {
   //---Setup---
   bool ucExit_f = false;                    //flag that is set when the user calls for an exit
   dispatcher_c dispatcher = dispatcher_c(); //object used to dispatch processes
   /*
   //set the path shell variable
   shellVars.push_back(shellVar());
   shellVars[0].name = "PATH";
   shellVars[0].data = (string)getenv("PATH");
   */

   //---Main Loop---
   do {
      string uiBuffer;
      //prompt
      cout << "$ ";
      getline(cin, uiBuffer);
      if(!cin) {
         continue;
      }
      //pre-checks
      //check for declaration of variable
      size_t eqPos = uiBuffer.find('=');
      if(eqPos != string::npos && eqPos != 0 && !isspace(uiBuffer[eqPos - 1])) {
         string varName = uiBuffer.substr(0, eqPos);
         string varData = uiBuffer.substr(eqPos + 1, string::npos);
         /*
         bool newVar = true;
         for(int i = 0; i < shellVars.size(); i++) {
            if(shellVars[i].name.compare(varName) == 0) {
               shellVars[i].data = varData;
               newVar = false;
            }
         }
         if(newVar) {
            shellVar newVar;
            newVar.name = varName;
            newVar.data = varData;
            shellVars.push_back(newVar);
         }
         */
         char buffer[uiBuffer.length() + 1];
         strncpy(buffer, uiBuffer.c_str(), uiBuffer.length() + 1);
         if(setenv(varName.c_str(), varData.c_str(), 1) < 0) {
            if(errno == EINVAL) {
               cerr << "[WARNING] uishell: " << varName << " is an invalid variable name\n";
            } else
            if(errno == ENOMEM) {
               cerr << "[WARNING] uishell: Not enough memory\n";
            } else {
               cerr << "[WARNING] uishell: Unkown error\n";
            }
         }
      } else
      //exit?
      if(uiBuffer.compare("exit") == 0) {
         ucExit_f = true;
      } else 
      if(uiBuffer.substr(0, 3).compare("cd ") == 0) {
         //chdir(argv[1])
         string path = uiBuffer.substr(3, string::npos);
         if(path != "") {
            if(chdir(path.c_str()) < 0) {
               cerr << "[WARNING]  uishell: Couldn't change pwd to: " << path << '\n';
            }
         } else {
            cerr << "[WARNING] uishell: cd needs an argument.\n   Try: cd {path}\n";
         }
      } else {
         bool background_f = false;
         //attempt to execute a command
         //background?
         if(uiBuffer.back() == '&') {
            background_f = true;
            uiBuffer.pop_back();
         }
         //make args
         char **argv;
         try {
            makearg(uiBuffer, &argv);
         }
         catch(invalid_argument &ia) {
            cerr << "[WARNING] uiShell: makearg: " << ia.what() << '\n';
            continue;
         }
         catch(runtime_error &re) {
            cerr << "[WARNING] uiShell: makearg: " << re.what() << '\n';
            continue;
         }
         //dispatch
         try {
            dispatcher.dispatch(argv, background_f);
         }
         catch(runtime_error &re) {
            cerr << "[WARNING] uiShell: dispatcher_c::dispatch: " << re.what() << '\n';
            continue;
         }
         catch(invalid_argument &ia) {
            cerr << "[WARNING] uiShell: dispacther_c::dispatch: " << ia.what() << '\n';
            continue;
         }
      }
      dispatcher.clearChildren();
      
   } while(cin && !ucExit_f);
   //---Exit---
   dispatcher.killChildren();
   exit(0);
}

//---Auxiliary Functions---
void expandVar(string& varName) {
   /*
   int shellI = 0;
   while(shellI < shellVars.size()) {
      if(!varName.compare(shellVars[shellI].name)) {
         varName = shellVars[shellI].data;
         return;
      }
      shellI++;
   }
   throw invalid_argument(varName + " is not a shell varaible");
   */
   char* buffer = getenv(varName.c_str());
   if(buffer == NULL) {
      throw invalid_argument(varName + " is not a shell varaible");
   } else {
      varName = (string)buffer;
   }
}

//---Function Definitions---
int makearg(string str, char **argv[]) {
   stringstream tokenStream = stringstream(str); //storage for the line containing the arguments
   string token = "";                            //temp storage for tokens
   vector<string> args = vector<string>();       //temp storage for the arguments before they are passed to argv
   int numArgs = 0;                              //number of arugments found
   //read tokens into the vector tokens
   while(!tokenStream.eof()) {
      static bool quotes_f = false; //flag that is set when reading a multi-token argument (such as a string with spaces)
      string buffer;
      //setup token
      token = "";
      do {
         //add input to buffer
         tokenStream >> buffer;
         //check for strings
         for(int i = 0; i < buffer.length(); i++) {
            if(buffer[i] == '"') {
               if(buffer[i - 1] != '\\') { //when escaped keep quote in token
                  buffer.erase(buffer.begin() + i);
               } else {
                  quotes_f = !quotes_f;
                  buffer.erase(buffer.begin() + i);
               }
            } else
            if(buffer[i] == '$') {
               buffer.erase(buffer.begin());
               if(buffer.back() == '"') {
                  quotes_f = !quotes_f;
                  buffer.erase(buffer.end());
               }
               string var = buffer.substr(i, string::npos);
               try {
                  expandVar(var);
               }
               catch(invalid_argument &ia) {
                  throw invalid_argument((string)"expandVar: " + (string)ia.what());
               }
               buffer.erase(buffer.begin() + i, buffer.end());
               buffer.append(var);
            }
         }
         //when in a string append buffer to token
         token.append(buffer);
      } while(quotes_f);
      //move token from temp storage to the vector
      args.push_back(token);
      //check for errors
      if(!tokenStream) {
         throw runtime_error("tokenStream: " + tokenStream.rdstate());
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
   //make the last argument null
   (*argv)[numArgs] = NULL;

   return numArgs;
}



