// Uso:  ./tetris           corre los tests y despues el juego
//       ./tetris --test    corre solo los tests (valgrind --leak-check=full ./tetris --test)

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ========= CONSTANTES DEL JUEGO =========
#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 800
#define GRID_ROWS 20
#define GRID_COLS 10
#define CELL_SIZE 35
#define OFFSET_X 75
#define OFFSET_Y 50
#define TICK_RATE 30

const int TETROMINOS[7][4][4][4] = {
    { {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}, {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}}, {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}}, {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}} },
    { {{2,0,0,0},{2,2,2,0},{0,0,0,0},{0,0,0,0}}, {{0,2,2,0},{0,2,0,0},{0,2,0,0},{0,0,0,0}}, {{0,0,0,0},{2,2,2,0},{0,0,2,0},{0,0,0,0}}, {{0,2,0,0},{0,2,0,0},{2,2,0,0},{0,0,0,0}} },
    { {{0,0,3,0},{3,3,3,0},{0,0,0,0},{0,0,0,0}}, {{0,3,0,0},{0,3,0,0},{0,3,3,0},{0,0,0,0}}, {{0,0,0,0},{3,3,3,0},{3,0,0,0},{0,0,0,0}}, {{3,3,0,0},{0,3,0,0},{0,3,0,0},{0,0,0,0}} },
    { {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}} },
    { {{0,5,5,0},{5,5,0,0},{0,0,0,0},{0,0,0,0}}, {{0,5,0,0},{0,5,5,0},{0,0,5,0},{0,0,0,0}}, {{0,0,0,0},{0,5,5,0},{5,5,0,0},{0,0,0,0}}, {{5,0,0,0},{5,5,0,0},{0,5,0,0},{0,0,0,0}} },
    { {{0,6,0,0},{6,6,6,0},{0,0,0,0},{0,0,0,0}}, {{0,6,0,0},{0,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,0,0,0},{6,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,6,0,0},{6,6,0,0},{0,6,0,0},{0,0,0,0}} },
    { {{7,7,0,0},{0,7,7,0},{0,0,0,0},{0,0,0,0}}, {{0,0,7,0},{0,7,7,0},{0,7,0,0},{0,0,0,0}}, {{0,0,0,0},{7,7,0,0},{0,7,7,0},{0,0,0,0}}, {{0,7,0,0},{7,7,0,0},{7,0,0,0},{0,0,0,0}} }
};

SDL_Color colors[8] = {
    {30, 30, 30, 255}, {0, 255, 255, 255}, {0, 0, 255, 255}, {255, 165, 0, 255},
    {255, 255, 0, 255}, {0, 255, 0, 255}, {128, 0, 128, 255}, {255, 0, 0, 255}
};

// ========= ESTRUCTURAS DE DATOS =========

// 1. Nodo para la fila del tablero (Lista Enlazada)
typedef struct RowNode {
    int cells[GRID_COLS]; 
    struct RowNode* next; 
} RowNode;

// 2. Nodo para la cola de próximas piezas (Queue)
typedef struct PieceQueueNode {
    int type; 
    struct PieceQueueNode* next;
} PieceQueueNode;

// 3. Nodo para el sistema de partículas 
typedef struct ParticleNode {
    float x, y;
    float vel_x, vel_y;
    int color;
    int life_timer;
    struct ParticleNode* next;
} ParticleNode;

typedef struct ActivePiece {
    int col, row;
    int type;      
    int rotation;  
    int active;
} ActivePiece;

typedef struct GameBoard {
    RowNode* first_row;             
    PieceQueueNode* queue_head;     
    PieceQueueNode* queue_tail;     
    ParticleNode* first_particle;   
    ActivePiece current_piece;
    int score;
    int tick_timer;
} GameBoard;

// ========= VARIABLES GLOBALES =========
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
GameBoard* game_board = NULL;

// ========= EJERCICIO 1: STRINGS (NO USAR string.h) =========

