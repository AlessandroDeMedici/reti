#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

char parola1[] = "casa";
char parola2[] = "albero";
char parola3[] = "scuola";
char parola4[] = "mamma";
char parola5[] = "papa";

char * parole[5];


void init()
{
	parole[0] = parola1;
	parole[1] = parola2;
	parole[2] = parola3;
	parole[3] = parola4;
	parole[4] = parola5;
}

int piuSimile(char * c){
	int minscore = 100000000;
	int indice = 100000000;

	for (int i = 0; i < 5; i++){
		// calcolo dello score
		char * cursore1 = parole[i];
		char * cursore2 = c;
		int score = 0;
		// parole identiche lo restituisco subito
		if (!strcmp(c,parole[i]))
			return i;
		while (*cursore1 != '\0' && *cursore2 != '\0'){
			score += abs(*cursore1 - *cursore2);
			cursore1++;
			cursore2++;
		}
		if (score < minscore){
			minscore = score;
			indice = i;
		}
		printf("parola: %s, score: %d\n",parole[i],score);
	}
	return indice;
}


int main(){
	char stringa[20];
	int indice;
	init();
	printf("Inserisci una parola: ");
	scanf("%s",stringa);
	indice = piuSimile(stringa);
	printf("\r\033[Kmaybe you meant:\nInserisci una parola: %s",parole[indice]);
	fprintf(stdin,"%s",parole[indice]);
	scanf("%s",stringa);
	return 0;
}
