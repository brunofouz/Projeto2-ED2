// Includes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

// Hash //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define hashSize 11
#define bucketSize 2

typedef struct {
    int qtd;
    int key[bucketSize];
    int RRN[bucketSize];
} typeBucket;

FILE *hash;

// B-tree /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAXKEYS 4
#define MINKEYS MAXKEYS/2
#define NIL (-1)

typedef struct {
    int keycount; // number of keys in page
    int key[MAXKEYS]; // the actual keys
    int offset[MAXKEYS]; // rrn for the corresponding keys
    int child[MAXKEYS+1]; // ptrs to rrns of descendants
} BTPAGE;

#define PAGESIZE sizeof(BTPAGE)

extern int root; // rrn of root page
FILE* btfd; // file descriptor of btree file
extern int infd; // file descriptor of input file

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Declarações de estruturas e descritores de arquivos globais

struct ap1Struct {
	int codControle, codCachorro;
	char nomeVacina[30];
	char dataVacina[5];
	char respAplic[30];
}; // Estrutura para o registro do arquivo de vacinas (AP1)

struct ap2Struct {
	int codCachorro;
	char raca[30];
	char nomeCachorro[30];
}; // Estrutura para o registro (tamanho fixo) do arquivo de cachorros (AP2)

struct indice1Struct {
	int codControle, offset1, offset2;
}; // Estrutura do índice principal (Indice1)

struct indice2Struct {
	int offset2;
	char nomeVacina[40];
};

struct indice1Struct indice1[5000];
struct indice2Struct indice2[5000];
int numVacina, numCachorro, numindice1, numindice2;

FILE *arqVacinas; // Arquivo principal 1 (AP1)
FILE *arqVacinasCompactada; // Arquivo principal 1 (AP1) temporário para o processo de compactação
FILE *arqCachorros; // Arquivo principal 2 (AP2)
FILE *arqIndice1; // Arquivo do índice principal (Indice1)
FILE *arqIndice2a; // Arquivo do índice secundário A (Indice2a)
FILE *arqIndice2b; // Arquivo do índice secundário B (Indice2b)

// Protótipos

void menu();
void criarVetores();
void abrirArquivos();
void cadastrarCachorro();
void cadastrarVacina();
void excluirVacina();
void compactarDadosVacina();
void consultarVacina();
void invalidarRegistro();
void reescreverOffset();
void completarIndice();
void completarListaIndice();
void ordenarIndices();
void adicionarIndice();
void trocarOffset();
void removerDoIndice2();
void salvarIndice1();
void salvarIndice2ab();
void reescreverVacina();
void atualizarIndice2();
void lerIndice2();
int obterOffset();
int procurarCachorro();
int buscarVacina();
int calcularTamanhoRegistro();
void openHash();
void inserthash();

/* prototypes */

void btclose();
bool btopen();
void btread(int rrn, BTPAGE *page_ptr);
void btwrite(int rrn, BTPAGE page_ptr);
int create_root(int key, int rrn, int left, int right);
int create_tree();
int getpage();
int getroot();
bool insert(int rrn, int key, int offset, int *promo_r_child, int *promo_key, int *promo_offset);
void ins_in_page(int key,int offset,int r_child, BTPAGE *p_page);
void pageinit(BTPAGE *p_page);
void putroot(int root);
bool search_node(int key, BTPAGE *p_page, int *pos);
void split(int key, int offset, int r_child, BTPAGE *p_oldpage, int *promo_key, int *promo_offset, int *promo_r_child, BTPAGE *p_newpage);
void InOrder(int root);

// Function principal (main) do programa

int main() {
	openHash();
	criarVetores();
	abrirArquivos();

	menu();

	salvarIndice1();
	salvarIndice2ab();

	fclose(arqCachorros);
	fclose(arqVacinas);
	fclose(arqIndice1);
	fclose(arqIndice2a);
	fclose(arqIndice2b);

	return 0;
}

// Function do menu principal

void menu() {
	int codigo;
	bool fim = false;
	do {
		char opcao;

		system("cls");
		printf("///////////////////////////////////////////////////\n");
		printf("//        Programa de Cadastro de Vacinas        //\n");
		printf("///////////////////////////////////////////////////\n\n");
		printf("[1] Cadastrar novo cachorro\n");
		printf("[2] Cadastrar nova vacina\n");
		printf("[3] Listar todos os cadastros (usando Arvore-B)\n");
		printf("[4] Listar um cadastro especifico (Hash)\n");
		printf("[5] Listar um cadastro especifico (Arvore-B)\n");
		printf("[6] Sair\n\n");
		printf("Escolha uma opcao: ");

		fflush(stdin);
		scanf("%c", &opcao);

		switch(opcao) {
			case '1':
				cadastrarCachorro();
				break;
			case '2':
				cadastrarVacina();
				break;
			case '3':
				/////////////////////////////////
				break;
			case '4':
				/////////////////////////////////
				break;
			case '5':
				/////////////////////////////////
				break;
			case '6':
				printf("Fechando o programa...\n");
				fim = true;
				break;
			default:
				printf("Opcao invalida!\n\n");
				system("pause");
		}
	} while (!fim);
}

// Function para abertura e verificação dos arquivos

