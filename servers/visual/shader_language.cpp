/*************************************************************************/
/*  shader_language.cpp                                                  */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "shader_language.h"
#include "core/os/os.h"
#include "core/print_string.h"
static bool _is_text_char(CharType c) {

	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

static bool _is_number(CharType c) {

	return (c >= '0' && c <= '9');
}

static bool _is_hex(CharType c) {

	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

String ShaderLanguage::get_operator_text(Operator p_op) {

	static const char *op_names[OP_MAX] = { "==",
		"!=",
		"<",
		"<=",
		">",
		">=",
		"&&",
		"||",
		"!",
		"-",
		"+",
		"-",
		"*",
		"/",
		"%",
		"<<",
		">>",
		"=",
		"+=",
		"-=",
		"*=",
		"/=",
		"%=",
		"<<=",
		">>=",
		"&=",
		"|=",
		"^=",
		"&",
		"|",
		"^",
		"~",
		"++",
		"--",
		"?",
		":",
		"++",
		"--",
		"()",
		"construct",
		"index" };

	return op_names[p_op];
}

const char *ShaderLanguage::token_names[TK_MAX] = {
	"EMPTY",
	"IDENTIFIER",
	"TRUE",
	"FALSE",
	"REAL_CONSTANT",
	"INT_CONSTANT",
	"TYPE_VOID",
	"TYPE_BOOL",
	"TYPE_BVEC2",
	"TYPE_BVEC3",
	"TYPE_BVEC4",
	"TYPE_INT",
	"TYPE_IVEC2",
	"TYPE_IVEC3",
	"TYPE_IVEC4",
	"TYPE_UINT",
	"TYPE_UVEC2",
	"TYPE_UVEC3",
	"TYPE_UVEC4",
	"TYPE_FLOAT",
	"TYPE_VEC2",
	"TYPE_VEC3",
	"TYPE_VEC4",
	"TYPE_MAT2",
	"TYPE_MAT3",
	"TYPE_MAT4",
	"TYPE_SAMPLER2D",
	"TYPE_ISAMPLER2D",
	"TYPE_USAMPLER2D",
	"TYPE_SAMPLER2DARRAY",
	"TYPE_ISAMPLER2DARRAY",
	"TYPE_USAMPLER2DARRAY",
	"TYPE_SAMPLER3D",
	"TYPE_ISAMPLER3D",
	"TYPE_USAMPLER3D",
	"TYPE_SAMPLERCUBE",
	"INTERPOLATION_FLAT",
	"INTERPOLATION_SMOOTH",
	"CONST",
	"PRECISION_LOW",
	"PRECISION_MID",
	"PRECISION_HIGH",
	"OP_EQUAL",
	"OP_NOT_EQUAL",
	"OP_LESS",
	"OP_LESS_EQUAL",
	"OP_GREATER",
	"OP_GREATER_EQUAL",
	"OP_AND",
	"OP_OR",
	"OP_NOT",
	"OP_ADD",
	"OP_SUB",
	"OP_MUL",
	"OP_DIV",
	"OP_MOD",
	"OP_SHIFT_LEFT",
	"OP_SHIFT_RIGHT",
	"OP_ASSIGN",
	"OP_ASSIGN_ADD",
	"OP_ASSIGN_SUB",
	"OP_ASSIGN_MUL",
	"OP_ASSIGN_DIV",
	"OP_ASSIGN_MOD",
	"OP_ASSIGN_SHIFT_LEFT",
	"OP_ASSIGN_SHIFT_RIGHT",
	"OP_ASSIGN_BIT_AND",
	"OP_ASSIGN_BIT_OR",
	"OP_ASSIGN_BIT_XOR",
	"OP_BIT_AND",
	"OP_BIT_OR",
	"OP_BIT_XOR",
	"OP_BIT_INVERT",
	"OP_INCREMENT",
	"OP_DECREMENT",
	"CF_IF",
	"CF_ELSE",
	"CF_FOR",
	"CF_WHILE",
	"CF_DO",
	"CF_SWITCH",
	"CF_CASE",
	"CF_BREAK",
	"CF_CONTINUE",
	"CF_RETURN",
	"CF_DISCARD",
	"BRACKET_OPEN",
	"BRACKET_CLOSE",
	"CURLY_BRACKET_OPEN",
	"CURLY_BRACKET_CLOSE",
	"PARENTHESIS_OPEN",
	"PARENTHESIS_CLOSE",
	"QUESTION",
	"COMMA",
	"COLON",
	"SEMICOLON",
	"PERIOD",
	"UNIFORM",
	"VARYING",
	"IN",
	"OUT",
	"INOUT",
	"RENDER_MODE",
	"HINT_WHITE_TEXTURE",
	"HINT_BLACK_TEXTURE",
	"HINT_NORMAL_TEXTURE",
	"HINT_ANISO_TEXTURE",
	"HINT_ALBEDO_TEXTURE",
	"HINT_BLACK_ALBEDO_TEXTURE",
	"HINT_COLOR",
	"HINT_RANGE",
	"SHADER_TYPE",
	"CURSOR",
	"ERROR",
	"EOF",
};

String ShaderLanguage::get_token_text(Token p_token) {

	String name = token_names[p_token.type];
	if (p_token.type == TK_INT_CONSTANT || p_token.type == TK_REAL_CONSTANT) {
		name += "(" + rtos(p_token.constant) + ")";
	} else if (p_token.type == TK_IDENTIFIER) {
		name += "(" + String(p_token.text) + ")";
	} else if (p_token.type == TK_ERROR) {
		name += "(" + String(p_token.text) + ")";
	}

	return name;
}

ShaderLanguage::Token ShaderLanguage::_make_token(TokenType p_type, const StringName &p_text) {

	Token tk;
	tk.type = p_type;
	tk.text = p_text;
	tk.line = tk_line;
	if (tk.type == TK_ERROR) {
		_set_error(p_text);
	}
	return tk;
}

const ShaderLanguage::KeyWord ShaderLanguage::keyword_list[] = {
	{ TK_TRUE, "true" },
	{ TK_FALSE, "false" },
	{ TK_TYPE_VOID, "void" },
	{ TK_TYPE_BOOL, "bool" },
	{ TK_TYPE_BVEC2, "bvec2" },
	{ TK_TYPE_BVEC3, "bvec3" },
	{ TK_TYPE_BVEC4, "bvec4" },
	{ TK_TYPE_INT, "int" },
	{ TK_TYPE_IVEC2, "ivec2" },
	{ TK_TYPE_IVEC3, "ivec3" },
	{ TK_TYPE_IVEC4, "ivec4" },
	{ TK_TYPE_UINT, "uint" },
	{ TK_TYPE_UVEC2, "uvec2" },
	{ TK_TYPE_UVEC3, "uvec3" },
	{ TK_TYPE_UVEC4, "uvec4" },
	{ TK_TYPE_FLOAT, "float" },
	{ TK_TYPE_VEC2, "vec2" },
	{ TK_TYPE_VEC3, "vec3" },
	{ TK_TYPE_VEC4, "vec4" },
	{ TK_TYPE_MAT2, "mat2" },
	{ TK_TYPE_MAT3, "mat3" },
	{ TK_TYPE_MAT4, "mat4" },
	{ TK_TYPE_SAMPLER2D, "sampler2D" },
	{ TK_TYPE_ISAMPLER2D, "isampler2D" },
	{ TK_TYPE_USAMPLER2D, "usampler2D" },
	{ TK_TYPE_SAMPLER2DARRAY, "sampler2DArray" },
	{ TK_TYPE_ISAMPLER2DARRAY, "isampler2DArray" },
	{ TK_TYPE_USAMPLER2DARRAY, "usampler2DArray" },
	{ TK_TYPE_SAMPLER3D, "sampler3D" },
	{ TK_TYPE_ISAMPLER3D, "isampler3D" },
	{ TK_TYPE_USAMPLER3D, "usampler3D" },
	{ TK_TYPE_SAMPLERCUBE, "samplerCube" },
	{ TK_INTERPOLATION_FLAT, "flat" },
	{ TK_INTERPOLATION_SMOOTH, "smooth" },
	{ TK_CONST, "const" },
	{ TK_PRECISION_LOW, "lowp" },
	{ TK_PRECISION_MID, "mediump" },
	{ TK_PRECISION_HIGH, "highp" },
	{ TK_CF_IF, "if" },
	{ TK_CF_ELSE, "else" },
	{ TK_CF_FOR, "for" },
	{ TK_CF_WHILE, "while" },
	{ TK_CF_DO, "do" },
	{ TK_CF_SWITCH, "switch" },
	{ TK_CF_CASE, "case" },
	{ TK_CF_BREAK, "break" },
	{ TK_CF_CONTINUE, "continue" },
	{ TK_CF_RETURN, "return" },
	{ TK_CF_DISCARD, "discard" },
	{ TK_UNIFORM, "uniform" },
	{ TK_VARYING, "varying" },
	{ TK_ARG_IN, "in" },
	{ TK_ARG_OUT, "out" },
	{ TK_ARG_INOUT, "inout" },
	{ TK_RENDER_MODE, "render_mode" },
	{ TK_HINT_WHITE_TEXTURE, "hint_white" },
	{ TK_HINT_BLACK_TEXTURE, "hint_black" },
	{ TK_HINT_NORMAL_TEXTURE, "hint_normal" },
	{ TK_HINT_ANISO_TEXTURE, "hint_aniso" },
	{ TK_HINT_ALBEDO_TEXTURE, "hint_albedo" },
	{ TK_HINT_BLACK_ALBEDO_TEXTURE, "hint_black_albedo" },
	{ TK_HINT_COLOR, "hint_color" },
	{ TK_HINT_RANGE, "hint_range" },
	{ TK_SHADER_TYPE, "shader_type" },

	{ TK_ERROR, NULL }
};

ShaderLanguage::Token ShaderLanguage::_get_token() {

#define GETCHAR(m_idx) (((char_idx + m_idx) < code.length()) ? code[char_idx + m_idx] : CharType(0))

	while (true) {
		char_idx++;
		switch (GETCHAR(-1)) {

			case 0:
				return _make_token(TK_EOF);
			case 0xFFFF:
				return _make_token(TK_CURSOR); //for completion
			case '\t':
			case '\r':
			case ' ':
				continue;
			case '\n':
				tk_line++;
				continue;
			case '/': {

				switch (GETCHAR(0)) {
					case '*': { // block comment

						char_idx++;
						while (true) {
							if (GETCHAR(0) == 0) {
								return _make_token(TK_EOF);
							}
							if (GETCHAR(0) == '*' && GETCHAR(1) == '/') {
								char_idx += 2;
								break;
							} else if (GETCHAR(0) == '\n') {
								tk_line++;
							}

							char_idx++;
						}

					} break;
					case '/': { // line comment skip

						while (true) {
							if (GETCHAR(0) == '\n') {
								tk_line++;
								char_idx++;
								break;
							}
							if (GETCHAR(0) == 0) {
								return _make_token(TK_EOF);
							}
							char_idx++;
						}

					} break;
					case '=': { // diveq

						char_idx++;
						return _make_token(TK_OP_ASSIGN_DIV);

					} break;
					default:
						return _make_token(TK_OP_DIV);
				}

				continue; //a comment, continue to next token
			} break;
			case '=': {

				if (GETCHAR(0) == '=') {
					char_idx++;
					return _make_token(TK_OP_EQUAL);
				}

				return _make_token(TK_OP_ASSIGN);

			} break;
			case '<': {
				if (GETCHAR(0) == '=') {
					char_idx++;
					return _make_token(TK_OP_LESS_EQUAL);
				} else if (GETCHAR(0) == '<') {
					char_idx++;
					if (GETCHAR(0) == '=') {
						char_idx++;
						return _make_token(TK_OP_ASSIGN_SHIFT_LEFT);
					}

					return _make_token(TK_OP_SHIFT_LEFT);
				}

				return _make_token(TK_OP_LESS);

			} break;
			case '>': {
				if (GETCHAR(0) == '=') {
					char_idx++;
					return _make_token(TK_OP_GREATER_EQUAL);
				} else if (GETCHAR(0) == '>') {
					char_idx++;
					if (GETCHAR(0) == '=') {
						char_idx++;
						return _make_token(TK_OP_ASSIGN_SHIFT_RIGHT);
					}

					return _make_token(TK_OP_SHIFT_RIGHT);
				}

				return _make_token(TK_OP_GREATER);

			} break;
			case '!': {
				if (GETCHAR(0) == '=') {
					char_idx++;
					return _make_token(TK_OP_NOT_EQUAL);
				}

				return _make_token(TK_OP_NOT);

			} break;
			//case '"' //string - no strings in shader
			//case '\'' //string - no strings in shader
			case '{':
				return _make_token(TK_CURLY_BRACKET_OPEN);
			case '}':
				return _make_token(TK_CURLY_BRACKET_CLOSE);
			case '[':
				return _make_token(TK_BRACKET_OPEN);
			case ']':
				return _make_token(TK_BRACKET_CLOSE);
			case '(':
				return _make_token(TK_PARENTHESIS_OPEN);
			case ')':
				return _make_token(TK_PARENTHESIS_CLOSE);
			case ',':
				return _make_token(TK_COMMA);
			case ';':
				return _make_token(TK_SEMICOLON);
			case '?':
				return _make_token(TK_QUESTION);
			case ':':
				return _make_token(TK_COLON);
			case '^':
				return _make_token(TK_OP_BIT_XOR);
			case '~':
				return _make_token(TK_OP_BIT_INVERT);
			case '&': {
				if (GETCHAR(0) == '=') {
					char_idx++;
					return _make_token(TK_OP_ASSIGN_BIT_AND);
				} else if (GETCHAR(0) == '&') {
					char_idx++;
					return _make_token(TK_OP_AND);
				}
				return _make_token(TK_OP_BIT_AND);
			} break;
			case '|': {

				if (GETCHAR(0) == '=') {
					char_idx++;
					return _make_token(TK_OP_ASSIGN_BIT_OR);
				} else if (GETCHAR(0) == '|') {
					char_idx++;
					return _make_token(TK_OP_OR);
				}
				return _make_token(TK_OP_BIT_OR);

			} break;
			case '*': {

				if (GETCHAR(0) == '=') {
					char_idx++;
					return _make_token(TK_OP_ASSIGN_MUL);
				}
				return _make_token(TK_OP_MUL);
			} break;
			case '+': {

				if (GETCHAR(0) == '=') {
					char_idx++;
					return _make_token(TK_OP_ASSIGN_ADD);
				} else if (GETCHAR(0) == '+') {

					char_idx++;
					return _make_token(TK_OP_INCREMENT);
				}

				return _make_token(TK_OP_ADD);
			} break;
			case '-': {

				if (GETCHAR(0) == '=') {
					char_idx++;
					return _make_token(TK_OP_ASSIGN_SUB);
				} else if (GETCHAR(0) == '-') {

					char_idx++;
					return _make_token(TK_OP_DECREMENT);
				}

				return _make_token(TK_OP_SUB);
			} break;
			case '%': {

				if (GETCHAR(0) == '=') {
					char_idx++;
					return _make_token(TK_OP_ASSIGN_MOD);
				}

				return _make_token(TK_OP_MOD);
			} break;
			default: {

				char_idx--; //go back one, since we have no idea what this is

				if (_is_number(GETCHAR(0)) || (GETCHAR(0) == '.' && _is_number(GETCHAR(1)))) {
					// parse number
					bool period_found = false;
					bool exponent_found = false;
					bool hexa_found = false;
					bool sign_found = false;
					bool float_suffix_found = false;

					String str;
					int i = 0;

					while (true) {
						if (GETCHAR(i) == '.') {
							if (period_found || exponent_found || hexa_found || float_suffix_found)
								return _make_token(TK_ERROR, "Invalid numeric constant");
							period_found = true;
						} else if (GETCHAR(i) == 'x') {
							if (hexa_found || str.length() != 1 || str[0] != '0')
								return _make_token(TK_ERROR, "Invalid numeric constant");
							hexa_found = true;
						} else if (GETCHAR(i) == 'e') {
							if (hexa_found || exponent_found || float_suffix_found)
								return _make_token(TK_ERROR, "Invalid numeric constant");
							exponent_found = true;
						} else if (GETCHAR(i) == 'f') {
							if (hexa_found || exponent_found)
								return _make_token(TK_ERROR, "Invalid numeric constant");
							float_suffix_found = true;
						} else if (_is_number(GETCHAR(i))) {
							if (float_suffix_found)
								return _make_token(TK_ERROR, "Invalid numeric constant");
						} else if (hexa_found && _is_hex(GETCHAR(i))) {

						} else if ((GETCHAR(i) == '-' || GETCHAR(i) == '+') && exponent_found) {
							if (sign_found)
								return _make_token(TK_ERROR, "Invalid numeric constant");
							sign_found = true;
						} else
							break;

						str += CharType(GETCHAR(i));
						i++;
					}

					CharType last_char = str[str.length() - 1];

					if (hexa_found) {
						//hex integers eg."0xFF" or "0x12AB", etc - NOT supported yet
						return _make_token(TK_ERROR, "Invalid (hexadecimal) numeric constant - Not supported");
					} else if (period_found || exponent_found || float_suffix_found) {
						//floats
						if (period_found) {
							if (float_suffix_found) {
								//checks for eg "1.f" or "1.99f" notations
								if (last_char != 'f') {
									return _make_token(TK_ERROR, "Invalid (float) numeric constant");
								}
							} else {
								//checks for eg. "1." or "1.99" notations
								if (last_char != '.' && !_is_number(last_char)) {
									return _make_token(TK_ERROR, "Invalid (float) numeric constant");
								}
							}
						} else if (float_suffix_found) {
							// if no period found the float suffix must be the last character, like in "2f" for "2.0"
							if (last_char != 'f') {
								return _make_token(TK_ERROR, "Invalid (float) numeric constant");
							}
						}

						if (float_suffix_found) {
							//strip the suffix
							str = str.left(str.length() - 1);
							//compensate reading cursor position
							char_idx += 1;
						}

						if (!str.is_valid_float()) {
							return _make_token(TK_ERROR, "Invalid (float) numeric constant");
						}
					} else {
						//integers
						if (!_is_number(last_char)) {
							return _make_token(TK_ERROR, "Invalid (integer) numeric constant");
						}
						if (!str.is_valid_integer()) {
							return _make_token(TK_ERROR, "Invalid numeric constant");
						}
					}

					char_idx += str.length();
					Token tk;
					if (period_found || exponent_found || float_suffix_found)
						tk.type = TK_REAL_CONSTANT;
					else
						tk.type = TK_INT_CONSTANT;

					tk.constant = str.to_double(); //won't work with hex
					tk.line = tk_line;

					return tk;
				}

				if (GETCHAR(0) == '.') {
					//parse period
					char_idx++;
					return _make_token(TK_PERIOD);
				}

				if (_is_text_char(GETCHAR(0))) {
					// parse identifier
					String str;

					while (_is_text_char(GETCHAR(0))) {

						str += CharType(GETCHAR(0));
						char_idx++;
					}

					//see if keyword
					//should be converted to a static map
					int idx = 0;

					while (keyword_list[idx].text) {

						if (str == keyword_list[idx].text) {

							return _make_token(keyword_list[idx].token);
						}
						idx++;
					}

					return _make_token(TK_IDENTIFIER, str);
				}

				if (GETCHAR(0) > 32)
					return _make_token(TK_ERROR, "Tokenizer: Unknown character #" + itos(GETCHAR(0)) + ": '" + String::chr(GETCHAR(0)) + "'");
				else
					return _make_token(TK_ERROR, "Tokenizer: Unknown character #" + itos(GETCHAR(0)));

			} break;
		}
	}
	ERR_PRINT("BUG");
	return Token();

#undef GETCHAR
}

String ShaderLanguage::token_debug(const String &p_code) {

	clear();

	code = p_code;

	String output;

	Token tk = _get_token();
	while (tk.type != TK_EOF && tk.type != TK_ERROR) {

		output += itos(tk_line) + ": " + get_token_text(tk) + "\n";
		tk = _get_token();
	}

	return output;
}

bool ShaderLanguage::is_token_variable_datatype(TokenType p_type) {
	return (
			p_type == TK_TYPE_VOID ||
			p_type == TK_TYPE_BOOL ||
			p_type == TK_TYPE_BVEC2 ||
			p_type == TK_TYPE_BVEC3 ||
			p_type == TK_TYPE_BVEC4 ||
			p_type == TK_TYPE_INT ||
			p_type == TK_TYPE_IVEC2 ||
			p_type == TK_TYPE_IVEC3 ||
			p_type == TK_TYPE_IVEC4 ||
			p_type == TK_TYPE_UINT ||
			p_type == TK_TYPE_UVEC2 ||
			p_type == TK_TYPE_UVEC3 ||
			p_type == TK_TYPE_UVEC4 ||
			p_type == TK_TYPE_FLOAT ||
			p_type == TK_TYPE_VEC2 ||
			p_type == TK_TYPE_VEC3 ||
			p_type == TK_TYPE_VEC4 ||
			p_type == TK_TYPE_MAT2 ||
			p_type == TK_TYPE_MAT3 ||
			p_type == TK_TYPE_MAT4);
}

bool ShaderLanguage::is_token_datatype(TokenType p_type) {

	return (
			p_type == TK_TYPE_VOID ||
			p_type == TK_TYPE_BOOL ||
			p_type == TK_TYPE_BVEC2 ||
			p_type == TK_TYPE_BVEC3 ||
			p_type == TK_TYPE_BVEC4 ||
			p_type == TK_TYPE_INT ||
			p_type == TK_TYPE_IVEC2 ||
			p_type == TK_TYPE_IVEC3 ||
			p_type == TK_TYPE_IVEC4 ||
			p_type == TK_TYPE_UINT ||
			p_type == TK_TYPE_UVEC2 ||
			p_type == TK_TYPE_UVEC3 ||
			p_type == TK_TYPE_UVEC4 ||
			p_type == TK_TYPE_FLOAT ||
			p_type == TK_TYPE_VEC2 ||
			p_type == TK_TYPE_VEC3 ||
			p_type == TK_TYPE_VEC4 ||
			p_type == TK_TYPE_MAT2 ||
			p_type == TK_TYPE_MAT3 ||
			p_type == TK_TYPE_MAT4 ||
			p_type == TK_TYPE_SAMPLER2D ||
			p_type == TK_TYPE_ISAMPLER2D ||
			p_type == TK_TYPE_USAMPLER2D ||
			p_type == TK_TYPE_SAMPLER2DARRAY ||
			p_type == TK_TYPE_ISAMPLER2DARRAY ||
			p_type == TK_TYPE_USAMPLER2DARRAY ||
			p_type == TK_TYPE_SAMPLER3D ||
			p_type == TK_TYPE_ISAMPLER3D ||
			p_type == TK_TYPE_USAMPLER3D ||
			p_type == TK_TYPE_SAMPLERCUBE);
}

ShaderLanguage::DataType ShaderLanguage::get_token_datatype(TokenType p_type) {

	return DataType(p_type - TK_TYPE_VOID);
}

bool ShaderLanguage::is_token_interpolation(TokenType p_type) {

	return (
			p_type == TK_INTERPOLATION_FLAT ||
			p_type == TK_INTERPOLATION_SMOOTH);
}

ShaderLanguage::DataInterpolation ShaderLanguage::get_token_interpolation(TokenType p_type) {

	if (p_type == TK_INTERPOLATION_FLAT)
		return INTERPOLATION_FLAT;
	else
		return INTERPOLATION_SMOOTH;
}

bool ShaderLanguage::is_token_precision(TokenType p_type) {

	return (
			p_type == TK_PRECISION_LOW ||
			p_type == TK_PRECISION_MID ||
			p_type == TK_PRECISION_HIGH);
}

ShaderLanguage::DataPrecision ShaderLanguage::get_token_precision(TokenType p_type) {

	if (p_type == TK_PRECISION_LOW)
		return PRECISION_LOWP;
	else if (p_type == TK_PRECISION_HIGH)
		return PRECISION_HIGHP;
	else
		return PRECISION_MEDIUMP;
}

String ShaderLanguage::get_precision_name(DataPrecision p_type) {
	switch (p_type) {
		case PRECISION_LOWP: return "lowp";
		case PRECISION_MEDIUMP: return "mediump";
		case PRECISION_HIGHP: return "highp";
		default:
			break;
	}
	return "";
}

String ShaderLanguage::get_datatype_name(DataType p_type) {

	switch (p_type) {

		case TYPE_VOID: return "void";
		case TYPE_BOOL: return "bool";
		case TYPE_BVEC2: return "bvec2";
		case TYPE_BVEC3: return "bvec3";
		case TYPE_BVEC4: return "bvec4";
		case TYPE_INT: return "int";
		case TYPE_IVEC2: return "ivec2";
		case TYPE_IVEC3: return "ivec3";
		case TYPE_IVEC4: return "ivec4";
		case TYPE_UINT: return "uint";
		case TYPE_UVEC2: return "uvec2";
		case TYPE_UVEC3: return "uvec3";
		case TYPE_UVEC4: return "uvec4";
		case TYPE_FLOAT: return "float";
		case TYPE_VEC2: return "vec2";
		case TYPE_VEC3: return "vec3";
		case TYPE_VEC4: return "vec4";
		case TYPE_MAT2: return "mat2";
		case TYPE_MAT3: return "mat3";
		case TYPE_MAT4: return "mat4";
		case TYPE_SAMPLER2D: return "sampler2D";
		case TYPE_ISAMPLER2D: return "isampler2D";
		case TYPE_USAMPLER2D: return "usampler2D";
		case TYPE_SAMPLER2DARRAY: return "sampler2DArray";
		case TYPE_ISAMPLER2DARRAY: return "isampler2DArray";
		case TYPE_USAMPLER2DARRAY: return "usampler2DArray";
		case TYPE_SAMPLER3D: return "sampler3D";
		case TYPE_ISAMPLER3D: return "isampler3D";
		case TYPE_USAMPLER3D: return "usampler3D";
		case TYPE_SAMPLERCUBE: return "samplerCube";
	}

	return "";
}

bool ShaderLanguage::is_token_nonvoid_datatype(TokenType p_type) {

	return is_token_datatype(p_type) && p_type != TK_TYPE_VOID;
}

void ShaderLanguage::clear() {

	current_function = StringName();

	completion_type = COMPLETION_NONE;
	completion_block = NULL;
	completion_function = StringName();

	error_line = 0;
	tk_line = 1;
	char_idx = 0;
	error_set = false;
	error_str = "";
	while (nodes) {
		Node *n = nodes;
		nodes = nodes->next;
		memdelete(n);
	}
}

bool ShaderLanguage::_find_identifier(const BlockNode *p_block, const Map<StringName, BuiltInInfo> &p_builtin_types, const StringName &p_identifier, DataType *r_data_type, IdentifierType *r_type, int *r_array_size) {

	if (p_builtin_types.has(p_identifier)) {

		if (r_data_type) {
			*r_data_type = p_builtin_types[p_identifier].type;
		}
		if (r_type) {
			*r_type = IDENTIFIER_BUILTIN_VAR;
		}

		return true;
	}

	FunctionNode *function = NULL;

	while (p_block) {

		if (p_block->variables.has(p_identifier)) {
			if (r_data_type) {
				*r_data_type = p_block->variables[p_identifier].type;
			}
			if (r_array_size) {
				*r_array_size = p_block->variables[p_identifier].array_size;
			}
			if (r_type) {
				*r_type = IDENTIFIER_LOCAL_VAR;
			}

			return true;
		}

		if (p_block->parent_function) {
			function = p_block->parent_function;
			break;
		} else {
			ERR_FAIL_COND_V(!p_block->parent_block, false);
			p_block = p_block->parent_block;
		}
	}

	if (function) {
		for (int i = 0; i < function->arguments.size(); i++) {
			if (function->arguments[i].name == p_identifier) {
				if (r_data_type) {
					*r_data_type = function->arguments[i].type;
				}
				if (r_type) {
					*r_type = IDENTIFIER_FUNCTION_ARGUMENT;
				}

				return true;
			}
		}
	}

	if (shader->varyings.has(p_identifier)) {
		if (r_data_type) {
			*r_data_type = shader->varyings[p_identifier].type;
		}
		if (r_type) {
			*r_type = IDENTIFIER_VARYING;
		}
		return true;
	}

	if (shader->uniforms.has(p_identifier)) {
		if (r_data_type) {
			*r_data_type = shader->uniforms[p_identifier].type;
		}
		if (r_type) {
			*r_type = IDENTIFIER_UNIFORM;
		}
		return true;
	}

	if (shader->constants.has(p_identifier)) {
		if (r_data_type) {
			*r_data_type = shader->constants[p_identifier].type;
		}
		if (r_type) {
			*r_type = IDENTIFIER_CONSTANT;
		}
		return true;
	}

	for (int i = 0; i < shader->functions.size(); i++) {

		if (!shader->functions[i].callable)
			continue;

		if (shader->functions[i].name == p_identifier) {
			if (r_data_type) {
				*r_data_type = shader->functions[i].function->return_type;
			}
			if (r_type) {
				*r_type = IDENTIFIER_FUNCTION;
			}
		}
	}

	return false;
}

bool ShaderLanguage::_validate_operator(OperatorNode *p_op, DataType *r_ret_type) {

	bool valid = false;
	DataType ret_type;

	switch (p_op->op) {
		case OP_EQUAL:
		case OP_NOT_EQUAL: {
			DataType na = p_op->arguments[0]->get_datatype();
			DataType nb = p_op->arguments[1]->get_datatype();
			valid = na == nb;
			ret_type = TYPE_BOOL;
		} break;
		case OP_LESS:
		case OP_LESS_EQUAL:
		case OP_GREATER:
		case OP_GREATER_EQUAL: {
			DataType na = p_op->arguments[0]->get_datatype();
			DataType nb = p_op->arguments[1]->get_datatype();

			valid = na == nb && (na == TYPE_UINT || na == TYPE_INT || na == TYPE_FLOAT);
			ret_type = TYPE_BOOL;

		} break;
		case OP_AND:
		case OP_OR: {
			DataType na = p_op->arguments[0]->get_datatype();
			DataType nb = p_op->arguments[1]->get_datatype();

			valid = na == nb && na == TYPE_BOOL;
			ret_type = TYPE_BOOL;

		} break;
		case OP_NOT: {

			DataType na = p_op->arguments[0]->get_datatype();
			valid = na == TYPE_BOOL;
			ret_type = TYPE_BOOL;

		} break;
		case OP_INCREMENT:
		case OP_DECREMENT:
		case OP_POST_INCREMENT:
		case OP_POST_DECREMENT:
		case OP_NEGATE: {
			DataType na = p_op->arguments[0]->get_datatype();
			valid = na > TYPE_BOOL && na < TYPE_MAT2;
			ret_type = na;
		} break;
		case OP_ADD:
		case OP_SUB:
		case OP_MUL:
		case OP_DIV: {
			DataType na = p_op->arguments[0]->get_datatype();
			DataType nb = p_op->arguments[1]->get_datatype();

			if (na > nb) {
				//make things easier;
				SWAP(na, nb);
			}

			if (na == nb) {
				valid = (na > TYPE_BOOL && na <= TYPE_MAT4);
				ret_type = na;
			} else if (na == TYPE_INT && nb == TYPE_IVEC2) {
				valid = true;
				ret_type = TYPE_IVEC2;
			} else if (na == TYPE_INT && nb == TYPE_IVEC3) {
				valid = true;
				ret_type = TYPE_IVEC3;
			} else if (na == TYPE_INT && nb == TYPE_IVEC4) {
				valid = true;
				ret_type = TYPE_IVEC4;
			} else if (na == TYPE_UINT && nb == TYPE_UVEC2) {
				valid = true;
				ret_type = TYPE_UVEC2;
			} else if (na == TYPE_UINT && nb == TYPE_UVEC3) {
				valid = true;
				ret_type = TYPE_UVEC3;
			} else if (na == TYPE_UINT && nb == TYPE_UVEC4) {
				valid = true;
				ret_type = TYPE_UVEC4;
			} else if (na == TYPE_FLOAT && nb == TYPE_VEC2) {
				valid = true;
				ret_type = TYPE_VEC2;
			} else if (na == TYPE_FLOAT && nb == TYPE_VEC3) {
				valid = true;
				ret_type = TYPE_VEC3;
			} else if (na == TYPE_FLOAT && nb == TYPE_VEC4) {
				valid = true;
				ret_type = TYPE_VEC4;
			} else if (p_op->op == OP_MUL && na == TYPE_FLOAT && nb == TYPE_MAT2) {
				valid = true;
				ret_type = TYPE_MAT2;
			} else if (p_op->op == OP_MUL && na == TYPE_FLOAT && nb == TYPE_MAT3) {
				valid = true;
				ret_type = TYPE_MAT3;
			} else if (p_op->op == OP_MUL && na == TYPE_FLOAT && nb == TYPE_MAT4) {
				valid = true;
				ret_type = TYPE_MAT4;
			} else if (p_op->op == OP_MUL && na == TYPE_VEC2 && nb == TYPE_MAT2) {
				valid = true;
				ret_type = TYPE_VEC2;
			} else if (p_op->op == OP_MUL && na == TYPE_VEC3 && nb == TYPE_MAT3) {
				valid = true;
				ret_type = TYPE_VEC3;
			} else if (p_op->op == OP_MUL && na == TYPE_VEC4 && nb == TYPE_MAT4) {
				valid = true;
				ret_type = TYPE_VEC4;
			}
		} break;
		case OP_ASSIGN_MOD:
		case OP_MOD: {
			/*
			 * The operator modulus (%) operates on signed or unsigned integers or integer vectors. The operand
			 * types must both be signed or both be unsigned. The operands cannot be vectors of differing size. If
			 * one operand is a scalar and the other vector, then the scalar is applied component-wise to the vector,
			 * resulting in the same type as the vector. If both are vectors of the same size, the result is computed
			 * component-wise.
			 */

			DataType na = p_op->arguments[0]->get_datatype();
			DataType nb = p_op->arguments[1]->get_datatype();

			if (na == TYPE_INT && nb == TYPE_INT) {
				valid = true;
				ret_type = TYPE_INT;
			} else if (na == TYPE_IVEC2 && nb == TYPE_INT) {
				valid = true;
				ret_type = TYPE_IVEC2;
			} else if (na == TYPE_IVEC3 && nb == TYPE_INT) {
				valid = true;
				ret_type = TYPE_IVEC3;
			} else if (na == TYPE_IVEC4 && nb == TYPE_INT) {
				valid = true;
				ret_type = TYPE_IVEC4;
			} else if (na == TYPE_IVEC2 && nb == TYPE_IVEC2) {
				valid = true;
				ret_type = TYPE_IVEC2;
			} else if (na == TYPE_IVEC3 && nb == TYPE_IVEC3) {
				valid = true;
				ret_type = TYPE_IVEC3;
			} else if (na == TYPE_IVEC4 && nb == TYPE_IVEC4) {
				valid = true;
				ret_type = TYPE_IVEC4;
				/////
			} else if (na == TYPE_UINT && nb == TYPE_UINT) {
				valid = true;
				ret_type = TYPE_UINT;
			} else if (na == TYPE_UVEC2 && nb == TYPE_UINT) {
				valid = true;
				ret_type = TYPE_UVEC2;
			} else if (na == TYPE_UVEC3 && nb == TYPE_UINT) {
				valid = true;
				ret_type = TYPE_UVEC3;
			} else if (na == TYPE_UVEC4 && nb == TYPE_UINT) {
				valid = true;
				ret_type = TYPE_UVEC4;
			} else if (na == TYPE_UVEC2 && nb == TYPE_UVEC2) {
				valid = true;
				ret_type = TYPE_UVEC2;
			} else if (na == TYPE_UVEC3 && nb == TYPE_UVEC3) {
				valid = true;
				ret_type = TYPE_UVEC3;
			} else if (na == TYPE_UVEC4 && nb == TYPE_UVEC4) {
				valid = true;
				ret_type = TYPE_UVEC4;
			}
		} break;
		case OP_ASSIGN_SHIFT_LEFT:
		case OP_ASSIGN_SHIFT_RIGHT:
		case OP_SHIFT_LEFT:
		case OP_SHIFT_RIGHT: {

			DataType na = p_op->arguments[0]->get_datatype();
			DataType nb = p_op->arguments[1]->get_datatype();

			if (na == TYPE_INT && nb == TYPE_INT) {
				valid = true;
				ret_type = TYPE_INT;
			} else if (na == TYPE_IVEC2 && nb == TYPE_INT) {
				valid = true;
				ret_type = TYPE_IVEC2;
			} else if (na == TYPE_IVEC3 && nb == TYPE_INT) {
				valid = true;
				ret_type = TYPE_IVEC3;
			} else if (na == TYPE_IVEC4 && nb == TYPE_INT) {
				valid = true;
				ret_type = TYPE_IVEC4;
			} else if (na == TYPE_IVEC2 && nb == TYPE_IVEC2) {
				valid = true;
				ret_type = TYPE_IVEC2;
			} else if (na == TYPE_IVEC3 && nb == TYPE_IVEC3) {
				valid = true;
				ret_type = TYPE_IVEC3;
			} else if (na == TYPE_IVEC4 && nb == TYPE_IVEC4) {
				valid = true;
				ret_type = TYPE_IVEC4;
			} else if (na == TYPE_UINT && nb == TYPE_UINT) {
				valid = true;
				ret_type = TYPE_UINT;
			} else if (na == TYPE_UVEC2 && nb == TYPE_UINT) {
				valid = true;
				ret_type = TYPE_UVEC2;
			} else if (na == TYPE_UVEC3 && nb == TYPE_UINT) {
				valid = true;
				ret_type = TYPE_UVEC3;
			} else if (na == TYPE_UVEC4 && nb == TYPE_UINT) {
				valid = true;
				ret_type = TYPE_UVEC4;
			} else if (na == TYPE_UVEC2 && nb == TYPE_UVEC2) {
				valid = true;
				ret_type = TYPE_UVEC2;
			} else if (na == TYPE_UVEC3 && nb == TYPE_UVEC3) {
				valid = true;
				ret_type = TYPE_UVEC3;
			} else if (na == TYPE_UVEC4 && nb == TYPE_UVEC4) {
				valid = true;
				ret_type = TYPE_UVEC4;
			}
		} break;
		case OP_ASSIGN: {
			DataType na = p_op->arguments[0]->get_datatype();
			DataType nb = p_op->arguments[1]->get_datatype();
			valid = na == nb;
			ret_type = na;
		} break;
		case OP_ASSIGN_ADD:
		case OP_ASSIGN_SUB:
		case OP_ASSIGN_MUL:
		case OP_ASSIGN_DIV: {

			DataType na = p_op->arguments[0]->get_datatype();
			DataType nb = p_op->arguments[1]->get_datatype();

			if (na == nb) {
				valid = (na > TYPE_BOOL && na < TYPE_MAT2) || (p_op->op == OP_ASSIGN_MUL && na >= TYPE_MAT2 && na <= TYPE_MAT4);
				ret_type = na;
			} else if (na == TYPE_IVEC2 && nb == TYPE_INT) {
				valid = true;
				ret_type = TYPE_IVEC2;
			} else if (na == TYPE_IVEC3 && nb == TYPE_INT) {
				valid = true;
				ret_type = TYPE_IVEC3;
			} else if (na == TYPE_IVEC4 && nb == TYPE_INT) {
				valid = true;
				ret_type = TYPE_IVEC4;
			} else if (na == TYPE_UVEC2 && nb == TYPE_UINT) {
				valid = true;
				ret_type = TYPE_UVEC2;
			} else if (na == TYPE_UVEC3 && nb == TYPE_UINT) {
				valid = true;
				ret_type = TYPE_UVEC3;
			} else if (na == TYPE_UVEC4 && nb == TYPE_UINT) {
				valid = true;
				ret_type = TYPE_UVEC4;
			} else if (na == TYPE_VEC2 && nb == TYPE_FLOAT) {
				valid = true;
				ret_type = TYPE_VEC2;
			} else if (na == TYPE_VEC3 && nb == TYPE_FLOAT) {
				valid = true;
				ret_type = TYPE_VEC3;
			} else if (na == TYPE_VEC4 && nb == TYPE_FLOAT) {
				valid = true;
				ret_type = TYPE_VEC4;
			} else if (p_op->op == OP_ASSIGN_MUL && na == TYPE_MAT2 && nb == TYPE_VEC2) {
				valid = true;
				ret_type = TYPE_MAT2;
			} else if (p_op->op == OP_ASSIGN_MUL && na == TYPE_MAT3 && nb == TYPE_VEC3) {
				valid = true;
				ret_type = TYPE_MAT3;
			} else if (p_op->op == OP_ASSIGN_MUL && na == TYPE_MAT4 && nb == TYPE_VEC4) {
				valid = true;
				ret_type = TYPE_MAT4;
			} else if (p_op->op == OP_ASSIGN_MUL && na == TYPE_VEC2 && nb == TYPE_MAT2) {
				valid = true;
				ret_type = TYPE_VEC2;
			} else if (p_op->op == OP_ASSIGN_MUL && na == TYPE_VEC3 && nb == TYPE_MAT3) {
				valid = true;
				ret_type = TYPE_VEC3;
			} else if (p_op->op == OP_ASSIGN_MUL && na == TYPE_VEC4 && nb == TYPE_MAT4) {
				valid = true;
				ret_type = TYPE_VEC4;
			}
		} break;
		case OP_ASSIGN_BIT_AND:
		case OP_ASSIGN_BIT_OR:
		case OP_ASSIGN_BIT_XOR:
		case OP_BIT_AND:
		case OP_BIT_OR:
		case OP_BIT_XOR: {

			/*
			 * The bitwise operators and (&), exclusive-or (^), and inclusive-or (|). The operands must be of type
			 * signed or unsigned integers or integer vectors. The operands cannot be vectors of differing size. If
			 * one operand is a scalar and the other a vector, the scalar is applied component-wise to the vector,
			 * resulting in the same type as the vector. The fundamental types of the operands (signed or unsigned)
			 * must match.
			 */

			DataType na = p_op->arguments[0]->get_datatype();
			DataType nb = p_op->arguments[1]->get_datatype();

			if (na > nb && p_op->op >= OP_BIT_AND) {
				//can swap for non assign
				SWAP(na, nb);
			}

			if (na == TYPE_INT && nb == TYPE_INT) {
				valid = true;
				ret_type = TYPE_INT;
			} else if (na == TYPE_IVEC2 && nb == TYPE_INT) {
				valid = true;
				ret_type = TYPE_IVEC2;
			} else if (na == TYPE_IVEC3 && nb == TYPE_INT) {
				valid = true;
				ret_type = TYPE_IVEC3;
			} else if (na == TYPE_IVEC4 && nb == TYPE_INT) {
				valid = true;
				ret_type = TYPE_IVEC4;
			} else if (na == TYPE_IVEC2 && nb == TYPE_IVEC2) {
				valid = true;
				ret_type = TYPE_IVEC2;
			} else if (na == TYPE_IVEC3 && nb == TYPE_IVEC3) {
				valid = true;
				ret_type = TYPE_IVEC3;
			} else if (na == TYPE_IVEC4 && nb == TYPE_IVEC4) {
				valid = true;
				ret_type = TYPE_IVEC4;
				/////
			} else if (na == TYPE_UINT && nb == TYPE_UINT) {
				valid = true;
				ret_type = TYPE_UINT;
			} else if (na == TYPE_UVEC2 && nb == TYPE_UINT) {
				valid = true;
				ret_type = TYPE_UVEC2;
			} else if (na == TYPE_UVEC3 && nb == TYPE_UINT) {
				valid = true;
				ret_type = TYPE_UVEC3;
			} else if (na == TYPE_UVEC4 && nb == TYPE_UINT) {
				valid = true;
				ret_type = TYPE_UVEC4;
			} else if (na == TYPE_UVEC2 && nb == TYPE_UVEC2) {
				valid = true;
				ret_type = TYPE_UVEC2;
			} else if (na == TYPE_UVEC3 && nb == TYPE_UVEC3) {
				valid = true;
				ret_type = TYPE_UVEC3;
			} else if (na == TYPE_UVEC4 && nb == TYPE_UVEC4) {
				valid = true;
				ret_type = TYPE_UVEC4;
			}
		} break;
		case OP_BIT_INVERT: { //unaries
			DataType na = p_op->arguments[0]->get_datatype();
			valid = na >= TYPE_INT && na < TYPE_FLOAT;
			ret_type = na;
		} break;
		case OP_SELECT_IF: {
			DataType na = p_op->arguments[0]->get_datatype();
			DataType nb = p_op->arguments[1]->get_datatype();
			DataType nc = p_op->arguments[2]->get_datatype();

			valid = na == TYPE_BOOL && (nb == nc);
			ret_type = nb;
		} break;
		default: {
			ERR_FAIL_V(false);
		}
	}

	if (r_ret_type)
		*r_ret_type = ret_type;
	return valid;
}

const ShaderLanguage::BuiltinFuncDef ShaderLanguage::builtin_func_defs[] = {
	//constructors
	{ "bool", TYPE_BOOL, { TYPE_BOOL, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec2", TYPE_BVEC2, { TYPE_BOOL, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec2", TYPE_BVEC2, { TYPE_BOOL, TYPE_BOOL, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec3", TYPE_BVEC3, { TYPE_BOOL, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec3", TYPE_BVEC3, { TYPE_BOOL, TYPE_BOOL, TYPE_BOOL, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec3", TYPE_BVEC3, { TYPE_BVEC2, TYPE_BOOL, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec3", TYPE_BVEC3, { TYPE_BOOL, TYPE_BVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec4", TYPE_BVEC4, { TYPE_BOOL, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec4", TYPE_BVEC4, { TYPE_BOOL, TYPE_BOOL, TYPE_BOOL, TYPE_BOOL, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec4", TYPE_BVEC4, { TYPE_BOOL, TYPE_BVEC2, TYPE_BOOL, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec4", TYPE_BVEC4, { TYPE_BVEC2, TYPE_BOOL, TYPE_BOOL, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec4", TYPE_BVEC4, { TYPE_BOOL, TYPE_BOOL, TYPE_BVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec4", TYPE_BVEC4, { TYPE_BOOL, TYPE_BVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec4", TYPE_BVEC4, { TYPE_BVEC3, TYPE_BOOL, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec4", TYPE_BVEC4, { TYPE_BVEC2, TYPE_BVEC2, TYPE_VOID }, TAG_GLOBAL },

	{ "float", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "vec2", TYPE_VEC2, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "vec2", TYPE_VEC2, { TYPE_FLOAT, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "vec3", TYPE_VEC3, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "vec3", TYPE_VEC3, { TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "vec3", TYPE_VEC3, { TYPE_VEC2, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "vec3", TYPE_VEC3, { TYPE_FLOAT, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "vec4", TYPE_VEC4, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "vec4", TYPE_VEC4, { TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "vec4", TYPE_VEC4, { TYPE_FLOAT, TYPE_VEC2, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "vec4", TYPE_VEC4, { TYPE_VEC2, TYPE_FLOAT, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "vec4", TYPE_VEC4, { TYPE_FLOAT, TYPE_FLOAT, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "vec4", TYPE_VEC4, { TYPE_FLOAT, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "vec4", TYPE_VEC4, { TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "vec4", TYPE_VEC4, { TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },

	{ "int", TYPE_INT, { TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec2", TYPE_IVEC2, { TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec2", TYPE_IVEC2, { TYPE_INT, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec3", TYPE_IVEC3, { TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec3", TYPE_IVEC3, { TYPE_INT, TYPE_INT, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec3", TYPE_IVEC3, { TYPE_IVEC2, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec3", TYPE_IVEC3, { TYPE_INT, TYPE_IVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec4", TYPE_IVEC4, { TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec4", TYPE_IVEC4, { TYPE_INT, TYPE_INT, TYPE_INT, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec4", TYPE_IVEC4, { TYPE_INT, TYPE_IVEC2, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec4", TYPE_IVEC4, { TYPE_IVEC2, TYPE_INT, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec4", TYPE_IVEC4, { TYPE_INT, TYPE_INT, TYPE_IVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec4", TYPE_IVEC4, { TYPE_INT, TYPE_IVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec4", TYPE_IVEC4, { TYPE_IVEC3, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec4", TYPE_IVEC4, { TYPE_IVEC2, TYPE_IVEC2, TYPE_VOID }, TAG_GLOBAL },

	{ "uint", TYPE_UINT, { TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec2", TYPE_UVEC2, { TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec2", TYPE_UVEC2, { TYPE_UINT, TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec3", TYPE_UVEC3, { TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec3", TYPE_UVEC3, { TYPE_UINT, TYPE_UINT, TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec3", TYPE_UVEC3, { TYPE_UVEC2, TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec3", TYPE_UVEC3, { TYPE_UINT, TYPE_UVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec4", TYPE_UVEC4, { TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec4", TYPE_UVEC4, { TYPE_UINT, TYPE_UINT, TYPE_UINT, TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec4", TYPE_UVEC4, { TYPE_UINT, TYPE_UVEC2, TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec4", TYPE_UVEC4, { TYPE_UVEC2, TYPE_UINT, TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec4", TYPE_UVEC4, { TYPE_UINT, TYPE_UINT, TYPE_UVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec4", TYPE_UVEC4, { TYPE_UINT, TYPE_UVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec4", TYPE_UVEC4, { TYPE_UVEC3, TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec4", TYPE_UVEC4, { TYPE_UVEC2, TYPE_UVEC2, TYPE_VOID }, TAG_GLOBAL },

	{ "mat2", TYPE_MAT2, { TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "mat3", TYPE_MAT3, { TYPE_VEC3, TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "mat4", TYPE_MAT4, { TYPE_VEC4, TYPE_VEC4, TYPE_VEC4, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "mat2", TYPE_MAT2, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "mat3", TYPE_MAT3, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "mat4", TYPE_MAT4, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	//conversion scalars

	{ "int", TYPE_INT, { TYPE_BOOL, TYPE_VOID }, TAG_GLOBAL },
	{ "int", TYPE_INT, { TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "int", TYPE_INT, { TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "int", TYPE_INT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "float", TYPE_FLOAT, { TYPE_BOOL, TYPE_VOID }, TAG_GLOBAL },
	{ "float", TYPE_FLOAT, { TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "float", TYPE_FLOAT, { TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "float", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "uint", TYPE_UINT, { TYPE_BOOL, TYPE_VOID }, TAG_GLOBAL },
	{ "uint", TYPE_UINT, { TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "uint", TYPE_UINT, { TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "uint", TYPE_UINT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "bool", TYPE_BOOL, { TYPE_BOOL, TYPE_VOID }, TAG_GLOBAL },
	{ "bool", TYPE_BOOL, { TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "bool", TYPE_BOOL, { TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "bool", TYPE_BOOL, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	//conversion vectors

	{ "ivec2", TYPE_IVEC2, { TYPE_BVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec2", TYPE_IVEC2, { TYPE_IVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec2", TYPE_IVEC2, { TYPE_UVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec2", TYPE_IVEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },

	{ "vec2", TYPE_VEC2, { TYPE_BVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "vec2", TYPE_VEC2, { TYPE_IVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "vec2", TYPE_VEC2, { TYPE_UVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "vec2", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },

	{ "uvec2", TYPE_UVEC2, { TYPE_BVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec2", TYPE_UVEC2, { TYPE_IVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec2", TYPE_UVEC2, { TYPE_UVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec2", TYPE_UVEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },

	{ "bvec2", TYPE_BVEC2, { TYPE_BVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec2", TYPE_BVEC2, { TYPE_IVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec2", TYPE_BVEC2, { TYPE_UVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec2", TYPE_BVEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },

	{ "ivec3", TYPE_IVEC3, { TYPE_BVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec3", TYPE_IVEC3, { TYPE_IVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec3", TYPE_IVEC3, { TYPE_UVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec3", TYPE_IVEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },

	{ "vec3", TYPE_VEC3, { TYPE_BVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "vec3", TYPE_VEC3, { TYPE_IVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "vec3", TYPE_VEC3, { TYPE_UVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "vec3", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },

	{ "uvec3", TYPE_UVEC3, { TYPE_BVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec3", TYPE_UVEC3, { TYPE_IVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec3", TYPE_UVEC3, { TYPE_UVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec3", TYPE_UVEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },

	{ "bvec3", TYPE_BVEC3, { TYPE_BVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec3", TYPE_BVEC3, { TYPE_IVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec3", TYPE_BVEC3, { TYPE_UVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec3", TYPE_BVEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },

	{ "ivec4", TYPE_IVEC4, { TYPE_BVEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec4", TYPE_IVEC4, { TYPE_IVEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec4", TYPE_IVEC4, { TYPE_UVEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "ivec4", TYPE_IVEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "vec4", TYPE_VEC4, { TYPE_BVEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "vec4", TYPE_VEC4, { TYPE_IVEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "vec4", TYPE_VEC4, { TYPE_UVEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "vec4", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "uvec4", TYPE_UVEC4, { TYPE_BVEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec4", TYPE_UVEC4, { TYPE_IVEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec4", TYPE_UVEC4, { TYPE_UVEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "uvec4", TYPE_UVEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "bvec4", TYPE_BVEC4, { TYPE_BVEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec4", TYPE_BVEC4, { TYPE_IVEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec4", TYPE_BVEC4, { TYPE_UVEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "bvec4", TYPE_BVEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	//conversion between matrixes

	{ "mat2", TYPE_MAT2, { TYPE_MAT3, TYPE_VOID }, TAG_GLOBAL },
	{ "mat2", TYPE_MAT2, { TYPE_MAT4, TYPE_VOID }, TAG_GLOBAL },
	{ "mat3", TYPE_MAT3, { TYPE_MAT2, TYPE_VOID }, TAG_GLOBAL },
	{ "mat3", TYPE_MAT3, { TYPE_MAT4, TYPE_VOID }, TAG_GLOBAL },
	{ "mat4", TYPE_MAT4, { TYPE_MAT2, TYPE_VOID }, TAG_GLOBAL },
	{ "mat4", TYPE_MAT4, { TYPE_MAT3, TYPE_VOID }, TAG_GLOBAL },

	//builtins - trigonometry

	{ "radians", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "radians", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "radians", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "radians", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "degrees", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "degrees", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "degrees", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "degrees", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "sin", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "sin", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "sin", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "sin", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "cos", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "cos", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "cos", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "cos", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "tan", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "tan", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "tan", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "tan", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "asin", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "asin", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "asin", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "asin", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "acos", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "acos", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "acos", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "acos", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "atan", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "atan", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "atan", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "atan", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "atan", TYPE_FLOAT, { TYPE_FLOAT, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "atan", TYPE_VEC2, { TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "atan", TYPE_VEC3, { TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "atan", TYPE_VEC4, { TYPE_VEC4, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "sinh", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "sinh", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "sinh", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "sinh", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "cosh", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "cosh", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "cosh", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "cosh", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "tanh", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "tanh", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "tanh", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "tanh", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "asinh", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "asinh", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "asinh", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "asinh", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "acosh", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "acosh", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "acosh", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "acosh", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "atanh", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "atanh", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "atanh", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "atanh", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	//builtins - exponential
	{ "pow", TYPE_FLOAT, { TYPE_FLOAT, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "pow", TYPE_VEC2, { TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "pow", TYPE_VEC3, { TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "pow", TYPE_VEC4, { TYPE_VEC4, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "exp", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "exp", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "exp", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "exp", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "log", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "log", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "log", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "log", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "exp2", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "exp2", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "exp2", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "exp2", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "log2", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "log2", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "log2", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "log2", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "sqrt", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "sqrt", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "sqrt", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "sqrt", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "inversesqrt", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "inversesqrt", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "inversesqrt", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "inversesqrt", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	//builtins - common
	{ "abs", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "abs", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "abs", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "abs", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "abs", TYPE_INT, { TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "abs", TYPE_IVEC2, { TYPE_IVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "abs", TYPE_IVEC3, { TYPE_IVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "abs", TYPE_IVEC4, { TYPE_IVEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "sign", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "sign", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "sign", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "sign", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "sign", TYPE_INT, { TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "sign", TYPE_IVEC2, { TYPE_IVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "sign", TYPE_IVEC3, { TYPE_IVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "sign", TYPE_IVEC4, { TYPE_IVEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "floor", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "floor", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "floor", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "floor", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "trunc", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "trunc", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "trunc", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "trunc", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "round", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "round", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "round", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "round", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "roundEven", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "roundEven", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "roundEven", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "roundEven", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "ceil", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "ceil", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "ceil", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "ceil", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "fract", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "fract", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "fract", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "fract", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "mod", TYPE_FLOAT, { TYPE_FLOAT, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "mod", TYPE_VEC2, { TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "mod", TYPE_VEC2, { TYPE_VEC2, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "mod", TYPE_VEC3, { TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "mod", TYPE_VEC3, { TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "mod", TYPE_VEC4, { TYPE_VEC4, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "mod", TYPE_VEC4, { TYPE_VEC4, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "modf", TYPE_FLOAT, { TYPE_FLOAT, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "modf", TYPE_VEC2, { TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "modf", TYPE_VEC3, { TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "modf", TYPE_VEC4, { TYPE_VEC4, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "min", TYPE_FLOAT, { TYPE_FLOAT, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "min", TYPE_VEC2, { TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "min", TYPE_VEC2, { TYPE_VEC2, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "min", TYPE_VEC3, { TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "min", TYPE_VEC3, { TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "min", TYPE_VEC4, { TYPE_VEC4, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "min", TYPE_VEC4, { TYPE_VEC4, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "min", TYPE_INT, { TYPE_INT, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "min", TYPE_IVEC2, { TYPE_IVEC2, TYPE_IVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "min", TYPE_IVEC2, { TYPE_IVEC2, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "min", TYPE_IVEC3, { TYPE_IVEC3, TYPE_IVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "min", TYPE_IVEC3, { TYPE_IVEC3, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "min", TYPE_IVEC4, { TYPE_IVEC4, TYPE_IVEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "min", TYPE_IVEC4, { TYPE_IVEC4, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },

	{ "min", TYPE_UINT, { TYPE_UINT, TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "min", TYPE_UVEC2, { TYPE_UVEC2, TYPE_UVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "min", TYPE_UVEC2, { TYPE_UVEC2, TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "min", TYPE_UVEC3, { TYPE_UVEC3, TYPE_UVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "min", TYPE_UVEC3, { TYPE_UVEC3, TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "min", TYPE_UVEC4, { TYPE_UVEC4, TYPE_UVEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "min", TYPE_UVEC4, { TYPE_UVEC4, TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },

	{ "max", TYPE_FLOAT, { TYPE_FLOAT, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "max", TYPE_VEC2, { TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "max", TYPE_VEC2, { TYPE_VEC2, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "max", TYPE_VEC3, { TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "max", TYPE_VEC3, { TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "max", TYPE_VEC4, { TYPE_VEC4, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "max", TYPE_VEC4, { TYPE_VEC4, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "max", TYPE_INT, { TYPE_INT, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "max", TYPE_IVEC2, { TYPE_IVEC2, TYPE_IVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "max", TYPE_IVEC2, { TYPE_IVEC2, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "max", TYPE_IVEC3, { TYPE_IVEC3, TYPE_IVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "max", TYPE_IVEC3, { TYPE_IVEC3, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "max", TYPE_IVEC4, { TYPE_IVEC4, TYPE_IVEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "max", TYPE_IVEC4, { TYPE_IVEC4, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },

	{ "max", TYPE_UINT, { TYPE_UINT, TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "max", TYPE_UVEC2, { TYPE_UVEC2, TYPE_UVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "max", TYPE_UVEC2, { TYPE_UVEC2, TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "max", TYPE_UVEC3, { TYPE_UVEC3, TYPE_UVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "max", TYPE_UVEC3, { TYPE_UVEC3, TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "max", TYPE_UVEC4, { TYPE_UVEC4, TYPE_UVEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "max", TYPE_UVEC4, { TYPE_UVEC4, TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },

	{ "clamp", TYPE_FLOAT, { TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "clamp", TYPE_VEC2, { TYPE_VEC2, TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "clamp", TYPE_VEC3, { TYPE_VEC3, TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "clamp", TYPE_VEC4, { TYPE_VEC4, TYPE_VEC4, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "clamp", TYPE_VEC2, { TYPE_VEC2, TYPE_FLOAT, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "clamp", TYPE_VEC3, { TYPE_VEC3, TYPE_FLOAT, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "clamp", TYPE_VEC4, { TYPE_VEC4, TYPE_FLOAT, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "clamp", TYPE_INT, { TYPE_INT, TYPE_INT, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "clamp", TYPE_IVEC2, { TYPE_IVEC2, TYPE_IVEC2, TYPE_IVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "clamp", TYPE_IVEC3, { TYPE_IVEC3, TYPE_IVEC3, TYPE_IVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "clamp", TYPE_IVEC4, { TYPE_IVEC4, TYPE_IVEC4, TYPE_IVEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "clamp", TYPE_IVEC2, { TYPE_IVEC2, TYPE_INT, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "clamp", TYPE_IVEC3, { TYPE_IVEC3, TYPE_INT, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "clamp", TYPE_IVEC4, { TYPE_IVEC4, TYPE_INT, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },

	{ "clamp", TYPE_UINT, { TYPE_UINT, TYPE_UINT, TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "clamp", TYPE_UVEC2, { TYPE_UVEC2, TYPE_UVEC2, TYPE_UVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "clamp", TYPE_UVEC3, { TYPE_UVEC3, TYPE_UVEC3, TYPE_UVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "clamp", TYPE_UVEC4, { TYPE_UVEC4, TYPE_UVEC4, TYPE_UVEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "clamp", TYPE_UVEC2, { TYPE_UVEC2, TYPE_UINT, TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "clamp", TYPE_UVEC3, { TYPE_UVEC3, TYPE_UINT, TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "clamp", TYPE_UVEC4, { TYPE_UVEC4, TYPE_UINT, TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },

	{ "mix", TYPE_FLOAT, { TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "mix", TYPE_VEC2, { TYPE_VEC2, TYPE_VEC2, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "mix", TYPE_VEC2, { TYPE_VEC2, TYPE_VEC2, TYPE_BVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "mix", TYPE_VEC2, { TYPE_VEC2, TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "mix", TYPE_VEC3, { TYPE_VEC3, TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "mix", TYPE_VEC3, { TYPE_VEC3, TYPE_VEC3, TYPE_BVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "mix", TYPE_VEC3, { TYPE_VEC3, TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "mix", TYPE_VEC4, { TYPE_VEC4, TYPE_VEC4, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "mix", TYPE_VEC4, { TYPE_VEC4, TYPE_VEC4, TYPE_BVEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "mix", TYPE_VEC4, { TYPE_VEC4, TYPE_VEC4, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "step", TYPE_FLOAT, { TYPE_FLOAT, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "step", TYPE_VEC2, { TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "step", TYPE_VEC3, { TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "step", TYPE_VEC4, { TYPE_VEC4, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "step", TYPE_VEC2, { TYPE_FLOAT, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "step", TYPE_VEC3, { TYPE_FLOAT, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "step", TYPE_VEC4, { TYPE_FLOAT, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "smoothstep", TYPE_FLOAT, { TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "smoothstep", TYPE_VEC2, { TYPE_VEC2, TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "smoothstep", TYPE_VEC3, { TYPE_VEC3, TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "smoothstep", TYPE_VEC4, { TYPE_VEC4, TYPE_VEC4, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "smoothstep", TYPE_VEC2, { TYPE_FLOAT, TYPE_FLOAT, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "smoothstep", TYPE_VEC3, { TYPE_FLOAT, TYPE_FLOAT, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "smoothstep", TYPE_VEC4, { TYPE_FLOAT, TYPE_FLOAT, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "isnan", TYPE_BOOL, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "isnan", TYPE_BVEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "isnan", TYPE_BVEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "isnan", TYPE_BVEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "isinf", TYPE_BOOL, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "isinf", TYPE_BVEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "isinf", TYPE_BVEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "isinf", TYPE_BVEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "floatBitsToInt", TYPE_INT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "floatBitsToInt", TYPE_IVEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "floatBitsToInt", TYPE_IVEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "floatBitsToInt", TYPE_IVEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "floatBitsToUint", TYPE_UINT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "floatBitsToUint", TYPE_UVEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "floatBitsToUint", TYPE_UVEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "floatBitsToUint", TYPE_UVEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "intBitsToFloat", TYPE_FLOAT, { TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "intBitsToFloat", TYPE_VEC2, { TYPE_IVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "intBitsToFloat", TYPE_VEC3, { TYPE_IVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "intBitsToFloat", TYPE_VEC4, { TYPE_IVEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "uintBitsToFloat", TYPE_FLOAT, { TYPE_UINT, TYPE_VOID }, TAG_GLOBAL },
	{ "uintBitsToFloat", TYPE_VEC2, { TYPE_UVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "uintBitsToFloat", TYPE_VEC3, { TYPE_UVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "uintBitsToFloat", TYPE_VEC4, { TYPE_UVEC4, TYPE_VOID }, TAG_GLOBAL },

	//builtins - geometric
	{ "length", TYPE_FLOAT, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "length", TYPE_FLOAT, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "length", TYPE_FLOAT, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "distance", TYPE_FLOAT, { TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "distance", TYPE_FLOAT, { TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "distance", TYPE_FLOAT, { TYPE_VEC4, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "dot", TYPE_FLOAT, { TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "dot", TYPE_FLOAT, { TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "dot", TYPE_FLOAT, { TYPE_VEC4, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "cross", TYPE_VEC3, { TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "normalize", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "normalize", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "normalize", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "reflect", TYPE_VEC3, { TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "refract", TYPE_VEC3, { TYPE_VEC3, TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "faceforward", TYPE_VEC2, { TYPE_VEC2, TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "faceforward", TYPE_VEC3, { TYPE_VEC3, TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "faceforward", TYPE_VEC4, { TYPE_VEC4, TYPE_VEC4, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "matrixCompMult", TYPE_MAT2, { TYPE_MAT2, TYPE_MAT2, TYPE_VOID }, TAG_GLOBAL },
	{ "matrixCompMult", TYPE_MAT3, { TYPE_MAT3, TYPE_MAT3, TYPE_VOID }, TAG_GLOBAL },
	{ "matrixCompMult", TYPE_MAT4, { TYPE_MAT4, TYPE_MAT4, TYPE_VOID }, TAG_GLOBAL },

	{ "outerProduct", TYPE_MAT2, { TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "outerProduct", TYPE_MAT3, { TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "outerProduct", TYPE_MAT4, { TYPE_VEC4, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "transpose", TYPE_MAT2, { TYPE_MAT2, TYPE_VOID }, TAG_GLOBAL },
	{ "transpose", TYPE_MAT3, { TYPE_MAT3, TYPE_VOID }, TAG_GLOBAL },
	{ "transpose", TYPE_MAT4, { TYPE_MAT4, TYPE_VOID }, TAG_GLOBAL },

	{ "determinant", TYPE_FLOAT, { TYPE_MAT2, TYPE_VOID }, TAG_GLOBAL },
	{ "determinant", TYPE_FLOAT, { TYPE_MAT3, TYPE_VOID }, TAG_GLOBAL },
	{ "determinant", TYPE_FLOAT, { TYPE_MAT4, TYPE_VOID }, TAG_GLOBAL },

	{ "inverse", TYPE_MAT2, { TYPE_MAT2, TYPE_VOID }, TAG_GLOBAL },
	{ "inverse", TYPE_MAT3, { TYPE_MAT3, TYPE_VOID }, TAG_GLOBAL },
	{ "inverse", TYPE_MAT4, { TYPE_MAT4, TYPE_VOID }, TAG_GLOBAL },

	{ "lessThan", TYPE_BVEC2, { TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "lessThan", TYPE_BVEC3, { TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "lessThan", TYPE_BVEC4, { TYPE_VEC4, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "lessThan", TYPE_BVEC2, { TYPE_IVEC2, TYPE_IVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "lessThan", TYPE_BVEC3, { TYPE_IVEC3, TYPE_IVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "lessThan", TYPE_BVEC4, { TYPE_IVEC4, TYPE_IVEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "lessThan", TYPE_BVEC2, { TYPE_UVEC2, TYPE_UVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "lessThan", TYPE_BVEC3, { TYPE_UVEC3, TYPE_UVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "lessThan", TYPE_BVEC4, { TYPE_UVEC4, TYPE_UVEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "greaterThan", TYPE_BVEC2, { TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "greaterThan", TYPE_BVEC3, { TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "greaterThan", TYPE_BVEC4, { TYPE_VEC4, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "greaterThan", TYPE_BVEC2, { TYPE_IVEC2, TYPE_IVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "greaterThan", TYPE_BVEC3, { TYPE_IVEC3, TYPE_IVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "greaterThan", TYPE_BVEC4, { TYPE_IVEC4, TYPE_IVEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "greaterThan", TYPE_BVEC2, { TYPE_UVEC2, TYPE_UVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "greaterThan", TYPE_BVEC3, { TYPE_UVEC3, TYPE_UVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "greaterThan", TYPE_BVEC4, { TYPE_UVEC4, TYPE_UVEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "lessThanEqual", TYPE_BVEC2, { TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "lessThanEqual", TYPE_BVEC3, { TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "lessThanEqual", TYPE_BVEC4, { TYPE_VEC4, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "lessThanEqual", TYPE_BVEC2, { TYPE_IVEC2, TYPE_IVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "lessThanEqual", TYPE_BVEC3, { TYPE_IVEC3, TYPE_IVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "lessThanEqual", TYPE_BVEC4, { TYPE_IVEC4, TYPE_IVEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "lessThanEqual", TYPE_BVEC2, { TYPE_UVEC2, TYPE_UVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "lessThanEqual", TYPE_BVEC3, { TYPE_UVEC3, TYPE_UVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "lessThanEqual", TYPE_BVEC4, { TYPE_UVEC4, TYPE_UVEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "greaterThanEqual", TYPE_BVEC2, { TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "greaterThanEqual", TYPE_BVEC3, { TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "greaterThanEqual", TYPE_BVEC4, { TYPE_VEC4, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "greaterThanEqual", TYPE_BVEC2, { TYPE_IVEC2, TYPE_IVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "greaterThanEqual", TYPE_BVEC3, { TYPE_IVEC3, TYPE_IVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "greaterThanEqual", TYPE_BVEC4, { TYPE_IVEC4, TYPE_IVEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "greaterThanEqual", TYPE_BVEC2, { TYPE_UVEC2, TYPE_UVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "greaterThanEqual", TYPE_BVEC3, { TYPE_UVEC3, TYPE_UVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "greaterThanEqual", TYPE_BVEC4, { TYPE_UVEC4, TYPE_UVEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "equal", TYPE_BVEC2, { TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "equal", TYPE_BVEC3, { TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "equal", TYPE_BVEC4, { TYPE_VEC4, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "equal", TYPE_BVEC2, { TYPE_IVEC2, TYPE_IVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "equal", TYPE_BVEC3, { TYPE_IVEC3, TYPE_IVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "equal", TYPE_BVEC4, { TYPE_IVEC4, TYPE_IVEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "equal", TYPE_BVEC2, { TYPE_UVEC2, TYPE_UVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "equal", TYPE_BVEC3, { TYPE_UVEC3, TYPE_UVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "equal", TYPE_BVEC4, { TYPE_UVEC4, TYPE_UVEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "equal", TYPE_BVEC2, { TYPE_BVEC2, TYPE_BVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "equal", TYPE_BVEC3, { TYPE_BVEC3, TYPE_BVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "equal", TYPE_BVEC4, { TYPE_BVEC4, TYPE_BVEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "notEqual", TYPE_BVEC2, { TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "notEqual", TYPE_BVEC3, { TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "notEqual", TYPE_BVEC4, { TYPE_VEC4, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "notEqual", TYPE_BVEC2, { TYPE_IVEC2, TYPE_IVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "notEqual", TYPE_BVEC3, { TYPE_IVEC3, TYPE_IVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "notEqual", TYPE_BVEC4, { TYPE_IVEC4, TYPE_IVEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "notEqual", TYPE_BVEC2, { TYPE_UVEC2, TYPE_UVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "notEqual", TYPE_BVEC3, { TYPE_UVEC3, TYPE_UVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "notEqual", TYPE_BVEC4, { TYPE_UVEC4, TYPE_UVEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "notEqual", TYPE_BVEC2, { TYPE_BVEC2, TYPE_BVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "notEqual", TYPE_BVEC3, { TYPE_BVEC3, TYPE_BVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "notEqual", TYPE_BVEC4, { TYPE_BVEC4, TYPE_BVEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "any", TYPE_BOOL, { TYPE_BVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "any", TYPE_BOOL, { TYPE_BVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "any", TYPE_BOOL, { TYPE_BVEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "all", TYPE_BOOL, { TYPE_BVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "all", TYPE_BOOL, { TYPE_BVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "all", TYPE_BOOL, { TYPE_BVEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "not", TYPE_BVEC2, { TYPE_BVEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "not", TYPE_BVEC3, { TYPE_BVEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "not", TYPE_BVEC4, { TYPE_BVEC4, TYPE_VOID }, TAG_GLOBAL },

	//builtins - texture
	{ "textureSize", TYPE_IVEC2, { TYPE_SAMPLER2D, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureSize", TYPE_IVEC2, { TYPE_ISAMPLER2D, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureSize", TYPE_IVEC2, { TYPE_USAMPLER2D, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureSize", TYPE_IVEC3, { TYPE_SAMPLER2DARRAY, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureSize", TYPE_IVEC3, { TYPE_ISAMPLER2DARRAY, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureSize", TYPE_IVEC3, { TYPE_USAMPLER2DARRAY, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureSize", TYPE_IVEC3, { TYPE_SAMPLER3D, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureSize", TYPE_IVEC3, { TYPE_ISAMPLER3D, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureSize", TYPE_IVEC3, { TYPE_USAMPLER3D, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureSize", TYPE_IVEC2, { TYPE_SAMPLERCUBE, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },

	{ "texture", TYPE_VEC4, { TYPE_SAMPLER2D, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "texture", TYPE_VEC4, { TYPE_SAMPLER2D, TYPE_VEC2, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "texture", TYPE_UVEC4, { TYPE_USAMPLER2D, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "texture", TYPE_UVEC4, { TYPE_USAMPLER2D, TYPE_VEC2, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "texture", TYPE_IVEC4, { TYPE_ISAMPLER2D, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "texture", TYPE_IVEC4, { TYPE_ISAMPLER2D, TYPE_VEC2, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "texture", TYPE_VEC4, { TYPE_SAMPLER2DARRAY, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "texture", TYPE_VEC4, { TYPE_SAMPLER2DARRAY, TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "texture", TYPE_UVEC4, { TYPE_USAMPLER2DARRAY, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "texture", TYPE_UVEC4, { TYPE_USAMPLER2DARRAY, TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "texture", TYPE_IVEC4, { TYPE_ISAMPLER2DARRAY, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "texture", TYPE_IVEC4, { TYPE_ISAMPLER2DARRAY, TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "texture", TYPE_VEC4, { TYPE_SAMPLER3D, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "texture", TYPE_VEC4, { TYPE_SAMPLER3D, TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "texture", TYPE_UVEC4, { TYPE_USAMPLER3D, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "texture", TYPE_UVEC4, { TYPE_USAMPLER3D, TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "texture", TYPE_IVEC4, { TYPE_ISAMPLER3D, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "texture", TYPE_IVEC4, { TYPE_ISAMPLER3D, TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "texture", TYPE_VEC4, { TYPE_SAMPLERCUBE, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "texture", TYPE_VEC4, { TYPE_SAMPLERCUBE, TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "textureProj", TYPE_VEC4, { TYPE_SAMPLER2D, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "textureProj", TYPE_VEC4, { TYPE_SAMPLER2D, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "textureProj", TYPE_VEC4, { TYPE_SAMPLER2D, TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureProj", TYPE_VEC4, { TYPE_SAMPLER2D, TYPE_VEC4, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "textureProj", TYPE_IVEC4, { TYPE_ISAMPLER2D, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "textureProj", TYPE_IVEC4, { TYPE_ISAMPLER2D, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "textureProj", TYPE_IVEC4, { TYPE_ISAMPLER2D, TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureProj", TYPE_IVEC4, { TYPE_ISAMPLER2D, TYPE_VEC4, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "textureProj", TYPE_UVEC4, { TYPE_USAMPLER2D, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "textureProj", TYPE_UVEC4, { TYPE_USAMPLER2D, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "textureProj", TYPE_UVEC4, { TYPE_USAMPLER2D, TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureProj", TYPE_UVEC4, { TYPE_USAMPLER2D, TYPE_VEC4, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "textureProj", TYPE_VEC4, { TYPE_SAMPLER3D, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "textureProj", TYPE_VEC4, { TYPE_SAMPLER3D, TYPE_VEC4, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "textureProj", TYPE_IVEC4, { TYPE_ISAMPLER3D, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "textureProj", TYPE_IVEC4, { TYPE_ISAMPLER3D, TYPE_VEC4, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "textureProj", TYPE_UVEC4, { TYPE_USAMPLER3D, TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },
	{ "textureProj", TYPE_UVEC4, { TYPE_USAMPLER3D, TYPE_VEC4, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "textureLod", TYPE_VEC4, { TYPE_SAMPLER2D, TYPE_VEC2, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureLod", TYPE_IVEC4, { TYPE_ISAMPLER2D, TYPE_VEC2, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureLod", TYPE_UVEC4, { TYPE_USAMPLER2D, TYPE_VEC2, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureLod", TYPE_VEC4, { TYPE_SAMPLER2DARRAY, TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureLod", TYPE_IVEC4, { TYPE_ISAMPLER2DARRAY, TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureLod", TYPE_UVEC4, { TYPE_USAMPLER2DARRAY, TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureLod", TYPE_VEC4, { TYPE_SAMPLER3D, TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureLod", TYPE_IVEC4, { TYPE_ISAMPLER3D, TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureLod", TYPE_UVEC4, { TYPE_USAMPLER3D, TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureLod", TYPE_VEC4, { TYPE_SAMPLERCUBE, TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "texelFetch", TYPE_VEC4, { TYPE_SAMPLER2D, TYPE_IVEC2, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "texelFetch", TYPE_IVEC4, { TYPE_ISAMPLER2D, TYPE_IVEC2, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "texelFetch", TYPE_UVEC4, { TYPE_USAMPLER2D, TYPE_IVEC2, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },

	{ "texelFetch", TYPE_VEC4, { TYPE_SAMPLER2DARRAY, TYPE_IVEC3, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "texelFetch", TYPE_IVEC4, { TYPE_ISAMPLER2DARRAY, TYPE_IVEC3, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "texelFetch", TYPE_UVEC4, { TYPE_USAMPLER2DARRAY, TYPE_IVEC3, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },

	{ "texelFetch", TYPE_VEC4, { TYPE_SAMPLER3D, TYPE_IVEC3, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "texelFetch", TYPE_IVEC4, { TYPE_ISAMPLER3D, TYPE_IVEC3, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },
	{ "texelFetch", TYPE_UVEC4, { TYPE_USAMPLER3D, TYPE_IVEC3, TYPE_INT, TYPE_VOID }, TAG_GLOBAL },

	{ "textureProjLod", TYPE_VEC4, { TYPE_SAMPLER2D, TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureProjLod", TYPE_VEC4, { TYPE_SAMPLER2D, TYPE_VEC4, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "textureProjLod", TYPE_IVEC4, { TYPE_ISAMPLER2D, TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureProjLod", TYPE_IVEC4, { TYPE_ISAMPLER2D, TYPE_VEC4, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "textureProjLod", TYPE_UVEC4, { TYPE_USAMPLER2D, TYPE_VEC3, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureProjLod", TYPE_UVEC4, { TYPE_USAMPLER2D, TYPE_VEC4, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "textureProjLod", TYPE_VEC4, { TYPE_SAMPLER3D, TYPE_VEC4, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureProjLod", TYPE_IVEC4, { TYPE_ISAMPLER3D, TYPE_VEC4, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "textureProjLod", TYPE_UVEC4, { TYPE_USAMPLER3D, TYPE_VEC4, TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },

	{ "textureGrad", TYPE_VEC4, { TYPE_SAMPLER2D, TYPE_VEC2, TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "textureGrad", TYPE_IVEC4, { TYPE_ISAMPLER2D, TYPE_VEC2, TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "textureGrad", TYPE_UVEC4, { TYPE_USAMPLER2D, TYPE_VEC2, TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "textureGrad", TYPE_VEC4, { TYPE_SAMPLER2DARRAY, TYPE_VEC3, TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "textureGrad", TYPE_IVEC4, { TYPE_ISAMPLER2DARRAY, TYPE_VEC3, TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "textureGrad", TYPE_UVEC4, { TYPE_USAMPLER2DARRAY, TYPE_VEC3, TYPE_VEC2, TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "textureGrad", TYPE_VEC4, { TYPE_SAMPLER3D, TYPE_VEC3, TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "textureGrad", TYPE_IVEC4, { TYPE_ISAMPLER3D, TYPE_VEC3, TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "textureGrad", TYPE_UVEC4, { TYPE_USAMPLER3D, TYPE_VEC3, TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "textureGrad", TYPE_VEC4, { TYPE_SAMPLERCUBE, TYPE_VEC3, TYPE_VEC3, TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },

	{ "dFdx", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "dFdx", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "dFdx", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "dFdx", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "dFdy", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "dFdy", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "dFdy", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "dFdy", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	{ "fwidth", TYPE_FLOAT, { TYPE_FLOAT, TYPE_VOID }, TAG_GLOBAL },
	{ "fwidth", TYPE_VEC2, { TYPE_VEC2, TYPE_VOID }, TAG_GLOBAL },
	{ "fwidth", TYPE_VEC3, { TYPE_VEC3, TYPE_VOID }, TAG_GLOBAL },
	{ "fwidth", TYPE_VEC4, { TYPE_VEC4, TYPE_VOID }, TAG_GLOBAL },

	//sub-functions

	//array
	{ "length", TYPE_INT, { TYPE_VOID }, TAG_ARRAY },

	{ NULL, TYPE_VOID, { TYPE_VOID }, TAG_GLOBAL }

};

const ShaderLanguage::BuiltinFuncOutArgs ShaderLanguage::builtin_func_out_args[] = {
	//constructors
	{ "modf", 1 },
	{ NULL, 0 }
};

bool ShaderLanguage::_validate_function_call(BlockNode *p_block, OperatorNode *p_func, DataType *r_ret_type) {

	ERR_FAIL_COND_V(p_func->op != OP_CALL && p_func->op != OP_CONSTRUCT, false);

	Vector<DataType> args;

	ERR_FAIL_COND_V(p_func->arguments[0]->type != Node::TYPE_VARIABLE, false);

	StringName name = static_cast<VariableNode *>(p_func->arguments[0])->name.operator String();

	for (int i = 1; i < p_func->arguments.size(); i++) {
		args.push_back(p_func->arguments[i]->get_datatype());
	}

	int argcount = args.size();

	bool failed_builtin = false;

	if (argcount <= 4) {
		// test builtins
		int idx = 0;

		while (builtin_func_defs[idx].name) {

			if (completion_class != builtin_func_defs[idx].tag) {
				idx++;
				continue;
			}

			if (name == builtin_func_defs[idx].name) {

				failed_builtin = true;
				bool fail = false;
				for (int i = 0; i < argcount; i++) {

					if (get_scalar_type(args[i]) == args[i] && p_func->arguments[i + 1]->type == Node::TYPE_CONSTANT && convert_constant(static_cast<ConstantNode *>(p_func->arguments[i + 1]), builtin_func_defs[idx].args[i])) {
						//all good, but needs implicit conversion later
					} else if (args[i] != builtin_func_defs[idx].args[i]) {
						fail = true;
						break;
					}
				}

				if (!fail && argcount < 4 && builtin_func_defs[idx].args[argcount] != TYPE_VOID)
					fail = true; //make sure the number of arguments matches

				if (!fail) {

					//make sure its not an out argument used in the wrong way
					int outarg_idx = 0;
					while (builtin_func_out_args[outarg_idx].name) {

						if (String(name) == builtin_func_out_args[outarg_idx].name) {
							int arg_idx = builtin_func_out_args[outarg_idx].argument;

							if (arg_idx < argcount) {

								if (p_func->arguments[arg_idx + 1]->type != Node::TYPE_VARIABLE) {
									_set_error("Argument " + itos(arg_idx + 1) + " of function '" + String(name) + "' is not a variable");
									return false;
								}
								StringName var_name = static_cast<const VariableNode *>(p_func->arguments[arg_idx + 1])->name;

								const BlockNode *b = p_block;
								bool valid = false;
								while (b) {
									if (b->variables.has(var_name)) {
										valid = true;
										break;
									}
									b = b->parent_block;
								}

								if (!valid) {
									_set_error("Argument " + itos(arg_idx + 1) + " of function '" + String(name) + "' can only take a local variable");
									return false;
								}
							}
						}

						outarg_idx++;
					}
					//implicitly convert values if possible
					for (int i = 0; i < argcount; i++) {

						if (get_scalar_type(args[i]) != args[i] || args[i] == builtin_func_defs[idx].args[i] || p_func->arguments[i + 1]->type != Node::TYPE_CONSTANT) {
							//can't do implicit conversion here
							continue;
						}

						//this is an implicit conversion
						ConstantNode *constant = static_cast<ConstantNode *>(p_func->arguments[i + 1]);
						ConstantNode *conversion = alloc_node<ConstantNode>();

						conversion->datatype = builtin_func_defs[idx].args[i];
						conversion->values.resize(1);

						convert_constant(constant, builtin_func_defs[idx].args[i], conversion->values.ptrw());
						p_func->arguments.write[i + 1] = conversion;
					}

					if (r_ret_type)
						*r_ret_type = builtin_func_defs[idx].rettype;

					return true;
				}
			}

			idx++;
		}
	}

	if (failed_builtin) {
		String err = "Invalid arguments for built-in function: " + String(name) + "(";
		for (int i = 0; i < argcount; i++) {
			if (i > 0)
				err += ",";

			if (p_func->arguments[i + 1]->type == Node::TYPE_CONSTANT && p_func->arguments[i + 1]->get_datatype() == TYPE_INT && static_cast<ConstantNode *>(p_func->arguments[i + 1])->values[0].sint < 0) {
				err += "-";
			}
			err += get_datatype_name(args[i]);
		}
		err += ")";
		_set_error(err);
		return false;
	}

	// try existing functions..

	StringName exclude_function;
	BlockNode *block = p_block;

	while (block) {

		if (block->parent_function) {
			exclude_function = block->parent_function->name;
		}
		block = block->parent_block;
	}

	if (name == exclude_function) {
		_set_error("Recursion is not allowed");
		return false;
	}

	for (int i = 0; i < shader->functions.size(); i++) {

		if (name != shader->functions[i].name)
			continue;

		if (!shader->functions[i].callable) {
			_set_error("Function '" + String(name) + " can't be called from source code.");
			return false;
		}

		FunctionNode *pfunc = shader->functions[i].function;

		if (pfunc->arguments.size() != args.size())
			continue;

		bool fail = false;

		for (int j = 0; j < args.size(); j++) {

			if (get_scalar_type(args[j]) == args[j] && p_func->arguments[j + 1]->type == Node::TYPE_CONSTANT && convert_constant(static_cast<ConstantNode *>(p_func->arguments[j + 1]), pfunc->arguments[j].type)) {
				//all good, but it needs implicit conversion later
			} else if (args[j] != pfunc->arguments[j].type) {
				fail = true;
				break;
			}
		}

		if (!fail) {

			//implicitly convert values if possible
			for (int k = 0; k < args.size(); k++) {

				if (get_scalar_type(args[k]) != args[k] || args[k] == pfunc->arguments[k].type || p_func->arguments[k + 1]->type != Node::TYPE_CONSTANT) {
					//can't do implicit conversion here
					continue;
				}

				//this is an implicit conversion
				ConstantNode *constant = static_cast<ConstantNode *>(p_func->arguments[k + 1]);
				ConstantNode *conversion = alloc_node<ConstantNode>();

				conversion->datatype = pfunc->arguments[k].type;
				conversion->values.resize(1);

				convert_constant(constant, pfunc->arguments[k].type, conversion->values.ptrw());
				p_func->arguments.write[k + 1] = conversion;
			}

			if (r_ret_type)
				*r_ret_type = pfunc->return_type;
			return true;
		}
	}

	return false;
}

bool ShaderLanguage::_parse_function_arguments(BlockNode *p_block, const Map<StringName, BuiltInInfo> &p_builtin_types, OperatorNode *p_func, int *r_complete_arg) {

	TkPos pos = _get_tkpos();
	Token tk = _get_token();

	if (tk.type == TK_PARENTHESIS_CLOSE) {
		return true;
	}

	_set_tkpos(pos);

	while (true) {

		if (r_complete_arg) {
			pos = _get_tkpos();
			tk = _get_token();

			if (tk.type == TK_CURSOR) {

				*r_complete_arg = p_func->arguments.size() - 1;
			} else {

				_set_tkpos(pos);
			}
		}

		Node *arg = _parse_and_reduce_expression(p_block, p_builtin_types);

		if (!arg) {

			return false;
		}

		p_func->arguments.push_back(arg);

		tk = _get_token();

		if (tk.type == TK_PARENTHESIS_CLOSE) {

			return true;
		} else if (tk.type != TK_COMMA) {
			// something is broken
			_set_error("Expected ',' or ')' after argument");
			return false;
		}
	}

	return true;
}

bool ShaderLanguage::is_token_operator(TokenType p_type) {

	return (p_type == TK_OP_EQUAL ||
			p_type == TK_OP_NOT_EQUAL ||
			p_type == TK_OP_LESS ||
			p_type == TK_OP_LESS_EQUAL ||
			p_type == TK_OP_GREATER ||
			p_type == TK_OP_GREATER_EQUAL ||
			p_type == TK_OP_AND ||
			p_type == TK_OP_OR ||
			p_type == TK_OP_NOT ||
			p_type == TK_OP_ADD ||
			p_type == TK_OP_SUB ||
			p_type == TK_OP_MUL ||
			p_type == TK_OP_DIV ||
			p_type == TK_OP_MOD ||
			p_type == TK_OP_SHIFT_LEFT ||
			p_type == TK_OP_SHIFT_RIGHT ||
			p_type == TK_OP_ASSIGN ||
			p_type == TK_OP_ASSIGN_ADD ||
			p_type == TK_OP_ASSIGN_SUB ||
			p_type == TK_OP_ASSIGN_MUL ||
			p_type == TK_OP_ASSIGN_DIV ||
			p_type == TK_OP_ASSIGN_MOD ||
			p_type == TK_OP_ASSIGN_SHIFT_LEFT ||
			p_type == TK_OP_ASSIGN_SHIFT_RIGHT ||
			p_type == TK_OP_ASSIGN_BIT_AND ||
			p_type == TK_OP_ASSIGN_BIT_OR ||
			p_type == TK_OP_ASSIGN_BIT_XOR ||
			p_type == TK_OP_BIT_AND ||
			p_type == TK_OP_BIT_OR ||
			p_type == TK_OP_BIT_XOR ||
			p_type == TK_OP_BIT_INVERT ||
			p_type == TK_OP_INCREMENT ||
			p_type == TK_OP_DECREMENT ||
			p_type == TK_QUESTION ||
			p_type == TK_COLON);
}

bool ShaderLanguage::convert_constant(ConstantNode *p_constant, DataType p_to_type, ConstantNode::Value *p_value) {

	if (p_constant->datatype == p_to_type) {
		if (p_value) {
			for (int i = 0; i < p_constant->values.size(); i++) {
				p_value[i] = p_constant->values[i];
			}
		}
		return true;
	} else if (p_constant->datatype == TYPE_INT && p_to_type == TYPE_FLOAT) {

		if (p_value) {
			p_value->real = p_constant->values[0].sint;
		}
		return true;
	} else if (p_constant->datatype == TYPE_UINT && p_to_type == TYPE_FLOAT) {

		if (p_value) {
			p_value->real = p_constant->values[0].uint;
		}
		return true;
	} else if (p_constant->datatype == TYPE_INT && p_to_type == TYPE_UINT) {
		if (p_constant->values[0].sint < 0) {
			return false;
		}
		if (p_value) {
			p_value->uint = p_constant->values[0].sint;
		}
		return true;
	} else if (p_constant->datatype == TYPE_UINT && p_to_type == TYPE_INT) {

		if (p_constant->values[0].uint > 0x7FFFFFFF) {
			return false;
		}
		if (p_value) {
			p_value->sint = p_constant->values[0].uint;
		}
		return true;
	} else
		return false;
}

bool ShaderLanguage::is_scalar_type(DataType p_type) {

	return p_type == TYPE_BOOL || p_type == TYPE_INT || p_type == TYPE_UINT || p_type == TYPE_FLOAT;
}

bool ShaderLanguage::is_sampler_type(DataType p_type) {

	return p_type == TYPE_SAMPLER2D ||
		   p_type == TYPE_ISAMPLER2D ||
		   p_type == TYPE_USAMPLER2D ||
		   p_type == TYPE_SAMPLER2DARRAY ||
		   p_type == TYPE_ISAMPLER2DARRAY ||
		   p_type == TYPE_USAMPLER2DARRAY ||
		   p_type == TYPE_SAMPLER3D ||
		   p_type == TYPE_ISAMPLER3D ||
		   p_type == TYPE_USAMPLER3D ||
		   p_type == TYPE_SAMPLERCUBE;
}

Variant ShaderLanguage::constant_value_to_variant(const Vector<ShaderLanguage::ConstantNode::Value> &p_value, DataType p_type, ShaderLanguage::ShaderNode::Uniform::Hint p_hint) {
	if (p_value.size() > 0) {
		Variant value;
		switch (p_type) {
			case ShaderLanguage::TYPE_BOOL:
				value = Variant(p_value[0].boolean);
				break;
			case ShaderLanguage::TYPE_BVEC2:
			case ShaderLanguage::TYPE_BVEC3:
			case ShaderLanguage::TYPE_BVEC4:
			case ShaderLanguage::TYPE_INT:
				value = Variant(p_value[0].sint);
				break;
			case ShaderLanguage::TYPE_IVEC2:
				value = Variant(Vector2(p_value[0].sint, p_value[1].sint));
				break;
			case ShaderLanguage::TYPE_IVEC3:
				value = Variant(Vector3(p_value[0].sint, p_value[1].sint, p_value[2].sint));
				break;
			case ShaderLanguage::TYPE_IVEC4:
				value = Variant(Plane(p_value[0].sint, p_value[1].sint, p_value[2].sint, p_value[3].sint));
				break;
			case ShaderLanguage::TYPE_UINT:
				value = Variant(p_value[0].uint);
				break;
			case ShaderLanguage::TYPE_UVEC2:
				value = Variant(Vector2(p_value[0].uint, p_value[1].uint));
				break;
			case ShaderLanguage::TYPE_UVEC3:
				value = Variant(Vector3(p_value[0].uint, p_value[1].uint, p_value[2].uint));
				break;
			case ShaderLanguage::TYPE_UVEC4:
				value = Variant(Plane(p_value[0].uint, p_value[1].uint, p_value[2].uint, p_value[3].uint));
				break;
			case ShaderLanguage::TYPE_FLOAT:
				value = Variant(p_value[0].real);
				break;
			case ShaderLanguage::TYPE_VEC2:
				value = Variant(Vector2(p_value[0].real, p_value[1].real));
				break;
			case ShaderLanguage::TYPE_VEC3:
				value = Variant(Vector3(p_value[0].real, p_value[1].real, p_value[2].real));
				break;
			case ShaderLanguage::TYPE_VEC4:
				if (p_hint == ShaderLanguage::ShaderNode::Uniform::HINT_COLOR) {
					value = Variant(Color(p_value[0].real, p_value[1].real, p_value[2].real, p_value[3].real));
				} else {
					value = Variant(Plane(p_value[0].real, p_value[1].real, p_value[2].real, p_value[3].real));
				}
				break;
			case ShaderLanguage::TYPE_MAT2:
				value = Variant(Transform2D(p_value[0].real, p_value[2].real, p_value[1].real, p_value[3].real, 0.0, 0.0));
				break;
			case ShaderLanguage::TYPE_MAT3: {
				Basis p;
				p[0][0] = p_value[0].real;
				p[0][1] = p_value[1].real;
				p[0][2] = p_value[2].real;
				p[1][0] = p_value[3].real;
				p[1][1] = p_value[4].real;
				p[1][2] = p_value[5].real;
				p[2][0] = p_value[6].real;
				p[2][1] = p_value[7].real;
				p[2][2] = p_value[8].real;
				value = Variant(p);
				break;
			}
			case ShaderLanguage::TYPE_MAT4: {
				Basis p;
				p[0][0] = p_value[0].real;
				p[0][1] = p_value[1].real;
				p[0][2] = p_value[2].real;
				p[1][0] = p_value[4].real;
				p[1][1] = p_value[5].real;
				p[1][2] = p_value[6].real;
				p[2][0] = p_value[8].real;
				p[2][1] = p_value[9].real;
				p[2][2] = p_value[10].real;
				Transform t = Transform(p, Vector3(p_value[3].real, p_value[7].real, p_value[11].real));
				value = Variant(t);
				break;
			}
			case ShaderLanguage::TYPE_ISAMPLER2DARRAY:
			case ShaderLanguage::TYPE_ISAMPLER2D:
			case ShaderLanguage::TYPE_ISAMPLER3D:
			case ShaderLanguage::TYPE_SAMPLER2DARRAY:
			case ShaderLanguage::TYPE_SAMPLER2D:
			case ShaderLanguage::TYPE_SAMPLER3D:
			case ShaderLanguage::TYPE_USAMPLER2DARRAY:
			case ShaderLanguage::TYPE_USAMPLER2D:
			case ShaderLanguage::TYPE_USAMPLER3D:
			case ShaderLanguage::TYPE_SAMPLERCUBE: {
				// Texture types, likely not relevant here.
				break;
			}
			case ShaderLanguage::TYPE_VOID:
				break;
		}
		return value;
	}
	return Variant();
}

void ShaderLanguage::get_keyword_list(List<String> *r_keywords) {

	Set<String> kws;

	int idx = 0;

	while (keyword_list[idx].text) {

		kws.insert(keyword_list[idx].text);
		idx++;
	}

	idx = 0;

	while (builtin_func_defs[idx].name) {

		kws.insert(builtin_func_defs[idx].name);

		idx++;
	}

	for (Set<String>::Element *E = kws.front(); E; E = E->next()) {
		r_keywords->push_back(E->get());
	}
}

void ShaderLanguage::get_builtin_funcs(List<String> *r_keywords) {

	Set<String> kws;

	int idx = 0;

	while (builtin_func_defs[idx].name) {

		kws.insert(builtin_func_defs[idx].name);

		idx++;
	}

	for (Set<String>::Element *E = kws.front(); E; E = E->next()) {
		r_keywords->push_back(E->get());
	}
}

ShaderLanguage::DataType ShaderLanguage::get_scalar_type(DataType p_type) {

	static const DataType scalar_types[] = {
		TYPE_VOID,
		TYPE_BOOL,
		TYPE_BOOL,
		TYPE_BOOL,
		TYPE_BOOL,
		TYPE_INT,
		TYPE_INT,
		TYPE_INT,
		TYPE_INT,
		TYPE_UINT,
		TYPE_UINT,
		TYPE_UINT,
		TYPE_UINT,
		TYPE_FLOAT,
		TYPE_FLOAT,
		TYPE_FLOAT,
		TYPE_FLOAT,
		TYPE_FLOAT,
		TYPE_FLOAT,
		TYPE_FLOAT,
		TYPE_FLOAT,
		TYPE_INT,
		TYPE_UINT,
		TYPE_FLOAT,
	};

	return scalar_types[p_type];
}

int ShaderLanguage::get_cardinality(DataType p_type) {
	static const int cardinality_table[] = {
		0,
		1,
		2,
		3,
		4,
		1,
		2,
		3,
		4,
		1,
		2,
		3,
		4,
		1,
		2,
		3,
		4,
		4,
		9,
		16,
		1,
		1,
		1,
		1,
	};

	return cardinality_table[p_type];
}

bool ShaderLanguage::_get_completable_identifier(BlockNode *p_block, CompletionType p_type, StringName &identifier) {

	identifier = StringName();

	TkPos pos = { 0, 0 };

	Token tk = _get_token();

	if (tk.type == TK_IDENTIFIER) {
		identifier = tk.text;
		pos = _get_tkpos();
		tk = _get_token();
	}

	if (tk.type == TK_CURSOR) {

		completion_type = p_type;
		completion_line = tk_line;
		completion_block = p_block;

		pos = _get_tkpos();
		tk = _get_token();

		if (tk.type == TK_IDENTIFIER) {
			identifier = identifier.operator String() + tk.text.operator String();
		} else {
			_set_tkpos(pos);
		}
		return true;
	} else if (identifier != StringName()) {
		_set_tkpos(pos);
	}

	return false;
}

bool ShaderLanguage::_is_operator_assign(Operator p_op) const {
	switch (p_op) {
		case OP_ASSIGN:
		case OP_ASSIGN_ADD:
		case OP_ASSIGN_SUB:
		case OP_ASSIGN_MUL:
		case OP_ASSIGN_DIV:
		case OP_ASSIGN_MOD:
		case OP_ASSIGN_SHIFT_LEFT:
		case OP_ASSIGN_SHIFT_RIGHT:
		case OP_ASSIGN_BIT_AND:
		case OP_ASSIGN_BIT_OR:
		case OP_ASSIGN_BIT_XOR:
			return true;
		default:
			return false;
	}

	return false;
}

bool ShaderLanguage::_validate_assign(Node *p_node, const Map<StringName, BuiltInInfo> &p_builtin_types, String *r_message) {

	if (p_node->type == Node::TYPE_OPERATOR) {

		OperatorNode *op = static_cast<OperatorNode *>(p_node);

		if (op->op == OP_INDEX) {
			return _validate_assign(op->arguments[0], p_builtin_types, r_message);

		} else if (_is_operator_assign(op->op)) {
			//chained assignment
			return _validate_assign(op->arguments[1], p_builtin_types, r_message);

		} else if (op->op == OP_CALL) {
			if (r_message)
				*r_message = RTR("Assignment to function.");
			return false;
		}

	} else if (p_node->type == Node::TYPE_MEMBER) {

		MemberNode *member = static_cast<MemberNode *>(p_node);
		return _validate_assign(member->owner, p_builtin_types, r_message);

	} else if (p_node->type == Node::TYPE_VARIABLE) {

		VariableNode *var = static_cast<VariableNode *>(p_node);

		if (shader->uniforms.has(var->name)) {
			if (r_message)
				*r_message = RTR("Assignment to uniform.");
			return false;
		}

		if (shader->varyings.has(var->name) && current_function != String("vertex")) {
			if (r_message)
				*r_message = RTR("Varyings can only be assigned in vertex function.");
			return false;
		}

		if (shader->constants.has(var->name)) {
			if (r_message)
				*r_message = RTR("Constants cannot be modified.");
			return false;
		}

		if (!(p_builtin_types.has(var->name) && p_builtin_types[var->name].constant)) {
			return true;
		}
	} else if (p_node->type == Node::TYPE_ARRAY) {
		return true;
	}

	if (r_message)
		*r_message = "Assignment to constant expression.";
	return false;
}

ShaderLanguage::Node *ShaderLanguage::_parse_expression(BlockNode *p_block, const Map<StringName, BuiltInInfo> &p_builtin_types) {

	Vector<Expression> expression;

	//Vector<TokenType> operators;

	while (true) {

		Node *expr = NULL;
		TkPos prepos = _get_tkpos();
		Token tk = _get_token();
		TkPos pos = _get_tkpos();

		if (tk.type == TK_PARENTHESIS_OPEN) {
			//handle subexpression

			expr = _parse_and_reduce_expression(p_block, p_builtin_types);
			if (!expr)
				return NULL;

			tk = _get_token();

			if (tk.type != TK_PARENTHESIS_CLOSE) {

				_set_error("Expected ')' in expression");
				return NULL;
			}

		} else if (tk.type == TK_REAL_CONSTANT) {

			ConstantNode *constant = alloc_node<ConstantNode>();
			ConstantNode::Value v;
			v.real = tk.constant;
			constant->values.push_back(v);
			constant->datatype = TYPE_FLOAT;
			expr = constant;

		} else if (tk.type == TK_INT_CONSTANT) {

			ConstantNode *constant = alloc_node<ConstantNode>();
			ConstantNode::Value v;
			v.sint = tk.constant;
			constant->values.push_back(v);
			constant->datatype = TYPE_INT;
			expr = constant;

		} else if (tk.type == TK_TRUE) {

			//handle true constant
			ConstantNode *constant = alloc_node<ConstantNode>();
			ConstantNode::Value v;
			v.boolean = true;
			constant->values.push_back(v);
			constant->datatype = TYPE_BOOL;
			expr = constant;

		} else if (tk.type == TK_FALSE) {

			//handle false constant
			ConstantNode *constant = alloc_node<ConstantNode>();
			ConstantNode::Value v;
			v.boolean = false;
			constant->values.push_back(v);
			constant->datatype = TYPE_BOOL;
			expr = constant;

		} else if (tk.type == TK_TYPE_VOID) {

			//make sure void is not used in expression
			_set_error("Void value not allowed in Expression");
			return NULL;
		} else if (is_token_nonvoid_datatype(tk.type)) {
			//basic type constructor

			OperatorNode *func = alloc_node<OperatorNode>();
			func->op = OP_CONSTRUCT;

			if (is_token_precision(tk.type)) {

				func->return_precision_cache = get_token_precision(tk.type);
				tk = _get_token();
			}

			VariableNode *funcname = alloc_node<VariableNode>();
			funcname->name = get_datatype_name(get_token_datatype(tk.type));
			func->arguments.push_back(funcname);

			tk = _get_token();
			if (tk.type != TK_PARENTHESIS_OPEN) {
				_set_error("Expected '(' after type name");
				return NULL;
			}

			int carg = -1;

			bool ok = _parse_function_arguments(p_block, p_builtin_types, func, &carg);

			if (carg >= 0) {
				completion_type = COMPLETION_CALL_ARGUMENTS;
				completion_line = tk_line;
				completion_block = p_block;
				completion_function = funcname->name;
				completion_argument = carg;
			}

			if (!ok)
				return NULL;

			if (!_validate_function_call(p_block, func, &func->return_cache)) {
				_set_error("No matching constructor found for: '" + String(funcname->name) + "'");
				return NULL;
			}

			expr = _reduce_expression(p_block, func);

		} else if (tk.type == TK_IDENTIFIER) {

			_set_tkpos(prepos);

			StringName identifier;

			_get_completable_identifier(p_block, COMPLETION_IDENTIFIER, identifier);

			tk = _get_token();
			if (tk.type == TK_PARENTHESIS_OPEN) {
				//a function
				StringName name = identifier;

				OperatorNode *func = alloc_node<OperatorNode>();
				func->op = OP_CALL;
				VariableNode *funcname = alloc_node<VariableNode>();
				funcname->name = name;
				func->arguments.push_back(funcname);

				int carg = -1;

				bool ok = _parse_function_arguments(p_block, p_builtin_types, func, &carg);

				//test if function was parsed first
				for (int i = 0; i < shader->functions.size(); i++) {
					if (shader->functions[i].name == name) {
						//add to current function as dependency
						for (int j = 0; j < shader->functions.size(); j++) {
							if (shader->functions[j].name == current_function) {
								shader->functions.write[j].uses_function.insert(name);
								break;
							}
						}
						break;
					}
				}

				if (carg >= 0) {
					completion_type = COMPLETION_CALL_ARGUMENTS;
					completion_line = tk_line;
					completion_block = p_block;
					completion_function = funcname->name;
					completion_argument = carg;
				}

				if (!ok)
					return NULL;

				if (!_validate_function_call(p_block, func, &func->return_cache)) {
					_set_error("No matching function found for: '" + String(funcname->name) + "'");
					return NULL;
				}
				completion_class = TAG_GLOBAL; // reset sub-class

				expr = func;

			} else {
				//an identifier

				_set_tkpos(pos);

				DataType data_type;
				IdentifierType ident_type;
				int array_size = 0;

				if (!_find_identifier(p_block, p_builtin_types, identifier, &data_type, &ident_type, &array_size)) {
					_set_error("Unknown identifier in expression: " + String(identifier));
					return NULL;
				}

				if (ident_type == IDENTIFIER_FUNCTION) {
					_set_error("Can't use function as identifier: " + String(identifier));
					return NULL;
				}

				Node *index_expression = NULL;
				Node *call_expression = NULL;

				if (array_size > 0) {
					tk = _get_token();

					if (tk.type != TK_BRACKET_OPEN && tk.type != TK_PERIOD) {
						_set_error("Expected '[' or '.'");
						return NULL;
					}

					if (tk.type == TK_PERIOD) {
						completion_class = TAG_ARRAY;
						call_expression = _parse_and_reduce_expression(p_block, p_builtin_types);
						if (!call_expression)
							return NULL;
						data_type = call_expression->get_datatype();
					} else { // indexing

						index_expression = _parse_and_reduce_expression(p_block, p_builtin_types);
						if (!index_expression)
							return NULL;

						if (index_expression->get_datatype() != TYPE_INT && index_expression->get_datatype() != TYPE_UINT) {
							_set_error("Only integer expressions are allowed for indexing");
							return NULL;
						}

						if (index_expression->type == Node::TYPE_CONSTANT) {
							ConstantNode *cnode = (ConstantNode *)index_expression;
							if (cnode) {
								if (!cnode->values.empty()) {
									int value = cnode->values[0].sint;
									if (value < 0 || value >= array_size) {
										_set_error(vformat("Index [%s] out of range [%s..%s]", value, 0, array_size - 1));
										return NULL;
									}
								}
							}
						}

						tk = _get_token();
						if (tk.type != TK_BRACKET_CLOSE) {
							_set_error("Expected ']'");
							return NULL;
						}
					}

					ArrayNode *arrname = alloc_node<ArrayNode>();
					arrname->name = identifier;
					arrname->datatype_cache = data_type;
					arrname->index_expression = index_expression;
					arrname->call_expression = call_expression;
					expr = arrname;

				} else {

					VariableNode *varname = alloc_node<VariableNode>();
					varname->name = identifier;
					varname->datatype_cache = data_type;
					expr = varname;
				}
			}

		} else if (tk.type == TK_OP_ADD) {
			continue; //this one does nothing
		} else if (tk.type == TK_OP_SUB || tk.type == TK_OP_NOT || tk.type == TK_OP_BIT_INVERT || tk.type == TK_OP_INCREMENT || tk.type == TK_OP_DECREMENT) {

			Expression e;
			e.is_op = true;

			switch (tk.type) {
				case TK_OP_SUB: e.op = OP_NEGATE; break;
				case TK_OP_NOT: e.op = OP_NOT; break;
				case TK_OP_BIT_INVERT: e.op = OP_BIT_INVERT; break;
				case TK_OP_INCREMENT: e.op = OP_INCREMENT; break;
				case TK_OP_DECREMENT: e.op = OP_DECREMENT; break;
				default: ERR_FAIL_V(NULL);
			}

			expression.push_back(e);
			continue;

		} else {
			_set_error("Expected expression, found: " + get_token_text(tk));
			return NULL;
			//nothing
		}

		ERR_FAIL_COND_V(!expr, NULL);

		/* OK now see what's NEXT to the operator.. */
		/* OK now see what's NEXT to the operator.. */
		/* OK now see what's NEXT to the operator.. */

		while (true) {
			TkPos pos2 = _get_tkpos();
			tk = _get_token();

			if (tk.type == TK_CURSOR) {
				//do nothing
			} else if (tk.type == TK_PERIOD) {

				StringName identifier;
				if (_get_completable_identifier(p_block, COMPLETION_INDEX, identifier)) {
					completion_base = expr->get_datatype();
				}

				if (identifier == StringName()) {
					_set_error("Expected identifier as member");
					return NULL;
				}
				DataType dt = expr->get_datatype();
				String ident = identifier;

				bool ok = true;
				DataType member_type;
				switch (dt) {
					case TYPE_BVEC2:
					case TYPE_IVEC2:
					case TYPE_UVEC2:
					case TYPE_VEC2: {

						int l = ident.length();
						if (l == 1) {
							member_type = DataType(dt - 1);
						} else if (l == 2) {
							member_type = dt;
						} else if (l == 3) {
							member_type = DataType(dt + 1);
						} else if (l == 4) {
							member_type = DataType(dt + 2);
						} else {
							ok = false;
							break;
						}

						const CharType *c = ident.ptr();
						for (int i = 0; i < l; i++) {

							switch (c[i]) {
								case 'r':
								case 'g':
								case 'x':
								case 'y':
									break;
								default:
									ok = false;
									break;
							}
						}

					} break;
					case TYPE_BVEC3:
					case TYPE_IVEC3:
					case TYPE_UVEC3:
					case TYPE_VEC3: {

						int l = ident.length();
						if (l == 1) {
							member_type = DataType(dt - 2);
						} else if (l == 2) {
							member_type = DataType(dt - 1);
						} else if (l == 3) {
							member_type = dt;
						} else if (l == 4) {
							member_type = DataType(dt + 1);
						} else {
							ok = false;
							break;
						}

						const CharType *c = ident.ptr();
						for (int i = 0; i < l; i++) {

							switch (c[i]) {
								case 'r':
								case 'g':
								case 'b':
								case 'x':
								case 'y':
								case 'z':
									break;
								default:
									ok = false;
									break;
							}
						}

					} break;
					case TYPE_BVEC4:
					case TYPE_IVEC4:
					case TYPE_UVEC4:
					case TYPE_VEC4: {

						int l = ident.length();
						if (l == 1) {
							member_type = DataType(dt - 3);
						} else if (l == 2) {
							member_type = DataType(dt - 2);
						} else if (l == 3) {
							member_type = DataType(dt - 1);
						} else if (l == 4) {
							member_type = dt;
						} else {
							ok = false;
							break;
						}

						const CharType *c = ident.ptr();
						for (int i = 0; i < l; i++) {

							switch (c[i]) {
								case 'r':
								case 'g':
								case 'b':
								case 'a':
								case 'x':
								case 'y':
								case 'z':
								case 'w':
									break;
								default:
									ok = false;
									break;
							}
						}

					} break;

					default: {
						ok = false;
					}
				}

				if (!ok) {

					_set_error("Invalid member for " + get_datatype_name(dt) + " expression: ." + ident);
					return NULL;
				}

				MemberNode *mn = alloc_node<MemberNode>();
				mn->basetype = dt;
				mn->datatype = member_type;
				mn->name = ident;
				mn->owner = expr;
				expr = mn;

				//todo
				//member (period) has priority over any operator
				//creates a subindexing expression in place

				/*} else if (tk.type==TK_BRACKET_OPEN) {
				//todo
				//subindexing has priority over any operator
				//creates a subindexing expression in place

	*/
			} else if (tk.type == TK_BRACKET_OPEN) {

				Node *index = _parse_and_reduce_expression(p_block, p_builtin_types);
				if (!index)
					return NULL;

				if (index->get_datatype() != TYPE_INT && index->get_datatype() != TYPE_UINT) {
					_set_error("Only integer datatypes are allowed for indexing");
					return NULL;
				}

				DataType member_type = TYPE_VOID;

				switch (expr->get_datatype()) {
					case TYPE_BVEC2:
					case TYPE_VEC2:
					case TYPE_IVEC2:
					case TYPE_UVEC2:
					case TYPE_MAT2:
						if (index->type == Node::TYPE_CONSTANT) {
							uint32_t index_constant = static_cast<ConstantNode *>(index)->values[0].uint;
							if (index_constant >= 2) {
								_set_error("Index out of range (0-1)");
								return NULL;
							}
						} else {
							_set_error("Only integer constants are allowed as index at the moment");
							return NULL;
						}

						switch (expr->get_datatype()) {
							case TYPE_BVEC2: member_type = TYPE_BOOL; break;
							case TYPE_VEC2: member_type = TYPE_FLOAT; break;
							case TYPE_IVEC2: member_type = TYPE_INT; break;
							case TYPE_UVEC2: member_type = TYPE_UINT; break;
							case TYPE_MAT2: member_type = TYPE_VEC2; break;
							default: break;
						}

						break;
					case TYPE_BVEC3:
					case TYPE_VEC3:
					case TYPE_IVEC3:
					case TYPE_UVEC3:
					case TYPE_MAT3:
						if (index->type == Node::TYPE_CONSTANT) {
							uint32_t index_constant = static_cast<ConstantNode *>(index)->values[0].uint;
							if (index_constant >= 3) {
								_set_error("Index out of range (0-2)");
								return NULL;
							}
						} else {
							_set_error("Only integer constants are allowed as index at the moment");
							return NULL;
						}

						switch (expr->get_datatype()) {
							case TYPE_BVEC3: member_type = TYPE_BOOL; break;
							case TYPE_VEC3: member_type = TYPE_FLOAT; break;
							case TYPE_IVEC3: member_type = TYPE_INT; break;
							case TYPE_UVEC3: member_type = TYPE_UINT; break;
							case TYPE_MAT3: member_type = TYPE_VEC3; break;
							default: break;
						}
						break;
					case TYPE_BVEC4:
					case TYPE_VEC4:
					case TYPE_IVEC4:
					case TYPE_UVEC4:
					case TYPE_MAT4:
						if (index->type == Node::TYPE_CONSTANT) {
							uint32_t index_constant = static_cast<ConstantNode *>(index)->values[0].uint;
							if (index_constant >= 4) {
								_set_error("Index out of range (0-3)");
								return NULL;
							}
						} else {
							_set_error("Only integer constants are allowed as index at the moment");
							return NULL;
						}

						switch (expr->get_datatype()) {
							case TYPE_BVEC4: member_type = TYPE_BOOL; break;
							case TYPE_VEC4: member_type = TYPE_FLOAT; break;
							case TYPE_IVEC4: member_type = TYPE_INT; break;
							case TYPE_UVEC4: member_type = TYPE_UINT; break;
							case TYPE_MAT4: member_type = TYPE_VEC4; break;
							default: break;
						}
						break;
					default: {
						_set_error("Object of type '" + get_datatype_name(expr->get_datatype()) + "' can't be indexed");
						return NULL;
					}
				}

				OperatorNode *op = alloc_node<OperatorNode>();
				op->op = OP_INDEX;
				op->return_cache = member_type;
				op->arguments.push_back(expr);
				op->arguments.push_back(index);
				expr = op;

				tk = _get_token();
				if (tk.type != TK_BRACKET_CLOSE) {
					_set_error("Expected ']' after indexing expression");
					return NULL;
				}

			} else if (tk.type == TK_OP_INCREMENT || tk.type == TK_OP_DECREMENT) {

				OperatorNode *op = alloc_node<OperatorNode>();
				op->op = tk.type == TK_OP_DECREMENT ? OP_POST_DECREMENT : OP_POST_INCREMENT;
				op->arguments.push_back(expr);

				if (!_validate_operator(op, &op->return_cache)) {
					_set_error("Invalid base type for increment/decrement operator");
					return NULL;
				}

				if (!_validate_assign(expr, p_builtin_types)) {
					_set_error("Invalid use of increment/decrement operator in constant expression.");
					return NULL;
				}
				expr = op;
			} else {

				_set_tkpos(pos2);
				break;
			}
		}

		Expression e;
		e.is_op = false;
		e.node = expr;
		expression.push_back(e);

		pos = _get_tkpos();
		tk = _get_token();

		if (is_token_operator(tk.type)) {

			Expression o;
			o.is_op = true;

			switch (tk.type) {

				case TK_OP_EQUAL: o.op = OP_EQUAL; break;
				case TK_OP_NOT_EQUAL: o.op = OP_NOT_EQUAL; break;
				case TK_OP_LESS: o.op = OP_LESS; break;
				case TK_OP_LESS_EQUAL: o.op = OP_LESS_EQUAL; break;
				case TK_OP_GREATER: o.op = OP_GREATER; break;
				case TK_OP_GREATER_EQUAL: o.op = OP_GREATER_EQUAL; break;
				case TK_OP_AND: o.op = OP_AND; break;
				case TK_OP_OR: o.op = OP_OR; break;
				case TK_OP_ADD: o.op = OP_ADD; break;
				case TK_OP_SUB: o.op = OP_SUB; break;
				case TK_OP_MUL: o.op = OP_MUL; break;
				case TK_OP_DIV: o.op = OP_DIV; break;
				case TK_OP_MOD: o.op = OP_MOD; break;
				case TK_OP_SHIFT_LEFT: o.op = OP_SHIFT_LEFT; break;
				case TK_OP_SHIFT_RIGHT: o.op = OP_SHIFT_RIGHT; break;
				case TK_OP_ASSIGN: o.op = OP_ASSIGN; break;
				case TK_OP_ASSIGN_ADD: o.op = OP_ASSIGN_ADD; break;
				case TK_OP_ASSIGN_SUB: o.op = OP_ASSIGN_SUB; break;
				case TK_OP_ASSIGN_MUL: o.op = OP_ASSIGN_MUL; break;
				case TK_OP_ASSIGN_DIV: o.op = OP_ASSIGN_DIV; break;
				case TK_OP_ASSIGN_MOD: o.op = OP_ASSIGN_MOD; break;
				case TK_OP_ASSIGN_SHIFT_LEFT: o.op = OP_ASSIGN_SHIFT_LEFT; break;
				case TK_OP_ASSIGN_SHIFT_RIGHT: o.op = OP_ASSIGN_SHIFT_RIGHT; break;
				case TK_OP_ASSIGN_BIT_AND: o.op = OP_ASSIGN_BIT_AND; break;
				case TK_OP_ASSIGN_BIT_OR: o.op = OP_ASSIGN_BIT_OR; break;
				case TK_OP_ASSIGN_BIT_XOR: o.op = OP_ASSIGN_BIT_XOR; break;
				case TK_OP_BIT_AND: o.op = OP_BIT_AND; break;
				case TK_OP_BIT_OR: o.op = OP_BIT_OR; break;
				case TK_OP_BIT_XOR: o.op = OP_BIT_XOR; break;
				case TK_QUESTION: o.op = OP_SELECT_IF; break;
				case TK_COLON: o.op = OP_SELECT_ELSE; break;
				default: {
					_set_error("Invalid token for operator: " + get_token_text(tk));
					return NULL;
				}
			}

			expression.push_back(o);

		} else {
			_set_tkpos(pos); //something else, so rollback and end
			break;
		}
	}

	/* Reduce the set set of expressions and place them in an operator tree, respecting precedence */

	while (expression.size() > 1) {

		int next_op = -1;
		int min_priority = 0xFFFFF;
		bool is_unary = false;
		bool is_ternary = false;

		for (int i = 0; i < expression.size(); i++) {

			if (!expression[i].is_op) {

				continue;
			}

			bool unary = false;
			bool ternary = false;

			int priority;
			switch (expression[i].op) {
				case OP_EQUAL: priority = 8; break;
				case OP_NOT_EQUAL: priority = 8; break;
				case OP_LESS: priority = 7; break;
				case OP_LESS_EQUAL: priority = 7; break;
				case OP_GREATER: priority = 7; break;
				case OP_GREATER_EQUAL: priority = 7; break;
				case OP_AND: priority = 12; break;
				case OP_OR: priority = 14; break;
				case OP_NOT:
					priority = 3;
					unary = true;
					break;
				case OP_NEGATE:
					priority = 3;
					unary = true;
					break;
				case OP_ADD: priority = 5; break;
				case OP_SUB: priority = 5; break;
				case OP_MUL: priority = 4; break;
				case OP_DIV: priority = 4; break;
				case OP_MOD: priority = 4; break;
				case OP_SHIFT_LEFT: priority = 6; break;
				case OP_SHIFT_RIGHT: priority = 6; break;
				case OP_ASSIGN: priority = 16; break;
				case OP_ASSIGN_ADD: priority = 16; break;
				case OP_ASSIGN_SUB: priority = 16; break;
				case OP_ASSIGN_MUL: priority = 16; break;
				case OP_ASSIGN_DIV: priority = 16; break;
				case OP_ASSIGN_MOD: priority = 16; break;
				case OP_ASSIGN_SHIFT_LEFT: priority = 16; break;
				case OP_ASSIGN_SHIFT_RIGHT: priority = 16; break;
				case OP_ASSIGN_BIT_AND: priority = 16; break;
				case OP_ASSIGN_BIT_OR: priority = 16; break;
				case OP_ASSIGN_BIT_XOR: priority = 16; break;
				case OP_BIT_AND: priority = 9; break;
				case OP_BIT_OR: priority = 11; break;
				case OP_BIT_XOR: priority = 10; break;
				case OP_BIT_INVERT:
					priority = 3;
					unary = true;
					break;
				case OP_INCREMENT:
					priority = 3;
					unary = true;
					break;
				case OP_DECREMENT:
					priority = 3;
					unary = true;
					break;
				case OP_SELECT_IF:
					priority = 15;
					ternary = true;
					break;
				case OP_SELECT_ELSE:
					priority = 15;
					ternary = true;
					break;

				default:
					ERR_FAIL_V(NULL); //unexpected operator
			}

			if (priority < min_priority) {
				// < is used for left to right (default)
				// <= is used for right to left
				next_op = i;
				min_priority = priority;
				is_unary = unary;
				is_ternary = ternary;
			}
		}

		ERR_FAIL_COND_V(next_op == -1, NULL);

		// OK! create operator..
		// OK! create operator..
		if (is_unary) {

			int expr_pos = next_op;
			while (expression[expr_pos].is_op) {

				expr_pos++;
				if (expr_pos == expression.size()) {
					//can happen..
					_set_error("Unexpected end of expression...");
					return NULL;
				}
			}

			//consecutively do unary opeators
			for (int i = expr_pos - 1; i >= next_op; i--) {

				OperatorNode *op = alloc_node<OperatorNode>();
				op->op = expression[i].op;
				if ((op->op == OP_INCREMENT || op->op == OP_DECREMENT) && !_validate_assign(expression[i + 1].node, p_builtin_types)) {

					_set_error("Can't use increment/decrement operator in constant expression.");
					return NULL;
				}
				op->arguments.push_back(expression[i + 1].node);

				expression.write[i].is_op = false;
				expression.write[i].node = op;

				if (!_validate_operator(op, &op->return_cache)) {

					String at;
					for (int j = 0; j < op->arguments.size(); j++) {
						if (j > 0)
							at += " and ";
						at += get_datatype_name(op->arguments[j]->get_datatype());
					}
					_set_error("Invalid arguments to unary operator '" + get_operator_text(op->op) + "' :" + at);
					return NULL;
				}
				expression.remove(i + 1);
			}

		} else if (is_ternary) {

			if (next_op < 1 || next_op >= (expression.size() - 1)) {
				_set_error("Parser bug...");
				ERR_FAIL_V(NULL);
			}

			if (next_op + 2 >= expression.size() || !expression[next_op + 2].is_op || expression[next_op + 2].op != OP_SELECT_ELSE) {
				_set_error("Missing matching ':' for select operator");
				return NULL;
			}

			OperatorNode *op = alloc_node<OperatorNode>();
			op->op = expression[next_op].op;
			op->arguments.push_back(expression[next_op - 1].node);
			op->arguments.push_back(expression[next_op + 1].node);
			op->arguments.push_back(expression[next_op + 3].node);

			expression.write[next_op - 1].is_op = false;
			expression.write[next_op - 1].node = op;
			if (!_validate_operator(op, &op->return_cache)) {

				String at;
				for (int i = 0; i < op->arguments.size(); i++) {
					if (i > 0)
						at += " and ";
					at += get_datatype_name(op->arguments[i]->get_datatype());
				}
				_set_error("Invalid argument to ternary ?: operator: " + at);
				return NULL;
			}

			for (int i = 0; i < 4; i++) {
				expression.remove(next_op);
			}

		} else {

			if (next_op < 1 || next_op >= (expression.size() - 1)) {
				_set_error("Parser bug...");
				ERR_FAIL_V(NULL);
			}

			OperatorNode *op = alloc_node<OperatorNode>();
			op->op = expression[next_op].op;

			if (expression[next_op - 1].is_op) {

				_set_error("Parser bug...");
				ERR_FAIL_V(NULL);
			}

			if (_is_operator_assign(op->op)) {

				String assign_message;
				if (!_validate_assign(expression[next_op - 1].node, p_builtin_types, &assign_message)) {

					_set_error(assign_message);
					return NULL;
				}
			}

			if (expression[next_op + 1].is_op) {
				// this is not invalid and can really appear
				// but it becomes invalid anyway because no binary op
				// can be followed by a unary op in a valid combination,
				// due to how precedence works, unaries will always disappear first

				_set_error("Parser bug...");
			}

			op->arguments.push_back(expression[next_op - 1].node); //expression goes as left
			op->arguments.push_back(expression[next_op + 1].node); //next expression goes as right
			expression.write[next_op - 1].node = op;

			//replace all 3 nodes by this operator and make it an expression

			if (!_validate_operator(op, &op->return_cache)) {

				String at;
				for (int i = 0; i < op->arguments.size(); i++) {
					if (i > 0)
						at += " and ";
					at += get_datatype_name(op->arguments[i]->get_datatype());
				}
				_set_error("Invalid arguments to operator '" + get_operator_text(op->op) + "' :" + at);
				return NULL;
			}

			expression.remove(next_op);
			expression.remove(next_op);
		}
	}

	return expression[0].node;
}

ShaderLanguage::Node *ShaderLanguage::_reduce_expression(BlockNode *p_block, ShaderLanguage::Node *p_node) {

	if (p_node->type != Node::TYPE_OPERATOR)
		return p_node;

	//for now only reduce simple constructors
	OperatorNode *op = static_cast<OperatorNode *>(p_node);

	if (op->op == OP_CONSTRUCT) {

		ERR_FAIL_COND_V(op->arguments[0]->type != Node::TYPE_VARIABLE, p_node);

		DataType type = op->get_datatype();
		DataType base = get_scalar_type(type);
		int cardinality = get_cardinality(type);

		Vector<ConstantNode::Value> values;

		for (int i = 1; i < op->arguments.size(); i++) {

			op->arguments.write[i] = _reduce_expression(p_block, op->arguments[i]);
			if (op->arguments[i]->type == Node::TYPE_CONSTANT) {
				ConstantNode *cn = static_cast<ConstantNode *>(op->arguments[i]);

				if (get_scalar_type(cn->datatype) == base) {
					for (int j = 0; j < cn->values.size(); j++) {
						values.push_back(cn->values[j]);
					}
				} else if (get_scalar_type(cn->datatype) == cn->datatype) {

					ConstantNode::Value v;
					if (!convert_constant(cn, base, &v)) {
						return p_node;
					}
					values.push_back(v);
				} else {
					return p_node;
				}

			} else {
				return p_node;
			}
		}

		if (values.size() == 1) {
			if (type >= TYPE_MAT2 && type <= TYPE_MAT4) {
				ConstantNode::Value value = values[0];
				ConstantNode::Value zero;
				zero.real = 0.0f;
				int size = 2 + (type - TYPE_MAT2);

				values.clear();
				for (int i = 0; i < size; i++) {
					for (int j = 0; j < size; j++) {
						values.push_back(i == j ? value : zero);
					}
				}
			} else {
				ConstantNode::Value value = values[0];
				for (int i = 1; i < cardinality; i++) {
					values.push_back(value);
				}
			}
		} else if (values.size() != cardinality) {
			ERR_PRINT("Failed to reduce expression, values and cardinality mismatch.");
			return p_node;
		}

		ConstantNode *cn = alloc_node<ConstantNode>();
		cn->datatype = op->get_datatype();
		cn->values = values;
		return cn;
	} else if (op->op == OP_NEGATE) {

		op->arguments.write[0] = _reduce_expression(p_block, op->arguments[0]);
		if (op->arguments[0]->type == Node::TYPE_CONSTANT) {

			ConstantNode *cn = static_cast<ConstantNode *>(op->arguments[0]);

			DataType base = get_scalar_type(cn->datatype);

			Vector<ConstantNode::Value> values;

			for (int i = 0; i < cn->values.size(); i++) {

				ConstantNode::Value nv;
				switch (base) {
					case TYPE_BOOL: {
						nv.boolean = !cn->values[i].boolean;
					} break;
					case TYPE_INT: {
						nv.sint = -cn->values[i].sint;
					} break;
					case TYPE_UINT: {
						// FIXME: This can't work on uint
						nv.uint = -cn->values[i].uint;
					} break;
					case TYPE_FLOAT: {
						nv.real = -cn->values[i].real;
					} break;
					default: {
					}
				}

				values.push_back(nv);
			}

			cn->values = values;
			return cn;
		}
	}

	return p_node;
}

ShaderLanguage::Node *ShaderLanguage::_parse_and_reduce_expression(BlockNode *p_block, const Map<StringName, BuiltInInfo> &p_builtin_types) {

	ShaderLanguage::Node *expr = _parse_expression(p_block, p_builtin_types);
	if (!expr) //errored
		return NULL;

	expr = _reduce_expression(p_block, expr);

	return expr;
}

Error ShaderLanguage::_parse_block(BlockNode *p_block, const Map<StringName, BuiltInInfo> &p_builtin_types, bool p_just_one, bool p_can_break, bool p_can_continue) {

	while (true) {

		TkPos pos = _get_tkpos();

		Token tk = _get_token();
		if (tk.type == TK_CURLY_BRACKET_CLOSE) { //end of block
			if (p_just_one) {
				_set_error("Unexpected '}'");
				return ERR_PARSE_ERROR;
			}

			return OK;

		} else if (is_token_precision(tk.type) || is_token_nonvoid_datatype(tk.type)) {
			DataPrecision precision = PRECISION_DEFAULT;
			if (is_token_precision(tk.type)) {
				precision = get_token_precision(tk.type);
				tk = _get_token();
				if (!is_token_nonvoid_datatype(tk.type)) {
					_set_error("Expected datatype after precision");
					return ERR_PARSE_ERROR;
				}
			}

			if (!is_token_variable_datatype(tk.type)) {
				_set_error("Invalid data type for variable (samplers not allowed)");
				return ERR_PARSE_ERROR;
			}

			DataType type = get_token_datatype(tk.type);

			tk = _get_token();

			Node *vardecl = NULL;

			while (true) {

				if (tk.type != TK_IDENTIFIER) {
					_set_error("Expected identifier after type");
					return ERR_PARSE_ERROR;
				}

				StringName name = tk.text;
				if (_find_identifier(p_block, p_builtin_types, name)) {
					_set_error("Redefinition of '" + String(name) + "'");
					return ERR_PARSE_ERROR;
				}

				BlockNode::Variable var;
				var.type = type;
				var.precision = precision;
				var.line = tk_line;
				var.array_size = 0;

				tk = _get_token();

				if (tk.type == TK_BRACKET_OPEN) {
					bool unknown_size = false;

					ArrayDeclarationNode *node = alloc_node<ArrayDeclarationNode>();
					node->datatype = type;
					node->precision = precision;
					vardecl = (Node *)node;

					ArrayDeclarationNode::Declaration decl;
					decl.name = name;
					decl.size = 0U;

					tk = _get_token();

					if (tk.type == TK_BRACKET_CLOSE) {
						unknown_size = true;
					} else {

						if (tk.type != TK_INT_CONSTANT || ((int)tk.constant) <= 0) {
							_set_error("Expected integer constant > 0 or ']'");
							return ERR_PARSE_ERROR;
						}

						tk = _get_token();

						if (tk.type != TK_BRACKET_CLOSE) {
							_set_error("Expected ']'");
							return ERR_PARSE_ERROR;
						}

						decl.size = ((uint32_t)tk.constant);
						var.array_size = decl.size;
					}

					bool full_def = false;

					tk = _get_token();
					if (tk.type == TK_OP_ASSIGN) {
						tk = _get_token();

						if (tk.type != TK_CURLY_BRACKET_OPEN) {

							if (unknown_size) {
								_set_error("Expected '{'");
								return ERR_PARSE_ERROR;
							}

							full_def = true;

							DataPrecision precision2 = PRECISION_DEFAULT;
							if (is_token_precision(tk.type)) {
								precision2 = get_token_precision(tk.type);
								tk = _get_token();
								if (!is_token_nonvoid_datatype(tk.type)) {
									_set_error("Expected datatype after precision");
									return ERR_PARSE_ERROR;
								}
							}
							if (!is_token_variable_datatype(tk.type)) {
								_set_error("Invalid data type for array");
								return ERR_PARSE_ERROR;
							}
							DataType type2 = get_token_datatype(tk.type);

							int array_size2 = 0;

							tk = _get_token();
							if (tk.type == TK_BRACKET_OPEN) {
								Node *n = _parse_and_reduce_expression(p_block, p_builtin_types);
								if (!n || n->type != Node::TYPE_CONSTANT || n->get_datatype() != TYPE_INT) {
									_set_error("Expected single integer constant > 0");
									return ERR_PARSE_ERROR;
								}

								ConstantNode *cnode = (ConstantNode *)n;
								if (cnode->values.size() == 1) {
									array_size2 = cnode->values[0].sint;
									if (array_size2 <= 0) {
										_set_error("Expected single integer constant > 0");
										return ERR_PARSE_ERROR;
									}
								} else {
									_set_error("Expected single integer constant > 0");
									return ERR_PARSE_ERROR;
								}

								tk = _get_token();
								if (tk.type != TK_BRACKET_CLOSE) {
									_set_error("Expected ']");
									return ERR_PARSE_ERROR;
								} else {
									tk = _get_token();
								}
							} else {
								_set_error("Expected '[");
								return ERR_PARSE_ERROR;
							}

							if (precision != precision2 || type != type2 || var.array_size != array_size2) {
								String error_str = "Cannot convert from '";
								if (precision2 != PRECISION_DEFAULT) {
									error_str += get_precision_name(precision2);
									error_str += " ";
								}
								error_str += get_datatype_name(type2);
								error_str += "[";
								error_str += itos(array_size2);
								error_str += "]'";
								error_str += " to '";
								if (precision != PRECISION_DEFAULT) {
									error_str += get_precision_name(precision);
									error_str += " ";
								}
								error_str += get_datatype_name(type);
								error_str += "[";
								error_str += itos(var.array_size);
								error_str += "]'";
								_set_error(error_str);
								return ERR_PARSE_ERROR;
							}
						}

						bool curly = tk.type == TK_CURLY_BRACKET_OPEN;

						if (unknown_size) {
							if (!curly) {
								_set_error("Expected '{'");
								return ERR_PARSE_ERROR;
							}
						} else {
							if (full_def) {
								if (curly) {
									_set_error("Expected '('");
									return ERR_PARSE_ERROR;
								}
							}
						}

						if (tk.type == TK_PARENTHESIS_OPEN || curly) { // initialization
							while (true) {

								Node *n = _parse_and_reduce_expression(p_block, p_builtin_types);
								if (!n) {
									return ERR_PARSE_ERROR;
								}

								if (var.type != n->get_datatype()) {
									_set_error("Invalid assignment of '" + get_datatype_name(n->get_datatype()) + "' to '" + get_datatype_name(var.type) + "'");
									return ERR_PARSE_ERROR;
								}

								tk = _get_token();
								if (tk.type == TK_COMMA) {
									decl.initializer.push_back(n);
									continue;
								} else if (!curly && tk.type == TK_PARENTHESIS_CLOSE) {
									decl.initializer.push_back(n);
									break;
								} else if (curly && tk.type == TK_CURLY_BRACKET_CLOSE) {
									decl.initializer.push_back(n);
									break;
								} else {
									if (curly)
										_set_error("Expected '}' or ','");
									else
										_set_error("Expected ')' or ','");
									return ERR_PARSE_ERROR;
								}
							}
							if (unknown_size) {
								decl.size = decl.initializer.size();
								var.array_size = decl.initializer.size();
							} else if (decl.initializer.size() != var.array_size) {
								_set_error("Array size mismatch");
								return ERR_PARSE_ERROR;
							}
							tk = _get_token();
						}
					} else {
						if (unknown_size) {
							_set_error("Expected array initialization");
							return ERR_PARSE_ERROR;
						}
					}

					node->declarations.push_back(decl);
				} else if (tk.type == TK_OP_ASSIGN) {

					VariableDeclarationNode *node = alloc_node<VariableDeclarationNode>();
					node->datatype = type;
					node->precision = precision;
					vardecl = (Node *)node;

					VariableDeclarationNode::Declaration decl;
					decl.name = name;
					decl.initializer = NULL;

					//variable created with assignment! must parse an expression
					Node *n = _parse_and_reduce_expression(p_block, p_builtin_types);
					if (!n)
						return ERR_PARSE_ERROR;

					decl.initializer = n;

					if (var.type != n->get_datatype()) {
						_set_error("Invalid assignment of '" + get_datatype_name(n->get_datatype()) + "' to '" + get_datatype_name(var.type) + "'");
						return ERR_PARSE_ERROR;
					}
					tk = _get_token();
					node->declarations.push_back(decl);
				} else {
					VariableDeclarationNode *node = alloc_node<VariableDeclarationNode>();
					node->datatype = type;
					node->precision = precision;
					vardecl = (Node *)node;

					VariableDeclarationNode::Declaration decl;
					decl.name = name;
					decl.initializer = NULL;
					node->declarations.push_back(decl);
				}

				p_block->statements.push_back(vardecl);

				p_block->variables[name] = var;

				if (tk.type == TK_COMMA) {
					tk = _get_token();
					//another variable
				} else if (tk.type == TK_SEMICOLON) {
					break;
				} else {
					_set_error("Expected ',' or ';' after variable");
					return ERR_PARSE_ERROR;
				}
			}
		} else if (tk.type == TK_CURLY_BRACKET_OPEN) {
			//a sub block, just because..
			BlockNode *block = alloc_node<BlockNode>();
			block->parent_block = p_block;
			_parse_block(block, p_builtin_types, false, p_can_break, p_can_continue);
			p_block->statements.push_back(block);
		} else if (tk.type == TK_CF_IF) {
			//if () {}
			tk = _get_token();
			if (tk.type != TK_PARENTHESIS_OPEN) {
				_set_error("Expected '(' after if");
				return ERR_PARSE_ERROR;
			}

			ControlFlowNode *cf = alloc_node<ControlFlowNode>();
			cf->flow_op = FLOW_OP_IF;
			Node *n = _parse_and_reduce_expression(p_block, p_builtin_types);
			if (!n)
				return ERR_PARSE_ERROR;

			tk = _get_token();
			if (tk.type != TK_PARENTHESIS_CLOSE) {
				_set_error("Expected ')' after expression");
				return ERR_PARSE_ERROR;
			}

			BlockNode *block = alloc_node<BlockNode>();
			block->parent_block = p_block;
			cf->expressions.push_back(n);
			cf->blocks.push_back(block);
			p_block->statements.push_back(cf);

			Error err = _parse_block(block, p_builtin_types, true, p_can_break, p_can_continue);
			if (err)
				return err;

			pos = _get_tkpos();
			tk = _get_token();
			if (tk.type == TK_CF_ELSE) {

				block = alloc_node<BlockNode>();
				block->parent_block = p_block;
				cf->blocks.push_back(block);
				err = _parse_block(block, p_builtin_types, true, p_can_break, p_can_continue);

			} else {
				_set_tkpos(pos); //rollback
			}
		} else if (tk.type == TK_CF_WHILE) {
			//if () {}
			tk = _get_token();
			if (tk.type != TK_PARENTHESIS_OPEN) {
				_set_error("Expected '(' after while");
				return ERR_PARSE_ERROR;
			}

			ControlFlowNode *cf = alloc_node<ControlFlowNode>();
			cf->flow_op = FLOW_OP_WHILE;
			Node *n = _parse_and_reduce_expression(p_block, p_builtin_types);
			if (!n)
				return ERR_PARSE_ERROR;

			tk = _get_token();
			if (tk.type != TK_PARENTHESIS_CLOSE) {
				_set_error("Expected ')' after expression");
				return ERR_PARSE_ERROR;
			}

			BlockNode *block = alloc_node<BlockNode>();
			block->parent_block = p_block;
			cf->expressions.push_back(n);
			cf->blocks.push_back(block);
			p_block->statements.push_back(cf);

			Error err = _parse_block(block, p_builtin_types, true, true, true);
			if (err)
				return err;
		} else if (tk.type == TK_CF_FOR) {
			//if () {}
			tk = _get_token();
			if (tk.type != TK_PARENTHESIS_OPEN) {
				_set_error("Expected '(' after for");
				return ERR_PARSE_ERROR;
			}

			ControlFlowNode *cf = alloc_node<ControlFlowNode>();
			cf->flow_op = FLOW_OP_FOR;

			BlockNode *init_block = alloc_node<BlockNode>();
			init_block->parent_block = p_block;
			init_block->single_statement = true;
			cf->blocks.push_back(init_block);
			if (_parse_block(init_block, p_builtin_types, true, false, false) != OK) {
				return ERR_PARSE_ERROR;
			}

			Node *n = _parse_and_reduce_expression(init_block, p_builtin_types);
			if (!n)
				return ERR_PARSE_ERROR;

			if (n->get_datatype() != TYPE_BOOL) {
				_set_error("Middle expression is expected to be boolean.");
				return ERR_PARSE_ERROR;
			}

			tk = _get_token();
			if (tk.type != TK_SEMICOLON) {
				_set_error("Expected ';' after middle expression");
				return ERR_PARSE_ERROR;
			}

			cf->expressions.push_back(n);

			n = _parse_and_reduce_expression(init_block, p_builtin_types);
			if (!n)
				return ERR_PARSE_ERROR;

			cf->expressions.push_back(n);

			tk = _get_token();
			if (tk.type != TK_PARENTHESIS_CLOSE) {
				_set_error("Expected ')' after third expression");
				return ERR_PARSE_ERROR;
			}

			BlockNode *block = alloc_node<BlockNode>();
			block->parent_block = init_block;
			cf->blocks.push_back(block);
			p_block->statements.push_back(cf);

			Error err = _parse_block(block, p_builtin_types, true, true, true);
			if (err)
				return err;

		} else if (tk.type == TK_CF_RETURN) {

			//check return type
			BlockNode *b = p_block;
			while (b && !b->parent_function) {
				b = b->parent_block;
			}

			if (!b) {
				_set_error("Bug");
				return ERR_BUG;
			}

			ControlFlowNode *flow = alloc_node<ControlFlowNode>();
			flow->flow_op = FLOW_OP_RETURN;

			pos = _get_tkpos();
			tk = _get_token();
			if (tk.type == TK_SEMICOLON) {
				//all is good
				if (b->parent_function->return_type != TYPE_VOID) {
					_set_error("Expected return with expression of type '" + get_datatype_name(b->parent_function->return_type) + "'");
					return ERR_PARSE_ERROR;
				}
			} else {
				_set_tkpos(pos); //rollback, wants expression
				Node *expr = _parse_and_reduce_expression(p_block, p_builtin_types);
				if (!expr)
					return ERR_PARSE_ERROR;

				if (b->parent_function->return_type != expr->get_datatype()) {
					_set_error("Expected return expression of type '" + get_datatype_name(b->parent_function->return_type) + "'");
					return ERR_PARSE_ERROR;
				}

				tk = _get_token();
				if (tk.type != TK_SEMICOLON) {
					_set_error("Expected ';' after return expression");
					return ERR_PARSE_ERROR;
				}

				flow->expressions.push_back(expr);
			}

			p_block->statements.push_back(flow);
		} else if (tk.type == TK_CF_DISCARD) {

			//check return type
			BlockNode *b = p_block;
			while (b && !b->parent_function) {
				b = b->parent_block;
			}
			if (!b) {
				_set_error("Bug");
				return ERR_BUG;
			}

			if (!b->parent_function->can_discard) {
				_set_error("Use of 'discard' is not allowed here.");
				return ERR_PARSE_ERROR;
			}

			ControlFlowNode *flow = alloc_node<ControlFlowNode>();
			flow->flow_op = FLOW_OP_DISCARD;

			pos = _get_tkpos();
			tk = _get_token();
			if (tk.type != TK_SEMICOLON) {
				//all is good
				_set_error("Expected ';' after discard");
			}

			p_block->statements.push_back(flow);
		} else if (tk.type == TK_CF_BREAK) {

			if (!p_can_break) {
				//all is good
				_set_error("Breaking is not allowed here");
			}

			ControlFlowNode *flow = alloc_node<ControlFlowNode>();
			flow->flow_op = FLOW_OP_BREAK;

			pos = _get_tkpos();
			tk = _get_token();
			if (tk.type != TK_SEMICOLON) {
				//all is good
				_set_error("Expected ';' after break");
			}

			p_block->statements.push_back(flow);
		} else if (tk.type == TK_CF_CONTINUE) {

			if (!p_can_break) {
				//all is good
				_set_error("Continuing is not allowed here");
			}

			ControlFlowNode *flow = alloc_node<ControlFlowNode>();
			flow->flow_op = FLOW_OP_CONTINUE;

			pos = _get_tkpos();
			tk = _get_token();
			if (tk.type != TK_SEMICOLON) {
				//all is good
				_set_error("Expected ';' after continue");
			}

			p_block->statements.push_back(flow);

		} else {

			//nothing else, so expression
			_set_tkpos(pos); //rollback
			Node *expr = _parse_and_reduce_expression(p_block, p_builtin_types);
			if (!expr)
				return ERR_PARSE_ERROR;
			p_block->statements.push_back(expr);
			tk = _get_token();

			if (tk.type != TK_SEMICOLON) {
				_set_error("Expected ';' after statement");
				return ERR_PARSE_ERROR;
			}
		}

		if (p_just_one)
			break;
	}

	return OK;
}

Error ShaderLanguage::_parse_shader(const Map<StringName, FunctionInfo> &p_functions, const Vector<StringName> &p_render_modes, const Set<String> &p_shader_types) {

	Token tk = _get_token();

	if (tk.type != TK_SHADER_TYPE) {
		_set_error("Expected 'shader_type' at the beginning of shader.");
		return ERR_PARSE_ERROR;
	}

	tk = _get_token();

	if (tk.type != TK_IDENTIFIER) {
		_set_error("Expected identifier after 'shader_type', indicating type of shader.");
		return ERR_PARSE_ERROR;
	}

	String shader_type_identifier;

	shader_type_identifier = tk.text;

	if (!p_shader_types.has(shader_type_identifier)) {

		String valid;
		for (Set<String>::Element *E = p_shader_types.front(); E; E = E->next()) {
			if (valid != String()) {
				valid += ", ";
			}
			valid += "'" + E->get() + "'";
		}
		_set_error("Invalid shader type, valid types are: " + valid);
		return ERR_PARSE_ERROR;
	}

	tk = _get_token();

	if (tk.type != TK_SEMICOLON) {
		_set_error("Expected ';' after 'shader_type <type>'.");
	}

	tk = _get_token();

	int texture_uniforms = 0;
	int uniforms = 0;

	while (tk.type != TK_EOF) {

		switch (tk.type) {
			case TK_RENDER_MODE: {

				while (true) {

					StringName mode;
					_get_completable_identifier(NULL, COMPLETION_RENDER_MODE, mode);

					if (mode == StringName()) {
						_set_error("Expected identifier for render mode");
						return ERR_PARSE_ERROR;
					}

					if (p_render_modes.find(mode) == -1) {
						_set_error("Invalid render mode: '" + String(mode) + "'");
						return ERR_PARSE_ERROR;
					}

					if (shader->render_modes.find(mode) != -1) {
						_set_error("Duplicate render mode: '" + String(mode) + "'");
						return ERR_PARSE_ERROR;
					}

					shader->render_modes.push_back(mode);

					tk = _get_token();
					if (tk.type == TK_COMMA) {
						//all good, do nothing
					} else if (tk.type == TK_SEMICOLON) {
						break; //done
					} else {
						_set_error("Unexpected token: " + get_token_text(tk));
						return ERR_PARSE_ERROR;
					}
				}
			} break;
			case TK_UNIFORM:
			case TK_VARYING: {

				bool uniform = tk.type == TK_UNIFORM;
				DataPrecision precision = PRECISION_DEFAULT;
				DataInterpolation interpolation = INTERPOLATION_SMOOTH;
				DataType type;
				StringName name;

				tk = _get_token();
				if (is_token_interpolation(tk.type)) {
					interpolation = get_token_interpolation(tk.type);
					tk = _get_token();
				}

				if (is_token_precision(tk.type)) {
					precision = get_token_precision(tk.type);
					tk = _get_token();
				}

				if (!is_token_datatype(tk.type)) {
					_set_error("Expected datatype. ");
					return ERR_PARSE_ERROR;
				}

				type = get_token_datatype(tk.type);

				if (type == TYPE_VOID) {
					_set_error("void datatype not allowed here");
					return ERR_PARSE_ERROR;
				}

				if (!uniform && (type < TYPE_FLOAT || type > TYPE_MAT4)) {
					_set_error("Invalid type for varying, only float,vec2,vec3,vec4,mat2,mat3,mat4 allowed.");
					return ERR_PARSE_ERROR;
				}

				tk = _get_token();
				if (tk.type != TK_IDENTIFIER) {
					_set_error("Expected identifier!");
					return ERR_PARSE_ERROR;
				}

				name = tk.text;

				if (_find_identifier(NULL, Map<StringName, BuiltInInfo>(), name)) {
					_set_error("Redefinition of '" + String(name) + "'");
					return ERR_PARSE_ERROR;
				}

				if (uniform) {

					ShaderNode::Uniform uniform2;

					if (is_sampler_type(type)) {
						uniform2.texture_order = texture_uniforms++;
						uniform2.order = -1;
					} else {
						uniform2.texture_order = -1;
						uniform2.order = uniforms++;
					}
					uniform2.type = type;
					uniform2.precision = precision;

					//todo parse default value

					tk = _get_token();

					if (tk.type == TK_COLON) {
						//hint

						tk = _get_token();
						if (tk.type == TK_HINT_WHITE_TEXTURE) {
							uniform2.hint = ShaderNode::Uniform::HINT_WHITE;
						} else if (tk.type == TK_HINT_BLACK_TEXTURE) {
							uniform2.hint = ShaderNode::Uniform::HINT_BLACK;
						} else if (tk.type == TK_HINT_NORMAL_TEXTURE) {
							uniform2.hint = ShaderNode::Uniform::HINT_NORMAL;
						} else if (tk.type == TK_HINT_ANISO_TEXTURE) {
							uniform2.hint = ShaderNode::Uniform::HINT_ANISO;
						} else if (tk.type == TK_HINT_ALBEDO_TEXTURE) {
							uniform2.hint = ShaderNode::Uniform::HINT_ALBEDO;
						} else if (tk.type == TK_HINT_BLACK_ALBEDO_TEXTURE) {
							uniform2.hint = ShaderNode::Uniform::HINT_BLACK_ALBEDO;
						} else if (tk.type == TK_HINT_COLOR) {
							if (type != TYPE_VEC4) {
								_set_error("Color hint is for vec4 only");
								return ERR_PARSE_ERROR;
							}
							uniform2.hint = ShaderNode::Uniform::HINT_COLOR;
						} else if (tk.type == TK_HINT_RANGE) {

							uniform2.hint = ShaderNode::Uniform::HINT_RANGE;
							if (type != TYPE_FLOAT && type != TYPE_INT) {
								_set_error("Range hint is for float and int only");
								return ERR_PARSE_ERROR;
							}

							tk = _get_token();
							if (tk.type != TK_PARENTHESIS_OPEN) {
								_set_error("Expected '(' after hint_range");
								return ERR_PARSE_ERROR;
							}

							tk = _get_token();

							float sign = 1.0;

							if (tk.type == TK_OP_SUB) {
								sign = -1.0;
								tk = _get_token();
							}

							if (tk.type != TK_REAL_CONSTANT && tk.type != TK_INT_CONSTANT) {
								_set_error("Expected integer constant");
								return ERR_PARSE_ERROR;
							}

							uniform2.hint_range[0] = tk.constant;
							uniform2.hint_range[0] *= sign;

							tk = _get_token();

							if (tk.type != TK_COMMA) {
								_set_error("Expected ',' after integer constant");
								return ERR_PARSE_ERROR;
							}

							tk = _get_token();

							sign = 1.0;

							if (tk.type == TK_OP_SUB) {
								sign = -1.0;
								tk = _get_token();
							}

							if (tk.type != TK_REAL_CONSTANT && tk.type != TK_INT_CONSTANT) {
								_set_error("Expected integer constant after ','");
								return ERR_PARSE_ERROR;
							}

							uniform2.hint_range[1] = tk.constant;
							uniform2.hint_range[1] *= sign;

							tk = _get_token();

							if (tk.type == TK_COMMA) {
								tk = _get_token();

								if (tk.type != TK_REAL_CONSTANT && tk.type != TK_INT_CONSTANT) {
									_set_error("Expected integer constant after ','");
									return ERR_PARSE_ERROR;
								}

								uniform2.hint_range[2] = tk.constant;
								tk = _get_token();
							} else {
								if (type == TYPE_INT) {
									uniform2.hint_range[2] = 1;
								} else {
									uniform2.hint_range[2] = 0.001;
								}
							}

							if (tk.type != TK_PARENTHESIS_CLOSE) {
								_set_error("Expected ','");
								return ERR_PARSE_ERROR;
							}

						} else {
							_set_error("Expected valid type hint after ':'.");
						}

						if (uniform2.hint != ShaderNode::Uniform::HINT_RANGE && uniform2.hint != ShaderNode::Uniform::HINT_NONE && uniform2.hint != ShaderNode::Uniform::HINT_COLOR && type <= TYPE_MAT4) {
							_set_error("This hint is only for sampler types");
							return ERR_PARSE_ERROR;
						}

						tk = _get_token();
					}

					if (tk.type == TK_OP_ASSIGN) {

						Node *expr = _parse_and_reduce_expression(NULL, Map<StringName, BuiltInInfo>());
						if (!expr)
							return ERR_PARSE_ERROR;
						if (expr->type != Node::TYPE_CONSTANT) {
							_set_error("Expected constant expression after '='");
							return ERR_PARSE_ERROR;
						}

						ConstantNode *cn = static_cast<ConstantNode *>(expr);

						uniform2.default_value.resize(cn->values.size());

						if (!convert_constant(cn, uniform2.type, uniform2.default_value.ptrw())) {
							_set_error("Can't convert constant to " + get_datatype_name(uniform2.type));
							return ERR_PARSE_ERROR;
						}
						tk = _get_token();
					}

					shader->uniforms[name] = uniform2;

					if (tk.type != TK_SEMICOLON) {
						_set_error("Expected ';'");
						return ERR_PARSE_ERROR;
					}
				} else {

					ShaderNode::Varying varying;
					varying.type = type;
					varying.precision = precision;
					varying.interpolation = interpolation;
					shader->varyings[name] = varying;

					tk = _get_token();
					if (tk.type != TK_SEMICOLON) {
						_set_error("Expected ';'");
						return ERR_PARSE_ERROR;
					}
				}

			} break;
			default: {
				//function or constant variable

				bool is_constant = false;
				DataPrecision precision = PRECISION_DEFAULT;
				DataType type;
				StringName name;

				if (tk.type == TK_CONST) {
					is_constant = true;
					tk = _get_token();
				}

				if (is_token_precision(tk.type)) {
					precision = get_token_precision(tk.type);
					tk = _get_token();
				}

				if (!is_token_datatype(tk.type)) {
					_set_error("Expected constant, function, uniform or varying ");
					return ERR_PARSE_ERROR;
				}

				if (!is_token_variable_datatype(tk.type)) {
					_set_error("Invalid data type for constants or function return (samplers not allowed)");
					return ERR_PARSE_ERROR;
				}

				type = get_token_datatype(tk.type);

				TkPos prev_pos = _get_tkpos();
				tk = _get_token();
				if (tk.type == TK_BRACKET_OPEN) {
					_set_error("Cannot use arrays as return types");
					return ERR_PARSE_ERROR;
				}
				_set_tkpos(prev_pos);

				_get_completable_identifier(NULL, COMPLETION_MAIN_FUNCTION, name);

				if (name == StringName()) {
					_set_error("Expected function name after datatype");
					return ERR_PARSE_ERROR;
				}

				if (_find_identifier(NULL, Map<StringName, BuiltInInfo>(), name)) {
					_set_error("Redefinition of '" + String(name) + "'");
					return ERR_PARSE_ERROR;
				}

				tk = _get_token();
				if (tk.type != TK_PARENTHESIS_OPEN) {
					if (type == TYPE_VOID) {
						_set_error("Expected '(' after function identifier");
						return ERR_PARSE_ERROR;
					}

					//variable

					while (true) {
						ShaderNode::Constant constant;
						constant.type = type;
						constant.precision = precision;
						constant.initializer = NULL;

						if (tk.type == TK_OP_ASSIGN) {

							if (!is_constant) {
								_set_error("Expected 'const' keyword before constant definition");
								return ERR_PARSE_ERROR;
							}

							//variable created with assignment! must parse an expression
							Node *expr = _parse_and_reduce_expression(NULL, Map<StringName, BuiltInInfo>());
							if (!expr)
								return ERR_PARSE_ERROR;

							if (expr->type != Node::TYPE_CONSTANT) {
								_set_error("Expected constant expression after '='");
								return ERR_PARSE_ERROR;
							}

							constant.initializer = static_cast<ConstantNode *>(expr);

							if (type != expr->get_datatype()) {
								_set_error("Invalid assignment of '" + get_datatype_name(expr->get_datatype()) + "' to '" + get_datatype_name(type) + "'");
								return ERR_PARSE_ERROR;
							}
							tk = _get_token();
						} else {
							_set_error("Expected initialization of constant");
							return ERR_PARSE_ERROR;
						}

						shader->constants[name] = constant;
						if (tk.type == TK_COMMA) {
							tk = _get_token();
							if (tk.type != TK_IDENTIFIER) {
								_set_error("Expected identifier after type");
								return ERR_PARSE_ERROR;
							}

							name = tk.text;
							if (_find_identifier(NULL, Map<StringName, BuiltInInfo>(), name)) {
								_set_error("Redefinition of '" + String(name) + "'");
								return ERR_PARSE_ERROR;
							}

							tk = _get_token();

						} else if (tk.type == TK_SEMICOLON) {
							break;
						} else {
							_set_error("Expected ',' or ';' after constant");
							return ERR_PARSE_ERROR;
						}
					}

					break;
				}

				Map<StringName, BuiltInInfo> builtin_types;
				if (p_functions.has(name)) {
					builtin_types = p_functions[name].built_ins;
				}

				ShaderNode::Function function;

				function.callable = !p_functions.has(name);
				function.name = name;

				FunctionNode *func_node = alloc_node<FunctionNode>();

				function.function = func_node;

				shader->functions.push_back(function);

				func_node->name = name;
				func_node->return_type = type;
				func_node->return_precision = precision;

				if (p_functions.has(name)) {
					func_node->can_discard = p_functions[name].can_discard;
				}

				func_node->body = alloc_node<BlockNode>();
				func_node->body->parent_function = func_node;

				tk = _get_token();

				while (true) {
					if (tk.type == TK_PARENTHESIS_CLOSE) {
						break;
					}

					ArgumentQualifier qualifier = ARGUMENT_QUALIFIER_IN;

					if (tk.type == TK_ARG_IN) {
						qualifier = ARGUMENT_QUALIFIER_IN;
						tk = _get_token();
					} else if (tk.type == TK_ARG_OUT) {
						qualifier = ARGUMENT_QUALIFIER_OUT;
						tk = _get_token();
					} else if (tk.type == TK_ARG_INOUT) {
						qualifier = ARGUMENT_QUALIFIER_INOUT;
						tk = _get_token();
					}

					DataType ptype;
					StringName pname;
					DataPrecision pprecision = PRECISION_DEFAULT;

					if (is_token_precision(tk.type)) {
						pprecision = get_token_precision(tk.type);
						tk = _get_token();
					}

					if (!is_token_datatype(tk.type)) {
						_set_error("Expected a valid datatype for argument");
						return ERR_PARSE_ERROR;
					}

					ptype = get_token_datatype(tk.type);

					if (ptype == TYPE_VOID) {
						_set_error("void not allowed in argument");
						return ERR_PARSE_ERROR;
					}

					tk = _get_token();

					if (tk.type == TK_BRACKET_OPEN) {
						_set_error("Arrays as parameters are not implemented yet");
						return ERR_PARSE_ERROR;
					}
					if (tk.type != TK_IDENTIFIER) {
						_set_error("Expected identifier for argument name");
						return ERR_PARSE_ERROR;
					}

					pname = tk.text;

					if (_find_identifier(func_node->body, builtin_types, pname)) {
						_set_error("Redefinition of '" + String(pname) + "'");
						return ERR_PARSE_ERROR;
					}
					FunctionNode::Argument arg;
					arg.type = ptype;
					arg.name = pname;
					arg.precision = pprecision;
					arg.qualifier = qualifier;

					func_node->arguments.push_back(arg);

					tk = _get_token();
					if (tk.type == TK_BRACKET_OPEN) {
						_set_error("Arrays as parameters are not implemented yet");
						return ERR_PARSE_ERROR;
					}

					if (tk.type == TK_COMMA) {
						tk = _get_token();
						//do none and go on
					} else if (tk.type != TK_PARENTHESIS_CLOSE) {
						_set_error("Expected ',' or ')' after identifier");
						return ERR_PARSE_ERROR;
					}
				}

				if (p_functions.has(name)) {
					//if one of the core functions, make sure they are of the correct form
					if (func_node->arguments.size() > 0) {
						_set_error("Function '" + String(name) + "' expects no arguments.");
						return ERR_PARSE_ERROR;
					}
					if (func_node->return_type != TYPE_VOID) {
						_set_error("Function '" + String(name) + "' must be of void return type.");
						return ERR_PARSE_ERROR;
					}
				}

				//all good let's parse inside the function!
				tk = _get_token();
				if (tk.type != TK_CURLY_BRACKET_OPEN) {
					_set_error("Expected '{' to begin function");
					return ERR_PARSE_ERROR;
				}

				current_function = name;

				Error err = _parse_block(func_node->body, builtin_types);
				if (err)
					return err;

				current_function = StringName();
			}
		}

		tk = _get_token();
	}

	return OK;
}

// skips over whitespace and /* */ and // comments
static int _get_first_ident_pos(const String &p_code) {

	int idx = 0;

#define GETCHAR(m_idx) (((idx + m_idx) < p_code.length()) ? p_code[idx + m_idx] : CharType(0))

	while (true) {
		if (GETCHAR(0) == '/' && GETCHAR(1) == '/') {
			idx += 2;
			while (true) {
				if (GETCHAR(0) == 0) return 0;
				if (GETCHAR(0) == '\n') {
					idx++;
					break; // loop
				}
				idx++;
			}
		} else if (GETCHAR(0) == '/' && GETCHAR(1) == '*') {
			idx += 2;
			while (true) {
				if (GETCHAR(0) == 0) return 0;
				if (GETCHAR(0) == '*' && GETCHAR(1) == '/') {
					idx += 2;
					break; // loop
				}
				idx++;
			}
		} else {
			switch (GETCHAR(0)) {
				case ' ':
				case '\t':
				case '\r':
				case '\n': {
					idx++;
				} break; // switch
				default:
					return idx;
			}
		}
	}

#undef GETCHAR
}

String ShaderLanguage::get_shader_type(const String &p_code) {

	bool reading_type = false;

	String cur_identifier;

	for (int i = _get_first_ident_pos(p_code); i < p_code.length(); i++) {

		if (p_code[i] == ';') {
			break;

		} else if (p_code[i] <= 32) {
			if (cur_identifier != String()) {
				if (!reading_type) {
					if (cur_identifier != "shader_type") {
						return String();
					}

					reading_type = true;
					cur_identifier = String();
				} else {
					return cur_identifier;
				}
			}
		} else {
			cur_identifier += String::chr(p_code[i]);
		}
	}

	if (reading_type)
		return cur_identifier;

	return String();
}

Error ShaderLanguage::compile(const String &p_code, const Map<StringName, FunctionInfo> &p_functions, const Vector<StringName> &p_render_modes, const Set<String> &p_shader_types) {

	clear();

	code = p_code;

	nodes = NULL;

	shader = alloc_node<ShaderNode>();
	Error err = _parse_shader(p_functions, p_render_modes, p_shader_types);

	if (err != OK) {
		return err;
	}
	return OK;
}

Error ShaderLanguage::complete(const String &p_code, const Map<StringName, FunctionInfo> &p_functions, const Vector<StringName> &p_render_modes, const Set<String> &p_shader_types, List<ScriptCodeCompletionOption> *r_options, String &r_call_hint) {

	clear();

	code = p_code;

	nodes = NULL;

	shader = alloc_node<ShaderNode>();
	Error err = _parse_shader(p_functions, p_render_modes, p_shader_types);
	if (err != OK)
		ERR_PRINT("Failed to parse shader");

	switch (completion_type) {

		case COMPLETION_NONE: {
			//do nothing
			return OK;
		} break;
		case COMPLETION_RENDER_MODE: {
			for (int i = 0; i < p_render_modes.size(); i++) {
				ScriptCodeCompletionOption option(p_render_modes[i], ScriptCodeCompletionOption::KIND_ENUM);
				r_options->push_back(option);
			}

			return OK;
		} break;
		case COMPLETION_MAIN_FUNCTION: {

			for (const Map<StringName, FunctionInfo>::Element *E = p_functions.front(); E; E = E->next()) {
				ScriptCodeCompletionOption option(E->key(), ScriptCodeCompletionOption::KIND_FUNCTION);
				r_options->push_back(option);
			}

			return OK;
		} break;
		case COMPLETION_IDENTIFIER:
		case COMPLETION_FUNCTION_CALL: {

			bool comp_ident = completion_type == COMPLETION_IDENTIFIER;
			Map<String, ScriptCodeCompletionOption::Kind> matches;
			StringName skip_function;
			BlockNode *block = completion_block;

			if (completion_class == TAG_GLOBAL) {
				while (block) {
					if (comp_ident) {
						for (const Map<StringName, BlockNode::Variable>::Element *E = block->variables.front(); E; E = E->next()) {

							if (E->get().line < completion_line) {
								matches.insert(E->key(), ScriptCodeCompletionOption::KIND_VARIABLE);
							}
						}
					}

					if (block->parent_function) {
						if (comp_ident) {
							for (int i = 0; i < block->parent_function->arguments.size(); i++) {
								matches.insert(block->parent_function->arguments[i].name, ScriptCodeCompletionOption::KIND_FUNCTION);
							}
						}
						skip_function = block->parent_function->name;
					}
					block = block->parent_block;
				}

				if (comp_ident && skip_function != StringName() && p_functions.has(skip_function)) {

					for (Map<StringName, BuiltInInfo>::Element *E = p_functions[skip_function].built_ins.front(); E; E = E->next()) {
						ScriptCodeCompletionOption::Kind kind = ScriptCodeCompletionOption::KIND_MEMBER;
						if (E->get().constant) {
							kind = ScriptCodeCompletionOption::KIND_CONSTANT;
						}
						matches.insert(E->key(), kind);
					}
				}

				if (comp_ident) {
					for (const Map<StringName, ShaderNode::Varying>::Element *E = shader->varyings.front(); E; E = E->next()) {
						matches.insert(E->key(), ScriptCodeCompletionOption::KIND_VARIABLE);
					}
					for (const Map<StringName, ShaderNode::Uniform>::Element *E = shader->uniforms.front(); E; E = E->next()) {
						matches.insert(E->key(), ScriptCodeCompletionOption::KIND_MEMBER);
					}
				}

				for (int i = 0; i < shader->functions.size(); i++) {
					if (!shader->functions[i].callable || shader->functions[i].name == skip_function)
						continue;
					matches.insert(String(shader->functions[i].name), ScriptCodeCompletionOption::KIND_FUNCTION);
				}

				int idx = 0;

				while (builtin_func_defs[idx].name) {

					matches.insert(String(builtin_func_defs[idx].name), ScriptCodeCompletionOption::KIND_FUNCTION);
					idx++;
				}

			} else { // sub-class
				int idx = 0;

				while (builtin_func_defs[idx].name) {
					if (builtin_func_defs[idx].tag == completion_class) {
						matches.insert(String(builtin_func_defs[idx].name), ScriptCodeCompletionOption::KIND_FUNCTION);
					}
					idx++;
				}
			}

			for (Map<String, ScriptCodeCompletionOption::Kind>::Element *E = matches.front(); E; E = E->next()) {
				ScriptCodeCompletionOption option(E->key(), E->value());
				if (E->value() == ScriptCodeCompletionOption::KIND_FUNCTION) {
					option.insert_text += "(";
				}
				r_options->push_back(option);
			}

			return OK;
		} break;
		case COMPLETION_CALL_ARGUMENTS: {

			for (int i = 0; i < shader->functions.size(); i++) {
				if (!shader->functions[i].callable)
					continue;
				if (shader->functions[i].name == completion_function) {

					String calltip;

					calltip += get_datatype_name(shader->functions[i].function->return_type);
					calltip += " ";
					calltip += shader->functions[i].name;
					calltip += "(";

					for (int j = 0; j < shader->functions[i].function->arguments.size(); j++) {

						if (j > 0)
							calltip += ", ";
						else
							calltip += " ";

						if (j == completion_argument) {
							calltip += CharType(0xFFFF);
						}

						calltip += get_datatype_name(shader->functions[i].function->arguments[j].type);
						calltip += " ";
						calltip += shader->functions[i].function->arguments[j].name;

						if (j == completion_argument) {
							calltip += CharType(0xFFFF);
						}
					}

					if (shader->functions[i].function->arguments.size())
						calltip += " ";
					calltip += ")";

					r_call_hint = calltip;
					return OK;
				}
			}

			int idx = 0;

			String calltip;

			while (builtin_func_defs[idx].name) {

				if (completion_function == builtin_func_defs[idx].name) {

					if (builtin_func_defs[idx].tag != completion_class) {
						idx++;
						continue;
					}

					if (calltip.length())
						calltip += "\n";

					calltip += get_datatype_name(builtin_func_defs[idx].rettype);
					calltip += " ";
					calltip += builtin_func_defs[idx].name;
					calltip += "(";

					bool found_arg = false;
					for (int i = 0; i < 4; i++) {

						if (builtin_func_defs[idx].args[i] == TYPE_VOID)
							break;

						if (i > 0)
							calltip += ", ";
						else
							calltip += " ";

						if (i == completion_argument) {
							calltip += CharType(0xFFFF);
						}

						calltip += get_datatype_name(builtin_func_defs[idx].args[i]);

						if (i == completion_argument) {
							calltip += CharType(0xFFFF);
						}

						found_arg = true;
					}

					if (found_arg)
						calltip += " ";
					calltip += ")";
				}
				idx++;
			}

			r_call_hint = calltip;

			return OK;

		} break;
		case COMPLETION_INDEX: {

			const char colv[4] = { 'r', 'g', 'b', 'a' };
			const char coordv[4] = { 'x', 'y', 'z', 'w' };

			int limit = 0;

			switch (completion_base) {
				case TYPE_BVEC2:
				case TYPE_IVEC2:
				case TYPE_UVEC2:
				case TYPE_VEC2: {
					limit = 2;

				} break;
				case TYPE_BVEC3:
				case TYPE_IVEC3:
				case TYPE_UVEC3:
				case TYPE_VEC3: {

					limit = 3;

				} break;
				case TYPE_BVEC4:
				case TYPE_IVEC4:
				case TYPE_UVEC4:
				case TYPE_VEC4: {

					limit = 4;

				} break;
				case TYPE_MAT2: limit = 2; break;
				case TYPE_MAT3: limit = 3; break;
				case TYPE_MAT4: limit = 4; break;
				default: {
				}
			}

			for (int i = 0; i < limit; i++) {
				r_options->push_back(ScriptCodeCompletionOption(String::chr(colv[i]), ScriptCodeCompletionOption::KIND_PLAIN_TEXT));
				r_options->push_back(ScriptCodeCompletionOption(String::chr(coordv[i]), ScriptCodeCompletionOption::KIND_PLAIN_TEXT));
			}

		} break;
	}

	return ERR_PARSE_ERROR;
}

String ShaderLanguage::get_error_text() {

	return error_str;
}

int ShaderLanguage::get_error_line() {

	return error_line;
}

ShaderLanguage::ShaderNode *ShaderLanguage::get_shader() {

	return shader;
}

ShaderLanguage::ShaderLanguage() {

	nodes = NULL;
	completion_class = TAG_GLOBAL;
}

ShaderLanguage::~ShaderLanguage() {

	clear();
}
