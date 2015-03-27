//////////////////////////////////////////////////////////////////////
//                                                                  //
//  Estrutura de Dados II                                           //
//  Projeto 2                                                       //
//                                                                  //
//  Alunos: Bruno Fouz Valente e Pedro Ivo Monteiro Privatto        //
//                                                                  //
//////////////////////////////////////////////////////////////////////

// Includes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define HASHSIZE 11
#define BUCKETSIZE 2

#define MAXKEYS 4
#define MINKEYS MAXKEYS/2
#define NIL (-1)
#define PAGESIZE sizeof(BTPAGE)

extern int root; // rrn of root page
extern int infd; // file descriptor of input file

// DeclaraÃ§Ãµes de estruturas e descritores de arquivos globais

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

typedef struct {
    int qtd;
    int key[BUCKETSIZE];
    int RRN[BUCKETSIZE];
} typeBucket; // Estrutura para o indice em Hash

typedef struct {
    int keycount; // number of keys in page
    int key[MAXKEYS]; // the actual keys
    int offset[MAXKEYS]; // rrn for the corresponding keys
    int child[MAXKEYS+1]; // ptrs to rrns of descendants
} BTPAGE; // Estrutura para o indice em Arvore-B

FILE *arqVacinas; // Arquivo principal 1 (AP1)
FILE *arqCachorros; // Arquivo principal 2 (AP2)
FILE *hash;
FILE *arqIndice1ArvB; // file descriptor of btree file

// ProtÃ³tipos

void menu();
void abrirArquivos();
void cadastrarCachorro();
void cadastrarVacina();
int procurarCachorro();
void insertHash();
void buscarVacinaHash();
void buscarVacinaArvB();

void btclose();
bool btopen();
void btread(int rrn, BTPAGE *page_ptr);
void btwrite(int rrn, BTPAGE page_ptr);
int create_root(int key, int rrn, int left, int right);
void create_tree();
int getpage();
int getroot();
bool insert(int rrn, int key, int offset, int *promo_r_child, int *promo_key, int *promo_offset);
void ins_in_page(int key,int offset,int r_child, BTPAGE *p_page);
void pageinit(BTPAGE *p_page);
void putroot(int root);
bool search_node(int key, BTPAGE *p_page, int *pos);
void split(int key, int offset, int r_child, BTPAGE *p_oldpage, int *promo_key, int *promo_offset, int *promo_r_child, BTPAGE *p_newpage);
void percorreVacinas(int root);

bool invalidarHash();
void createHash();

// Function principal (main) do programa

int main() {
	abrirArquivos();

	menu();

	fclose(arqCachorros);
	fclose(arqVacinas);
	fclose(hash);
	btclose();

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
				percorreVacinas(getroot());
				break;
			case '4':
				buscarVacinaHash();
				break;
			case '5':
				buscarVacinaArvB();
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

// Function para abertura e verificaÃ§Ã£o dos arquivos

void abrirArquivos() {
	arqCachorros = fopen("AP2.dat", "r+b");
	if (arqCachorros == NULL) {
		arqCachorros = fopen("AP2.dat", "w+b");
	}

	arqVacinas = fopen("AP1.dat", "r+b");
	if (arqVacinas == NULL) {
		arqVacinas = fopen("AP1.dat", "w+b");
	}

	hash = fopen("Indice1Hash.dat", "r+b");
    if ((hash == NULL) || invalidarHash()) {
        createHash();
	}
}

// Function para procurar o cÃ³digo de um cachorro no AP2 e retornar se existe ou nÃ£o

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
		return NIL;
	}
}

// Function para o cadastramento de uma vacina (item 2 do menu)