void abrirArquivos() {
	arqCachorros = fopen("AP2.dat", "r+b");
	if (arqCachorros == NULL) {
		arqCachorros = fopen("AP2.dat", "w+b");
		arqVacinas = fopen("AP1.dat", "w+b");
		int n = -1;
		fwrite(&n, sizeof(int), 1, arqVacinas);
		fwrite(&n, sizeof(int), 1, arqVacinas);
		arqIndice1 = fopen("Indice1.dat", "w+b");
		arqIndice2a = fopen("Indice2a.dat", "w+b");
		arqIndice2b = fopen("Indice2b.dat", "w+b");
	} else {
		arqVacinas = fopen("AP1.dat", "r+b");
		if (arqVacinas == NULL) {
			arqVacinas = fopen("AP1.dat", "w+b");
			int n = -1;
			fwrite(&n, sizeof(int), 1, arqVacinas);
			fwrite(&n, sizeof(int), 1, arqVacinas);
			arqIndice1 = fopen("Indice1.dat", "w+b");
			arqIndice2a = fopen("Indice2a.dat", "w+b");
			arqIndice2b = fopen("Indice2b.dat", "w+b");
		} else {
			fseek(arqVacinas,0,2);
			if ( (ftell(arqVacinas)) > 4) {
				arqIndice1 = fopen("Indice1.dat", "r+b");
				arqIndice2a = fopen("Indice2a.dat", "r+b");
				arqIndice2b = fopen("Indice2b.dat", "r+b");
				if ( (arqIndice1 == NULL) || (arqIndice2a == NULL) || (arqIndice2b == NULL)) {
					arqIndice1 = fopen("Indice1.dat", "w+b");
					arqIndice2a = fopen("Indice2a.dat", "w+b");
					arqIndice2b = fopen("Indice2b.dat", "w+b");
					completarListaIndice();
				} else {
					char indicador;
					fread(&indicador, 1, 1, arqIndice1);
					if (&indicador == "!") {
						arqIndice1 = fopen("Indice1.dat", "w+b");
						arqIndice2a = fopen("Indice2a.dat", "r+b");
						arqIndice2b = fopen("Indice2b.dat", "r+b");
						completarListaIndice();
					} else {
						completarIndice();
					}
				}
			} else {
				arqVacinas = fopen("AP1.dat", "w+b");
				arqIndice1 = fopen("Indice1.dat", "w+b");
				arqIndice2a = fopen("Indice2a.dat", "w+b");
				arqIndice2b = fopen("Indice2b.dat", "w+b");
			}

			arqIndice2a = fopen("Indice2a.dat", "r+b");
			arqIndice2b = fopen("Indice2b.dat", "r+b");
		}
	}
}

// Function para obter o offset

int obterOffset(int tam) {
	int regTam, offset;
	int offset1 = 4;
	int offsetAux = -1;

	fseek(arqVacinas, offset1, 0);
	fread(&offset1, sizeof(int), 1, arqVacinas);

	if (offset1 != -1) {
		do {
			fseek(arqVacinas, offset1, 0);
			fread(&regTam, sizeof(int), 1, arqVacinas);

			if (tam <= regTam) {
				fseek(arqVacinas, 1, 1);
				offset = offset1;
				fread(&offset1, sizeof(int), 1, arqVacinas);
				fseek(arqVacinas, offsetAux + 5, 0);
				fwrite(&offset1, sizeof(int), 1, arqVacinas);

				return offset;
			}

			fseek(arqVacinas, 1, 1);
			offsetAux = offset1;
			fread(&offset1, sizeof(int), 1, arqVacinas);
		} while (offset1 != -1);
	}

	return offset1;
}

// Function para adicionar um registro ao vetor de Índices

void adicionarIndice(struct ap1Struct aux, int offset1) {
	numindice1++;
	indice1[numindice1].codControle = aux.codControle;
	indice1[numindice1].offset1 = offset1;
	indice1[numindice1].offset2 = -1;

	int contador = 0;
	bool achou = false;

	while ( (contador <= numindice2) && (!achou) ) {
		if (strcmp(indice2[contador].nomeVacina, aux.nomeVacina) == 0) {
			achou = true;
			if (indice2[contador].offset2 == -1) {
				indice2[numindice2].offset2 = numindice1;
			} else {
				int aux = 0;
				int offset2 = indice2[contador].offset2;
				do {
					aux = offset2;
					offset2 = indice1[offset2].offset2;
				} while (offset2 != -1) ;
				indice1[aux].offset2 = numindice1;
			}
		} else {
			contador++;
		}
	}

	if (!achou) {
		numindice2++;
		strcpy(indice2[numindice2].nomeVacina, aux.nomeVacina);
		indice2[numindice2].offset2 = numindice1;
	}
}

// Function para trocar o offset recém reescrito

void trocarOffset(int codigo, int offset1) {
	bool achou = false;
	int contador = 0;

	while ((contador <= numindice1) && (!achou)) {
		if (indice1[contador].codControle == codigo) {
			achou = true;
			invalidarRegistro(indice1[contador].offset1);
			reescreverOffset(indice1[contador].offset1);
			indice1[contador].offset1 = offset1;
		}
		contador++;
	}
}

// Function para adicionar o registro da vacina recém-cadastrada no arquivo AP1