char* strDuplicate(char* src) {

    int i = 0; //Inicializa un contador de caracteres

    while (src[i] != '\0') {
        i++; //Al finalizar el ciclo, i almacena la longitud del string sin contar el terminador '\0'.
    }

    //Utilizamos la longitud (i) del string más 1 byte extra para el terminador '\0' y reservamos la memoria.
    char* resultado = (char*) malloc((i + 1) * sizeof(char));

    if (resultado == NULL) {
        return NULL; //Se verifica que el malloc se ejecutó bien.
    }

    //El contador i se incrementa en 1 por cada carácter recorrido, hasta encontrar el terminador '\0'.
    for (i = 0; src[i] != '\0'; i++) {
        resultado[i] = src[i]; //El carácter en la i-ésima posición se copia en el nuevo string.
    }

    resultado[i] = '\0'; //Se agrega el terminador '\0' al nuevo string.
    return resultado; //Devuelve el nuevo string duplicado.
}


int strCompare(char* s1, char* s2) {

    int i = 0;

    //Comparar los caracteres de los dos strings hasta encontrar una diferencia.
    while (s1[i] != '\0' && s2[i] != '\0') {

        if (s1[i] == s2[i]) {
            i++;
        } else {

            //Si son diferentes, vemos cuál carácter es menor.
            if (s1[i] < s2[i]) {
                return 1;
            } else {
                return -1;
            }
        }
    }

    //Si los dos strings terminaron al mismo tiempo, son iguales.
    if (s1[i] == '\0' && s2[i] == '\0') {
        return 0;
    }

    //Si uno termina antes, ese string es menor que el otro.
    if (s1[i] == '\0') {
        return 1;
    }

    return -1;
}


char* strConcatenate(char* src1, char* src2) {

    //Buscamos la longitud de los strings que vamos a utilizar.
    int longitud = 0;
    int j = 0;

    //Este primer while recorre el primer string.
    while (src1[j] != '\0') {
        longitud++; //Guarda su longitud.
        j++;
    }

    j = 0;

    //Recorre el segundo string.
    while (src2[j] != '\0') {
        longitud++; //Suma su longitud a la longitud ya guardada.
        j++;
    }

    //Se pide memoria con malloc usando la longitud total + 1.
    char* resultado = (char*) malloc((longitud + 1) * sizeof(char));

    if (resultado == NULL) {
        return NULL;
    }

    int indice = 0;
    j = 0;

    while (src1[j] != '\0') {
        resultado[indice] = src1[j];
        indice++;
        j++;
    }

    j = 0;

    while (src2[j] != '\0') {
        resultado[indice] = src2[j];
        indice++;
        j++;
    }

    resultado[indice] = '\0';

    //Liberamos los strings originales porque ya los copiamos.
    free(src1);
    free(src2);

    return resultado;
}



// ========= EJERCICIO 2: FUNCIONES A IMPLEMENTAR (EL TP) =========


GameBoard* gameBoardNew() {

    // Reservamos memoria para el tablero.
    GameBoard* board = malloc(sizeof(GameBoard));
    if (board == NULL) {
        return NULL;
    }

    //Inicializamos los punteros y los valores del tablero.
    board->first_row = NULL;
    board->queue_head = NULL;
    board->queue_tail = NULL;
    board->first_particle = NULL;
    board->current_piece.active = 0;
    board->score = 0;
    board->tick_timer = 0;
    RowNode* last = NULL;
    int i = 0;

    //Creamos las 20 filas y las conectamos entre sí.
    while (i < GRID_ROWS) {
        RowNode* row = malloc(sizeof(RowNode));

        //Si falla malloc, liberamos lo que ya habiamos creado.
        if (row == NULL) {
            RowNode* curr = board->first_row;
            while (curr != NULL) {
                RowNode* next = curr->next;
                free(curr);
                curr = next;
            }
            free(board);
            return NULL;
        }

        row->next = NULL;
        int c = 0;
        while (c < GRID_COLS) {
            row->cells[c] = 0;
            c++;
        }

        //Si es la primera fila, la guardamos en first_row.
        //Si no, la conectamos con la fila anterior.
        if (last == NULL) {
            board->first_row = row;
        } else {
            last->next = row;
        }

        last = row;
        i++;
    }

    return board;
}



