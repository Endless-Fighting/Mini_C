//
//  test.hpp
//  Mini_C
//
//  Created by 潇湘夜雨 on 2018/9/23.
//  Copyright © 2018年 ssyram. All rights reserved.
//

#ifndef test_hpp
#define test_hpp

#include "../src/lexer.h"
#include <iostream>
using std::cout;
using std::endl;
using std::cin;
using namespace Mini_C::lexer;

namespace TEST
{

	template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
	template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;

	namespace TEST_LEXER
	{
		void test_lexer(const char* s, const std::size_t line_num);
		void num_print(const Mini_C::lexer::numeric_t& _num);
		void outputLexVector(const std::variant<std::vector<token_t>, analyzers::Token_Ex> &result,
			const std::size_t line_num);
	}

}
#endif /* test_hpp */
