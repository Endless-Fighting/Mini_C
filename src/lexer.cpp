//
//  lexer.cpp
//  Mini_C
//
//  Created by 潇湘夜雨 on 2018/9/21.
//  Copyright © 2018年 ssyram. All rights reserved.
//

#include "lexer.h"
#include <vector>
#include <functional>
#include <unordered_set>

// As lexical analyzer, I must assume that except for the appearance of some unknown character which is definitely wrong input, the input is all right.
// the mission of it is to divide them into the right sequence, give each of them the type that as fidelity as possible and the corresponding right value, if it has.

using namespace Mini_C::lexer;
using std::unordered_set;
using std::size_t;
using std::function;
using std::vector;
using std::string;
using std::variant;
using std::make_tuple;
using std::unordered_map;
using keyword_it = unordered_map<string, type>::const_iterator;

/**
 * analyzers
 */
namespace Mini_C::lexer::analyzers {
	//if returns true, means that it has tackled the word.
	//thus it doesn't need to worry about if the pos == size at the begginning of analyzers.
	using analyzer = function<bool(const char *, size_t&, const size_t, vector<token_t> &)>;

	namespace supporters {
		const static unordered_set<char> combindableOperatorSet = {
				'|', '&', '+', '^', '*', '/', '!', '.', ':', '=', '<', '>'
		};
		const static unordered_set<char> singleOperatorSet = {
				'(', ')', '{', '}', '[', ']', ';', ',', '?'
		};
		const static unordered_set<char> secondCombinableOperatorSet = {
				'+', '=', '|', '&', '.', ':', '<', '>'
		};
		const static unordered_set<char> dividerCharSet = {
				' ', '\n', '\t'
		};

		inline bool isDivider(char c) {
			return dividerCharSet.find(c) != dividerCharSet.end();
		}
		inline bool isWordBeginning(char c) {
			return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
		}
		inline bool isNum(char c) {
			return c >= '0' && c <= '9';
		}
		inline bool isNumBegin(char c) {
			return isNum(c) || c == '.';
		}
		inline bool canInWord(char c) {
			return isWordBeginning(c) && isNum(c);
		}
		inline bool isCombindableOperatorChar(char c) {
			return combindableOperatorSet.find(c) != combindableOperatorSet.end();
		}
		inline bool isSingleSymbolChar(char c) {
			return singleOperatorSet.find(c) != singleOperatorSet.end();
		}
		inline bool isSecondCombinableOpearatorChar(char c) {
			return secondCombinableOperatorSet.find(c) != secondCombinableOperatorSet.end();
		}
		template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
		template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;
		bool rIsInUnminusableSituation(vector<token_t> &r) {
			auto&& rb = r.back();
			return std::visit([&](auto&& r)->bool {
				using t = std::decay_t<decltype(r)>;
				if constexpr (std::is_same_v<t, identifier>)
					return true;
				else if constexpr (std::is_same_v<t, numeric_t>)
					return true;
				else if constexpr (std::is_same_v<t, type>)
					if (std::get<type>(rb) == keywords.find("]")->second || std::get<type>(rb) == keywords.find(")")->second)
						return true;
				return false;
			}, rb);
		}
		inline bool isHex(char c) {
			return isNum(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
		}
		inline int turnHex(char c) {
			if (isNum(c))
				return c - '0';
			else if (c >= 'a' && c <= 'f')
				return c - 'a' + 10;
			else
				return c - 'A' + 10;
		}

		// unnecessity
		// follow numbers
		const static unordered_set<char> followNumberCharSet = { '+', '-', '*', '/', ')', ']', '}', '|', '&', '^', ',', ':', ';' };
		inline bool canFollowNumber(char c) {
			return followNumberCharSet.find(c) != followNumberCharSet.end();
		}
	}

#define write_analyzer(funcname) bool funcname(const char *s, size_t& pos, const size_t size, vector<token_t> &r)

	write_analyzer(word_analyzer) {
		if (!supporters::isWordBeginning(s[pos]))
			return false;
		size_t length = 1;
		for (; supporters::canInWord(s[pos + length]); length++);


		string ns(s + pos, length);
		keyword_it it = keywords.find(ns);
		if (it != keywords.end())
			r.push_back(it->second);
		else
			r.push_back(std::move(ns));

		pos += length;

		return true;
	}

	// target:
	//      to get the longest string of operators
	// organization:
	//      1. get the longest string of operators
	//      2. loop to get the longest one that conforms to the rules
	write_analyzer(combindable_operator_analyzer) {
		if (!supporters::isCombindableOperatorChar(s[pos]))
			return false;

		size_t begin = pos;
		// for that the end of string is '\0' which is not combindable operator char, so there is no need to check the boundary

		// here uses a somehow ambiguous analyze process
		// for that it does not need a rigid DFA for every single combinable openning character
		while (supporters::isSecondCombinableOpearatorChar(s[pos++]));
		// loop to change
		keyword_it it;
		for (; (it = keywords.find(string(s + begin, pos - begin))) == keywords.end(); --pos);
		r.push_back(it->second);

		return true;
	}

	write_analyzer(single_symbol_analyzer) {
		if (!supporters::isSingleSymbolChar(s[pos]))
			return false;

		++pos;
		r.push_back(keywords.find(string(1, s[pos]))->second);
		return true;
	}

