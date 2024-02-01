
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cassert>
#include <algorithm>
#include <windows.h> // winapi, only for copying to clipboard


// settings
int g_param_iter_count = 1000*1000;
int g_buf_len = 0;

// stats
int g_stats_current_iter = 0;
int g_stats_current_char = 0;
int g_stats_iter = 0;


void stats_print()
{
	int one_percent = g_param_iter_count / 100 * 2; // 2 for shorter line in console
	
	if (g_stats_iter == 0 && g_stats_current_char == 0)
	{
		// header
		printf("  Progress [");
		for (int iter = 0; iter < g_param_iter_count; iter += one_percent)
		{
			printf(" ");
		}
		printf("]\n");
	}
	
	if (g_stats_iter == 0)
	{
		// begin line
		printf("char %2d/%2d [", g_stats_current_char + 1, g_buf_len);
	}

	g_stats_current_iter += 1;
	g_stats_iter += 1;
	
	if (g_stats_current_iter >= one_percent)
	{
		// progress
		g_stats_current_iter = 0;
		printf(".");
	}
	if (g_stats_iter >= g_param_iter_count)
	{
		// end line
		g_stats_iter = 0;
		printf("]\n");
	}
}


int gen_some_number(int iter)
{
	int res = 1;
	for (int i = 0; i < 1000 * 1000; i++)
	{
		res *= iter;
		res ^= iter;
		res ^= i;
		res &= 0xff;
	}
	return res;
}


int encode_single_value(int val, int prev_encoded_value)
{
	int res = val;

	const char* some_string = "Niech sie pan nad tym zastanowi";
	int len = (int)strlen(some_string);
	
	for (int i = 0; i < g_param_iter_count; i++)
	{
		stats_print();

		int stringcode = some_string[i % len];
		int itercode = gen_some_number(i);

		res += stringcode;
		res ^= itercode;
		res += prev_encoded_value;
		res ^= prev_encoded_value;
	}

	return res;
}


void encode_string( const char* in_buf, char* out_buf )
{
	int len = (int)strlen(in_buf);
	out_buf[len] = 0;

	int res[100] = { 0, };
	for (g_stats_current_char = 0; in_buf[g_stats_current_char]; g_stats_current_char++)
	{
		int prev_encoded_value = g_stats_current_char == 0 ? 0 : res[g_stats_current_char - 1];
		res[g_stats_current_char] = encode_single_value(in_buf[g_stats_current_char] - '0', prev_encoded_value);
	}

	// fix for '0'..'9' digits char codes and store
	for (int i = 0; in_buf[i]; i++)
	{
		res[i] &= 0xffff;
		res[i] %= 10;
		res[i] += '0';
		out_buf[i] = res[i];
	}
}


void copy_to_clipboard(const char* buf)
{
	OpenClipboard(GetDesktopWindow());
	EmptyClipboard();
	HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, strlen(buf) + 1);
	if (!handle)
	{
		CloseClipboard();
		return;
	}
	void* mem = GlobalLock(handle);
	if (!mem)
	{
		CloseClipboard();
		return;
	}
	memcpy(mem, buf, strlen(buf) + 1);
	GlobalUnlock(handle);
	SetClipboardData(CF_TEXT, handle);
	CloseClipboard();
	GlobalFree(handle);
}


int main(int argc, char* argv[ ])
{
	// settings

	if (argc >= 2)
	{
		g_param_iter_count = std::clamp( atoi(argv[1]), 1000*10, 1000*1000*10);		
	}	

	//

	const char* in_buf = "1267650600228229401496703205376";
	char out_buf[100] = { 0, };
	g_buf_len = (int)strlen(in_buf);
	assert(g_buf_len < 100);

	//

	printf("Long-lasting password generation v1.0\n\n");
	printf("Usage: PasswordGen [iter_count]\n\n");
	printf("  - iter_count: number of iterations (default 1000000)\n\n\n");
	printf("Generating %d-chars length password with iter_count=%d\n\n", g_buf_len, g_param_iter_count);

	// encode

	encode_string(in_buf, out_buf);
	copy_to_clipboard(out_buf);

	// result

	printf("\n\nGenerated password:\n");
	printf("%s\n\n", out_buf);
	printf("Password is copied to the clipboard. Store password, then type something and press enter to exit\n");

	// wait for input

	char line[100] = { 0, };
	scanf_s("%s", line, 99);
}