void gameBoardDelete(GameBoard* board) {
    //Si el tablero no existe, no hay nada que liberar.
    if (board == NULL) {
        return;
    }

    //Recorremos las filas y liberamos cada una.
    RowNode* row = board->first_row;
    while (row != NULL) {
        RowNode* next = row->next;
        free(row);
        row = next;
    }

    //Hacemos lo mismo con las piezas de la cola.
    PieceQueueNode* piece = board->queue_head;
    while (piece != NULL) {
        PieceQueueNode* next = piece->next;
        free(piece);
        piece = next;
    }

    //Liberamos tambien todas las particulas.
    ParticleNode* part = board->first_particle;
    while (part != NULL) {
        ParticleNode* next = part->next;
        free(part);
        part = next;
    }

    //Al final liberamos el tablero.
    free(board); //Al final liberamos el tablero.
}


void gameBoardAddParticle(GameBoard* board, float x, float y, int color) {
    ParticleNode* memdePN = (ParticleNode*) malloc(sizeof(ParticleNode)); //Reservamos memoria para una nueva particula.

    if (memdePN == NULL) {
        return;
    }

    //Guardamos la posicion y le damos velocidades aleatorias.
    memdePN->x = x;
    memdePN->y = y;
    memdePN->vel_x = (rand() % 5) / 10.0;
    memdePN->vel_y = (rand() % 10) / 10.0;
    memdePN->life_timer = 2;
    memdePN->color = color;

    //Agregamos la particula al principio de la lista.
    memdePN->next = board->first_particle;
    board->first_particle = memdePN;
}



int gameBoardCheckAndClearLines(GameBoard* board) {
    RowNode* actual = board->first_row;
    RowNode* anterior = NULL;
    int eliminadas = 0;
    int fila = 0;

    while (actual != NULL) {
        RowNode* siguiente = actual->next;

        int i = 0;
        int filacompleta = 1;

        // Revisar si todas las celdas están ocupadas.
        while (i < GRID_COLS && filacompleta == 1) {
            if (actual->cells[i] <= 0) {
                filacompleta = 0;
            }
            i++;
        }

        if (filacompleta == 1) {

            // Crear una nueva fila vacia.
            RowNode* nuevafila = (RowNode*) malloc(sizeof(RowNode));
            if (nuevafila == NULL) {
                return eliminadas;
            }

            for (int j = 0; j < GRID_COLS; j++) {
                nuevafila->cells[j] = 0;
            }

            // Generar una particula por cada celda.
            for (int j = 0; j < GRID_COLS; j++) {
                float x = OFFSET_X + j * CELL_SIZE;
                float y = OFFSET_Y + fila * CELL_SIZE;
                gameBoardAddParticle(board, x, y, actual->cells[j]);
            }

            // Desenlazar la fila completa.
            if (anterior == NULL) {
                board->first_row = siguiente;
            } else {
                anterior->next = siguiente;
            }

            free(actual);

            // Insertar la fila vacía en la cabeza.
            nuevafila->next = board->first_row;
            board->first_row = nuevafila;

            if (anterior == NULL) {
                anterior = nuevafila;
            }

            actual = siguiente;
            eliminadas++;
        } else {
            anterior = actual;
            actual = siguiente;
        }

        fila++;
    }

    return eliminadas;
}



void gameBoardEnqueuePiece(GameBoard* board, int type) {

    if (board == NULL) {
        return;
    }

    //Creamos un nuevo nodo para guardar la pieza.
    PieceQueueNode* nuevo = (PieceQueueNode*) malloc(sizeof(PieceQueueNode));

    if (nuevo == NULL) {
        return;
    }

    nuevo->type = type;
    nuevo->next = NULL;

    //Si la cola esta vacia, el nuevo nodo es el primero y el ultimo.
    if (board->queue_head == NULL) {
        board->queue_head = nuevo;
        board->queue_tail = nuevo;
    } else {

        //Si ya hay piezas, agregamos el nodo al final de la cola.
        board->queue_tail->next = nuevo;
        board->queue_tail = nuevo;
    }
}



