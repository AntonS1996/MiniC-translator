#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <fstream>
#include <cstdlib>
#include <iterator>
#include "Scaner.h"

using namespace std;


// Lexical Analyzer

Lexem::Lexem()
{
	LexType = "";
	LexValue = "";
}

Lexem::Lexem(string type)
{
	LexType = type;
	LexValue = "";
}

Lexem::Lexem(string type, string value)
{
	LexType = type;
	LexValue = value;
}

Lexem::~Lexem()
{

}

string Lexem::GetType()
{
	return LexType;
}

string Lexem::GetValue()
{
	return LexValue;
}

void Lexem::SetType(string t)
{
	LexType = t;
}

void Lexem::SetValue(string v)
{
	LexValue = v;
}

SymbolTable::SymbolTable()
{
	map<string, int> tmp1 = { { "var", 1 }, { "func", 2 }, { "arr" , 3} };
	IdKind = tmp1;
	map<string, int> tmp2 = { { "char", 1 }, { "int", 2 } };
	IdType = tmp2;
	code = 0;
	tmpCode = 1;
}

SymbolTable::~SymbolTable()
{

}

int SymbolTable::GetCode()
{
	return code;
}

int SymbolTable::GetTmpCode()
{
	return tmpCode;
}

void SymbolTable::Set(int code, string col, string val)
{
	table[code][col] = val;
}

bool isInMap(map<string, string> a, string str)
{
	for (auto item = a.begin(); item != a.end(); ++item)
		if (item->first == str)
			return true;
	return false;
}

bool isInTable(map<int, map<string, string>> tab, int a)
{
	for (auto item = tab.begin(); item != tab.end(); ++item)
		if (item->first == a)
			return true;
	return false;
}

void SymbolTable::makeRow(string str)
{
	if (!isInMap(strings, str))
	{
		strings[str] = to_string(code);
		createRow(str, "", "", 0, 0, -1);
	}
}

void SymbolTable::createRow(string str, string kind, string type, int len, int Default, int scope)
{
	map<string, string> tmp;
	table[code] = tmp;
	table[code]["str"] = str;
	table[code]["kind"] = kind;
	table[code]["type"] = type;
	table[code]["len"] = to_string(len);
	table[code]["Default"] = to_string(Default);
	table[code]["scope"] = to_string(scope);
	code += 1;
}

string SymbolTable::getIdFromSymbolTable(string str)
{
	makeRow(str);
	return strings[str];
}

string SymbolTable::getSymbolFromSymbolTable(int code)
{
	return table[code]["str"];
}

bool SymbolTable::setIdType(int code, int scope, string type, int Default)
{
	string typeValue;
	if ((scope == -1 && table[code]["kind"] != "") || (scope > -1 && code != findCodeForIdInScope(scope, code)))
	{
		cout << "Error: id " << table[code]["str"] << " is already defined" << endl;
		return false;
	}
	if (type == "char") typeValue = "char";
	if (type == "int") typeValue = "int";
	if (scope == -1)
	{
		table[code]["kind"] = "var";
		table[code]["type"] = typeValue;
		table[code]["Default"] = to_string(Default);
	}
	else
		createRow(table[code]["str"], "var", typeValue, 0, Default, scope);
	return true;
}

bool SymbolTable::setArrType(int code, int scope, string type, int len)
{
	string typeValue;
	if ((scope == -1 && table[code]["kind"] != "") || (scope > -1 && code != findCodeForIdInScope(scope, code)))
	{
		cout << "Error: id " << table[code]["str"] << " is already defined" << endl;
		return false;
	}
	if (type == "char") typeValue = "char";
	if (type == "int") typeValue = "int";
	if (scope == -1)
	{
		table[code]["kind"] = "arr";
		table[code]["type"] = typeValue;
		table[code]["len"] = to_string(len);
	}
	else
		createRow(table[code]["str"], "arr", typeValue, len, 0, scope);
	return true;
}

bool SymbolTable::setFuncType(int code, string type, int len)
{
	string typeValue;
	if (table[code]["kind"] != "")
	{
		cout << "Error: id " << table[code]["str"] << " is already defined" << endl;
		return false;
	}
	if (type == "char") typeValue = "char";
	if (type == "int") typeValue = "int";
	table[code]["kind"] = "func";
	table[code]["type"] = typeValue;
	table[code]["len"] = to_string(len);
	return true;
}

void SymbolTable::printTable()
{
	cout << "SYMBOL TABLE\n-------------------------------------------------------" << endl;
	cout << "code\tname\tkind\ttype\tlen\tdefault\tscope" << endl;
	string kind, idtype, len, Default, scope;
	for (auto row = table.begin(); row != table.end(); ++row)
	{
		if (table[row->first]["kind"] == "var") kind = "var";
		else if (table[row->first]["kind"] == "func") kind = "func";
		else if (table[row->first]["kind"] == "arr") kind = "array";
		else kind = "None";
		if (table[row->first]["type"] == "int") idtype = "int";
		else if (table[row->first]["type"] == "char") idtype = "char";
		else idtype = "None";
		if (table[row->first]["len"] == "0") len = "None";
		else len = table[row->first]["len"];
		if (table[row->first]["Default"] == "0") Default = "None";
		else Default = table[row->first]["Default"];
		if (table[row->first]["scope"] == "0") scope = "None";
		else scope = table[row->first]["scope"];
		cout << row->first << "\t" << table[row->first]["str"] << "\t" << kind << "\t" << idtype << "\t" << len << "\t" << Default << "\t" << scope << endl;
	}
}

string SymbolTable::Alloc(int scope)
{
	string tmp;
	tmp = "[tmp" + to_string(tmpCode) + "]";
	makeRow(tmp);
	tmpCode += 1;
	table[code - 1]["kind"] = "var";
	table[code - 1]["type"] = "int";
	table[code - 1]["scope"] = to_string(scope);
	return strings[tmp];
}

bool SymbolTable::checkId(int scope, Lexem attr) // по умолчанию (None) Type = ""
{
	if (attr.GetType() == "") return false;
	attr.SetValue(to_string(findCodeForIdInScope(scope, stoi(attr.GetValue()))));
	if (stoi(attr.GetValue()) == -1) return false;
	if (table[stoi(attr.GetValue())]["kind"] == "var") return true;
	if (table[stoi(attr.GetValue())]["kind"] == "")
	{
		cout << "Error: id '" << table[stoi(attr.GetValue())]["str"] << "' is undefined" << endl;
		return false;
	}
	if (table[stoi(attr.GetValue())]["kind"] == "func")
	{
		cout << "Error: function name '" << table[stoi(attr.GetValue())]["str"] << "' is used as a variable" << endl;
		return false;
	}
	if (table[stoi(attr.GetValue())]["kind"] == "arr")
	{
		cout << "Error: array name '" << table[stoi(attr.GetValue())]["str"] << "' is used as a variable" << endl;
		return false;
	}
	cout << "Error: id '" << table[stoi(attr.GetValue())]["str"] << "' is " << table[stoi(attr.GetValue())]["kind"] << ", not a variable" << endl;
	return false;
}

