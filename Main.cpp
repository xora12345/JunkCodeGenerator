#pragma optimize("", off)
#define _WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#define zero(x) memset(&x, 0, sizeof(x));
#define arr_size(x) ((sizeof(x) / sizeof(*x)) - 1)
#define arr_rand(x) (rand() % arr_size(x))
#define rand_from(x) x[arr_rand(x)]

char g_buff[MAX_PATH];
const char g_chars[] = { "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" };

const char * randstr(int len)
{
	for(int i = 0; i < len; ++i)
		g_buff[i] = rand_from(g_chars);
	g_buff[len] = '\0';

	return g_buff;
}

void init_head(std::string & buff)
{
	const char * guard = randstr(15);
	buff.append("#define _CRT_SECURE_NO_WARNINGS\n#pragma optimize(\"\", off)\n#ifndef ");
	buff.append(guard);
	buff.append("\n#define ");
	buff.append(guard);
	buff.append("\n");
	buff.append("int ___strlen(const char * str) { if(!str) return 0; int len = 0; while('\\0' != *str) ++len; return len; }\n\n");
}

void start_block(std::string & buff, int nr)
{
	buff.append("\n#define __JUNK_CODE"); buff.append(std::to_string(nr)); buff.append("__ ");
}

void uninit_head(std::string & buff)
{
	buff.append("\n#endif");
}

struct g_vardata
{
	std::string name;
	enum { _char, _short, _int, _long, _str } type;
};
std::vector<g_vardata> g_remember_var_names;
static const char * g_data_types[] = { "char", "short", "int", "long" };

#define __rand_var(buff, type, pick, maxx) buff.append(g_data_types[pick]); buff.append(" "); \
buff.append(randstr(10)); g_remember_var_names.push_back({ g_buff, type }); buff.append(" = "); \
buff.append(std::to_string(rand() % maxx)); buff.append("; \\\n")
#define __rand_str(buff) buff.append("const char * "); buff.append(randstr(10)); g_remember_var_names.push_back({ g_buff, g_vardata::_str }); \
buff.append(" = \""); buff.append(randstr(rand() % 35)); buff.append("\"; \\\n")

void add_rand_var(std::string & buff)
{
	int pick = arr_rand(g_data_types);
	switch(pick)
	{
		case(0): __rand_var(buff, g_vardata::_char, pick, CHAR_MAX); break;
		case(1): __rand_var(buff, g_vardata::_short, pick, SHRT_MAX); break;
		case(2): __rand_var(buff, g_vardata::_int, pick, INT_MAX); break;
		case(3): __rand_var(buff, g_vardata::_long, pick, LONG_MAX); break;
	}
	__rand_str(buff);
}

void add_var_shf(std::string & buff)
{
	for(int i = 0; i < g_remember_var_names.size(); ++i)
	{
		g_vardata & curr = g_remember_var_names[i];
		if(g_vardata::_str != curr.type)
		{
			buff.append(curr.name); buff.append(" << ");
			switch(curr.type)
			{
				case(g_vardata::_char): buff.append(std::to_string(rand() % 7)); break;
				case(g_vardata::_short): buff.append(std::to_string(rand() % 15)); break;
				case(g_vardata::_int): buff.append(std::to_string(rand() % 31)); break;
				case(g_vardata::_long): buff.append(std::to_string(rand() % 63)); break;
				default: break;
			}
			buff.append("; \\\n");
		}
	}
}

void add_var_loop(std::string & buff)
{
	std::string mgc = randstr(15);
	buff.append("volatile unsigned char ");
	buff.append(mgc);
	buff.append(" = 0;\\\n");
	for(int i = 0; i < g_remember_var_names.size(); ++i)
	{
		g_vardata & curr = g_remember_var_names[i];
		ULONGLONG max = 0;
		switch(curr.type)
		{
			case(g_vardata::_char): max = CHAR_MAX; break;
			case(g_vardata::_short): max = SHRT_MAX; break;
			case(g_vardata::_int): max = INT_MAX; break;
			case(g_vardata::_long): max = LONG_MAX; break;
			case(g_vardata::_str):
			{
				buff.append("if(1 == " + mgc + ") { for(int i = 0; i < ___strlen(" + curr.name + "); ++i) {");
				buff.append("++" + curr.name + "; if(*" + curr.name + " == '\\0') break; }}\\\n" );
				if(i + 1 == g_remember_var_names.size())
				{
					buff.append(mgc + " = 1; \\");
				}
				continue;
			}
			default: break;
		}
		int min = 100 + (rand() % 10000);
		buff.append("if(1 == " + mgc + ") { for(ULONGLONG i = 0; i < (");
		buff.append(std::to_string(max));
		buff.append(" - ");
		buff.append(std::to_string(min));
		buff.append("); ++i) { ");
		buff.append(curr.name);
		buff.append("++; } } \\\n");

		if(i + 1 == g_remember_var_names.size())
		{
			buff.append(mgc + " = 1; \\");
		}
	}
}

void gen_junk_blk(std::string & buff, int nr)
{
	start_block(buff, nr);
	for(int i = 0; i < 25; ++i)
	{
		add_rand_var(buff);
	}
	add_var_shf(buff);
	add_var_loop(buff);
}

void gen_jnk_blks(std::string & buff, int blk_nr)
{
	srand(GetTickCount());
	buff.reserve(0x1000);
	init_head(buff);
	for(int i = 0; i < blk_nr; ++i)
	{
		gen_junk_blk(buff, i);
		buff.erase(buff.end() - 2, buff.end());
	}
	uninit_head(buff);
}

int main(int argc, char ** argv)
{
	if(argc == 1)
		return 0;

	int nr = std::stoi(argv[1]);
	if(nr > 50)
		nr = 50;

	if(nr < 0)
		nr = 1;

	std::string res{};
	
	gen_jnk_blks(res, nr);

	std::ofstream f;
	f.open("junk.h");
	f << res;
	f.close();

	return 0;
}
