/* Parser driver code.
 * Author: Shihao Song
 * Modified by: Naga Kandasamy
 * Date: September 14, 2021
 */
#include "lexer/lexer.hh"
#include "parser/parser.hh"

#include <iomanip>
#include <iostream>

using namespace Frontend;

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "%s file-name\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    Parser parser(argv[1]); // Instantiate Parser object
    parser.printStatements();

    exit(EXIT_SUCCESS);
}