int SymbolTable::findCodeForIdInScope(int scope, int code)
{
	if (!isInTable(table, code)) return -1;
	string varname = table[code]["str"];
	int result = -1;
	for (auto row = table.begin(); row != table.end(); ++row)
	{
		if (table[row->first]["str"] == varname && table[row->first]["scope"] == to_string(scope)) return row->first;
		if (table[row->first]["str"] == varname && table[row->first]["scope"] == "-1") result = row->first;
	}
	return result;
}

bool SymbolTable::checkArr(int scope, Lexem attr)
{
	if (attr.GetType() == "") return false;
	attr.SetValue(to_string(findCodeForIdInScope(scope, stoi(attr.GetValue()))));
	if (stoi(attr.GetValue()) == -1) return false;
	if (table[stoi(attr.GetValue())]["kind"] == "arr") return true;
	if (table[stoi(attr.GetValue())]["kind"] == "")
	{
		cout << "Error: id '" << table[stoi(attr.GetValue())]["str"] << "' is undefined" << endl;
		return false;
	}
	if (table[stoi(attr.GetValue())]["kind"] == "func")
	{
		cout << "Error: function name '" << table[stoi(attr.GetValue())]["str"] << "' is used as an array" << endl;
		return false;
	}
	if (table[stoi(attr.GetValue())]["kind"] == "var")
	{
		cout << "Error: variable name '" << table[stoi(attr.GetValue())]["str"] << "' is used as an array" << endl;
		return false;
	}
	cout << "Error: id '" << table[stoi(attr.GetValue())]["str"] << "' is " << table[stoi(attr.GetValue())]["kind"] << ", not an array" << endl;
	return false;
}

bool SymbolTable::checkFunc(int scope, Lexem attr, int len)
{
	if (attr.GetType() == "") return false;
	attr.SetValue(to_string(findCodeForIdInScope(scope, stoi(attr.GetValue()))));
	if (table[stoi(attr.GetValue())]["kind"] == "func")
	{
		if (table[stoi(attr.GetValue())]["len"] != to_string(len))
		{
			cout << "Error: function " << table[stoi(attr.GetValue())]["str"] << " expects " << table[stoi(attr.GetValue())]["len"] << " arguments, got " << len << endl;
			return false;
		}
		return true;
	}
	if (table[stoi(attr.GetValue())]["kind"] == "")
	{
		cout << "Error: id '" << table[stoi(attr.GetValue())]["str"] << "' is undefined" << endl;
		return false;
	}
	if (table[stoi(attr.GetValue())]["kind"] == "arr")
	{
		cout << "Error: array name '" << table[stoi(attr.GetValue())]["str"] << "' is used as a function" << endl;
		return false;
	}
	if (table[stoi(attr.GetValue())]["kind"] == "var")
	{
		cout << "Error: variable name '" << table[stoi(attr.GetValue())]["str"] << "' is used as a function" << endl;
		return false;
	}
	cout << "Error: id '" << table[stoi(attr.GetValue())]["str"] << "' is " << table[stoi(attr.GetValue())]["kind"] << ", not an array" << endl;
	return false;
}

StringTable::StringTable()
{
	num = 0;
}

StringTable::~StringTable()
{

}


int StringTable::GetNum()
{
	return num;
}

bool IsInMap(map<string, int> a, string str)
{
	for (auto item = a.begin(); item != a.end(); ++item)
		if (item->first == str)
			return true;
	return false;
}

bool IsInTable(map<int, string> tab, int n)
{
	for (auto item = tab.begin(); item != tab.end(); ++item)
		if (item->first == n)
			return true;
	return false;
}

int StringTable::getIdFromStringTable(string str)
{
	if (!IsInMap(strings, str))
	{
		strings[str] = num;
		table[num] = str;
		num += 1;
	}
	return strings[str];
}

string StringTable::getStringFromStringTable(int id)
{
	if (IsInTable(table, id))
		return table[id];
}

void StringTable::printTable()
{
	cout << "STRING TABLE\n------------" << endl;
	for (auto row = table.begin(); row != table.end(); ++row)
		cout << row->first << "\t" << table[row->first] << endl;
}

bool isInStr(string str, string a)
{
	string s;
	for (int i = 0; i < str.size(); ++i)
	{
		s = str[i];
		if (s == a)
			return true;
	}
	return false;
}

Scanner::Scanner(string filename)
{
	map<string, string> tmp1 = { { "int", "kwint" }, { "char", "kwchar" }, { "if", "kwif" },
	{ "else", "kwelse" }, { "switch", "kwswitch" }, { "case", "kwcase" },
	{ "while", "kwwhile" }, { "for", "kwfor" }, { "return", "kwreturn" },
	{ "in", "kwin" }, { "out", "kwout" }, { "default", "kwdefault" } };
	keywords = tmp1;
	map<string, string> tmp2 = { { "lpar", "[lpar]" }, { "rpar", "[rpar]" }, { "lbrace", "[lbrace]" },
	{ "rbrace", "[rbrace]" }, { "lbracket", "[lbracket]" }, { "rbracket", "[rbracket]" },
	{ "semicolon", "[semicolon]" }, { "comma", "[comma]" }, { "colon", "[colon]" },
	{ "opassign", "[opassign]" }, { "opplus", "[opplus]" }, { "opminus", "[opminus]" },
	{ "opmult", "[opmult]" }, { "opinc", "[opinc]" }, { "opeq", "[opeq]" }, { "opne", "[opne]" },
	{ "oplt", "[oplt]" }, { "opgt", "[opgt]" }, { "ople", "[ople]" }, { "opnot", "[opnot]" },
	{ "opor", "[opor]" }, { "opand", "[opand]" }, { "kwint", "[kwint]" }, { "kwchar", "[kwchar]" },
	{ "kwif", "[kwif]" }, { "kwelse", "[kwelse]" }, { "kwswitch", "[kwswitch]" }, { "kwcase", "[kwcase]" },
	{ "kwwhile", "[kwwhile]" }, { "kwfor", "[kwfor]" }, { "kwreturn", "[kwreturn]" }, { "kwin", "[kwin]" },
	{ "kwout", "[kwout]" }, { "kwdefault", "[kwdefault]" }, { "opge", "[opge]" },
	{ "id", "[id]" }, { "num", "[num]" }, { "chr", "[chr]" }, { "str", "[str]" } };
	LexemStrings = tmp2;
	Letter = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
	Digit = "0123456789";
	file.open(filename + ".minic");
	Status = 1;
	Filename = filename;
	savedChar = "";
	State = 0;
	Value = "";
}

Scanner::~Scanner()
{
	file.close();
}

int Scanner::getStatus()
{
	return Status;
}

void Scanner::printLexem(Lexem lexem)
{
	if (lexem.GetType() == "num") cout << "[num, " << lexem.GetValue() << "]" << endl;
	else if (lexem.GetType() == "chr") cout << "[chr, '" << lexem.GetValue() << "']" << endl;
	else if (lexem.GetType() == "str") cout << "[str, " << lexem.GetValue() << ":'" << stringTable.getStringFromStringTable(stoi(lexem.GetValue())) << "']" << endl;
	else if (lexem.GetType() == "id") cout << "[id, " << lexem.GetValue() << ":" << symbolTable.getSymbolFromSymbolTable(stoi(lexem.GetValue())) << "]" << endl;
	else cout << LexemStrings[lexem.GetType()] << endl;
}