int gameBoardDequeuePiece(GameBoard* board, int* out_type) {

    //Si no hay tablero o no tenemos donde guardar el tipo, devolvemos 0.
    if (board == NULL || out_type == NULL) {
        return 0;
    }

    if (board->queue_head == NULL) {
        return 0;
    }

    //Guardamos el tipo de la primera pieza antes de eliminarla.
    PieceQueueNode* actual = board->queue_head;
    *out_type = actual->type;

    board->queue_head = actual->next;

    //Si ya no quedan piezas, tambien dejamos el tail en NULL.
    if (board->queue_head == NULL) {
        board->queue_tail = NULL;
    }

    free(actual);
    return 1;
}

// ========= LOGICA DE DIBUJADO Y COLISIONES (NO MODIFICAR) =========

RowNode* getRowAt(GameBoard* board, int index) {
    RowNode* curr = board->first_row;
    for(int i = 0; i < index && curr != NULL; i++) curr = curr->next;
    return curr;
}

int canMove(GameBoard* board, int target_col, int target_row, int target_rot) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (TETROMINOS[board->current_piece.type][target_rot][r][c] != 0) {
                int board_col = target_col + c;
                int board_row = target_row + r;
                if (board_col < 0 || board_col >= GRID_COLS || board_row >= GRID_ROWS) return 0;
                if (board_row >= 0) {
                    RowNode* row_node = getRowAt(board, board_row);
                    if (row_node && row_node->cells[board_col] != 0) return 0;
                }
            }
        }
    }
    return 1;
}

void lockPiece(GameBoard* board) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            int val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][r][c];
            if (val != 0) {
                RowNode* row_node = getRowAt(board, board->current_piece.row + r);
                if (row_node) row_node->cells[board->current_piece.col + c] = val;
            }
        }
    }
    board->current_piece.active = 0;
    board->score += gameBoardCheckAndClearLines(board) * 100;
}

// Devuelve 0 si la pieza nueva no entra (GAME OVER), 1 en otro caso.
int spawnNextPiece(GameBoard* board) {
    int type;
    if (gameBoardDequeuePiece(board, &type)) {
        board->current_piece.type = type;
        board->current_piece.rotation = 0;
        board->current_piece.col = GRID_COLS / 2 - 2;
        board->current_piece.row = 0;
        board->current_piece.active = 1;
        gameBoardEnqueuePiece(board, rand() % 7);
        if (!canMove(board, board->current_piece.col, board->current_piece.row, board->current_piece.rotation)) {
            printf("GAME OVER. Puntaje final: %d\n", board->score);
            board->current_piece.active = 0;
            return 0;   // main sale del loop y libera tablero y SDL
        }
    }
    return 1;
}

// Devuelve 0 cuando termina la partida.
int gameBoardUpdate(GameBoard* board) {
    if (!board) return 0;
    if (!board->current_piece.active && !spawnNextPiece(board)) return 0;

    board->tick_timer++;
    if (board->tick_timer >= TICK_RATE) {
        board->tick_timer = 0;
        if (canMove(board, board->current_piece.col, board->current_piece.row + 1, board->current_piece.rotation)) {
            board->current_piece.row++;
        } else {
            lockPiece(board);
        }
    }

    ParticleNode* curr_part = board->first_particle;
    ParticleNode* prev_part = NULL;
    while (curr_part) {
        curr_part->x += curr_part->vel_x;
        curr_part->y += curr_part->vel_y;
        curr_part->vel_y += 0.5f;
        curr_part->life_timer--;

        if (curr_part->life_timer <= 0) {
            ParticleNode* to_delete = curr_part;
            if (prev_part == NULL) {
                board->first_particle = curr_part->next;
                curr_part = board->first_particle;
            } else {
                prev_part->next = curr_part->next;
                curr_part = prev_part->next;
            }
            free(to_delete);
        } else {
            prev_part = curr_part;
            curr_part = curr_part->next;
        }
    }
    return 1;
}

