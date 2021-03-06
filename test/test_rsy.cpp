#ifdef RSY_TEST
#include <iostream>
#include <variant>
#include <vector>
#include <string>
#include <exception>
#include <type_traits>
#include "../src/lexer.h"
#include "../src/miniC_exception.h"
#include "../src/preprocess.h"
#include "test.h"
#include <fstream>
#include <iomanip>
#include <fstream>

std::ofstream os;
const char* out_path = "D:/Git_repositories/Mini_C/doc/result.lr1";
const char* program_txt = "D:/Git_repositories/Mini_C/test/test/rsy.txt";
auto ____ = []()
{
	os.open(out_path, std::ios::out | std::ios::trunc);
	if (!os.is_open())
	{
		std::cout << "Can't open file: " << out_path << std::endl;
		exit(0);
	}
	return 0;
}();
#define out os
#define OUTPUT_DFA
#include "../src/lr1.hpp"

void rsy_lexer_test()
{

	bool preprocess = false;
	std::string file;
	try { file = Mini_C::preprocess::preprocess(program_txt, os); preprocess = true; }
	catch (const Mini_C::MiniC_Base_Exception& e) { e.printException(); std::cout << std::endl; }
	catch (const std::exception& e) { std::cout << e.what() << std::endl << std::endl; }
	catch (...) { std::cout << "WTF: Unexpected Exception" << std::endl << std::endl; }
	if (!preprocess) return;

	bool lex = false;
	Mini_C::lexer::Lexer _lexer;
	try { _lexer.tokenize(file); lex = true; }
	catch (const Mini_C::MiniC_Base_Exception& e) { e.printException(); std::cout << std::endl; }
	catch (const std::exception& e) { std::cout << e.what() << std::endl << std::endl; }
	catch (...) { std::cout << "WTF: Unexpected Exception" << std::endl << std::endl; }
	if (!lex) return;

	_lexer.print(out);
	out << "\n-----------------------------------------\n" << std::endl;

	std::vector<std::pair<Mini_C::lexer::Token, std::string>> error_result =
		Mini_C::LR1::analyze(_lexer);
	if (error_result.size() > 0)
	{
		out << "\n----------------------------------------------------------------------------------\n";
		out << "\n------------------------------------  errors  ------------------------------------\n";
		out << "\n----------------------------------------------------------------------------------\n";
	}
	for (auto const&[token, str] : error_result)
	{
		out << "\n-----------------------------------------\n";
		Mini_C::TEST::outputToken(token, out);
		out << str << std::endl;
	}
	if (error_result.size() == 0) std::cout << "ok" << std::endl;

	out << "\n-----------------------------------------\n" << std::endl;

#undef out
#undef out

}


namespace Mini_C::parser
{
	void eval();
	void apply();
}

void rsy_parser_test()
{
	using namespace Mini_C::parser;




}


void test()
{
	rsy_lexer_test();
	// rsy_parser_test();
}


int main()
{
	::test();
	system("pause");
	return 0;
}
#endif // RSY_TEST