void adicionarVacina(struct ap1Struct aux) {
    bool promoted; // boolean: tells if a promotion from below
	int root, // rrn of root page
	promo_rrn; // rrn promoted from below
	int promo_key, promo_offset; // key promoted from below

	char buffer[105];
	sprintf(buffer, "*%d|%d|%s|%s|%s|", aux.codControle, aux.codCachorro, &aux.nomeVacina, &aux.dataVacina, &aux.respAplic);
	int tamanho = strlen(buffer);
	int offset1 = obterOffset(tamanho);
	int key = aux.codControle;

	if (offset1 == -1) {
		fseek(arqVacinas, 0, 2);
		offset1 = ftell(arqVacinas);
		fwrite(&tamanho, sizeof(int), 1, arqVacinas);
	} else {
		fseek(arqVacinas, offset1 + sizeof(int), 0);
	}

	fwrite(buffer, sizeof(char), strlen(buffer), arqVacinas);

	insertHash(key, offset1);

	if (btopen()) {
	    root = getroot();
	}
    else {
	    root = create_tree();
	}

	promoted = insert(root, key, offset1, &promo_rrn, &promo_key, &promo_offset);
	if (promoted)
		root = create_root(promo_key, promo_offset, root, promo_rrn);

    btclose();
}

// Function para procurar o código de um cachorro no AP2 e retornar se existe ou não

int procurarCachorro(int codigo) {
	fseek(arqCachorros, 0, 2);
	int tamArq = ftell(arqCachorros) / 12;
	fseek(arqCachorros, 0, 0);

	struct ap2Struct aux;
	int contador = 0;
	bool achou = false;

	while ( (contador < tamArq) && (!achou) ) {
		fread(&aux.codCachorro, sizeof(aux), 1, arqCachorros);
		if (aux.codCachorro == codigo)  {
			achou = true;
		}
		contador++;
	}

	if (achou) {
		return aux.codCachorro;
	} else {
		return -1;
	}
}

// Function para o cadastramento de uma vacina (item 2 do menu)

void cadastrarVacina() {
	struct ap1Struct temporario;
	struct ap2Struct tempCachorro;
	char buffer[105];

	do {
		system("cls");
		printf("Cadastro de Vacina\n\n");
		printf("Digite o codigo do cachorro: ");
		scanf("%d", &temporario.codCachorro);

		if (procurarCachorro(temporario.codCachorro) == -1) {
			printf("\nCachorro nao encontrado! ");
			bool sair = false;
			do {
				char opcao;

				printf("Deseja cadastrar um novo cachorro? (1=Sim, 0=Nao):  ");

				fflush(stdin);
				scanf("%c", &opcao);

				switch (opcao) {
					case '1':
						cadastrarCachorro();
						sair = true;
						break;
					case '0':
						sair = true;
						break;
					default:
						printf("Opcao invalida!\n\n");
						system("pause");
						break;
				}
			} while (!sair);
		}
	} while (procurarCachorro(temporario.codCachorro) == -1);

	printf("Codigo de controle: ");
	scanf("%d", &temporario.codControle);
	printf("Nome da vacina: ");
	scanf("%s", &temporario.nomeVacina);
	printf("Data da vacinacao: ");
	scanf("%s", &temporario.dataVacina);
	printf("Responsavel pela aplicacao: ");
	scanf("%s", &temporario.respAplic);
	fseek(arqIndice1, 0, 0);
	fwrite("!", 1, 1, arqIndice1);
	adicionarVacina(temporario);

	printf("\n");
	printf("Dados da vacina:\n");
	printf("    Codigo da vacina: %d\n", temporario.codControle);
	printf("    Nome da vacina: %s\n", temporario.nomeVacina);
	printf("    Data da vacinacao: %s\n", temporario.dataVacina);
	printf("    Responsavel pela aplicacao: %s\n\n", temporario.respAplic);

	fseek(arqCachorros, ((temporario.codCachorro) * sizeof(tempCachorro)), 0);
	fread(&tempCachorro, sizeof(tempCachorro), 1, arqCachorros);

	printf("Dados do cachorro que recebera a vacina:\n");
	printf("    Codigo do cachorro: %d\n", tempCachorro.codCachorro);
	printf("    Nome do cachorro: %s\n", tempCachorro.nomeCachorro);
	printf("    Raca: %s\n\n", tempCachorro.raca);
	system("pause");
}

// Function para o cadastramento de um cachorro (item 1 do menu)

void cadastrarCachorro() {
	struct ap2Struct temporario;

	system("cls");
	printf("Cadastro de Cachorro\n\n");
	fseek(arqCachorros, 0, 2);
	temporario.codCachorro = ftell(arqCachorros) / sizeof(struct ap2Struct);

	printf("Nome do cachorro: ");
	scanf("%s", temporario.nomeCachorro);
	printf("Raca: ");
	scanf("%s", temporario.raca);

	fseek(arqCachorros, 0, 2);
	fwrite(&temporario, sizeof(struct ap2Struct), 1, arqCachorros);

	printf("\n");
	printf("Dados do cachorro:\n");
	printf("    Codigo do cachorro: %d\n", temporario.codCachorro);
	printf("    Nome do cachorro: %s\n", temporario.nomeCachorro);
	printf("    Raca: %s\n\n", temporario.raca);
	system("pause");
}

// Function para reescrever o offset de um registro

void reescreverOffset(int newOffset) {
	bool fim = false;
	int offset1;
	int offsettemporario = -1;

	fseek(arqVacinas, sizeof(int), 0);
	fread(&offset1, sizeof(int), 1, arqVacinas);

	while (!(fim)) {
		if ((offset1 == -1) || (newOffset < offset1)) {
			fim = true;
			fseek(arqVacinas, (offsettemporario + sizeof(int) + sizeof(char)), 0);
			fwrite(&newOffset, sizeof(int), 1, arqVacinas);
			fseek(arqVacinas, (newOffset + sizeof(int) + sizeof(char)), 0);
			fwrite(&offset1, sizeof(int), 1, arqVacinas);
		}
		fseek(arqVacinas, (offset1 + sizeof(int) + sizeof(char)), 0);
		offsettemporario = offset1;
		fread(&offset1, sizeof(int), 1, arqVacinas);
	}
}