void gameBoardDraw(GameBoard* board) {
    if (!board) return;
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect board_rect = {OFFSET_X - 2, OFFSET_Y - 2, GRID_COLS * CELL_SIZE + 4, GRID_ROWS * CELL_SIZE + 4};
    SDL_RenderDrawRect(renderer, &board_rect);

    RowNode* current_row = board->first_row;
    int r = 0;
    while (current_row) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (current_row->cells[c] > 0) {
                SDL_Color col = colors[current_row->cells[c]];
                SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                SDL_Rect cell_rect = {OFFSET_X + c * CELL_SIZE, OFFSET_Y + r * CELL_SIZE, CELL_SIZE - 1, CELL_SIZE - 1};
                SDL_RenderFillRect(renderer, &cell_rect);
            }
        }
        current_row = current_row->next;
        r++;
    }

    if (board->current_piece.active) {
        for(int pr = 0; pr < 4; pr++) {
            for(int pc = 0; pc < 4; pc++) {
                int cell_val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][pr][pc];
                if (cell_val != 0) {
                    SDL_Color col = colors[cell_val];
                    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                    SDL_Rect p_rect = {OFFSET_X + (board->current_piece.col + pc) * CELL_SIZE, 
                                       OFFSET_Y + (board->current_piece.row + pr) * CELL_SIZE, 
                                       CELL_SIZE - 1, CELL_SIZE - 1};
                    SDL_RenderFillRect(renderer, &p_rect);
                }
            }
        }
    }

    ParticleNode* curr_part = board->first_particle;
    while (curr_part) {
        SDL_Color col = colors[curr_part->color];
        int alpha = curr_part->life_timer * 8;   // se satura: SDL recibe un Uint8
        if (alpha > 255) alpha = 255;
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, alpha);
        SDL_Rect part_rect = {(int)curr_part->x, (int)curr_part->y, 6, 6};
        SDL_RenderFillRect(renderer, &part_rect);
        curr_part = curr_part->next;
    }
    SDL_RenderPresent(renderer);
}

int inicializar() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 0;
    window = SDL_CreateWindow("Tetris Base - TP", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) return 0;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) return 0;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return 1;
}

// ========= TESTS (COMPLETAR) =========
  void correrTests() {
    // TODO: Implementar los casos minimos del enunciado. Por ejemplo:
    //  - strDuplicate: "", "x", un string con distintos caracteres.


char* dup1 = strDuplicate(""); // cuando es un string vacio
char* dup2 = strDuplicate("y"); //cuando tengo un solo caracter
char* dup3 = strDuplicate("td 2026"); // cuando tengo disitintos caracteres


//libero la memorua de los 3 dup que cree
free(dup1);
free(dup2);
free(dup3);
    //  - strCompare: "" vs "", "a" vs "a", y una desigualdad en ambos sentidos (s1/s2 y s2/s1).


char* cmp_1 = strcomapre("a");
char* cmp_2 = strcompare ("a");


int m1= strCompare (cmp_1,cmp_2); //el caso test a vs a
//libero la memoria pedida
free(cmp_1);
free(cmp_2);
//ahora me fijo en un caso en el que esten vacios
char* cmp_vacio1 = strDuplicate("");
char* cmp_vacio2 = strDuplicate ("");


int m2= strCompare (cmp_vacio1,cmp_vacio2);
//libero la memoria pedida
free(cmp_vacio1);
free(cmp_vacio2);


  //caso de desigualdad, ej: a vs s este caso seria a<s
  char* cmp_chico = strDuplicate("a");
  char* cmp_grande = strDuplicate("s");
  int m3 = strCuplicate (cmp_chico, cmp_grande);
    free(cmp_chico);
    free(cmp_grande);
    //ahora inverso s<a
 char* cmp_chico2 = strDuplicate("s");
  char* cmp_grande2 = strDuplicate("a");
  int m4 = strCuplicate (cmp_chico2, cmp_grande2);
    free(cmp_chico2);
    free(cmp_grande2);


    //casos de strCocnatenate
//strConcatenate libera internamente asi que se crean con StrDuplicate


//strg vacio + str de 3 caracteres
char*cas_1 = strDuplicate("");
char*cas_2 = strDuplicate("aei");
char* res = strConcatenate(cas_1,cas_2); //la funcion str concatenate libera cas_1 y cas_2
free(res); //libera lo que pedimos para obtener la respuesta


// caso de str 5 caracteres + strg 5 caracteres
char*cas_3 = strDuplicate("holis");
char*cas_4 = strDuplicate("holas");
char* res2 = strConcatenate(cas_3,cas_4);
free(res2); //libera resultado final
}
//logica del tablero 
void test4_1() {
    int out;
    GameBoard* b = gameBoardNew();
    int i = 0;
    while (i < 5) {
        gameBoardEnqueuePiece(b, i);
        i++;
    }
    int ordenOk = 1;
    i = 0;
    while (i < 5) {
        int r = gameBoardDequeuePiece(b, &out);
        if (r != 1 || out != i) {
            ordenOk = 0;
        }
        i++;
    }
    verificar(ordenOk, "4.1: 5 pops respetan el orden FIFO");
    verificar(gameBoardDequeuePiece(b, &out) == 0, "4.1: el sexto pop devuelve 0");
    verificar(b->queue_head == NULL, "4.1: queue_head en NULL");
    verificar(b->queue_tail == NULL, "4.1: queue_tail en NULL");
    gameBoardEnqueuePiece(b, 6);
    verificar(gameBoardDequeuePiece(b, &out) == 1 && out == 6, "4.1: push y pop despues de vaciar");
    gameBoardDelete(b);
}
void test4_2() {
    GameBoard* b = gameBoardNew();
    llenarFila(b, 19);
    RowNode* cimaVieja = b->first_row;
    verificar(gameBoardCheckAndClearLines(b) == 1, "4.2: devuelve 1");
    verificar(b->first_row != cimaVieja, "4.2: first_row es un nodo nuevo");
    verificar(filaVacia(b->first_row), "4.2: la nueva first_row esta vacia");
    verificar(contarFilas(b) == GRID_ROWS, "4.2: la lista sigue con 20 filas");
    gameBoardDelete(b);
}