void Scanner::printSymbolTable()
{
	symbolTable.printTable();
}

void Scanner::printStringTable()
{
	stringTable.printTable();
}

string Scanner::getIdType(string id)
{
	if (isInMap(keywords, id)) return keywords[id];
	else return "id";
}

string Scanner::getNextChar()
{
	string Char;
	if (savedChar != "")
	{
		Char = savedChar;
		savedChar = "";
		return Char;
	}
	char c;
	file.get(c);
	Char = c;
	if (Char == "")
	{
		Status = 0;
		return "";
	}
	else
		return Char;
}

void Scanner::returnChar(string chr)
{
	savedChar = chr;
}

Lexem Scanner::getNextLexem()
{
	while (1)
	{
		string Char;
		Char = getNextChar();
		if (State == 0)
		{
			if (file.eof()) return Lexem("eof");
			if (isInStr(Digit, Char))
			{
				Value = Char;
				State = 1;
				continue;
			}
			if (Char == "'")
			{
				Value = "";
				State = 2;
				continue;
			}
			if (Char == "\"")
			{
				Value = "";
				State = 4;
				continue;
			}
			if (isInStr(Letter, Char))
			{
				Value = Char;
				State = 5;
				continue;
			}
			if (Char == "<")
			{
				State = 8;
				continue;
			}
			if (Char == "-")
			{
				State = 6;
				continue;
			}
			if (Char == "!")
			{
				State = 7;
				continue;
			}
			if (Char == "=")
			{
				State = 9;
				continue;
			}
			if (Char == "+")
			{
				State = 10;
				continue;
			}
			if (Char == "|")
			{
				State = 11;
				continue;
			}
			if (Char == "&")
			{
				State = 12;
				continue;
			}
			if (Char == "*") return Lexem("opmult");
			if (Char == " " || Char == "\t" || Char == "\n") continue;
			if (Char == ">") return Lexem("opgt");
			if (Char == "(") return Lexem("lpar");
			if (Char == ")") return Lexem("rpar");
			if (Char == "{") return Lexem("lbrace");
			if (Char == "}") return Lexem("rbrace");
			if (Char == "[") return Lexem("lbracket");
			if (Char == "]") return Lexem("rbracket");
			if (Char == ":") return Lexem("colon");
			if (Char == ";") return Lexem("semicolon");
			if (Char == ",") return Lexem("comma");
			return Lexem("error");
		}
		if (State == 1)
		{
			if (Char == "" || !isInStr(Digit, Char))
			{
				State = 0;
				returnChar(Char);
				return Lexem("num", Value);
			}
			Value = to_string(stoi(Value) * 10 + stoi(Char));
			continue;
		}
		if (State == 2)
		{
			if (Char == "") return Lexem("error");
			if (Char == "'") return Lexem("error");
			Value = Char;
			State = 3;
			continue;
		}
		if (State == 3)
		{
			if (Char == "") return Lexem("error");
			if (Char == "'")
			{
				State = 0;
				return Lexem("chr", Value);
			}
			return Lexem("error");
		}
		if (State == 4)
		{
			if (Char == "") return Lexem("error");
			if (Char == "") return Lexem("error");
			if (Char == "\"")
			{
				State = 0;
				return Lexem("str", to_string(stringTable.getIdFromStringTable(Value)));
			}
			Value += Char;
			continue;
		}
		if (State == 5)
		{
			if ((Char != "") && (isInStr(Letter, Char) || isInStr(Digit, Char)))
			{
				Value += Char;
				continue;
			}
			returnChar(Char);
			State = 0;
			if (getIdType(Value) != "id") return Lexem(getIdType(Value));
			return Lexem("id", symbolTable.getIdFromSymbolTable(Value));
		}
		if (State == 6)
		{
			if (Char != "" && isInStr(Digit, Char))
			{
				Value = to_string((-1) * stoi(Char));
				State = 1;
				continue;
			}
			returnChar(Char);
			State = 0;
			return Lexem("opminus");
		}
		if (State == 7)
		{
			State = 0;
			if (Char == "=")
				return Lexem("opne");
			returnChar(Char);
			return Lexem("opnot");
		}
		if (State == 8)
		{
			State = 0;
			if (Char == "=")
				return Lexem("ople");
			returnChar(Char);
			return Lexem("oplt");
		}
		if (State == 9)
		{
			State = 0;
			if (Char == "=")
				return Lexem("opeq");
			returnChar(Char);
			return Lexem("opassign");
		}
		if (State == 10)
		{
			State = 0;
			if (Char == "+")
				return Lexem("opinc");
			returnChar(Char);
			return Lexem("opplus");
		}
		if (State == 11)
		{
			if (Char == "|")
			{
				State = 0;
				return Lexem("opor");
			}
			return Lexem("error");
		}
		if (State == 12)
		{
			if (Char == "&")
			{
				State = 0;
				return Lexem("opand");
			}
			return Lexem("error");
		}
	}
}

SymbolTable &Scanner::GetSymbolTable()
{
	SymbolTable &lsymbolTable = symbolTable;
	return lsymbolTable;
}

map<string, string> &Scanner::GetLexemStrings()
{
	map<string, string> &lLexemStrings = LexemStrings;
	return lLexemStrings;
}


// Syntax Analizer

bool isInList(string list[], int len, string a)
{
	for (int i = 0; i < len; ++i)
		if (a == list[i])
			return true;
	return false;
}

AOp::AOp()
{
	type = "";
	value = "";
	index = "";
}

AOp::AOp(string Type, string Value, string Index)
{
	type = Type;
	value = Value;
	index = Index;
}

AOp::~AOp()
{

}

string AOp::GetType()
{
	return type;
}

string AOp::GetValue()
{
	return value;
}

string AOp::GetIndex()
{
	return index;
}

void AOp::SetType(string Type)
{
	type = Type;
}

void AOp::SetValue(string Value)
{
	value = Value;
}

void AOp::SetIndex(string Index)
{
	index = Index;
}


Translator::Translator(string filename) : scanner{ filename }, lexem{}
{
	map<string, string> tmp1 = { { "Aadd", "ADD" }, { "Asub", "SUB" }, { "Amul", "MUL" }, { "Adiv", "DIV" },
	{ "Aneg", "NEG" }, { "Aand", "AND" }, { "Aor", "OR" }, { "Anot", "NOT" }, { "Amov", "MOV" }, { "Aeq", "EQ" },
	{ "Ane", "NE" }, { "Agt", "GT" }, { "Alt", "LT" }, { "Age", "GE" }, { "Ale", "LE" }, { "Ajmp", "JMP" },
	{ "Ain", "IN" }, { "Aout", "OUT" }, { "Albl", "LBL" }, { "Aparam", "PARAM" }, { "Acall" , "CALL" }, { "Aret", "RET" } };
	AtomStrings = tmp1;
	const0.SetType("num");
	const0.SetValue("0");
	const1.SetType("num");
	const1.SetValue("1");
	Filename = filename;
	Status = scanner.getStatus();
	LabelCount = 0;
	outfile.open(filename + ".atom");
	lexem = scanner.getNextLexem();
}