void cadastrarVacina() {
	struct ap1Struct temporario;
	struct ap2Struct tempCachorro;
	char buffer[105];
	int offset;

	fseek(arqVacinas, 0, 2);
	offset = ftell(arqVacinas);

	do {
		system("cls");
		printf("Cadastro de Vacina\n\n");
		printf("Digite o codigo do cachorro: ");
		scanf("%d", &temporario.codCachorro);

		if (procurarCachorro(temporario.codCachorro) == NIL) {
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
	} while (procurarCachorro(temporario.codCachorro) == NIL);

	printf("Codigo de controle: ");
	scanf("%d", &temporario.codControle);
	printf("Nome da vacina: ");
	scanf("%s", &temporario.nomeVacina);
	printf("Data da vacinacao: ");
	scanf("%s", &temporario.dataVacina);
	printf("Responsavel pela aplicacao: ");
	scanf("%s", &temporario.respAplic);

	fseek(arqVacinas, 0, 2);
	fwrite(&temporario, sizeof(struct ap1Struct), 1, arqVacinas);

	printf("\n");

	// Adiciona no indice do Hash

	printf("// Indice Hash //\n\n");

	int key = temporario.codControle;
	insertHash(key, offset);

	printf("\n");

	// Adiciona no indice da Arvore-B

	printf("// Indice Arvore-B //\n\n");

	bool promoted; // boolean: tells if a promotion from below
	int root, // rrn of root page
	promo_rrn; // rrn promoted from below
	int promo_key, promo_offset; // key promoted from below

	if (btopen()) {
		root = getroot();
	} else {
		create_tree();
		root = NIL;
	}

	promoted = insert(root, key, offset, &promo_rrn, &promo_key, &promo_offset);
	if (promoted) {
		root = create_root(promo_key, promo_offset, root, promo_rrn);
	}

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

void createHash() {
    int i;

    typeBucket bucket;
    bucket.qtd = 0;
    for (i = 0; i < BUCKETSIZE; i++) {
        bucket.key[i] = NIL;
        bucket.RRN[i] = NIL;
    }

    hash = fopen("Indice1Hash.dat", "w+b");

    for (i = 1; i <= HASHSIZE; i++)
        fwrite(&bucket,sizeof(bucket),1,hash);
}

bool invalidarHash() {
    fseek(hash,0,2);
    int fileSize = ftell(hash);
    fseek(hash,0,0);

    if (fileSize != (HASHSIZE * (BUCKETSIZE * 2 * sizeof(int) + sizeof(int))) )
        return true;
}

int hashFunction(int key) {
    int value = (key % HASHSIZE);
    return value;
}

void progressiveOverflow(int key, int RRN, int address, int refAddress, int *count) {
    fseek(hash, address * sizeof(typeBucket), 0);

    typeBucket bucket;
    fread(&bucket, sizeof(typeBucket), 1, hash);

    switch (bucket.qtd) {
        case 0:
            bucket.qtd++;
            bucket.key[0] = key;
            bucket.RRN[0] = RRN;

            fseek(hash, - sizeof(typeBucket), 1);
            fwrite(&bucket, sizeof(typeBucket), 1, hash);

            break;
        case 1:
            bucket.qtd++;
            bucket.key[1] = key;
            bucket.RRN[1] = RRN;

            fseek(hash, - sizeof(typeBucket), 1);
            fwrite(&bucket, sizeof(typeBucket), 1, hash);

            break;
        case 2:
            *count = *count + 1;

            printf("Colisao!\n");
			printf("Tentativa %d\n", *count);

            if ((address + 1) > (HASHSIZE - 1)) {
                address = 0;
            } else {
                address++;
			}

            if (address != refAddress) {
                progressiveOverflow(key, RRN, address, refAddress, count);
            } else {
                *count = NIL;
			}

            break;
    }
}

void insertHash(int key, int RRN) {
    int address = hashFunction(key);

    printf("Endereco %d\n", address);
    int count = 0;

    progressiveOverflow(key, RRN, address, address, &count);

    if (count != NIL) {
        printf("Chave %d inserida com sucesso\n", key);
    } else {
        printf("Chave %d nao inserida, hash cheio!\n", key);
	}
}

int searchHashR(int key, int *address, int refAddress, int *count) {
    fseek(hash,(*address)*sizeof(typeBucket),0);

    typeBucket bucket;
    fread(&bucket,sizeof(typeBucket),1,hash);

    switch (bucket.qtd) {
        case 0:
            *count = NIL;
            break;
        case 1:
            if ( bucket.key[0] == key ) {
                return bucket.RRN[0];
            }
            else {
                *count = NIL;
            }
            break;
        case 2:
            if ( bucket.key[0] == key )
                return bucket.RRN[0];
            else if ( bucket.key[1] == key )
                return bucket.RRN[1];
            else {
                *count = *count + 1;

                if ( (*address + 1) > (HASHSIZE - 1) )
                    *address = 0;
                else
                    (*address)++;

                if (*address != refAddress) {
                    searchHashR(key, address, refAddress, count);
                }
                else {
                    *count = NIL;
                }
            }

            break;
    }
}

bool searchHash(int key) {
    int address = hashFunction(key);
    int count = 1;

    int RRN = searchHashR(key, &address, address, &count);

    if (count != NIL) {
        printf("Chave %d encontrada, endereco %d, %d acessos\n", key, address, count);
		return true;
    } else {
        printf("Chave %d nao encontrada\n", key);
		return false;
	}
}

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

	found = search_node (key, &page, &pos);
	if (found) {
	    printf ("Chave %d duplicada\n", key);
		return (0);
	}

	if (page.keycount < MAXKEYS) {
	    ins_in_page(p_b_key, p_b_offset, p_b_rrn, &page);
		btwrite(rrn, page);
		printf("Chave %d inserida com sucesso\n", p_b_key);
		return false;
	} else {
 	    split(p_b_key, p_b_offset, p_b_rrn, &page, promo_key, promo_offset, promo_r_child, &newpage);
 	    printf("Divisao de no\n");
		btwrite(rrn, page);
		btwrite(*promo_r_child, newpage);
		printf("Chave %d inserida com sucesso\n", p_b_key);
		return true;
	}
}

bool btopen() {
    arqIndice1ArvB = fopen("Indice1ArvB.dat", "r+b");
    return (arqIndice1ArvB != NULL);
}

void btclose() {
    close(arqIndice1ArvB);
}

int getroot() {
    int root;
    fseek(arqIndice1ArvB, 0, 0);

    fread(&root, sizeof(int), 1, arqIndice1ArvB);

    return (root);
}

void putroot(int root) {
	fseek(arqIndice1ArvB, 0, 0);
	fwrite(&root, sizeof(int), 1, arqIndice1ArvB);
}

int create_root(int key, int offset, int left, int right) {
    BTPAGE page;
    int rrn = getpage();

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

void create_tree() {
	arqIndice1ArvB = fopen("Indice1ArvB.dat", "w+b");
	fclose(arqIndice1ArvB);
	btopen();

	fseek(arqIndice1ArvB, 0, 0);
	int n = NIL;
	fwrite(&n, sizeof(int), 1, arqIndice1ArvB);
}

int getpage() {
    fseek(arqIndice1ArvB,0,2);

    int addr = ftell(arqIndice1ArvB);

    return (addr / PAGESIZE);
}

void btread(int rrn, BTPAGE *page_ptr) {
    int addr = rrn*PAGESIZE + 4;
    fseek(arqIndice1ArvB, addr, 0);
    fread(&(*page_ptr),sizeof(BTPAGE),1,arqIndice1ArvB);
}

void btwrite(int rrn, BTPAGE page_ptr) {
    int addr = rrn*PAGESIZE + 4;
    fseek(arqIndice1ArvB, addr, 0);
    fwrite(&page_ptr,sizeof(BTPAGE),1,arqIndice1ArvB);
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

// Function para percorrer o indice em Arvore-B  (item 3 do menu)

void percorreVacinas(int root) {
	struct ap1Struct temporario;
	struct ap2Struct tempCachorro;
    int value;

    BTPAGE page;

    btread(root, &page);

    int i;
    int pos = 0;

    for (i = 0; i <= (page.keycount + page.keycount); i++) {
        value = pos/2;

        if ((pos % 2) == 1) {
            printf("Chave: %d \n", page.key[value]);
            printf("RRN: %d \n\n", page.offset[value]);

			fseek(arqVacinas, page.offset[value], 0);
			fread(&temporario, sizeof(struct ap1Struct), 1, arqVacinas);

			printf("Dados da vacina:\n");
			printf("Codigo da vacina: %d\n", temporario.codControle);
			printf("Nome da vacina: %s\n", temporario.nomeVacina);
			printf("Data da vacinacao: %s\n", temporario.dataVacina);
			printf("Responsavel pela aplicacao: %s\n\n", temporario.respAplic);

			fseek(arqCachorros, ((temporario.codCachorro) * sizeof(tempCachorro)), 0);
			fread(&tempCachorro, sizeof(tempCachorro), 1, arqCachorros);

			printf("Dados do cachorro que recebera a vacina:\n");
			printf("Codigo do cachorro: %d\n", tempCachorro.codCachorro);
			printf("Nome do cachorro: %s\n", tempCachorro.nomeCachorro);
			printf("Raca: %s\n\n", tempCachorro.raca);
			system("pause");
        } else {
            if (page.child[value] != NIL) {
                percorreVacinas(page.child[value]);
			}
        }
        pos++;
     }
}

int searchRecord(int key, int page_ptr) {
    BTPAGE page;

    btread(page_ptr, &page);

    int pos = 0;
    bool found = false;

	while ( (pos < page.keycount) && (!found) ) {
		if (key == page.key[pos]) {
			found = true;
			return page.offset[pos];
		} else if (key < page.key[pos]) {
			found = true;
			if (page.child[pos] != NIL) {
				return searchRecord(key, page.child[pos]);
			} else {
				return NIL;
			}
		}

		pos++;
	}

	if (!found) {
		if (page.child[pos] != NIL) {
			return searchRecord(key, page.child[pos]);
		} else {
			return NIL;
		}
	}
}

// Function para a busca de vacina no indice em Hash (item 4 do menu)

void buscarVacinaHash() {
	struct ap1Struct temporario;
	struct ap2Struct tempCachorro;
	int codigo;

	printf("Digite o codigo da vacina: ");
	scanf("%d", &codigo);

	int address = hashFunction(codigo);
    int count = 1;

    int RRN = searchHashR(codigo, &address, address, &count);

	bool encontrado = searchHash(codigo);

	if (encontrado) {
		printf("\n");

		fseek(arqVacinas, RRN, 0);
		fread(&temporario, sizeof(struct ap1Struct), 1, arqVacinas);

		printf("Dados da vacina:\n");
		printf("Codigo da vacina: %d\n", temporario.codControle);
		printf("Nome da vacina: %s\n", temporario.nomeVacina);
		printf("Data da vacinacao: %s\n", temporario.dataVacina);
		printf("Responsavel pela aplicacao: %s\n\n", temporario.respAplic);

		fseek(arqCachorros, ((temporario.codCachorro) * sizeof(tempCachorro)), 0);
		fread(&tempCachorro, sizeof(tempCachorro), 1, arqCachorros);

		printf("Dados do cachorro que recebera a vacina:\n");
		printf("Codigo do cachorro: %d\n", tempCachorro.codCachorro);
		printf("Nome do cachorro: %s\n", tempCachorro.nomeCachorro);
		printf("Raca: %s\n\n", tempCachorro.raca);
		system("pause");
	} else {
		printf("\n");
	}
}

// Function para a busca de vacina no indice em Arvore-B (item 5 do menu)

void buscarVacinaArvB() {
	struct ap1Struct temporario;
	struct ap2Struct tempCachorro;
	int codigo, RRN;

	printf("Digite o codigo da vacina: ");
	scanf("%d", &codigo);

	RRN = searchRecord(codigo, getroot());

	int page_ptr = getroot();
	BTPAGE page;
	btread(page_ptr, &page);

	int pos = 0;
	bool found = false;

	if (codigo == page.key[pos]) {
		found = true;
		printf("Chave %d encontrada, pagina %d, posicao %d", codigo, page_ptr, pos);
	}

	if (RRN != NIL) {
		fseek(arqVacinas, RRN, 0);
		fread(&temporario, sizeof(struct ap1Struct), 1, arqVacinas);

		printf("Dados da vacina:\n");
		printf("Codigo da vacina: %d\n", temporario.codControle);
		printf("Nome da vacina: %s\n", temporario.nomeVacina);
		printf("Data da vacinacao: %s\n", temporario.dataVacina);
		printf("Responsavel pela aplicacao: %s\n\n", temporario.respAplic);

		fseek(arqCachorros, ((temporario.codCachorro) * sizeof(tempCachorro)), 0);
		fread(&tempCachorro, sizeof(tempCachorro), 1, arqCachorros);

		printf("Dados do cachorro que recebera a vacina:\n");
		printf("Codigo do cachorro: %d\n", tempCachorro.codCachorro);
		printf("Nome do cachorro: %s\n", tempCachorro.nomeCachorro);
		printf("Raca: %s\n\n", tempCachorro.raca);
		system("pause");
	} else {
		printf("Chave %d nao encontada\n", codigo);
	}
}
