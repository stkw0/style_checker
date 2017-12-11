#include <iostream>
#include <memory>
#include <regex>
#include <unordered_map>

#include <cctype>

#include <clang-c/Index.h>

using std::regex;

// Return a copy of the CXString in a std::string
std::string ToString(CXString&& src_str) {
    const std::string dst_str{clang_getCString(src_str)};
    clang_disposeString(src_str);
    return dst_str;
}

struct Location {
    std::string file;
    unsigned line;
};

Location getLocation(CXSourceLocation src_loc) {
    CXFile file;
    unsigned line;
    clang_getSpellingLocation(src_loc, &file, &line, nullptr, nullptr);

    auto file_name = ToString(clang_getFileName(file));
    return Location{file_name, line};
}

void report(const std::string& what, const std::string& name, const Location& loc) {
    std::cout << what << ": " << name << ". File: " << loc.file
              << ". Line: " << loc.line << std::endl;
}

struct Cursor {
    CXCursor current;
    CXCursor parent;
};

class FileUnit {
  public:
    FileUnit(const CXCursor& c) : _first_cursor(c) {
    }

    void addCursor(const Cursor& c) {
        _cursors.emplace_back(c);
    }

    void checkCursor(const Cursor& cursor) {
        auto c = cursor.current;
        auto kind = clang_getCursorKind(c);
        auto name = ToString(clang_getCursorSpelling(c));
        auto loc = getLocation(clang_getCursorLocation(c));

        bool valid_name = true;
        if(auto r = _naming_rules.find(kind);
           r != _naming_rules.end() && !std::regex_match(name, r->second))
            valid_name = false;

        switch(kind) {
        case CXCursor_VarDecl:
            if(!isConst(c) && !valid_name) report("Variable", name, loc);
            if(!isStatic(c) && !isConst(c) &&
               clang_equalCursors(clang_getCursorSemanticParent(c), _first_cursor))
                report("Global variable", name, loc);
            break;
        default:
            if(!valid_name)
                report(ToString(clang_getCursorKindSpelling(kind)), name, loc);
            break;
        }
    }

    void checkCursors() {
        for(const auto& c : _cursors) checkCursor(c);
    }

  private:
    bool isConst(const CXCursor& c) {
        return clang_isConstQualifiedType(clang_getCursorType(c));
    }

    bool isStatic(const CXCursor& c) {
        return clang_Cursor_getStorageClass(c) == CX_SC_Static;
    }

  private:
    CXCursor _first_cursor;
    std::vector<Cursor> _cursors;

    std::unordered_map<CXCursorKind, std::regex> _naming_rules = {
        {CXCursor_CXXMethod, regex("([a-z][a-zA-Z0-9]+)")},
        {CXCursor_FunctionDecl, regex("main|([A-Z][a-zA-Z0-9]+)")},
        {CXCursor_FieldDecl, regex("^_([a-z0-9]+).*")},
        {CXCursor_VarDecl, regex("([a-z]?[_a-z0-9]+)")},
    };
};

CXChildVisitResult visitFunction(CXCursor c, [[maybe_unused]] CXCursor parent,
                                 CXClientData client_data) {
    if(clang_Location_isFromMainFile(clang_getCursorLocation(c)) == 0)
        return CXChildVisit_Continue;

    auto fu = static_cast<FileUnit*>(client_data);
    fu->addCursor({c, parent});
    return CXChildVisit_Recurse;
}

int main(int argc, char* argv[]) {
    if(argc != 2) {
        std::cerr << "Not enough parameters" << std::endl;
        return EXIT_FAILURE;
    }

    CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit unit = clang_parseTranslationUnit(
        index, argv[1], nullptr, 0, nullptr, 0, CXTranslationUnit_None);

    if(!unit) {
        std::cerr << "Unable to parse translation unit. Quitting." << std::endl;
        return EXIT_FAILURE;
    }

    CXCursor cursor = clang_getTranslationUnitCursor(unit);
    FileUnit f{cursor};
    clang_visitChildren(cursor, visitFunction, &f);
    f.checkCursors();

    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);
}
