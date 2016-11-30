/*
 *   Copyright 2012, valabau
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 * 
 * translate.cpp
 *
 *  Created on: 21/06/2012
 *      Author: valabau
 */



#include <cstdlib>
#include <casmacat/compat.h>
#include <casmacat/utils.h>
#include <casmacat/IImtEngine.h>
#include <casmacat/Plugin.h>
#include <iostream>
#include <fstream>
#include <iterator>
#include <cassert>

using namespace casmacat;
using namespace std;

int main(int argc, char* argv[]) {

    if (argc != 2) {
      cerr << "Usage: " << argv[0] << " test\n";
      return EXIT_FAILURE;
    }


    string imt_plugin_fn = "./catti-plugin.so";
    string args = "./Test/wg.paths";

    Plugin<IInteractiveMtFactory> imt_plugin(imt_plugin_fn, args);
    IInteractiveMtFactory *imt_factory = imt_plugin.create();

    IInteractiveMtEngine *imt = imt_factory->createInstance();

    if (imt == 0) {
      cerr << "Plugin could not be instantiated\n";
    }
    else {
      cerr << "Plugin loaded\n";

      for (int i = 1; i < argc; i++) {
        ifstream file(argv[i]);
        string source;
        vector<string> tok_source, tok_target;

        while(getline(file, source)) {
          tokenize(source, tok_source);

          IInteractiveMtSession *session = imt->newSession(tok_source);

          session->setPrefix(vector<string>(), vector<string>(), false, tok_target);
           
          cout << source << "|||";
          copy(tok_target.begin(), tok_target.end(), ostream_iterator<string>(cout, " "));
	  cout << "\n";
            

          vector<string> prefix;
         prefix.push_back("Geschäfts");        

          session->setPrefix(prefix, vector<string>(), false, tok_target);
          cout << source << "|||";
          copy(tok_target.begin(), tok_target.end(), ostream_iterator<string>(cout, " "));
          cout << "\n";


          session->setPrefix(prefix, vector<string>(), true, tok_target);
          cout << source << "|||";
          copy(tok_target.begin(), tok_target.end(), ostream_iterator<string>(cout, " "));
          cout << "\n";
	
	    
        }
      }
    }

    delete imt;
    delete imt_factory;

    return EXIT_SUCCESS;
}