// Function para marcar um registro como disponível para reuso

void invalidarRegistro(int pos) {
	fseek(arqVacinas, (pos + 4), 0);
	fwrite("!", sizeof(char), 1, arqVacinas);
}

// Function para remover do indice secundario

void removerDoIndice2(int pos, int codigo) {
	char nome[100];
	int tam;
	int soma = 0;

	fseek(arqVacinas, pos, 0);
	fread(&tam, sizeof(int), 1, arqVacinas);
	fseek(arqVacinas, 1, 1);
	fread(&nome, tam - 1, 1, arqVacinas);
	strtok(nome, "|");
	soma += strlen(nome) + 1;
	fseek(arqVacinas, pos + 5 + soma, 0);
	fread(&nome, tam - 1 - soma, 1, arqVacinas);
	strtok(nome, "|");
	soma += strlen(nome) + 1;
	fseek(arqVacinas, pos + 5 + soma, 0);
	fread(&nome, tam - 1 - soma, 1, arqVacinas);
	strtok(nome, "|");

	int contador = 0;
	bool achou = false;
	bool achou2 = false;

	while (!achou) {
		if (strcmp(indice2[contador].nomeVacina, nome) == 0) {
			achou = true;
			int offset1 = indice2[contador].offset2;
			if (indice1[offset1].codControle == codigo) {
				indice2[contador].offset2 = indice1[offset1].offset2;
			} else {
				int offsetAnt;
				do {
					offsetAnt = offset1;
					offset1 = indice1[offset1].offset2;
					if (indice1[offset1].codControle == codigo) {
						achou2 = true;
						indice1[offsetAnt].offset2 = indice1[offset1].offset2;
					}
				} while (!achou2);
			}
		} else {
			contador++;
		}
	}
}

// Function para excluir uma vacina

void excluirVacina(int codigo) {
	bool achou = false;
	int contador = 0;

	while ((contador <= numindice1) && (!achou)) {
		if (indice1[contador].codControle == codigo) {
			achou = true;
			removerDoIndice2(indice1[contador].offset1, codigo);
			invalidarRegistro(indice1[contador].offset1);
			reescreverOffset(indice1[contador].offset1);
			indice1[contador].codControle = -1;
		}
		contador++;
	}

	if (!achou) {
		printf("Nao ha nenhuma vacina de codigo %d cadastrada!\n", codigo);
	} else {
		printf("Vacina referente ao codigo %d removida!\n", codigo);
	}
	system("pause");
}

// Function para encontrar um registro e retorná-lo como saída

struct ap1Struct encontraRegistro(int pos) {
	struct ap1Struct temporario;
	int tam, tam2, temp, soma;
	char str[40];

	fseek(arqVacinas, pos, 0);
	fread(&tam, sizeof(int), 1, arqVacinas);
	fseek(arqVacinas, sizeof(char), 1);

	fread(&str, tam - 1, 1, arqVacinas);
	strtok(str, "|");
	soma = strlen(str) + 1;
	temp = atoi(str);
	temporario.codControle = temp;

	fseek(arqVacinas, pos + 5 + soma, 0);
	fread(&str, tam - soma - 1, 1, arqVacinas);
	strtok(str, "|");
	soma += strlen(str) + 1;
	temp = atoi(str);
	temporario.codCachorro = temp;

	fseek(arqVacinas, pos + 5 + soma, 0);
	fread(&str, tam - soma - 1, 1, arqVacinas);
	strtok(str, "|");
	soma += strlen(str) + 1;
	strcpy(temporario.nomeVacina, str);

	fseek(arqVacinas, pos + 5 + soma, 0);
	fread(&str, tam - soma - 1, 1, arqVacinas);
	strtok(str, "|");
	soma += strlen(str) + 1;
	strcpy(temporario.dataVacina, str);

	fseek(arqVacinas, pos + 5 + soma, 0);
	fread(&str, tam - soma - 1, 1, arqVacinas);
	strtok(str, "|");
	soma += strlen(str) + 1;
	strcpy(temporario.respAplic, str);

	return temporario;
}

// Function para buscar uma vacina no vetor de Índices

int buscarVacina(int codigo) {
	int contador = 0;

	while (contador <= numindice1) {
		if (indice1[contador].codControle == codigo) {
			return indice1[contador].offset1;
		}
		contador++;
	}

	return -1;
}

// Function para calcular o tamanho de um registro

int calcularTamanhoRegistro(struct ap1Struct temporario) {
	char buffer[105];

	sprintf(buffer, "*%d|%d|%s|%s|%s|", temporario.codControle, temporario.codCachorro, &temporario.nomeVacina, &temporario.dataVacina, &temporario.respAplic);

	return strlen(buffer);
}

// Function para escrever as alterações da vacina no Arquivo de Vacinas (AP1)

void reescreverVacina(struct ap1Struct temporario, int offset1) {
	char buffer[105];
	sprintf(buffer, "*%d|%d|%s|%s|%s|", temporario.codControle, temporario.codCachorro, &temporario.nomeVacina, &temporario.dataVacina, &temporario.respAplic);
	int tamanho = strlen(buffer);

	fseek(arqVacinas, offset1 + sizeof(int), 0);
	fwrite(buffer, sizeof(char), strlen(buffer), arqVacinas);
}