Translator::~Translator()
{
	outfile.close();
}

int Translator::getStatus()
{
	return Status;
}

bool Translator::getNextLexem()
{
	lexem = scanner.getNextLexem();
	return !lexicalError();
}

AOp Translator::Alloc(int scope)
{
	AOp aop("mem", scanner.GetSymbolTable().Alloc(scope), "");
	return aop;
}

AOp Translator::newlab()
{
	AOp aop("lbl", to_string(newLabel()), "");
	return aop;
}

string Translator::printAOp(AOp op)
{
	if (op.GetType() == "" && op.GetValue() == "" && op.GetIndex() == "") return "";
	if (op.GetType() == "num") return "'" + op.GetValue() + "'";
	else if (op.GetType() == "mem") return op.GetValue();
	else if (op.GetType() == "str") return "S" + op.GetValue();
	else if (op.GetType() == "lbl") return "L" + op.GetValue();
	else if (op.GetType() == "arr") return op.GetValue() + "[" + op.GetIndex() + "]";
	else return op.GetValue();
}

void Translator::generateAtom(int scope, string atom, AOp op1, AOp op2, AOp op3)
{
	if (Status > 0)
	{
		outfile << to_string(scope) + "\t(" + AtomStrings[atom] + ", " + printAOp(op1) + ", " + printAOp(op2) + ", " + printAOp(op3) + ")\n";
	}
}

void Translator::printSymbolTable()
{
	scanner.printSymbolTable();
}

void Translator::printStringTable()
{
	scanner.printStringTable();
}

int Translator::newLabel()
{
	int tmp = LabelCount;
	LabelCount += 1;
	return tmp;
}

bool Translator::lexicalError()
{
	if (lexem.GetType() == "error")
	{
		cout << "Lexical error: " << lexem.GetValue() << endl;
		return true;
	}
	return false;
}

bool Translator::takeTerm(string lextype)
{
	if (lexem.GetType() != lextype)
	{
		cout << "Syntax error: expected " << scanner.GetLexemStrings()[lextype] << ", got " << scanner.GetLexemStrings()[lexem.GetType()] << endl;
		return false;
	}
	return getNextLexem();
}

bool Translator::checkId(int scope, Lexem attr)
{
	return scanner.GetSymbolTable().checkId(scope, attr);
}

bool Translator::checkArr(int scope, Lexem attr)
{
	return scanner.GetSymbolTable().checkArr(scope, attr);
}

bool Translator::checkFunc(int scope, Lexem attr, int params)
{
	return scanner.GetSymbolTable().checkFunc(scope, attr, params);
}

bool Translator::setIdType(int code, int scope, string type, int Default)
{
	return scanner.GetSymbolTable().setIdType(code, scope, type, Default);
}

bool Translator::setArrType(int code, int scope, string type, int len)
{
	return scanner.GetSymbolTable().setArrType(code, scope, type, len);
}

bool Translator::setFuncType(int code, string type, int len)
{
	return scanner.GetSymbolTable().setFuncType(code, type, len);
}

void Translator::opCopy(AOp &from_, AOp &to_)
{
	to_.SetType(from_.GetType());
	to_.SetValue(from_.GetValue());
	to_.SetIndex(from_.GetIndex());
}

bool Translator::translate()
{
	if (!StmtList(-1)) return false;
	if (lexicalError()) return false;
	if (!takeTerm("eof")) return false;
	return true;
}

bool Translator::E(int scope, AOp &p)
{
	return E7(scope, p);
}

bool Translator::E7(int scope, AOp &p)
{
	AOp q;
	AOp &lq = q;
	return E6(scope, lq) && E7_(scope, lq, p);
}

bool Translator::E7_(int scope, AOp &p, AOp &q)
{
	if (lexicalError()) return false;
	if (lexem.GetType() == "opor")
	{
		if (!takeTerm("opor")) return false;
		AOp s = Alloc(scope);
		AOp &ls = s;
		AOp r;
		AOp &lr = r;
		if (!E6(scope, lr)) return false;
		generateAtom(scope, "Aor", p, r, s);
		if (!E7_(scope, ls, q)) return false;
		return true;
	}
	AOp &lp = p;
	AOp &lq = q;
	opCopy(lp, lq);
	return true;
}

bool Translator::E6(int scope, AOp &p)
{
	AOp q;
	AOp &lq = q;
	return E5(scope, lq) && E6_(scope, lq, p);
}

bool Translator::E6_(int scope, AOp &p, AOp &q)
{
	if (lexicalError()) return false;
	if (lexem.GetType() == "opand")
	{
		if (!takeTerm("opand")) return false;
		AOp s = Alloc(scope);
		AOp &ls = s;
		AOp r;
		AOp &lr = r;
		if (!E5(scope, lr)) return false;
		generateAtom(scope, "Aand", p, r, s);
		if (!E6_(scope, ls, q)) return false;
		return true;
	}
	AOp &lp = p;
	AOp &lq = q;
	opCopy(lp, lq);
	return true;
}

bool Translator::E5(int scope, AOp &p)
{
	AOp q;
	AOp &lq = q;
	return E4(scope, lq) && E5_(scope, lq, p);
}

bool Translator::E5_(int scope, AOp &p, AOp &q)
{
	if (lexicalError()) return false;
	string lex_type_list[] = { "opeq", "opne", "opnot", "opgt", "oplt", "ople" };
	if (isInList(lex_type_list, 6, lexem.GetType()))
	{
		Lexem tmp = lexem;
		AOp s = Alloc(scope);
		AOp r;
		AOp &lr = r;
		AOp l = newlab();
		getNextLexem();
		if (!E4(scope, lr)) return false;
		AOp n;
		generateAtom(scope, "Amov", const1, n, s);
		if (tmp.GetType() == "opeq") generateAtom(scope, "Aeq", p, r, l);
		else if (tmp.GetType() == "opne") generateAtom(scope, "Ane", p, r, l);
		else if (tmp.GetType() == "opgt") generateAtom(scope, "Agt", p, r, l);
		else if (tmp.GetType() == "oplt") generateAtom(scope, "Alt", p, r, l);
		else if (tmp.GetType() == "ople") generateAtom(scope, "Ale", p, r, l);
		generateAtom(scope, "Amov", const0, n, s);
		generateAtom(scope, "Albl", n, n, l);
		AOp &ls = s;
		AOp &lq = q;
		opCopy(ls, lq);
		return true;
	}
	AOp &lp = p;
	AOp &lq = q;
	opCopy(lp, lq);
	return true;
}

bool Translator::E4(int scope, AOp &p)
{
	AOp q;
	AOp &lq = q;
	return E3(scope, lq) && E4_(scope, lq, p);
}

