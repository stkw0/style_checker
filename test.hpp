int banned_global_variable;
const int unbanned_global_variable;
constexpr int unbanned_global_variable2 = 2;

void ValidFuncName();
void invalidFuncName();
void invalid_funcName();

class A {
   void validFuncName();
   void invalid_FuncName();
   void Invalid();

   int invalid_member;
   int _valid_member;
   int _Invalid_member;
};