// Function para atualizar o Indice secundário

void atualizarIndice2(char *nomeAnt, char *nome, int codigo) {
	int contador = 0;
	int ref;
	bool achou = false;
	bool achou2 = false;

	while (!achou) {
		if (strcmp(indice2[contador].nomeVacina, nomeAnt) == 0) {
			achou = true;
			int offset1 = indice2[contador].offset2;
			if (indice1[offset1].codControle == codigo) {
				indice2[contador].offset2 = indice1[offset1].offset2;
				ref = offset1;
			} else {
				int offsetAnt;
				do {
					offsetAnt = offset1;
					offset1 = indice1[offset1].offset2;
					if (indice1[offset1].codControle == codigo) {
						achou2 = true;
						indice1[offsetAnt].offset2 = indice1[offset1].offset2;
						ref = offset1;
					}
				} while (!achou2);
			}
		} else {
			contador++;
		}
	}

	contador = 0;
	achou = false;

	while ( (contador <= numindice2) && (!achou) ){
		if (strcmp(indice2[contador].nomeVacina, nome) == 0) {
			achou = true;
			if (indice2[contador].offset2 == -1) {
				indice2[numindice2].offset2 = ref;
			} else {
				int temporario = 0;
				int offset2 = indice2[contador].offset2;
				do {
					temporario = offset2;
					offset2 = indice1[offset2].offset2;
				} while (offset2 != -1) ;
				indice1[temporario].offset2 = ref;
			}
		} else {
			contador++;
		}
	}

	if (!achou) {
		numindice2++;
		strcpy(indice2[numindice2].nomeVacina, nome);
		indice2[numindice2].offset2 = ref;
	}

	indice1[ref].offset2 = -1;
}

// Function para a consulta de vacinas (item 5 do menu)

void consultarVacina() {
	int codProcurado;
	int RRN = 0, tam, offsetAP1, soma;
	bool achou = 0;
	char str[100];

	system("cls");
	printf("Digite o codigo da vacina a ser procurada: ");
	scanf("%d", &codProcurado);

	while ((indice1[RRN].codControle != -1) && (!achou)) {
		if (indice1[RRN].codControle == codProcurado) {
			achou = true;
			offsetAP1 = indice1[RRN].offset1;
			printf("\nOffset do registro: %d\n",offsetAP1);
			fseek(arqVacinas, offsetAP1, 0);
			fread(&tam, sizeof(int), 1, arqVacinas);

			fseek(arqVacinas, 1, 1);
			fread(&str, tam - 1, 1, arqVacinas);
			strtok(str, "|");
			soma = strlen(str) + 1;
			printf("\nCodigo da vacina: %s", str);

			fseek(arqVacinas, offsetAP1 + 5 + soma, 0);
			fread(&str, tam - soma - 1, 1, arqVacinas);
			strtok(str, "|");
			soma += strlen(str) + 1;
			printf("\nCodigo do cachorro: %s", str);

			fseek(arqVacinas, offsetAP1 + 5 + soma, 0);
			fread(&str, tam - soma - 1, 1, arqVacinas);
			strtok(str, "|");
			soma += strlen(str) + 1;
			printf("\nNome da vacina: %s", str);

			fseek(arqVacinas, offsetAP1 + 5 + soma, 0);
			fread(&str, tam - soma - 1, 1, arqVacinas);
			strtok(str, "|");
			soma += strlen(str) + 1;
			printf("\nData da vacina: %s", str);

			fseek(arqVacinas, offsetAP1 + 5 + soma, 0);
			fread(&str, tam - soma - 1, 1, arqVacinas);
			strtok(str, "|");
			soma += strlen(str) + 1;
			printf("\nResponsavel pela aplicacao: %s\n", str);
		} else {
			RRN++;
		}
	}

	if (!achou) {
		printf("Vacina referente ao codigo %d nao encontrada.\n\n", codProcurado);
	}

	system("pause");
}

void criarVetores() {
	numindice1 = -1;
	numindice2 = -1;

	int i;

	for (i = 0; i <= 5000; i++) {
		indice1[i].codControle = -1;
	}
}

// Function para salvar o índice primário no arquivo (Indice1.dat)

void salvarIndice1() {
	fseek(arqIndice1, 0, 0);
	fwrite("*", 1, 1, arqIndice1);

	int contador = 0;

	while (contador <= numindice1) {
		if (indice1[contador].codControle != -1) {
			fwrite(&indice1[contador].codControle, sizeof(int), 1, arqIndice1);
			fwrite(&indice1[contador].offset1, sizeof(int), 1, arqIndice1);
		}
		contador++;
	} 
}

// Function para salvar os índices secundários nos arquivos (Indice2a.dat e Indice2b.dat)

void salvarIndice2ab() {
	fseek(arqIndice2a, 0, 0);
	fseek(arqIndice2b, 0, 0);

	int contador = 0;

	struct indice2Struct temporario;
	while (contador <= numindice2) {
		strcpy(temporario.nomeVacina, indice2[contador].nomeVacina);
		temporario.offset2 =  indice2[contador].offset2;
		fwrite(&temporario, sizeof(temporario), 1, arqIndice2a);
		int offset1 = indice2[contador].offset2;
		while (offset1 != -1) {
			fwrite(&indice1[offset1].codControle, sizeof(int), 1, arqIndice2b);
			fwrite(&indice1[offset1].offset2, sizeof(int), 1, arqIndice2b);
			offset1 = indice1[offset1].offset2;
		}
		contador++;
	}
}


