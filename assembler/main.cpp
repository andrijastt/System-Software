#include "assembler.hpp"
#include "exceptions.hpp"

int main(int argc, char const *argv[]){
  
  // cout << operandsForJumps << endl;
  // cout << operandsForData << endl;
  cout << listSymbolsAndLiterals << endl;

  try{
    string options = argv[1];
    string outputFile = argv[2];
    string inputFile = argv[3];

    if(options != "-o"){
      throw InputException();
    }

  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << '\n';
  }
  

  return 0;
}