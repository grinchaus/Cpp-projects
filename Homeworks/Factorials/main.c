#include <stdint.h>
#include <stdio.h>

// Функция для узнавания размера числа
uint8_t lenNumber(uint32_t n)
{
	int8_t l = 0;
	do
	{
		l++;
		n /= 10;
	} while (n != 0);
	return l;
}

// Функция для вывода строки из "+" и "-"
void lineOfMinus(uint8_t numb, uint8_t fact)
{
	for (uint8_t i = 0; i < 2; ++i)
	{
		putchar('+');
		for (uint8_t j = 0; j < numb + 2; j++)
		{
			putchar('-');
		}
		numb = fact;
	}
	printf("+\n");
}

// Функция для вывода чисел и их факториалов
uint32_t printFact(uint32_t i, int8_t align, uint32_t fact, uint8_t maxLengthNumb, uint8_t maxLengthFact)
{
	if (i == 0)
	{
		fact = 1;
	}
	else
	{
		fact = ((uint64_t)fact * i) % 0x7FFFFFFF;
	}
	if (align == 0)
	{
		uint8_t a = maxLengthNumb + 2;
		uint8_t b = (maxLengthNumb + 2 - lenNumber(i)) >> 1;
		uint8_t c = maxLengthFact + 1;
		uint8_t d = maxLengthFact - lenNumber(fact);
		printf("|%*i%*s|%*i%*s|\n", a - b, i, b, "", c - (d >> 1), fact, (d + 2) >> 1, "");
	}
	else
	{
		printf("| %*i | %*i |\n", align * maxLengthNumb, i, align * maxLengthFact, fact);
	}
	return fact;
}

int main()
{
	int32_t n_start, n_end;
	int8_t align;
	int8_t error = scanf("%d %d %hhd", &n_start, &n_end, &align);
	if (error != 3 || (n_start < 0 || n_end < 0) || align < -1 || align > 1)
	{
		fprintf(stderr, "Wrong input! First and second argument must be >= 0. Third argument in [-1;1]");
		return 1;
	}
	uint32_t fact = 1, startFact = 1;
	uint8_t maxLengthFact = 0;
	uint8_t maxLengthNumb = lenNumber((n_end >= n_start) ? n_end : UINT16_MAX);
	// Узнаём максимальную длинну числа в виде факториала,
	//  при этом запоминаем с какого числа можно начать счёт факториала(оптимизация)
	uint32_t i = 1, nachWhile = (n_end >= n_start) ? n_end : UINT16_MAX;
	while (i <= nachWhile)
	{
		fact = ((uint64_t)fact * i) % 0x7FFFFFFF;
		if (lenNumber(fact) > maxLengthFact)
		{
			maxLengthFact = lenNumber(fact);
		}
		if (i == n_start - 1)
		{
			startFact = fact;
		}
		i++;
	}
	maxLengthFact = (2 > maxLengthFact) ? 2 : maxLengthFact;
	lineOfMinus(maxLengthNumb, maxLengthFact);
	// Вывод второй строки, в которой содержаться n и n!
	if (align == 0)
	{
		uint8_t a = maxLengthNumb + 2;
		uint8_t b = (maxLengthNumb + 1) >> 1;
		uint8_t c = maxLengthFact + 2;
		uint8_t d = maxLengthFact >> 1;
		printf("|%*s%*s|%*s%*s|\n", a - b, "n", b, "", c - d, "n!", d, "");
	}
	else
	{
		printf("| %*s | %*s |\n", align * maxLengthNumb, "n", align * maxLengthFact, "n!");
	}
	lineOfMinus(maxLengthNumb, maxLengthFact);
	i = n_start;
	// Запускаем цикл на вывод нужных нам чисел в таблицу
	while (n_end != i)
	{
		startFact = printFact(i, align, startFact, maxLengthNumb, maxLengthFact);
		i = (i + 1) % (UINT16_MAX + 1);
	}
	printFact(i, align, startFact, maxLengthNumb, maxLengthFact);
	lineOfMinus(maxLengthNumb, maxLengthFact);
	return 0;
}