bool Translator::E4_(int scope, AOp &p, AOp &q)
{
	if (lexicalError()) return false;
	string lex_type_list[] = { "opplus", "opminus" };
	if (isInList(lex_type_list, 2, lexem.GetType()))
	{
		Lexem tmp = lexem;
		AOp s = Alloc(scope);
		AOp &ls = s;
		AOp r;
		AOp &lr = r;
		getNextLexem();
		if (!E3(scope, lr)) return false;
		if (tmp.GetType() == "opplus") generateAtom(scope, "Aadd", p, r, s);
		else if (tmp.GetType() == "opminus") generateAtom(scope, "Asub", p, r, s);
		if (!E4_(scope, ls, q)) return false;
		return true;
	}
	AOp &lp = p;
	AOp &lq = q;
	opCopy(lp, lq);
	return true;
}

bool Translator::E3(int scope, AOp &p)
{
	AOp q;
	AOp &lq = q;
	return E2(scope, lq) && E3_(scope, lq, p);
}

bool Translator::E3_(int scope, AOp &p, AOp &q)
{
	if (lexicalError()) return false;
	if (lexem.GetType() == "opmult")
	{
		if (!takeTerm("opmult")) return false;
		AOp s = Alloc(scope);
		AOp &ls = s;
		AOp r;
		AOp &lr = r;
		if (!E2(scope, lr)) return false;
		generateAtom(scope, "Amul", p, r, s);
		if (!E3_(scope, ls, q)) return false;
		return true;
	}
	AOp &lp = p;
	AOp &lq = q;
	opCopy(lp, lq);
	return true;
}

bool Translator::E2(int scope, AOp &p)
{
	if (lexicalError()) return false;
	if (lexem.GetType() == "opnot")
	{
		if (!takeTerm("opnot")) return false;
		AOp r = Alloc(scope);
		AOp q;
		AOp &lq = q;
		if (!E1(scope, lq)) return false;
		AOp n;
		generateAtom(scope, "Anot", q, n, r);
		AOp &lr = r;
		AOp &lp = p;
		opCopy(lr, lp);
		return true;
	}
	return E1(scope, p);
}

bool Translator::E1(int scope, AOp &p)
{
	if (lexicalError()) return false;
	if (lexem.GetType() == "opinc")
	{
		if (!takeTerm("opinc")) return false;
		AOp r("mem", lexem.GetValue(), "");
		AOp &lr = r;
		if (!takeTerm("id")) return false;
		return E1__(scope, lr, p);
	}
	if (lexem.GetType() == "lpar")
	{
		if (!takeTerm("lpar")) return false;
		AOp &lp = p;
		if (!E(scope, lp)) return false;
		if (!takeTerm("rpar")) return false;
		return true;
	}
	string lex_type_list[] = { "num", "chr" };
	if (isInList(lex_type_list, 2, lexem.GetType()))
	{
		p.SetType("num");
		p.SetValue(lexem.GetValue());
		if (!getNextLexem()) return false;
		return true;
	}
	if (lexem.GetType() == "id")
	{
		AOp r("mem", lexem.GetValue(), "");
		AOp &lr = r;
		if (!takeTerm("id")) return false;
		return E1_(scope, lr, p);
	}
	cout << "Syntax error: unexpected lexem '" << lexem.GetType() << "' in expression, expected ++, (, num, id" << endl;
	return false;
}

bool Translator::E1_(int scope, AOp &p, AOp &q)
{
	if (lexicalError()) return false;
	if (lexem.GetType() == "opinc")
	{
		if (!takeTerm("opinc")) return false;
		Lexem P(p.GetType(), p.GetValue());
		if (!checkId(scope, P)) return false;
		AOp r = Alloc(scope);
		AOp n;
		generateAtom(scope, "Amov", p, n, r);
		generateAtom(scope, "Aadd", p, const1, p);
		AOp &lr = r;
		AOp &lq = q;
		opCopy(lr, lq);
		return true;
	}
	if (lexem.GetType() == "lbracket")
	{
		if (!takeTerm("lbracket")) return false;
		Lexem P(p.GetType(), p.GetValue());
		if (!checkArr(scope, P)) return false;
		AOp r;
		AOp &lr = r;
		if (!E(scope, lr)) return false;
		if (!takeTerm("rbracket")) return false;
		AOp s("arr", p.GetValue(), r.GetIndex());
		AOp &ls = s;
		return E1___(scope, ls, q);
	}
	if (lexem.GetType() == "lpar")
	{
		if (!takeTerm("lpar")) return false;
		AOp N;
		if (!ArgList(scope, N)) return false;
		Lexem P(p.GetType(), p.GetValue());
		if (!checkFunc(scope, P, stoi(N.GetValue()))) return false;
		if (!takeTerm("rpar")) return false;
		AOp r = Alloc(scope);
		AOp n;
		generateAtom(scope, "Acall", p, n, r);
		AOp &lr = r;
		AOp &lq = q;
		opCopy(lr, lq);
		return true;
	}
	Lexem P(p.GetType(), p.GetValue());
	if (!checkId(scope, P)) return false;
	AOp &lp = p;
	AOp &lq = q;
	opCopy(lp, lq);
	return true;
}

bool Translator::E1__(int scope, AOp &p, AOp &q)
{
	if (lexicalError()) return false;
	if (lexem.GetType() == "lbracket")
	{
		if (!takeTerm("lbracket")) return false;
		Lexem P(p.GetType(), p.GetValue());
		if (!checkArr(scope, P)) return false;
		AOp r;
		AOp &lr = r;
		if (!E(scope, lr)) return false;
		if (!takeTerm("rbracket")) return false;
		q.SetType("arr");
		q.SetValue(p.GetValue());
		q.SetIndex(r.GetIndex());
		generateAtom(scope, "Aadd", q, const1, q);
		return true;
	}
	Lexem P(p.GetType(), p.GetValue());
	if (!checkId(scope, P)) return false;
	generateAtom(scope, "Aadd", p, const1, p);
	AOp &lp = p;
	AOp &lq = q;
	opCopy(lp, lq);
	return true;
}

bool Translator::E1___(int scope, AOp &p, AOp &q)
{
	if (lexicalError()) return false;
	if (lexem.GetType() == "opinc")
	{
		if (!takeTerm("opinc")) return false;
		AOp r = Alloc(scope);
		AOp n;
		generateAtom(scope, "Amov", p, n, r);
		generateAtom(scope, "Aadd", p, const1, p);
		AOp &lr = r;
		AOp &lq = q;
		opCopy(lr, lq);
		return true;
	}
	AOp &lp = p;
	AOp &lq = q;
	opCopy(lp, lq);
	return true;
}

bool Translator::ArgList(int scope, AOp &n)
{
	if (lexicalError()) return false;
	string lex_type_list[] = { "opinc", "lpar", "opnot", "num", "id", "chr" };
	if (isInList(lex_type_list, 6, lexem.GetType()))
	{
		AOp p;
		AOp &lp = p;
		if (!E(scope, lp)) return false;
		AOp m;
		AOp &lm = m;
		if (!ArgList_(scope, lm)) return false;
		AOp N;
		generateAtom(scope, "Aparam", N, N, p);
		n.SetValue(to_string(stoi(m.GetValue()) + 1));
		return true;
	}
	n.SetValue("0");
	return true;
}

