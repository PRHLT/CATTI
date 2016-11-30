#ifndef PLUGIN_HPP
#define PLUGIN_HPP

// Based on 
// http://www.isotton.com/devel/docs/C++-dlopen-mini-HOWTO/C++-dlopen-mini-HOWTO.html

#include <iostream>
#include <cassert>
#include <stdexcept>
#include <typeinfo>

#include <string>
#include <vector>
// #include <cstdint>
#include <cstring>

#include <casmacat/utils.h>
#include <casmacat/Context.h>

//#define PLUGIN_USE_LIBTOOL


#ifdef PLUGIN_USE_LIBTOOL
  #include <ltdl.h>
#else
  #include <dlfcn.h>
#endif


namespace casmacat {

  // Tokenize a string at spaces/quotation blocks
  template <typename T>
  void tokenize_args(const T str, std::vector<T> &tokens) {
    if (not str.empty()) {
      tokens.clear();
      char inside_quote = 0;
      char last_char = 0;
      T token;

      for (typename T::const_iterator it = str.begin(); it < str.end(); last_char = *it, ++it) {
        if (last_char != '\\' and T("\"'").find(*it) != T::npos) {
          if (inside_quote == 0 ) {
            inside_quote = *it;
          }
          else if (*it == inside_quote) {
            inside_quote = 0;
          }
          continue;
        }

        // If in quote append data regardless of what it is
        if (inside_quote) {
          token += *it;
        }
        else {
          if (*it == ' ') {
            if (not token.empty()) {
              tokens.push_back(token);
              token.clear();
            }
          }
          else {
            token += *it;
          }
        }
      }

      if (not token.empty()) {
        tokens.push_back(token);
      }
    }
  }


  template <typename value_type>
  class Plugin {
    typedef value_type* (*create_fn)(int argc, char *argv[], Context *context);
    typedef void (*destroy_fn)(value_type *);
    typedef const std::type_info& (*plugin_type_fn)();

    std::string plugin_fn;
    std::string default_args;
    std::string create_symbol_name;
    std::string destroy_symbol_name;
    std::string plugin_type_name;

    create_fn create_;
    destroy_fn destroy_;
    plugin_type_fn plugin_type_;

#ifdef PLUGIN_USE_LIBTOOL
    lt_dlhandle library_h_;
#else
    void *library_h_;
#endif
  public:

    Plugin(const std::string &_plugin_fn, const std::string &_default_args = "",
    		   const std::string &_plugin_name = "plugin")
           : plugin_fn(_plugin_fn), default_args(_default_args),
             create_symbol_name("new_" + _plugin_name), destroy_symbol_name("delete_" + _plugin_name),
             plugin_type_name(_plugin_name + "_type")
    {
      using std::cout;
      using std::cerr;

      const char* dlsym_error = NULL;
#ifdef PLUGIN_USE_LIBTOOL
      #define plugin_dlopen  lt_dlopenadvise
      #define plugin_dlerror lt_dlerror
      #define plugin_dlsym   lt_dlsym
      #define plugin_dlclose lt_dlclose

      dlsym_error = plugin_dlerror();
      if (lt_dlinit() != 0 or dlsym_error) {
          std::cerr << "Cannot initialize library system: " << dlsym_error << std::endl;
          throw std::runtime_error(dlsym_error); 
      }
      lt_dladvise advise;
      lt_dladvise_init(&advise);
      lt_dladvise_ext(&advise);
      lt_dladvise_global(&advise);

#else
      #define plugin_dlopen  dlopen
      #define plugin_dlerror dlerror
      #define plugin_dlsym   dlsym
      #define plugin_dlclose dlclose

      int advise = RTLD_NOW | RTLD_GLOBAL;
#endif
     
      if (plugin_fn.empty()) {
        cerr << "Plugin undefined: " << dlsym_error  << std::endl;
        throw std::runtime_error(dlsym_error);
      }
  
      // load the dynamic 
      cerr << "Loading plug-in from '" << plugin_fn << "'" << std::endl;
      library_h_ = plugin_dlopen(plugin_fn.c_str(), advise);
      dlsym_error = plugin_dlerror();
      if (library_h_ == 0 or dlsym_error) {
          cerr << "Cannot load library: " << dlsym_error << std::endl;
          throw std::runtime_error(dlsym_error); 
      }

      // load the plugin type
      plugin_type_ = reinterpret_cast<plugin_type_fn>(plugin_dlsym(library_h_, plugin_type_name.c_str()));
      dlsym_error = plugin_dlerror();
      if (dlsym_error) {
          cerr << "Cannot load symbol '" << plugin_type_name << "': " << dlsym_error << std::endl;
          throw std::runtime_error(dlsym_error);
      }
      else if (not plugin_type_) {
          cerr << "Incompatible symbol '" << plugin_type_name << "': " << dlsym_error << std::endl;
          throw std::runtime_error(dlsym_error);
      }

      // check names instead of types since some c++ compilers do not work
//      if (plugin_type_() != typeid(value_type)) {
      if (strcmp(plugin_type_().name(), typeid(value_type).name()) != 0) {
        cerr << "Incompatible plugin: '" << plugin_fn << "' is a plugin of type '" << plugin_type_().name() << "' instead of '" << typeid(value_type).name() << "'" << std::endl;
        throw std::runtime_error(dlsym_error);
      }

      // load the creator
      create_ = reinterpret_cast<create_fn>(plugin_dlsym(library_h_, create_symbol_name.c_str()));
      dlsym_error = plugin_dlerror();
      if (dlsym_error) {
          cerr << "Cannot load symbol '" << create_symbol_name << "': " << dlsym_error << std::endl;
          throw std::runtime_error(dlsym_error); 
      }
      else if (not create_) {
          cerr << "Incompatible symbol '" << create_symbol_name << "': " << dlsym_error << std::endl;
          throw std::runtime_error(dlsym_error);
      }

       // load the destroyer 
      destroy_ = reinterpret_cast<destroy_fn>(plugin_dlsym(library_h_, destroy_symbol_name.c_str()));
      dlsym_error = plugin_dlerror();
      if (dlsym_error) {
          cerr << "Cannot load symbol '" << destroy_symbol_name << "': " << dlsym_error << std::endl;
          throw std::runtime_error(dlsym_error); 
      }
      else if (not destroy_) {
          cerr << "Incompatible symbol '" << destroy_symbol_name << "': " << dlsym_error << std::endl;
          throw std::runtime_error(dlsym_error);
      }
  
#ifdef PLUGIN_USE_LIBTOOL
      lt_dladvise_destroy(&advise);
#endif
    }

    virtual ~Plugin() {
      plugin_dlclose(library_h_);
#ifdef PLUGIN_USE_LIBTOOL
      lt_dlexit();
#endif
    };

    // Note: value_type needs to be explicitly declared so that SWIG does not complain
    value_type *createCArgs(int argc, char *argv[], Context *context = 0) {
      return create_(argc, argv, context);
    }


    value_type *createVectorStringArgs(const std::vector<std::string> &args, Context *context = 0) {
      int argc = args.size() + 1;
      char **argv = new char *[argc + 1];
      std::string name = plugin_fn;
      argv[0] = new char[strlen(name.c_str()) + 1];
      strcpy(argv[0], name.c_str());
      for (size_t argc = 0;argc < args.size(); argc++) {
        argv[argc + 1] = new char[strlen(args[argc].c_str()) + 1];
        strcpy(argv[argc + 1], args[argc].c_str());
      }
      argv[argc] = NULL;

      value_type *value = create_(argc, argv, context);

      for (int i = 0; i < argc; i++) {
        delete[] argv[i];
      }
      delete[] argv;

      return value;
    }

    value_type *createStringArgs(const std::string &cmd, Context *context = 0) {
      std::vector<std::string> args;
      tokenize_args<std::string>(cmd, args);
      return createVectorStringArgs(args, context);
    }

    value_type *create(Context *context = 0) {
      return createStringArgs(default_args, context);
    }

    void destroy(value_type *obj) {
      destroy_(obj);
    }

  private:
    // Following the rule of three
    Plugin(const Plugin&);   // Disallow copy contructor
    Plugin& operator=(const Plugin&); // Disallow assignment operator

  };

}

#endif
