#include <iostream>
#include <memory>
#include <cctype>
#include <clang-c/Index.h>

using namespace std;

class String {
   public:
      String(CXString&& str) : _str(str) {}
      ~String() {
         clang_disposeString(_str);
      }

      std::string getStr() const{
         return clang_getCString(_str);
      }
private:
   CXString _str;
};

bool haveUnderscore(const std::string& name) {
   for(auto& c : name)
      if(c == '_')
         return true;

   return false;
}

bool isMethodName(const std::string& name) {
   if(isupper(name[0]) || haveUnderscore(name)) 
      return false;

   return true;
}

bool isFuncName(const std::string& name) {
   if(name == "main") return true ;

   if(islower(name[0]) || haveUnderscore(name)) 
      return false;

   return true;
}

struct Location {
   std::string file;
   unsigned line;
};

Location getLocation(CXSourceLocation src_loc) {
    CXFile file;
   unsigned line;
   clang_getSpellingLocation(src_loc, &file, &line, nullptr, nullptr);

   auto file_name = String(clang_getFileName(file)).getStr();
   return Location{file_name, line};
}

 CXChildVisitResult visitFunction(CXCursor c, CXCursor parent, CXClientData client_data) {
   if(clang_Location_isFromMainFile(clang_getCursorLocation(c)) == 0)
                  return CXChildVisit_Continue;

               
               auto kind = clang_getCursorKind(c);
               if(kind == CXCursor_CXXMethod) {
                  auto name = String(clang_getCursorSpelling(c)).getStr();
                  if(!isMethodName(name)) {
                     auto loc = getLocation(clang_getCursorLocation(c));
                     std::cout << "Method: " << name << ". File: " <<  loc.file << ". Line: " << loc.line << endl;
                  }
               } else if(kind == CXCursor_FunctionDecl) {
                  auto name = String(clang_getCursorSpelling(c)).getStr();
                  if(!isFuncName(name)) {
                    auto loc = getLocation(clang_getCursorLocation(c));
                     std::cout << "Function: " <<  name << ". File: " <<  loc.file << ". Line: " << loc.line << endl;
                  }
               }

               return CXChildVisit_Recurse;
 }

int main(int argc, char* argv[]) {
   if(argc != 2) {
      std::cerr << "Not enough parameters" << std::endl;
      return EXIT_FAILURE;
   }

   CXIndex index = clang_createIndex(0,0);
   CXTranslationUnit unit = clang_parseTranslationUnit(index, argv[1], nullptr, 0, nullptr, 
         0, CXTranslationUnit_None);

   if(!unit) {
      std::cerr << "Unable to parse translation unit. Quitting." << std::endl;
      return EXIT_FAILURE;
   }

   CXCursor cursor = clang_getTranslationUnitCursor(unit);
   clang_visitChildren(cursor, visitFunction, nullptr);
   
   clang_disposeTranslationUnit(unit);
   clang_disposeIndex(index);
}