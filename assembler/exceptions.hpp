#include <exception>

class InputException: public std::exception{

public:
  const char* what() const throw(){
    return "Wrong terminal input";
  };
};