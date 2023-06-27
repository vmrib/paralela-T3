all: mmul.c
	mpic++ -O3 -Wall -o mmul mmul.c