#include <string.h>
#include <malloc.h>
#include <unistd.h>
extern "C" {
	#include <reprint/reprint_stdio.h>
}

#include <cstdio>
#include <vector>
#include <string>
#include <benejson/pull.hh>
#include "posix.hh"

using BNJ::PullParser;

/* Maximum output or input string is 4096. */
#define MAX_STRING 4096

enum {
	KEY_EXPECTED
	,KEY_EXPR
	,COUNT_KEYS
};

static const char* s_keys[COUNT_KEYS] = {
	"expected"
	,"expr"
};


int main(int argc, const char* argv[]){

	/* Read json from std input. */
	FD_Reader reader(0);
	uint32_t pstack[8];
	PullParser parser(8, pstack);

	/* Initialize parsing session. buffer is I/O scratch space. */
	uint8_t buffer[2048];
	parser.Begin(buffer, 2048, &reader);

	try {
		/* Expect map. */
		parser.Pull();
		BNJ::VerifyMap(parser);

		while(parser.Pull() != PullParser::ST_ASCEND_MAP){
			/* Expect map. */
			if(parser.GetState() != PullParser::ST_MAP)
				return 1;

			/* Key name will be test name. */
			char key[512];
			unsigned keylen = BNJ::GetKey(key, 512, parser);

			/* Expected output. */
			uint8_t expected[MAX_STRING];
			expected[0] = '\0';
			unsigned expected_length = 0;

			/* Marshall the data into this array. */
			uint8_t data[MAX_STRING];

			char exprbuffer[MAX_STRING];
			/* Look for specific entries. */
			while(parser.Pull(s_keys, COUNT_KEYS) != PullParser::ST_ASCEND_MAP){
				switch(parser.GetValue().key_enum){
					case KEY_EXPR:
						/* This is an array. The first element is always the reprint expression.
						 * The values following are the input data.*/
						{
							BNJ::VerifyList(parser);
							parser.Pull();

							parser.ChunkRead8((char*) exprbuffer, MAX_STRING);

							void* out = data;

							/* Iterate through the expression, looking for input specifiers. */
							const char* i = exprbuffer;
							uint16_t specifier = 0;
							while((i = reprint_get_next_param(&specifier, i))){

								/* Pull data from JSON. */
								if(PullParser::ST_DATUM != parser.Pull())
									throw PullParser::invalid_value("Early end of array!", parser);

								const bnj_val& v = parser.GetValue();
								unsigned val_type = bnj_val_type(&v);
								if(BNJ_NUMERIC == val_type){
									/* If value had a minus, then read it as signed int. Otherwise
									 * read it as unsigned. */
									if(!(specifier & REP_FLAG_SPECIFIER_IS_REGISTER)){
										/* If integral process as signed or unsigned.
										 * Otherwise process as float. */

										if(((specifier >> 4) & 0x7) < 4){
											if(BNJ_VFLAG_NEGATIVE_SIGNIFICAND & v.type){
												int x;
												BNJ::Get(x, parser);

												out = reprint_marshall_signed(out, specifier, x);
												if(!out)
													throw PullParser::invalid_value("Type mismatch!", parser);
											}
											else{
												unsigned u;
												BNJ::Get(u, parser);

												out = reprint_marshall_unsigned(out, specifier, u);
												if(!out)
													throw PullParser::invalid_value("Type mismatch!", parser);
											}
										}
										else if(((specifier >> 4) & 0x7) == 6){
											double d;
											BNJ::Get(d, parser);
											out = reprint_marshall_bin_floating_pt(out, specifier, d);
											if(!out)
												throw PullParser::invalid_value("Type mismatch!", parser);
										}
										else{
											throw PullParser::invalid_value("Type mismatch!", parser);
										}
									}
									else{
										unsigned u;
										BNJ::Get(u, parser);

										out = reprint_marshall_unsigned(out, REP_REGISTER_SPECIFIER, u);
										if(!out)
											throw PullParser::invalid_value("Type mismatch!", parser);
									}
								}
								else if(BNJ_SPECIAL == val_type){
									/* Only accept NaN or +- Infinity */
									double d;
									BNJ::Get(d, parser);

									out = reprint_marshall_bin_floating_pt(out, specifier, d);
									if(!out)
										throw PullParser::invalid_value("Type mismatch!", parser);
								}
								else if(BNJ_STRING == val_type){
									/* Get the length and just dynamically allocate a string.
									 * And we'll be lazy and rely on program exit to reap
									 * dynamically allocated memory.*/
									const unsigned len = bnj_strlen8(&v);
									uint8_t* str = new uint8_t[len + 1];
									parser.ChunkRead8((char*)str, len + 1);

									/* If character specifier, marshall the first char. */
									if(((specifier >> 4) & 0x7) == 0x5){
										out = reprint_marshall_char(out, specifier, str[0]);
									}
									else{
										out = reprint_marshall_pointer(out, specifier, str);
										if(!out)
											throw PullParser::invalid_value("Type mismatch!", parser);
									}
								}
								else{
									/* Can't do anything with this, reject. */
									throw PullParser::invalid_value("Unusable data type!", parser);
								}
							}

							/* Verify we reached end of array. */
							if(parser.Pull() != PullParser::ST_ASCEND_LIST)
								throw PullParser::invalid_value("Extra elements in value!", parser);
						}
						break;

					case KEY_EXPECTED:
						expected_length = parser.ChunkRead8((char*)expected, MAX_STRING);
						break;

					/* Unrecognized key. */
					default:
						throw PullParser::invalid_value("Unrecognized key!", parser);
				}
			}

			uint8_t buffer[MAX_STRING];
			memset(buffer, 0, MAX_STRING);

			/* Print to buffer */
			int output_len =
				resnprintf_packed(buffer, MAX_STRING, exprbuffer, data);

			if(output_len < 0)
				throw std::runtime_error("Error in resnprintpf!");

			if(expected_length != (unsigned)output_len
				|| memcmp(buffer, expected, expected_length))
			{
				reprintf("FAILED: \f=dp\n'\f=dp'\n'\f=dp'\n\n",
					keylen, key, output_len, buffer, expected_length, expected);
			}
		}
	}
	catch(const std::exception& e){
		refprintf(stderr, "\fdp\n", e.what());
		return 1;
	}

	close(1);

	return 0;
}
