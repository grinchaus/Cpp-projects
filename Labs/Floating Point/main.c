#include "return_codes.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct Digit
{
	bool sign;

	uint64_t mantissa;
	int32_t exponent;
} Digit;

void add(uint8_t rounding, int8_t bit, Digit *digit1, Digit *digit2, Digit *digit_res);

void normalization(int8_t bit, Digit *digit, uint8_t *last_bit, uint8_t *kol)
{
	uint64_t not_one = ((digit->mantissa) - ((digit->mantissa) & ((bit == 23) ? 0x7fffff : 0x3FF))) >> bit;
	while ((digit->mantissa) >> bit >= 2 && digit->mantissa != 0)
	{
		digit->exponent++;
		*last_bit = (digit->mantissa & 1);
		*kol += *last_bit;
		digit->mantissa >>= 1;
		not_one >>= 1;
	}
	while ((((digit->mantissa) >> bit) & 1) != 1 && digit->mantissa != 0)
	{
		digit->mantissa <<= 1;
		digit->exponent--;
	}
}

void parse_numb(int8_t bit, const int32_t *num, Digit *digit)
{
	int8_t len_bit = bit == 23 ? 127 : 15;
	int32_t const_exponent, const_mantissa;
	if (bit == 23)
	{
		// для digit->sign отсаётся делать только так, потому что иначе тесты начинают падать
		digit->sign = ((*num) & 0x80000000) >> len_bit;
		const_exponent = 0x7f800000;
		const_mantissa = 0x7fffff;
	}
	else
	{
		digit->sign = ((*num) & 0x8000) >> len_bit;
		const_exponent = 0x7C00;
		const_mantissa = 0x3FFl;
	}
	digit->exponent = (((*num) & const_exponent) >> bit);
	digit->mantissa = (*num) & const_mantissa;
	if (digit->exponent == 0 && digit->mantissa == 0 || digit->exponent == len_bit * 2 + 1)
	{
		return;
	}
	else if (digit->exponent == 0)
	{
		int8_t k = 0;
		uint64_t number = digit->mantissa;
		while (number > 0)
		{
			k++;
			number = number >> 1;
		}
		digit->mantissa <<= bit + 1 - k;
		digit->exponent -= bit - k;
	}
	else
	{
		digit->mantissa += 1 << bit;
	}
	digit->exponent -= len_bit;
	uint8_t trash1, trash2 = 0;
	normalization(bit, digit, &trash1, &trash2);
}

void rounder(uint8_t rounding, int8_t bit, Digit *digit, uint8_t *last_bit, uint8_t *kol)
{
	switch (rounding)
	{
	case 1:
		if (*last_bit == 1 && (*kol) == 1 && ((digit->mantissa + 1) % 2) == 0 || *last_bit == 1 && *kol > 1)
		{
			digit->mantissa++;
		}
		break;
	case 2:
		if (digit->sign == 0 && *kol != 0)
		{
			digit->mantissa++;
		}
		break;
	case 3:
		if (digit->sign == 1 && *kol != 0)
		{
			digit->mantissa--;
		}
		break;
	}
	normalization(bit, digit, last_bit, kol);
}

bool is_inf(const Digit *digit, int8_t len_bit)
{
	return digit->exponent >= len_bit * 2 + 1 && digit->mantissa == 0;
}

bool is_nan(const Digit *digit, int8_t len_bit)
{
	return digit->exponent == len_bit * 2 + 1 && digit->mantissa != 0;
}

bool is_zero(const Digit *digit)
{
	return digit->exponent == 0 && digit->mantissa == 0;
}

void print_digit(int8_t bit, Digit *digit)
{
	int8_t len_bit = (bit == 23) ? 127 : 15;
	if (is_nan(digit, len_bit))
	{
		printf("nan");
		return;
	}
	(digit->sign == 1) ? putchar('-') : 0;
	if (digit->exponent >= len_bit + 1)
	{
		printf("inf");
		return;
	}
	if (is_zero(digit))
	{
		printf("0x0.");
	}
	else
	{
		digit->mantissa -= 1 << bit;
		printf("0x1.");
	}
	if (bit == 23)
	{
		printf("%06" PRIx64 "p%+d", ((digit->mantissa) << 1), digit->exponent);
	}
	else
	{
		printf("%03" PRIx64 "p%+d", ((digit->mantissa) << 2), digit->exponent);
	}
}

bool is_Bigger(Digit *digit1, Digit *digit2)
{
	return digit1->exponent > digit2->exponent || digit1->exponent == digit2->exponent && digit1->mantissa > digit2->mantissa;
}

