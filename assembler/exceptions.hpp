#include <exception>

class InputException: public std::exception{

public:
  const char* what() const throw(){
    return "Wrong terminal input";
  };
};

class NonexistantInputFileException: public std::exception{

public:
  const char* what() const throw(){
    return "Input file doesn't exist";
  };
};

class BadSyntaxException: public std::exception{

public:
  const char* what() const throw(){

    return "Syntax error at line: ";
  };
};

class NoSectionException: public std::exception{

public:
  const char* what() const throw(){

    return "Not in section @ line: ";
  };
};