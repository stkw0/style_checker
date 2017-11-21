#include <iostream>
#include <cctype>
#include <clang-c/Index.h>

using namespace std;

ostream& operator<<(ostream& stream, const CXString& str) {
   stream << clang_getCString(str);
   clang_disposeString(str);
   return stream;
}

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
   clang_visitChildren(cursor, 
         [](CXCursor c, CXCursor parent, CXClientData client_data) 
            {
               if(clang_Location_isFromMainFile(clang_getCursorLocation(c)) == 0)
                  return CXChildVisit_Continue;

               
               auto kind = clang_getCursorKind(c);
               if(kind == CXCursor_CXXMethod) {
                  auto cxstring = clang_getCursorSpelling(c);
                  auto name = clang_getCString(cxstring);
                  if(!isMethodName(name)) {
                     auto loc = clang_getCursorLocation(c);
                     CXFile file;
                     unsigned line;
                     clang_getSpellingLocation(loc, &file, &line, nullptr, nullptr);
                     auto cxstring2 = clang_getFileName(file);
                     std::cout << "ERR Method: " << name << ". File: " <<  clang_getCString(cxstring2) << ". Line: " << line << endl;
                     clang_disposeString(cxstring2);
                  }
                  clang_disposeString(cxstring);
               } else if(kind == CXCursor_FunctionDecl) {
                  auto cxstring = clang_getCursorSpelling(c);
                  auto name = clang_getCString(cxstring);
                  clang_disposeString(cxstring);
               }

               return CXChildVisit_Recurse;
            },
         nullptr);
   
   clang_disposeTranslationUnit(unit);
   clang_disposeIndex(index);
}
