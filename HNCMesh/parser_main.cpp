#include "pinnacleLexer.h"
#include "pinnacleParser.h"

#include <cassert>
#include <map>
#include <string>
#include <iostream>

using namespace std;

class ExprTreeEvaluator {
    map<string,int> memory;
public:
    int run(pANTLR3_BASE_TREE);
};

pANTLR3_BASE_TREE getChild(pANTLR3_BASE_TREE, unsigned);
const char* getText(pANTLR3_BASE_TREE tree);

int main(int argc, char* argv[])
{
  pANTLR3_INPUT_STREAM input;
  ppinnacleLexer lex;
  pANTLR3_COMMON_TOKEN_STREAM tokens;
  ppinnacleParser parser;

  assert(argc > 1);
  input = antlr3AsciiFileStreamNew((pANTLR3_UINT8)argv[1]);
  lex = pinnacleLexerNew(input);
  tokens = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT,
                                            TOKENSOURCE(lex));
  parser = pinnacleParserNew(tokens);

  cout << "Starting parse of " << argv[1] << endl;

  parser->start(parser);

//  parser->prog(parser);

//  pinnacleParser_prog_return r = parser->prog(parser);

//  pANTLR3_BASE_TREE tree = r.tree;

//  ExprTreeEvaluator eval;
//  int rr = eval.run(tree);
//  cout << "Evaluator result: " << rr << '\n';

  parser->free(parser);
  tokens->free(tokens);
  lex->free(lex);
  input->close(input);

  return 0;
}

