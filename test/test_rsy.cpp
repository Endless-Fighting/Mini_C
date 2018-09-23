#ifdef RSY_TEST
#include <iostream>
#include <variant>
#include <vector>
#include <string>
#include <type_traits>
#include "../src/lexer.h"
#include "../src/miniC_exception.h"
#include <fstream>
#include <iomanip>

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;

void test_lexer(const char*, const std::size_t);
void test()
{
	constexpr std::size_t MAXSIZE = 128;
	char buffer[MAXSIZE];
	const char* filename = "./test/rsy1.txt";
	std::ifstream inputFile{ filename, std::ios::in };
	if (!inputFile.is_open()) {
		std::cout << "failed to open: " << std::quoted(filename) << std::endl;
	}
	else {
		try {
			std::size_t line_num = 0;
			while (!inputFile.eof())
			{
				inputFile.getline(buffer, MAXSIZE - 1);
				line_num++;
				test_lexer(buffer, line_num);
				std::cout << std::endl << std::endl;
			}
			inputFile.close();
		}
		catch (const Mini_C::MiniC_Base_Exception& e)
		{
			e.printException();
		}
		catch (...)
		{
			std::cout << "WTF: Unexpected Exception" << std::endl;
		}
	}
}


void test_lexer(const char* s, const std::size_t line_num)
{
	auto result = Mini_C::lexer::tokenize(s, strlen(s));
	std::visit(overloaded{
		[line_num](const Mini_C::lexer::analyzers::Token_Ex& e) {
		throw Mini_C::MiniC_Universal_Exception{
			std::move(const_cast<Mini_C::lexer::analyzers::Token_Ex&>(e)._msg),
			line_num, e._position }; },
			[](const std::vector<Mini_C::lexer::token_t>& tokens) {
			for (auto const& token : tokens)
				std::visit(overloaded{
				[](const Mini_C::lexer::type& _type) { std::cout << "type: " << Mini_C::lexer::type2str(_type) << std::endl; },
				[](const Mini_C::lexer::identifier& _identifier) { std::cout << "identifier: " << _identifier << std::endl; },
				[](const Mini_C::lexer::numeric_t& _num) { std::cout << "numeric: " << std::quoted(Mini_C::lexer::type2str(num_t2type(std::get<Mini_C::lexer::numeric_type>(_num)))) << " " << std::get<double>(_num) << std::endl; },
				[](const Mini_C::lexer::string_literal_t& _str) { std::cout << "string literal: " << std::quoted(std::get<const std::string>(_str)) << std::endl; },
				[](auto) { std::cout << "WTF: tokenizer" << std::endl; },
					}, token); },
		}, result);
}

int main()
{
	test();
	std::getchar();
	return 0;
}
#endif // RSY_TEST