bool Translator::ArgList_(int scope, AOp &n)
{
	if (lexicalError()) return false;
	if (lexem.GetType() == "comma")
	{
		if (!takeTerm("comma")) return false;
		AOp p;
		AOp &lp = p;
		if (!E(scope, lp)) return false;
		AOp m;
		AOp &lm = m;
		if (!ArgList_(scope, lm)) return false;
		AOp N;
		generateAtom(scope, "Aparam", N, N, p);
		n.SetValue(to_string(stoi(m.GetValue()) + 1));
		return true;
	}
	n.SetValue("0");
	return true;
}

bool Translator::StmtList(int scope)
{
	if (lexicalError()) return false;
	string lex_type_list[] = { "kwchar", "kwint", "id", "kwwhile", "kwfor", "kwif", "kwswitch", "kwin", "kwout", "semicolon", "lbrace", "kwreturn" };
	cout << "lexem: " << lexem.GetType() << endl;
	if (isInList(lex_type_list, 12, lexem.GetType()))
		return Stmt(scope) && StmtList(scope);
	return true;
}

bool Translator::Stmt(int scope)
{
	if (lexicalError()) return false;
	string lex_type_list[] = { "kwchar", "kwint" };
	if (isInList(lex_type_list, 2, lexem.GetType()))
		return DeclareStmt(scope);
	string lex_type_list1[] = { "id", "kwwhile", "kwfor", "kwif", "kwswitch", "kwin", "kwout", "semicolon", "lbrace", "opreturn" };
	if (isInList(lex_type_list1, 10, lexem.GetType()))
		if (scope == -1)
		{
			cout << "Error: operator is outside function definition" << endl;
			return false;
		}
	if (lexem.GetType() == "id") return AssignOrCallOp(scope);
	if (lexem.GetType() == "kwwhile") return WhileOp(scope);
	if (lexem.GetType() == "kwfor") return ForOp(scope);
	if (lexem.GetType() == "kwif") return IfOp(scope);
	if (lexem.GetType() == "kwswitch") return SwitchOp(scope);
	if (lexem.GetType() == "kwin") return IOp(scope);
	if (lexem.GetType() == "kwout") return OOp(scope);
	if (lexem.GetType() == "lbrace")
	{
		if (!takeTerm("lbrace")) return false;
		if (!StmtList(scope)) return false;
		if (!takeTerm("rbrace")) return false;
		return true;
	}
	if (lexem.GetType() == "kwreturn")
	{
		if (!takeTerm("kwreturn")) return false;
		AOp p;
		AOp &lp = p;
		if (!E(scope, lp)) return false;
		AOp n;
		generateAtom(scope, "Aret", n, n, p);
		if (!takeTerm("semicolon")) return false;
		return true;
	}
	if (lexem.GetType() == "semicolon")
	{
		if (!takeTerm("semicolon")) return false;
		return true;
	}
	cout << "Syntax error: forbidden lexem '" << lexem.GetType() << "'" << endl;
	return false;
}

bool Translator::DeclareStmt(int scope)
{
	if (lexicalError()) return false;
	string lex_type_list[] = { "kwchar", "kwint" };
	if (isInList(lex_type_list, 2, lexem.GetType()))
	{
		AOp p;
		AOp &lp = p;
		if (!Type(scope, lp)) return false; //
		AOp q("mem", lexem.GetValue(), "");
		AOp &lq = q;
		cout << "lexem.value: " << lexem.GetValue() << endl;
		if (!takeTerm("id")) return false; //
		cout << "lexem: " << lexem.GetType() << endl;
		return DeclareStmt_(scope, lp, lq); //
	}
	return false;
}

bool Translator::DeclareStmt_(int scope, AOp &p, AOp &q)
{
	if (lexicalError()) return false;
	if (lexem.GetType() == "lpar")
	{
		if (scope > -1)
		{
			cout << "Error: Function definition inside function" << endl;
			return false;
		}
		if (!takeTerm("lpar")) return false;
		AOp N;
		AOp &lN = N;
		if (!ParamList(scope, lN, q)) return false;
		cout << "lexem: " << lexem.GetType() << endl;
		if (!takeTerm("rpar")) return false;
		cout << "lexem: " << lexem.GetType() << endl;
		if (!setFuncType(stoi(q.GetValue()), p.GetValue(), stoi(N.GetValue()))) return false;
		if (!takeTerm("lbrace")) return false;
		cout << "lexem: " << lexem.GetType() << endl;
		if (!StmtList(stoi(q.GetValue()))) return false;
		if (!takeTerm("rbrace")) return false;
		AOp n;
		AOp tmpAOp("num", "0", "");
		generateAtom(stoi(q.GetValue()), "Aret", n, n, tmpAOp);
		return true;
	}
	if (lexem.GetType() == "opassign")
	{
		if (!takeTerm("opassign")) return false;
		int val = stoi(lexem.GetValue());
		string lex_type_list[] = { "num", "chr" };
		if (isInList(lex_type_list, 2, lexem.GetType()))
		{
			if (!getNextLexem()) return false;
		}
		else
			return false;
		if (!setIdType(stoi(q.GetValue()), scope, p.GetValue(), val)) return false;
		if (!DeclVarList_(scope, p)) return false;
		if (!takeTerm("semicolon")) return false;
		return true;
	}
	if (lexem.GetType() == "lbracket")
	{
		if (!takeTerm("lbracket")) return false;
		int num = stoi(lexem.GetValue());
		if (!takeTerm("num")) return false;
		if (!setArrType(stoi(q.GetValue()), scope, p.GetValue(), num)) return false;
		if (!takeTerm("rbracket")) return false;
		if (!DeclVarList_(scope, p)) return false;
		if (!takeTerm("semicolon")) return false;
		return true;
	}
	if (!setIdType(stoi(q.GetValue()), scope, p.GetValue(), 0)) return false;
	if (!DeclVarList_(scope, p)) return false;
	if (!takeTerm("semicolon")) return false;
	return true;
}

bool Translator::Type(int scope, AOp &p)
{
	if (lexicalError()) return false;
	if (lexem.GetType() == "kwchar")
	{
		p.SetValue("char");
		getNextLexem();
		cout << "lexem: " << lexem.GetType() << endl;
		return true;
	}
	if (lexem.GetType() == "kwint")
	{
		p.SetValue("int");
		getNextLexem();
		cout << "lexem: " << lexem.GetType() << endl;
		return true;
	}
	return false;
}

bool Translator::DeclVarList_(int scope, AOp &p)
{
	if (lexicalError()) return false;
	if (lexem.GetType() == "comma")
	{
		if (!takeTerm("comma")) return false;
		AOp q("mem", lexem.GetValue(), "");
		AOp &lq = q;
		if (!takeTerm("id")) return false;
		if (!InitVar(scope, p, lq)) return false;
		return DeclVarList_(scope, p);
	}
	return true;
}

