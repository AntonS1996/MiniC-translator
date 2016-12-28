#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <fstream>
#include <cstdlib>
#include <iterator>

using namespace std;

/*
class LexType
{
private:
	map<string, int> LexTypeList;

public:
	LexType();
	~LexType();
	int GetLex(string lex);
};
*/

// Lexical Analyzer

class Lexem
{
private:
	string LexType;
	string LexValue;

public:
	Lexem();
	Lexem(string type);
	Lexem(string type, string value);
	~Lexem();
	string GetType();
	string GetValue();
	void SetType(string t);
	void SetValue(string v);
};


class SymbolTable
{
private:
	map<string, int> IdKind;
	map<string, int> IdType;
	int code;
	int tmpCode;
	map<int, map<string, string>> table;
	map<string, string> strings;

public:
	SymbolTable();
	~SymbolTable();
	int GetCode();
	int GetTmpCode();
	void Set(int code, string col, string val);
	void makeRow(string str);
	void createRow(string str, string kind, string type, int len, int Default, int scope);
	string getIdFromSymbolTable(string str);
	string getSymbolFromSymbolTable(int code);
	bool setIdType(int code, int scope, string type, int Default);
	bool setArrType(int code, int scope, string type, int len);
	bool setFuncType(int code, string type, int len);
	void printTable();
	string Alloc(int scope);
	bool checkId(int scope, Lexem attr);
	int findCodeForIdInScope(int scope, int code);
	bool checkArr(int scope, Lexem attr);
	bool checkFunc(int scope, Lexem attr, int len);
};

class StringTable
{
private:
	map<int, string> table;
	map<string, int> strings;
	int num;

public:
	StringTable();
	~StringTable();
	int GetNum();
	int getIdFromStringTable(string str);
	string getStringFromStringTable(int id);
	void printTable();
};

class Scanner
{
private:
	map<string, string> keywords;
	map<string, string> LexemStrings;
	string Letter;
	string Digit;
	ifstream file;
	int Status;
	string Filename;
	string savedChar;
	int State;
	string Value;
	SymbolTable symbolTable;
	StringTable stringTable;

public:
	Scanner(string filename);
	~Scanner();
	int getStatus();
	void printLexem(Lexem lexem);
	void printSymbolTable();
	void printStringTable();
	string getIdType(string id);
	string getNextChar();
	void returnChar(string chr);
	Lexem getNextLexem();
	SymbolTable &GetSymbolTable();
	map<string, string> &GetLexemStrings();
};


// Syntax Analyzer

/*
class AtomType
{
private:
	map<string, int> AtomTypeList;

public:
	AtomType();
	~AtomType();
	int GetAtom(string atom);
};

class AOpType
{
private:
	map<string, int> AOpTypeList;

public:
	AOpType();
	~AOpType();
	int GetAOp(string AOp);
};
*/

class AOp
{
private:
	string type;
	string value;
	string index;

public:
	AOp();
	AOp(string Type, string Value, string Index);
	~AOp();
	string GetType();
	string GetValue();
	string GetIndex();
	void SetType(string Type);
	void SetValue(string Value);
	void SetIndex(string Index);
};

class Translator
{
private:
	map<string, string> AtomStrings;
	AOp const0;
	AOp const1;
	string Filename;
	ofstream outfile;
	int Status;
	int LabelCount;

public:
	Translator(string filename);
	~Translator();
	int getStatus();
	bool getNextLexem();
	AOp Alloc(int scope); // scope = -1
	AOp newlab();
	string printAOp(AOp op);
	void generateAtom(int scope, string atom, AOp op1, AOp op2, AOp op3);
	void printSymbolTable();
	void printStringTable();
	int newLabel();
	bool lexicalError();
	bool takeTerm(string lextype);
	bool checkId(int scope, Lexem attr); // scope = -1, attr = None
	bool checkArr(int scope, Lexem attr); // scope = -1, attr = None
	bool checkFunc(int scope, Lexem attr, int params); // scope = -1, attr = None, params = 0
	bool setIdType(int code, int scope, string type, int Default); // Default = 0
	bool setArrType(int code, int scope, string type, int len);
	bool setFuncType(int code, string type, int len);
	void opCopy(AOp &from_, AOp &to_);
	bool translate();
	bool E(int scope, AOp &p);
	bool E7(int scope, AOp &p);
	bool E7_(int scope, AOp &p, AOp &q);
	bool E6(int scope, AOp &p);
	bool E6_(int scope, AOp &p, AOp &q);
	bool E5(int scope, AOp &p);
	bool E5_(int scope, AOp &p, AOp &q);
	bool E4(int scope, AOp &p);
	bool E4_(int scope, AOp &p, AOp &q);
	bool E3(int scope, AOp &p);
	bool E3_(int scope, AOp &p, AOp &q);
	bool E2(int scope, AOp &p);
	bool E1(int scope, AOp &p);
	bool E1_(int scope, AOp &p, AOp &q);
	bool E1__(int scope, AOp &p, AOp &q);
	bool E1___(int scope, AOp &p, AOp &q);
	bool ArgList(int scope, AOp &n);
	bool ArgList_(int scope, AOp &n);
	bool StmtList(int scope);
	bool Stmt(int scope);
	bool DeclareStmt(int scope);
	bool DeclareStmt_(int scope, AOp &p, AOp &q);
	bool Type(int scope, AOp &p);
	bool DeclVarList_(int scope, AOp &p);
	bool InitVar(int scope, AOp &p, AOp &q);
	bool ParamList(int scope, AOp &n, AOp &p);
	bool ParamList_(int scope, AOp &n, AOp &p);
	bool AssignOrCallOp(int scope);
	bool AssignOrCall(int scope);
	bool AssignOrCall_(int scope, AOp &p);
	bool WhileOp(int scope);
	bool ForOp(int scope);
	bool ForInit(int scope);
	bool ForExp(int scope, AOp &p);
	bool ForLoop(int scope);
	bool ForLoop_(int scope, AOp &p);
	bool IfOp(int scope);
	bool ElsePart(int scope);
	bool SwitchOp(int scope);
	bool Cases(int scope, AOp &p, AOp &end);
	bool Cases_(int scope, AOp &p, AOp &end, AOp &Default);
	bool ACase(int scope, AOp &p, AOp &end, AOp &Default);
	bool IOp(int scope);
	bool IOp_(int scope, AOp &p);
	bool OOp(int scope);
	bool OOp_(int scope);

	Scanner scanner;
	Lexem lexem;

};