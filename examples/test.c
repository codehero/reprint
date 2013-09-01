/* reprint_stdio.c
 * Copyright (c) 2013 David Bender. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:

 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * */


#include <unistd.h>
#include <reprint/reprint.h>

void print(void){
	char out;
	while(reprint_cb(&out))
		write(1, &out, 1);
}

int main(int argc, const char* argv[]){

	/* Fixed point number XX.XX. */
	{
		unsigned u = 10234;
		const char test[] = "TEST \f2<E\n";
		reprint_init(test, &u, 1);
		print();
	}

	/* Numbers in field widths. Pad with '_' (0x5F) chars*/
	/* printf equivalent:      none  */
	{
		unsigned u = 10234;
		const char test[] = "TEST \f0137:8E\n";
		reprint_init(test, &u, 1);
		print();
	}

	/* Numbers in field widths. */
	{
		unsigned u = 10234;
		/* printf equivalent:      %-08u  */
		const char test[] = "TEST \f,8E\n";
		reprint_init(test, &u, 1);
		print();
	}

	/* numbers. */
	{
		unsigned u = 10234;
		const char test[] = "TEST \fE\n";
		reprint_init(test, &u, 1);
		print();
	}

	{
		const char test[] = "TEST\n";

		/* Should not crash. */
		reprint_init(test, NULL, 1);
		print();
	}

	/* Test registers passed in from user.  */
	{
		const char test[] = "TEST \f;h\n";
		const char chars[] = "abcdef";
		for(unsigned i = 0; i < sizeof(chars) - 1; ++i){
			struct test {
				reprint_reg_t r;
				uint8_t c;
			};

			struct test t;
			t.r = i;
			t.c = chars[i];
			reprint_init(test, &t, 1);
			print();
		}
	}

	/* Now with right padding Test registers passed in from user.  */
	{
		const char test[] = "TEST \f045:;8h\n";
		const char chars[] = "abcdefghi";
		for(unsigned i = 0; i < sizeof(chars) - 1; ++i){
			struct test {
				reprint_reg_t r;
				uint8_t c;
			};

			struct test t;
			t.r = i;
			t.c = chars[i];
			reprint_init(test, &t, 1);
			print();
		}
	}

	/* Now with left padding Test registers passed in from user.  */
	{
		const char test[] = "TEST \f,045:;8h\n";
		const char chars[] = "abcdefghi";
		for(unsigned i = 0; i < sizeof(chars) - 1; ++i){
			struct test {
				reprint_reg_t r;
				uint8_t c;
			};

			struct test t;
			t.r = i;
			t.c = chars[i];
			reprint_init(test, &t, 1);
			print();
		}
	}

	return 0;
}