bool Translator::InitVar(int scope, AOp &p, AOp &q)
{
	if (lexicalError()) return false;
	if (lexem.GetType() == "opassign")
	{
		if (!takeTerm("opassign")) return false;
		int num = stoi(lexem.GetValue());
		if (!takeTerm("num")) return false;
		if (!setIdType(stoi(q.GetValue()), scope, p.GetValue(), num)) return false;
		return true;
	}
	if (lexem.GetType() == "lbracket")
	{
		if (!takeTerm("lbracket")) return false;
		int num = stoi(lexem.GetValue());
		if (!takeTerm("num")) return false;
		if (!setArrType(stoi(q.GetValue()), scope, p.GetValue(), num)) return false;
		if (!takeTerm("rbracket")) return false;
		return true;
	}
	if (!setIdType(stoi(q.GetValue()), scope, p.GetValue(), 0)) return false;
	return true;
}

bool Translator::ParamList(int scope, AOp &n, AOp &p)
{
	if (lexicalError()) return false;
	string lex_type_list[] = { "kwchar", "kwint" };
	if (isInList(lex_type_list, 2, lexem.GetType()))
	{
		AOp q;
		AOp &lq = q;
		if (!Type(scope, lq)) return false;
		AOp r("mem", lexem.GetValue(), "");
		if (!takeTerm("id")) return false;
		if (!setIdType(stoi(r.GetValue()), stoi(p.GetValue()), q.GetValue(), 0)) return false;
		AOp s;
		AOp &ls = s;
		if (!ParamList_(scope, ls, p)) return false;
		n.SetValue(to_string(stoi(s.GetValue()) + 1));
		return true;
	}
	n.SetValue("0");
	return true;
}

bool Translator::ParamList_(int scope, AOp &n, AOp &p)
{
	if (lexicalError()) return false;
	if (lexem.GetType() == "comma")
	{
		if (!takeTerm("comma")) return false;
		AOp q;
		AOp &lq = q;
		if (!Type(scope, lq)) return false;
		AOp r("mem", lexem.GetValue(), "");
		if (!takeTerm("id")) return false;
		if (!setIdType(stoi(r.GetValue()), stoi(p.GetValue()), q.GetValue(), 0)) return false;
		AOp s;
		AOp &ls = s;
		if (!ParamList_(scope, ls, p)) return false;
		n.SetValue(to_string(stoi(s.GetValue()) + 1));
		return true;
	}
	n.SetValue("0");
	return true;
}

bool Translator::AssignOrCallOp(int scope)
{
	if (lexicalError()) return false;
	if (!AssignOrCall(scope)) return false;
	if (!takeTerm("semicolon")) return false;
	return true;
}

bool Translator::AssignOrCall(int scope)
{
	if (lexicalError()) return false;
	AOp p("mem", lexem.GetValue(), "");
	AOp &lp = p;
	if (!takeTerm("id")) return false;
	return AssignOrCall_(scope, lp);
}

bool Translator::AssignOrCall_(int scope, AOp &p)
{
	if (lexicalError()) return false;
	if (lexem.GetType() == "opassign")
	{
		if (!takeTerm("opassign")) return false;
		Lexem P(p.GetType(), p.GetValue());
		if (!checkId(scope, P)) return false;
		AOp q;
		if (!E(scope, q)) return false;
		AOp n;
		generateAtom(scope, "Amov", q, n, p);
		return true;
	}
	if (lexem.GetType() == "lbracket")
	{
		if (!takeTerm("lbracket")) return false;
		Lexem P(p.GetType(), p.GetValue());
		if (!checkArr(scope, P)) return false;
		AOp q;
		if (!E(scope, q)) return false;
		if (!takeTerm("rbracket")) return false;
		if (!takeTerm("opassign")) return false;
		AOp r;
		AOp &lr = r;
		if (!E(scope, lr)) return false;
		AOp n;
		AOp tmpAOp("arr", p.GetValue(), q.GetIndex());
		generateAtom(scope, "Amov", r, n, tmpAOp);
		return true;
	}
	if (lexem.GetType() == "lpar")
	{
		if (!takeTerm("lpar")) return false;
		AOp N;
		AOp &lN = N;
		if (!ArgList(scope, lN)) return false;
		Lexem P(p.GetType(), p.GetValue());
		if (!checkFunc(scope, P, stoi(N.GetValue()))) return false;
		if (!takeTerm("rpar")) return false;
		AOp r = Alloc(scope);
		AOp n;
		generateAtom(scope, "Acall", p, n, r);
		return true;
	}
	cout << "Syntax error (Assign_): unexpected lexem " << endl;
	return false;
}

bool Translator::WhileOp(int scope)
{
	if (lexicalError()) return false;
	if (!takeTerm("kwwhile")) return false;
	AOp l1 = newlab();
	AOp l2 = newlab();
	AOp n;
	generateAtom(scope, "Albl", n, n, l1);
	if (!takeTerm("lpar")) return false;
	AOp p;
	if (!E(scope, p)) return false;
	if (!takeTerm("rpar")) return false;
	generateAtom(scope, "Aeq", p, const0, l2);
	if (!Stmt(scope)) return false;
	generateAtom(scope, "Ajmp", n, n, l1);
	generateAtom(scope, "Albl", n, n, l2);
	return true;
}

bool Translator::ForOp(int scope)
{
	if (lexicalError()) return false;
	AOp l1 = newlab();
	AOp l2 = newlab();
	AOp l3 = newlab();
	AOp l4 = newlab();
	if (!takeTerm("kwfor")) return false;
	if (!takeTerm("lpar")) return false;
	if (!ForInit(scope)) return false;
	if (!takeTerm("semicolon")) return false;
	AOp n;
	generateAtom(scope, "Albl", n, n, l1);
	AOp p;
	AOp &lp = p;
	if (!ForExp(scope, lp)) return false;
	if (!takeTerm("semicolon")) return false;
	generateAtom(scope, "Aeq", p, const0, l4);
	generateAtom(scope, "Ajmp", n, n, l3);
	generateAtom(scope, "Albl", n, n, l2);
	if (!ForLoop(scope)) return false;
	generateAtom(scope, "Ajmp", n, n, l1);
	if (!takeTerm("rpar")) return false;
	generateAtom(scope, "Albl", n, n, l3);
	if (!Stmt(scope)) return false;
	generateAtom(scope, "Ajmp", n, n, l2);
	generateAtom(scope, "Albl", n, n, l4);
	return true;
}

bool Translator::ForInit(int scope)
{
	if (lexicalError()) return false;
	if (lexem.GetType() == "id")
		return AssignOrCall(scope);
	return true;
}

bool Translator::ForExp(int scope, AOp &p)
{
	if (lexicalError()) return false;
	string lex_type_list[] = { "opinc", "lpar", "opnot", "num", "id", "chr" };
	if (isInList(lex_type_list, 6, lexem.GetType()))
		return E(scope, p);
	p.SetType("num");
	p.SetValue("1");
	return true;
}

bool Translator::ForLoop(int scope)
{
	if (lexicalError()) return false;
	if (lexem.GetType() == "id")
		return AssignOrCall(scope);
	if (lexem.GetType() == "opinc")
	{
		if (!takeTerm("opinc")) return false;
		AOp p("mem", lexem.GetValue(), "");
		AOp &lp = p;
		if (!takeTerm("id")) return false;
		return ForLoop_(scope, lp);
	}
	return true;
}