// Function para criar o índice a partir do Arquivo de Vacinas (AP1)

void completarIndice() {
	fseek(arqIndice2a, 0, 2);
	int tamArq = ftell(arqIndice2a) / 44;
	fseek(arqIndice2a, 0, 0);

	struct indice2Struct temporario;

	int codigo, offset2;
	int contador = 0;

	while (contador < tamArq) {
		fread(&temporario, sizeof(temporario), 1, arqIndice2a);
		numindice2++;
		strcpy(indice2[numindice2].nomeVacina, temporario.nomeVacina);
		indice2[numindice2].offset2 = temporario.offset2;
		int offset1 = temporario.offset2;
		while (offset1 != -1) {
			fread(&codigo, sizeof(int), 1, arqIndice2b);
			fread(&offset2, sizeof(int), 1, arqIndice2b);
			indice1[offset1].codControle = codigo;
			indice1[offset1].offset1 = 777;
			indice1[offset1].offset2 = offset2;
			if (offset1 > numindice1) {
				numindice1 = offset1;
			}
			offset1 = offset2;
		}
		contador++;
	}

	fseek(arqIndice1, 1, 0);
	contador = 0;

	int offset1;

	while (contador <= numindice1) {
		if (indice1[contador].codControle != -1) {
			fseek(arqIndice1, 4, 1);
			fread(&offset1, sizeof(int), 1, arqIndice1);
			indice1[contador].offset1 = offset1;
		}
		contador++;
	}
}

// Function para carregar o índice do disco para a RAM

void completarListaIndice() {
	int pos = 8;
	int tam = 0;
	char indicador;
	char str[100];
	int codigo = 0;
	int ref, soma, contador;
	bool achou;
	numindice1 = -1;
	numindice2 = -1;

	fseek(arqVacinas, 0, 2);
	int tamArq = ftell(arqVacinas);
	fseek(arqVacinas, 8, 0);

	do {
		fread(&tam, sizeof(int), 1, arqVacinas);
		fread(&indicador, sizeof(char), 1, arqVacinas);

		if (indicador == '*') {
			ref = ftell(arqVacinas);
			fread(&str, tam - 1, 1, arqVacinas);
			strtok(str, "|");
			codigo = atoi(str);
			soma = strlen(str)+1;

			numindice1++;
			indice1[numindice1].codControle = codigo;
			indice1[numindice1].offset1 = pos;
			indice1[numindice1].offset2 = -1;

			fseek(arqVacinas, ref + soma, 0);
			fread(&str, tam - 1 - soma , 1, arqVacinas);
			strtok(str, "|");
			soma += strlen(str) + 1;
			fseek(arqVacinas, ref + soma, 0);
			fread(&str, tam - 1 - soma , 1, arqVacinas);
			strtok(str, "|");

			achou = false;
			contador = 0;

			while ((contador <= numindice2) && (!achou)) {
				if (strcmp(indice2[contador].nomeVacina, str) == 0) {
					achou = true;
					if (indice2[contador].offset2 == -1) {
						indice2[numindice2].offset2 = numindice1;
					} else {
						int temporario = 0;
						int offset2 = indice2[contador].offset2;
						do {
							temporario = offset2;
							offset2 = indice1[offset2].offset2;
						} while (offset2 != -1) ;
						indice1[temporario].offset2 = numindice1;
					}
				} else {
					contador++;
				}
			}

			if (!achou) {
				numindice2++;
				strcpy(indice2[numindice2].nomeVacina, str);
				indice2[numindice2].offset2 = numindice1;
			}

		} else {
			fseek(arqVacinas, (tam - sizeof(char)), 1);
		}

		pos += tam + sizeof(int);
	} while (pos < tamArq);
}

// Hash ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void createHash() {
    int i;

    typeBucket bucket;
    bucket.qtd = 0;
    for (i = 0; i < bucketSize; i++) {
        bucket.key[i] = -1;
        bucket.RRN[i] = -1;
    }

    hash = fopen("hash.bin","w+b");

    for (i = 1; i <= hashSize; i++)
        fwrite(&bucket,sizeof(bucket),1,hash);
}

bool invalidHash() {
    fseek(hash,0,2);
    int fileSize = ftell(hash);
    fseek(hash,0,0);

    if ( fileSize != (hashSize*(bucketSize*2*sizeof(int) + sizeof(int))) )
        return true;
}

void openHash() {
    hash = fopen("hash.bin","r+b");
    if ( (hash == NULL) || invalidHash() )
        createHash();
}

int hashFunction(int key) {
    int value = (key/hashSize);

    while (value > (hashSize - 1) ) {
        value = value/hashSize;
    }

    return value;
}

