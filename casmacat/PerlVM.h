
#ifndef CASMACAT_PERL_VM_H_
#define CASMACAT_PERL_VM_H_


struct PerlInterpreter;

namespace casmacat {

class PerlVM {
  static size_t _num_perl_vm_instances;
  PerlInterpreter *vm;
public:

};

}

#endif /* CASMACAT_PERL_VM_H_ */