void divide(uint8_t rounding, int8_t bit, Digit *digit1, Digit *digit2, Digit *digit_res)
{
	int8_t len_bit = (bit == 23) ? 127 : 15;
	if (is_nan(digit1, len_bit) || is_nan(digit2, len_bit) != 0 || is_inf(digit1, len_bit) && is_inf(digit2, len_bit) ||
		is_zero(digit1) && is_zero(digit2))
	{
		*digit_res = (Digit){ .sign = 0, .exponent = len_bit * 2 + 1, .mantissa = 1 };
		return;
	}
	else if (is_zero(digit1) || is_inf(digit2, len_bit))
	{
		*digit_res = (Digit){ .sign = (digit1->sign + digit2->sign) % 2, .exponent = 0, .mantissa = 0 };
		return;
	}
	else if (is_zero(digit2) || is_inf(digit1, len_bit))
	{
		*digit_res = (Digit){ .sign = (digit1->sign + digit2->sign) % 2, .exponent = len_bit * 2 + 1, .mantissa = 0 };
		return;
	}
	digit1->mantissa <<= 40;
	digit1->exponent -= 40;
	digit1->mantissa /= digit2->mantissa;
	digit1->exponent -= digit2->exponent - bit;
	digit1->sign = (digit1->sign + digit2->sign) % 2;
	uint8_t last_bit = 0, kol = 0;
	normalization(bit, digit1, &last_bit, &kol);
	rounder(rounding, bit, digit1, &last_bit, &kol);
	(*digit_res) = *digit1;
}

void multiply(uint8_t rounding, int8_t bit, Digit *digit1, Digit *digit2, Digit *digit_res)
{
	digit1->sign = (digit1->sign + digit2->sign) % 2;
	int8_t len_bit = (bit == 23) ? 127 : 15;
	if (is_nan(digit1, len_bit) || is_nan(digit2, len_bit) || is_inf(digit1, len_bit) && is_zero(digit2) ||
		is_inf(digit2, len_bit) && is_zero(digit1))
	{
		*digit_res = (Digit){ .sign = 0, .exponent = len_bit * 2 + 1, .mantissa = 1 };
		return;
	}
	else if (is_zero(digit1) || is_inf(digit1, len_bit))
	{
		*digit_res = (Digit){ .sign = digit1->sign, .exponent = digit1->exponent, .mantissa = digit1->mantissa };
		return;
	}
	else if (is_zero(digit2) || is_inf(digit2, len_bit))
	{
		*digit_res = (Digit){ .sign = digit1->sign, .exponent = digit2->exponent, .mantissa = digit2->mantissa };
		return;
	}
	digit1->exponent += digit2->exponent - bit;
	digit1->mantissa *= digit2->mantissa;
	uint8_t last_bit = 0, kol = 0;
	normalization(bit, digit1, &last_bit, &kol);
	rounder(rounding, bit, digit1, &last_bit, &kol);
	(*digit_res) = *digit1;
}

void subtract(uint8_t rounding, int8_t bit, Digit *digit1, Digit *digit2, Digit *digit_res)
{
	int8_t len_bit = (bit == 23) ? 127 : 15;
	if (is_zero(digit1) && is_zero(digit2))
	{
		*digit_res = *digit1;
		digit_res->sign = (digit1->sign == 1 && digit2->sign == 0) ? 1 : 0;
		return;
	}
	else if (is_inf(digit1, len_bit) && is_inf(digit2, len_bit) && digit1->sign == digit2->sign ||
			 is_nan(digit1, len_bit) || is_nan(digit2, len_bit))
	{
		*digit_res = (Digit){ .sign = 0, .exponent = len_bit * 2 + 1, .mantissa = 1 };
		return;
	}
	else if (is_zero(digit1) || is_inf(digit2, len_bit))
	{
		digit2->sign = (digit2->sign + 1) % 2;
		*digit_res = *digit2;
		return;
	}
	else if (is_zero(digit2) || is_inf(digit1, len_bit))
	{
		*digit_res = *digit1;
		return;
	}
	if (digit1->sign == 1 && digit2->sign == 1)
	{
		digit1->sign = digit2->sign = 0;
		subtract(rounding, bit, digit1, digit2, digit_res);
		digit_res->sign = (digit_res->sign + 1) % 2;
		return;
	}
	else if (digit1->sign == 1 || digit2->sign == 1)
	{
		int32_t buff = digit1->sign;
		digit1->sign = digit2->sign = 0;
		if (is_Bigger(digit1, digit2))
		{
			add(rounding, bit, digit1, digit2, digit_res);
		}
		else
		{
			add(rounding, bit, digit2, digit1, digit_res);
		}
		digit_res->sign = buff;
		return;
	}
	digit1->sign = 1;
	if (is_Bigger(digit1, digit2))
	{
		digit1->sign = 0;
	}
	uint8_t last_bit = 0, kol = 0;
	if (digit1->exponent - digit2->exponent < 39 && is_Bigger(digit1, digit2))
	{
		while (digit2->exponent < digit1->exponent && digit1->mantissa > 0)
		{
			digit1->exponent--;
			(digit1->mantissa) <<= 1;
		}
		(digit1->mantissa) -= (digit2->mantissa);
	}
	else if (digit2->exponent - digit1->exponent < 39 && is_Bigger(digit2, digit1))
	{
		while (digit2->exponent > digit1->exponent && digit2->mantissa > 0)
		{
			digit2->exponent--;
			(digit2->mantissa) <<= 1;
		}
		(digit1->mantissa) = (digit2->mantissa) - (digit1->mantissa);
	}
	else
	{
		last_bit = 1;
		kol = 10;
		(digit1->mantissa)--;
	}
	normalization(bit, digit1, &last_bit, &kol);
	rounder(rounding, bit, digit1, &last_bit, &kol);
	*digit_res = *digit1;
}