void progressiveOverflow(int key, int RRN, int address, int refAddress, int *count) {
    fseek(hash,address*sizeof(typeBucket),0);

    typeBucket bucket;
    fread(&bucket,sizeof(typeBucket),1,hash);

    switch (bucket.qtd) {
        case 0:
            bucket.qtd++;
            bucket.key[0] = key;
            bucket.RRN[0] = RRN;
            fseek(hash,-sizeof(typeBucket),1);
            fwrite(&bucket,sizeof(typeBucket),1,hash);
            break;
        case 1:
            bucket.qtd++;
            bucket.key[1] = key;
            bucket.RRN[1] = RRN;
            fseek(hash,-sizeof(typeBucket),1);
            fwrite(&bucket,sizeof(typeBucket),1,hash);
            break;
        case 2:
            *count = *count + 1;

            printf("Colisao - Tentativa %d\n", *count);

            if ( (address + 1) > (hashSize - 1) )
                address = 0;
            else
                address++;

            if (address != refAddress)
                progressiveOverflow(key, RRN, address, refAddress, count);
            else
                *count = -1;

            break;
    }
}

void insertHash(int key, int RRN) {
    int address = hashFunction(key);

    printf("Endereco %d gerado para a chave %d\n", address, key);
	printf("Lel");
    int count = 0;

    progressiveOverflow(key, RRN, address, address, &count);

    if (count != -1)
        printf("Chave %d inserida com sucesso\n", key);
    else
        printf("Chave %d nao inserida, hash cheio!\n", key);
}

int searchHashR(int key, int *address, int refAddress, int *count) {
    fseek(hash,(*address)*sizeof(typeBucket),0);

    typeBucket bucket;
    fread(&bucket,sizeof(typeBucket),1,hash);

    switch (bucket.qtd) {
        case 0:
            *count = -1;
            break;
        case 1:
            if ( bucket.key[0] == key ) {
                return bucket.RRN[0];
            }
            else {
                *count = -1;
            }
            break;
        case 2:
            if ( bucket.key[0] == key )
                return bucket.RRN[0];
            else if ( bucket.key[1] == key )
                return bucket.RRN[1];
            else {
                *count = *count + 1;

                if ( (*address + 1) > (hashSize - 1) )
                    *address = 0;
                else
                    (*address)++;

                if (*address != refAddress) {
                    searchHashR(key, address, refAddress, count);
                }
                else {
                    *count = -1;
                }
            }

            break;
    }
}

void searchHash(int key) {
    int address = hashFunction(key);
    int count = 1;

    int RRN = searchHashR(key, &address, address, &count);

    if (count != -1)
        printf("Chave %d encontrada, endereco %d, %d acessos\n", key, address, count);
    else
        printf("Chave %d nao encontrada\n", key);
}

/*
int main() {
    openHash();

    insertHash(2,2);
    searchHash(2);

    fclose(hash);

    return 0;
}
*/

// B-tree ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//=========================================================
//=======================MAIN==============================
//=========================================================

/*
int main() {
    bool promoted; // boolean: tells if a promotion from below
	int root, // rrn of root page
	promo_rrn; // rrn promoted from below
	int promo_key, promo_offset; // key promoted from below
	int key, offset = 0; // next key to insert in tree
	if (btopen()) {
	    root = getroot();
	}
    else {
	    root = create_tree();
	}
	while (key != -1) {
        printf("Digite um numero: ");
        scanf("%d", &key);
        printf("Digite um RRN: ");
        scanf("%d", &offset);
	    promoted = insert(root, key, offset, &promo_rrn, &promo_key, &promo_offset);
		if (promoted)
		    root = create_root(promo_key, promo_offset, root, promo_rrn);
	}

    printf("  \n *percurso : \n");
    InOrder(getroot());
    getch();
    btclose();
}
*/

bool insert(int rrn, int key, int offset, int *promo_r_child, int *promo_key, int *promo_offset) {
    BTPAGE page, // current page
	newpage; // new page created if split occurs
	bool found, promoted; // boolean values
	int pos;
	int p_b_rrn = NIL; // rrn promoted from below
	int p_b_key = key; // key promoted from below
	int p_b_offset = offset;
	if (rrn == NIL) {
	    *promo_key = key;
	    *promo_offset = offset;
		*promo_r_child = NIL;
		return true;
	}
    btread(rrn, &page);

	found = search_node ( key, &page, &pos);
	if (found){
	    printf ("Chave %d duplicada! \n", key);
		return(0);
	}

	promoted = insert(page.child[pos], key, offset, &p_b_rrn, &p_b_key, &p_b_offset);
	if (!promoted){
	    return false;
	}

	if (page.keycount < MAXKEYS) {
        printf("key %d rrn %d \n", p_b_key, p_b_offset);
	    ins_in_page(p_b_key, p_b_offset, p_b_rrn, &page);
		btwrite(rrn, page);
		return false;
	}
	else {
 	    split(p_b_key, p_b_offset, p_b_rrn, &page, promo_key, promo_offset, promo_r_child, &newpage);
 	    printf("key %d rrn %d \n", p_b_key, p_b_offset);
		btwrite(rrn, page);
		btwrite(*promo_r_child, newpage);
		return true;
	}
}

bool btopen() {
    btfd = fopen("btree.bin", "r+b");
    return (btfd != NULL);
}

void btclose() {
    close(btfd);
}

int getroot() {
    int root;
    fseek(btfd, 0, 0);

    fread(&root, sizeof(int), 1, btfd);

    /*
    if (root == -4) {
        printf("Error: Unable to get root. \007\n");
        return 1; //se não der certo retorna 1 by bruno fouz
	}
	*/

    return (root);
}

void putroot(int root) {
	    fseek(btfd, 0, 0);
		fwrite(&root, sizeof(int), 1, btfd);
}