bool Translator::ForLoop_(int scope, AOp &p)
{
	if (lexicalError()) return false;
	if (lexem.GetType() == "lbracket")
	{
		Lexem P(p.GetType(), p.GetValue());
		if (!checkArr(scope, P)) return false;
		if (!takeTerm("lbracket")) return false;
		AOp r;
		AOp &lr = r;
		if (!E(scope, lr)) return false;
		if (!takeTerm("rbracket")) return false;
		AOp tmp = AOp("arr", p.GetValue(), r.GetIndex());
		generateAtom(scope, "Aadd", tmp, const1, tmp);
		return true;
	}
	Lexem P(p.GetType(), p.GetValue());
	if (!checkId(scope, P)) return false;
	generateAtom(scope, "Aadd", p, const1, p);
	return true;
}

bool Translator::IfOp(int scope)
{
	if (lexicalError()) return false;
	AOp l1 = newlab();
	AOp l2 = newlab();
	if (!takeTerm("kwif")) return false;
	if (!takeTerm("lpar")) return false;
	AOp p;
	AOp &lp = p;
	if (! E(scope, lp) ) return false;
	if (!takeTerm("rpar")) return false;
	generateAtom(scope, "Aeq", p, const0, l1);
	if (!Stmt(scope)) return false;
	AOp n;
	generateAtom(scope, "Ajmp", n, n, l2);
	generateAtom(scope, "Albl", n, n, l1);
	if (!ElsePart(scope)) return false;
	generateAtom(scope, "Albl", n, n, l2);
	return true;
}

bool Translator::ElsePart(int scope)
{
	if (lexicalError()) return false;
	if (lexem.GetType() == "kwelse")
	{
		if (!takeTerm("kwelse")) return false;
		return Stmt(scope);
	}
	return true;
}

bool Translator::SwitchOp(int scope)
{
	if (lexicalError()) return false;
	AOp end = newlab();
	AOp &lend = end;
	if (!takeTerm("kwswitch")) return false;
	if (!takeTerm("lpar")) return false;
	AOp p;
	AOp &lp = p;
	if (!E(scope, lp)) return false;
	if (!takeTerm("rpar")) return false;
	if (!takeTerm("lbrace")) return false;
	if (!Cases(scope, lp, lend)) return false;
	if (!takeTerm("rbrace")) return false;
	AOp n;
	generateAtom(scope, "Albl", n, n, end);
	return true;
}

bool Translator::Cases(int scope, AOp &p, AOp &end)
{
	if (lexicalError()) return false;
	AOp def1;
	AOp &ldef1 = def1;
	if (!ACase(scope, p, end, ldef1)) return false;
	return Cases_(scope, p, end, ldef1);
}

bool Translator::Cases_(int scope, AOp &p, AOp &end, AOp &Default)
{
	if (lexicalError()) return false;
	string lex_type_list[] = { "kwcase", "kwdefault" };
	if (isInList(lex_type_list, 2, lexem.GetType()))
	{
		AOp default1;
		AOp &ldefault1 = default1;
		if (!ACase(scope, p, end, ldefault1)) return false;
		if (Default.GetType() == "lbl" && default1.GetType() == "lbl")
		{
			cout << "Syntax error: two default sections" << endl;
			return false;
		}
		if (default1.GetType() == "lbl") Default = default1;
		return Cases_(scope, p, end, Default);
	}
	AOp q;
	if (Default.GetType() == "lbl") q = Default;
	else q = end;
	AOp n;
	generateAtom(scope, "Ajmp", n, n, q);
	return true;
}

bool Translator::ACase(int scope, AOp &p, AOp &end, AOp &Default)
{
	if (lexicalError()) return false;
	AOp next = newlab();
	if (lexem.GetType() == "kwcase")
	{
		if (!takeTerm("kwcase")) return false;
		AOp val("num", lexem.GetValue(), "");
		if (!takeTerm("num")) return false;
		generateAtom(scope, "Ane", p, val, next);
		if (!takeTerm("colon")) return false;
		if (!Stmt(scope)) return false;
		AOp n;
		generateAtom(scope, "Ajmp", n, n, end);
		generateAtom(scope, "Albl", n, n, next);
		return true;
	}
	if (lexem.GetType() == "kwdefault")
	{
		AOp default1 = newlab();
		if (!takeTerm("kwdefault")) return false;
		if (!takeTerm("colon")) return false;
		AOp n;
		generateAtom(scope, "Ajmp", n, n, next);
		generateAtom(scope, "Albl", n, n, default1);
		if (!Stmt(scope)) return false;
		generateAtom(scope, "Ajmp", n, n, end);
		generateAtom(scope, "Albl", n, n, next);
		AOp &ldefault1 = default1;
		AOp &lDefault = Default;
		opCopy(ldefault1, lDefault);
		return true;
	}
	cout << "Syntax error (ACase): unexpected lexem " << endl;
	return false;
}

bool Translator::IOp(int scope)
{
	if (lexicalError()) return false;
	if (!takeTerm("kwin")) return false;
	AOp p("mem", lexem.GetValue(), "");
	AOp &lp = p;
	if (!takeTerm("id")) return false;
	if (!IOp_(scope, lp)) return false;
	if (!takeTerm("semicolon")) return false;
	return true;
}

bool Translator::IOp_(int scope, AOp &p)
{
	if (lexicalError()) return false;
	if (lexem.GetType() == "lbracket")
	{
		Lexem P(p.GetType(), p.GetValue());
		if (!checkArr(scope, P)) return false;
		if (!takeTerm("lbracket")) return false;
		AOp q;
		AOp &lq = q;
		if (!E(scope, lq)) return false;
		if (!takeTerm("rbracket")) return false;
		AOp n;
		AOp tmpAOp("arr", p.GetValue(), q.GetIndex());
		generateAtom(scope, "Ain", n, n, tmpAOp);
		return true;
	}
	Lexem P(p.GetType(), p.GetValue());
	if (!checkId(scope, P)) return false;
	AOp n;
	generateAtom(scope, "Ain", n, n, p);
	return true;
}

bool Translator::OOp(int scope)
{
	if (lexicalError()) return false;
	if (!takeTerm("kwout")) return false;
	if (!OOp_(scope)) return false;
	if (!takeTerm("semicolon")) return false;
	return true;
}

bool Translator::OOp_(int scope)
{
	if (lexicalError()) return false;
	string lex_type_list[] = { "opinc", "lpar", "opnot", "num", "id", "chr" };
	if (isInList(lex_type_list, 6, lexem.GetType()))
	{
		AOp p;
		AOp &lp = p;
		if (!E(scope, lp)) return false;
		AOp n;
		generateAtom(scope, "Aout", n, n, p);
		return true;
	}
	if (lexem.GetType() == "str")
	{
		AOp s("str", lexem.GetValue(), "");
		if (!takeTerm("str")) return false;
		AOp n;
		generateAtom(scope, "Aout", n, n, s);
		return true;
	}
	cout << "Syntax error (OOp_): unexpected lexem " << endl;
	return false;
}




int main()
{
	Translator t("myprog");
	if (t.translate())
		cout << "Syntax OK" << endl;
	
	t.printSymbolTable();
	t.printStringTable();
	
	return 0;
}