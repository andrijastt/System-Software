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

class MulitpleDefinitionOfSymbolException: public std::exception{

public:
  const char* what() const throw(){
    return "Multiple definition of same symbol found!";
  };
};

class UndefinedSymbolException: public std::exception{

public:
  const char* what() const throw(){
    return "Undefined symbol found!";
  };
};