int create_root(int key, int offset, int left, int right) {
    BTPAGE page;
    int rrn;
	rrn = getpage();
	pageinit(&page);
	page.key[0] = key;
	page.offset[0] = offset;
	page.child[0] = left;
	page.child[1] = right;
	page.keycount = 1;
	btwrite(rrn, page);
	putroot(rrn);
	return(rrn);
}

int create_tree() {
	int key, offset;
	btfd = fopen("btree.bin", "w+b");
	fclose(btfd);
	btopen();
	printf("Digite chave: ");
	scanf("%d", &key);
	printf("Digite RRN: ");
	scanf("%d", &offset);
	//key = getchar();
	//key = atoi(key);
	return (create_root(key, offset, NIL, NIL));
}

int getpage() {
    fseek(btfd,0,2);

    int addr = ftell(btfd);

    return (addr / PAGESIZE);
}

void btread(int rrn, BTPAGE *page_ptr) {
    int addr = rrn*PAGESIZE + 4;
    fseek(btfd, addr, 0);
    fread(&(*page_ptr),sizeof(BTPAGE),1,btfd);
}

void btwrite(int rrn, BTPAGE page_ptr) {
    int addr = rrn*PAGESIZE + 4;
    fseek(btfd, addr, 0);
    fwrite(&page_ptr,sizeof(BTPAGE),1,btfd);
}

void pageinit(BTPAGE *p_page) {
	int j;
	for (j = 0; j < MAXKEYS; j++){
        p_page->key[j] = NIL;
        p_page->offset[j] = NIL;
        p_page->child[j] = NIL;
	}
    p_page->child[MAXKEYS] = NIL;
}

bool search_node(int key, BTPAGE *p_page, int *pos) {
    int i;
	for (i = 0; i < p_page->keycount && key > p_page->key[i]; i++);
	    *pos = i;
		if (*pos < p_page->keycount && key == p_page->key[*pos]) {
		    return true;
		}
        else {
            return false;
	}
}

void ins_in_page(int key,int offset,int r_child, BTPAGE *p_page) {
    int j;
	for(j = p_page-> keycount; key < p_page->key[j-1] && j > 0; j--) {
	    p_page->key[j] = p_page->key[j-1];
	    p_page->offset[j] = p_page->offset[j-1];
		p_page->child[j+1] = p_page->child[j];
	}
	p_page->keycount++;
	p_page->key[j] = key;
	p_page->offset[j] = offset;
	p_page->child[j+1] = r_child;
}

void split(int key, int offset, int r_child, BTPAGE *p_oldpage, int *promo_key, int *promo_offset, int *promo_r_child, BTPAGE *p_newpage) {
    int j;
	int mid;
	int workkeys[MAXKEYS+1];
	int workoffset[MAXKEYS+1];
	int workchil[MAXKEYS+2];
	for (j = 0; j < MAXKEYS; j++){
	    workkeys[j] = p_oldpage->key[j];
	    workoffset[j] = p_oldpage->offset[j];
		workchil[j] = p_oldpage->child[j];
	}
	workchil[j] = p_oldpage->child[j];
	for (j = MAXKEYS; key < workkeys[j-1] && j > 0; j--){
	    workkeys[j] = workkeys[j-1];
	    workoffset[j] = workoffset[j-1];
	    workchil[j+1] = workchil[j];
	}
	workkeys[j] = key;
	workoffset[j] = offset;
	workchil[j+1] = r_child;
 	*promo_r_child = getpage();
	pageinit(p_newpage);
	for (j = 0; j < MINKEYS; j++){
	    p_oldpage->key[j] = workkeys[j];
	    p_oldpage->offset[j] = workoffset[j];
		p_oldpage->child[j] = workchil[j];
		p_newpage->key[j] = workkeys[j+1+MINKEYS];
		p_newpage->offset[j] = workoffset[j+1+MINKEYS];
		p_newpage->child[j] = workchil[j+1+MINKEYS];
		printf("--- key: %d \n ", p_newpage->key[j]);
		printf("--- offset: %d \n", p_newpage->offset[j]);
		p_oldpage->key[j+MINKEYS] = NIL;
		p_oldpage->offset[j+MINKEYS] = NIL;
		p_oldpage->child[j+1+MINKEYS] = NIL;
	}
	p_oldpage->child[MINKEYS] = workchil[MINKEYS];
	p_newpage->child[MINKEYS] = workchil[j+1+MINKEYS];
	p_newpage->keycount = MAXKEYS - MINKEYS;
	p_oldpage->keycount = MINKEYS;
	*promo_key = workkeys[MINKEYS];
	*promo_offset = workoffset[MINKEYS];
}

void InOrder(int root) {
    int value;

    BTPAGE page;

    fseek(btfd,root,0);

    btread(root, &page);

    int i;
    int pos = 0;

    for (i = 0; i <= (page.keycount + page.keycount); i++) {
        value = pos/2;

        if ( (pos % 2) == 1 ) {
            printf("chave: %d \n", page.key[value]);
            printf("rrn: %d \n", page.offset[value]);
        }
        else {
            if (page.child[value] != NIL)
                InOrder(page.child[value]);
        }
        pos++;
     }

}

/*
int searchRecord(int key) {
    int value;

    BTPAGE page;

    fseek(btfd,root,0);

    btread(root, &page);

    int i;
    int pos = 0;
    bool found = false;

    while ( (pos < MAXKEYS) || (!found) ){
        if (key < page.key[pos])
            found = true;
            searchRecord(page.child[pos]);
        else
            pos++;
    }

    if
}
*/
