/* Driver code for the Lexer.
 * 
 * Created by: Shihao Song
 * Modified by: Naga Kandasamy
 * Date modified: August 10, 2021
*/


#include "lexer/lexer.hh"

#include <iomanip>
#include <iostream>

using namespace Frontend;

int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s source-code-file\n", argv[0]);
        exit(EXIT_SUCCESS);
    }
    
    Lexer lexer(argv[1]);       // Instantiate the lexer
    Token tok;                  // Instantiate a token 
    
    while (lexer.getToken(tok)) {
        std::cout << std::setw(12)
                  << tok.prinTokenType() << " | "
                  << tok.getLiteral() << "\n";
    }

    exit(EXIT_SUCCESS);
}