	// only get the number, but can be specified to accept the condition that tells whether it's minus
	bool inner_number_analyzer(const char *s, size_t &pos, const size_t size, vector<token_t> &r, const bool isMinus = false) {

		auto generateNumberException = [&]() {
			throw Token_Ex("not a valid number", pos);
		};

		auto hexAnalyzer = [&]() {
			pos += 2;
			if (isMinus || !supporters::isHex(s[pos]))
				generateNumberException();

			double value = supporters::turnHex(s[pos]);
			while (true)
				if (supporters::isHex(s[++pos]))
					value = value * 16 + supporters::turnHex(s[pos]);
				else if (s[pos] == '_')
					continue;
				else
					break;

			// unnecessity
			// check if it's followed by '.' to generate things.
//            while (supporters::isDivider(s[pos])) ++pos;
//            char c = s[pos];
//            if (c == '.')
//                generateNumberException();

			r.push_back(make_tuple(value, numeric_type::U32));
		};

		auto defaultAnalyzer = [&]() {
			double value = 0;
			numeric_type type = numeric_type::I32;

			auto decimalAnalyzer = [&]() {
				int decimal = 0;
				type = numeric_type::F32;

				auto f64Analyzer = [&]() {
					// here pos is where e/E appears + 1
					if (s[pos] != '-' && !supporters::isNum(s[pos]))
						generateNumberException();

					int right = 0;
					bool rminus = false;
					if (s[pos] == '-') {
						rminus = true;
						++pos;
					}
					type = numeric_type::F64;

					while (true)
						if (supporters::isNum(s[++pos]))
							right = right * 10 + (s[pos] - '0');
						else if (s[pos] == '_')
							continue;
						else
							break;

					r.push_back(make_tuple(value * pow(10, rminus ? -right : right), type));
				};

				//default is f32 analyzer
				while (true) {
					char c = s[pos++];
					if (supporters::isNum(c))
						value += (c - '0') * pow(10, decimal--);
					else if (c == '_')
						continue;
					else if (c == '.')
						if (!decimal)
							--decimal;
						else
							generateNumberException();
					else if (c == 'e' || c == 'E') {
						f64Analyzer();
						return;
					}
					else
						break;
				}

				r.push_back(make_tuple(value, type));
			};


			//default is i32 analyzer.
			while (true) {
				char c = s[pos++];
				if (supporters::isNum(c))
					value = value * 10 + (c - '0');
				else if (c == '_')
					continue;
				else if (c == '.' || c == 'e' || c == 'E') {
					decimalAnalyzer();
					return;
				}
				else if (c == 'u' || c == 'U') {
					if (isMinus)
						generateNumberException();
					type = numeric_type::U32;
					break;
				}
				else
					break;
			}

			r.push_back(make_tuple(value, type));
		};





		if (s[pos] == '0' && pos + 1 < size && s[pos + 1] == 'x')
			hexAnalyzer();
		else
			defaultAnalyzer();

		// unneccesity
		// see if the following char is allowed
		while (supporters::isDivider(s[pos])) ++pos;
		if (pos < size && !supporters::canFollowNumber(s[pos]))
			throw Token_Ex("not valid following content", pos);

		return true;
	}

	write_analyzer(number_analyzer) {
		if (!supporters::isNumBegin(s[pos]))
			return false;

		return inner_number_analyzer(s, pos, size, r);
	}

	write_analyzer(minus_analyzer) {
		if (s[pos] != '-')
			return false;

		size_t begin = pos;
		// to the next meaningful char
		while (++pos < size && supporters::isDivider(s[pos]));
		// see if it's minus
		if (supporters::isNumBegin(s[pos]) && !supporters::rIsInUnminusableSituation(r))
			return inner_number_analyzer(s, pos, size, r, true);
		else
			// there exists divider(s) or it's the end, or it could not be
			// a reasonable char
			if (begin != pos - 1 || pos == size ||
				(s[pos] != '-' && s[pos] != '='))
				r.push_back(keywords.find("-")->second);
			else //pos == begin + 1
				r.push_back(keywords.find(string(s + begin, 2))->second);

		return true;
	}
#undef write_analyzer
}
constexpr size_t analyzerNum = 5;
analyzers::analyzer analyzer[] = {
		analyzers::word_analyzer,
		analyzers::number_analyzer,
		analyzers::minus_analyzer,
		analyzers::single_symbol_analyzer,
		analyzers::combindable_operator_analyzer,
};

std::variant<std::vector<token_t>, analyzers::Token_Ex> tokenize(const char *s, const size_t size) {
	vector<token_t> r;
	bool ok;
	for (size_t pos = 0; pos < size; ) {
		ok = false;
		if (analyzers::supporters::isDivider(s[pos])) {
			++pos;
			continue;
		}
		for (size_t i = 0; i < analyzerNum; i++) {
			//if it returns true, means it've done, and there's no need to tackle this round again
			try {
				ok = analyzer[i](s, pos, size, r);
				if (ok)
					break;
			}
			catch (analyzers::Token_Ex& e) {
				return e;
			}
		}
		if (!ok)
			return analyzers::Token_Ex("not a recognizable character.", pos);
	}

	return r;
}