void add(uint8_t rounding, int8_t bit, Digit *digit1, Digit *digit2, Digit *digit_res)
{
	int8_t len_bit = (bit == 23) ? 127 : 15;
	if (is_zero(digit1) && is_zero(digit2))
	{
		*digit_res = *digit1;
		digit_res->sign = (digit1->sign + digit2->sign) == 2;
		return;
	}
	else if (is_inf(digit1, len_bit) && is_inf(digit2, len_bit) && digit1->sign != digit2->sign ||
			 is_nan(digit1, len_bit) || is_nan(digit2, len_bit))
	{
		*digit_res = (Digit){ .sign = 0, .exponent = len_bit * 2 + 1, .mantissa = 1 };
		return;
	}
	else if (is_zero(digit1) || is_inf(digit2, len_bit))
	{
		*digit_res = *digit2;
		return;
	}
	else if (is_zero(digit2) || is_inf(digit1, len_bit))
	{
		*digit_res = *digit1;
		return;
	}
	if (digit1->sign == 1 && digit2->sign == 1)
	{
		digit1->sign = digit2->sign = 0;
		add(rounding, bit, digit1, digit2, digit_res);
		digit_res->sign = 1;
		return;
	}
	else if (digit1->sign == 1)
	{
		digit1->sign = 0;
		subtract(rounding, bit, digit2, digit1, digit_res);
		return;
	}
	else if (digit2->sign == 1)
	{
		digit2->sign = 0;
		subtract(rounding, bit, digit1, digit2, digit_res);
		return;
	}
	uint8_t last_bit = 0, kol = 0;
	if (digit1->exponent - digit2->exponent < 39)
	{
		while (digit2->exponent != digit1->exponent && digit1->mantissa > 0)
		{
			digit1->exponent--;
			(digit1->mantissa) <<= 1;
		}
		(digit1->mantissa) += (digit2->mantissa);
	}
	else
	{
		while (digit2->exponent != digit1->exponent && digit2->mantissa != 0)
		{
			last_bit = (digit2->mantissa) & 1;
			kol += last_bit;
			digit2->exponent++;
			(digit2->mantissa) >>= 1;
		}
	}
	normalization(bit, digit1, &last_bit, &kol);
	rounder(rounding, bit, digit1, &last_bit, &kol);
	digit1->sign = 0;
	*digit_res = *digit1;
}

int main(int argc, char **argv)
{
	int8_t format, operation;
	uint8_t rounding;
	if (argc < 4 || sscanf(argv[1], "%c", &format) == 0 || sscanf(argv[2], "%hhd", &rounding) == 0)
	{
		fprintf(stderr, "Wrong input! The minimum number of arguments is 3");
		return ERROR_ARGUMENTS_INVALID;
	}
	int32_t num1, num2;
	if (format != 'f' && format != 'h' || rounding < 0 || rounding > 3 || !sscanf(argv[3], "%x", &num1))
	{
		fprintf(stderr, "Wrong input! First arg must be 'h' or 'f' second arg in [0;3] and third is 16-x number");
		return ERROR_ARGUMENTS_INVALID;
	}
	int8_t bit = (format == 'f') ? 23 : 10;
	if (argc == 4)
	{
		Digit first_digit;
		parse_numb(bit, &num1, &first_digit);
		print_digit(bit, &first_digit);
		return SUCCESS;
	}
	if (argc != 6)
	{
		fprintf(stderr, "Wrong input! There can be either 3 or 5 arguments in total");
		return ERROR_ARGUMENTS_INVALID;
	}
	if (!sscanf(argv[4], "%c", &operation) || !sscanf(argv[5], "%x", &num2))
	{
		fprintf(stderr, "Wrong input! Forth arg is operation and fifth arg is 16-x number");
		return ERROR_ARGUMENTS_INVALID;
	}
	Digit digit_res, digit1, digit2;
	parse_numb(bit, &num1, &digit1);
	parse_numb(bit, &num2, &digit2);
	int8_t a;
	switch (operation)
	{
	case '+':
		if (is_Bigger(&digit1, &digit2))
		{
			add(rounding, bit, &digit1, &digit2, &digit_res);
		}
		else
		{
			add(rounding, bit, &digit2, &digit1, &digit_res);
		}
		break;
	case '*':
		multiply(rounding, bit, &digit1, &digit2, &digit_res);
		break;
	case '/':
		divide(rounding, bit, &digit1, &digit2, &digit_res);
		break;
	case '-':
		subtract(rounding, bit, &digit1, &digit2, &digit_res);
		break;
	default:
		fprintf(stderr, "Wrong input! Forth arg is operation");
		return ERROR_ARGUMENTS_INVALID;
	}
	print_digit(bit, &digit_res);
	return SUCCESS;
}