3. 
void test4_3() {
    GameBoard* b = gameBoardNew();
    llenarFila(b, 0);
    llenarFila(b, 1);
    verificar(gameBoardCheckAndClearLines(b) == 2, "4.3: devuelve 2");
    verificar(contarFilas(b) == GRID_ROWS, "4.3: la lista sigue con 20 filas");
    gameBoardDelete(b);
}


}

// Compara contra "--test" sin depender de string.h ni del Ejercicio 1.
int esFlagTest(char* arg) {
    char* flag = "--test";
    int i = 0;
    while (arg[i] != '\0' && arg[i] == flag[i]) i++;
    return arg[i] == '\0' && flag[i] == '\0';
}

int main(int argc, char* args[]) {
    srand(time(NULL));

    // Los tests corren antes de SDL. Con --test termina aca, asi valgrind solo
    // reporta la memoria del TP (SDL tiene bloques propios que no se liberan).
    correrTests();
    if (argc > 1 && esFlagTest(args[1])) return 0;

    if (!inicializar()) return 1;

    game_board = gameBoardNew();
    if (game_board == NULL) {
        printf("ADVERTENCIA: Implementa gameBoardNew() para iniciar el juego.\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    for(int i=0; i<3; i++) gameBoardEnqueuePiece(game_board, rand() % 7);

    SDL_Event e;
    int game_over = 0;

    while (!game_over) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) game_over = 1;
            if (e.type == SDL_KEYDOWN && game_board->current_piece.active) {
                if (e.key.keysym.sym == SDLK_LEFT && canMove(game_board, game_board->current_piece.col - 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col--;
                if (e.key.keysym.sym == SDLK_RIGHT && canMove(game_board, game_board->current_piece.col + 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col++;
                if (e.key.keysym.sym == SDLK_DOWN && canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                if (e.key.keysym.sym == SDLK_UP) {
                    int next_rot = (game_board->current_piece.rotation + 1) % 4;
                    if (canMove(game_board, game_board->current_piece.col, game_board->current_piece.row, next_rot)) game_board->current_piece.rotation = next_rot;
                }
                if (e.key.keysym.sym == SDLK_SPACE) {
                     while(canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                     lockPiece(game_board);
                }
            }
        }
        if (!gameBoardUpdate(game_board)) game_over = 1;
        gameBoardDraw(game_board);
        SDL_Delay(16);
    }
    gameBoardDelete(game_board);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
