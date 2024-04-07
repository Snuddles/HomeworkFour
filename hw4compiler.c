// Authors : Ingrid Lucas and Raulph Peltrop
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/* Error Handling
    In Token List
        -1 =  Number too long.
        -2 = Name too long.
        -3 = Invalid symbols.
*/

/*


*/

// global array thats stores the values of digits greater than 5 and identifiers greater than 11

int state_one(char c, char identifier_or_digit[], int i);
int state_identifier(char c, char identifier_or_digit[], int i);
int state_Digits(char identifier_or_digit[]);
// linear search through symbol table looking at name
int symbol_table_check(char identifier[12]);
void insert(char identifier[12], int kind, int val, int addr, int mark);
int program();
void block();
void const_declaration();
int var_declaration();
void proc_declaration();
void statements();
void conditions();
void factor();
void search_symbol_table();
void expression();
void emit(char operand[6], int L, int M, int O);
int convert_string_to_int(char M[]);
void findBase(int table_index);
typedef struct
{
    int kind;      // const = 1, var = 2, proc = 3
    char name[12]; // name up to 11 chars plus the null character
    int val;       // number (ASCII value)
    int level;     // L level
    int addr;      // M address
    int mark;      // to indicate unavailable or deleted
} symbol;

#define MAX_SYMBOL_TABLE_SIZE 500
// GLOBAL VARIABLES
FILE *file;
int table_index;
symbol *symbol_table;
int token_list_index;
char *tokenList;
int lexographical_level;
int ccx; // current code index
int procedure_address = 3;
int INC_CCX;

typedef struct
{
    char operand[6];
    int M;
    int L;
    int O; // operand
} operand;
operand opr_array[500];


typedef enum
{
    oddsym = 1, // this was previously skipsym
    identsym,
    numbersym,
    plussym,
    minussym,
    multsym,
    slashsym,
    fisym,
    eqsym,
    neqsym,
    lessym,
    leqsym,
    gtrsym,
    geqsym,
    lparentsym,
    rparentsym,
    commasym,
    semicolonsym,
    periodsym,
    becomessym,
    beginsym,
    endsym,
    ifsym,
    thensym,
    whilesym,
    dosym,
    callsym,
    constsym,
    varsym,
    procsym,
    writesym,
    readsym,
    elsesym
} token_type;

int main(int argc, char *argv[])
{
    // if (argc != 2)
    // {
    //     printf("Must include input.txt file or input{1..15}.txt files\n");
    //     exit(1);
    // }
    int token_size = 10;
    lexographical_level = 0;
    ccx = 0;
    tokenList = calloc(token_size + 1, sizeof(char));

     file = fopen(argv[1], "r");
    //file = fopen("testcase5.txt", "r"); // for testing
    // print the file
    char inputCharacter;
    while ((inputCharacter = fgetc(file)) != EOF) {
        printf("%c", inputCharacter);
    }
    printf("\n");
    rewind(file);
    // Initializing symbol table
    table_index = 1;
    symbol_table = malloc(sizeof(symbol) * MAX_SYMBOL_TABLE_SIZE);
    if (file != NULL)
    {
        char c;
        // Comments handling
        while ((c = fgetc(file)) != EOF)
        {
            char tempFutureCharacter = fgetc(file);
            if (c == '/' && tempFutureCharacter == '*')
            {
                while (true)
                {
                    // to avoid possible skip over set c = to futureChar and update future char then do loop comparison
                    c = tempFutureCharacter;
                    tempFutureCharacter = fgetc(file);
                    if (c == '*' && tempFutureCharacter == '/')
                    {
                        c = fgetc(file);
                        break;
                    }
                }
            }
            else
            {
                // reset pointer back to c
                if (tempFutureCharacter != EOF)
                {
                    fseek(file, -1, SEEK_CUR);
                }
            }
            if (c == EOF)
                break;
            if (isspace(c))
            {
                continue; // scanner reads space and skips
            }
            char identifier_or_digit[12] = ""; // empty array to store string
            int n = state_one(c, identifier_or_digit, 0);
            if(n==fisym){
                continue;
            }
            char token[5];
            sprintf(token, "%d", n); // converts the digit into a string
            int size_one = strlen(tokenList);
            int size_two = strlen(token) + 1;
            int size_three = strlen(identifier_or_digit) + 1;
            // TokenList array doubles in size
            while ((size_one + size_two + size_three) > token_size)
            {
                token_size *= 2;
                tokenList = realloc(tokenList, sizeof(char) * token_size + 1);
                int length = strlen(tokenList);
                tokenList[length] = '\0';
            }
            strcat(tokenList, token);
            strcat(tokenList, " ");
            // When identifier_or_digit is an identifer or digit, it adds the identifier_or_digit array
            if (n == identsym || n == numbersym)
            {
                strcat(tokenList, identifier_or_digit);
                strcat(tokenList, " ");
            }
        }
    }
    else
    {
        printf("Error: opening file.\n");
        exit(1);
    }
    int HALT = 1;
    token_list_index = 0;
    while (HALT)
    {
        HALT = program();

        break;
    }
    printf("No errors, program is syntactically correct.\n\n");
    printf("Assembly Code:\n\n");
    printf("Line OP L M\n");

    // Create elf.txt file
    FILE *elf_ptr;
    char filename[] = "elf.txt";
    elf_ptr = fopen(filename, "w");
    if (elf_ptr == NULL){
        printf("Error opening elf.txt file\n");
        exit(1);
    }

    for (int i = 0; i < ccx; i++)
    {
        printf("%d %s %d %d\n", i, opr_array[i].operand, opr_array[i].L, opr_array[i].M);
         fprintf(elf_ptr, "%d %d %d\n",opr_array[i].O, opr_array[i].L, opr_array[i].M);

    }
     fclose(elf_ptr); 
    fclose(file);
    free(tokenList);
    free(symbol_table);
    printf("\n");
}
int state_one(char c, char identifier_or_digit[], int i)
{
    char futureCharacter = fgetc(file);
    // if the character I'm on is an alphabetic letter
    if (isalpha(c))
    {
        identifier_or_digit[i] = c;
        if (isalnum(futureCharacter))
        {
            return state_identifier(futureCharacter, identifier_or_digit, i + 1);
        }
        if (futureCharacter != EOF)
        {
            fseek(file, -1, SEEK_CUR);
        }
        return identsym; // returns a single character
    }
    // else if the character I am on is a number
    else if (isdigit(c))
    {
        if (futureCharacter != EOF)
        {
            fseek(file, -1, SEEK_CUR);
        }
        identifier_or_digit[i] = c;
        return state_Digits(identifier_or_digit);
    }

    // if the character I am on is one of the special symbols
    switch (c)
    {
    case '<':
        if (futureCharacter == '=')
        {
            return leqsym;
        }
        if (futureCharacter == '>')
        {
            return neqsym;
        }
        if (futureCharacter != EOF)
        {
            fseek(file, -1, SEEK_CUR);
        }
        return lessym;
        break;
    case '>':
        if (futureCharacter == '=')
        {
            return geqsym;
        }
        if (futureCharacter != EOF)
        {
            fseek(file, -1, SEEK_CUR);
        }
        return gtrsym;
        break;
    case ':':
        if (futureCharacter == '=')
        {
            return becomessym;
        }
        if (futureCharacter != EOF)
        {
            fseek(file, -1, SEEK_CUR);
        }
        return -3;
    }

    if (futureCharacter != EOF)
    {
        fseek(file, -1, SEEK_CUR);
    }

    switch (c)
    {
    case '+':
        return plussym;
    case '-':
        return minussym;
    case '*':
        return multsym;
    case '/':
        return slashsym;
    case ',':
        return commasym;
    case '=':
        return eqsym;
    case '(':
        return lparentsym;
    case ')':
        return rparentsym;
    case '.':
        return periodsym;
    case ';':
        return semicolonsym;
    default:
        printf("Symbol doesn't exist.");
        exit(1);
    }
    if (futureCharacter == EOF)
    {
        return 0; // 0 means it's an EOF
    }
    return 0;
}

int state_identifier(char c, char identifier_or_digit[], int i)
{
    char futureCharacter = fgetc(file);

    identifier_or_digit[i] = c;

    if (i == 10 && isalnum(futureCharacter))
    { // the identifier_or_digit is already full, and the next character is an alphabet/digit

        printf("Error: Identifier too long.");
        exit(1);
    }
    else if (!isalnum(futureCharacter))
    {

        identifier_or_digit[i + 1] = '\0'; // adds null terminator to the string
        if (futureCharacter != EOF)
        {
            fseek(file, -1, SEEK_CUR); // moves the filepointer back one space
        }
        // Reserved Words
        if (strcmp(identifier_or_digit, "var") == 0)
        {
            return varsym;
        }
        else if (strcmp(identifier_or_digit, "const") == 0)
        {
            return constsym;
        }
        else if (strcmp(identifier_or_digit, "begin") == 0)
        {
            return beginsym;
        }
        else if (strcmp(identifier_or_digit, "end") == 0)
        {
            return endsym;
        }
        else if (strcmp(identifier_or_digit, "if") == 0)
        {
            return ifsym;
        }
        else if (strcmp(identifier_or_digit, "then") == 0)
        {
            return thensym;
        }
        else if (strcmp(identifier_or_digit, "fi") == 0)
        {
            return fisym;
        }
        else if (strcmp(identifier_or_digit, "while") == 0)
        {
            return whilesym;
        }
        else if (strcmp(identifier_or_digit, "do") == 0)
        {
            return dosym;
        }
        else if (strcmp(identifier_or_digit, "read") == 0)
        {
            return readsym;
        }
        else if (strcmp(identifier_or_digit, "write") == 0)
        {
            return writesym;
        }
        else if (strcmp(identifier_or_digit, "procedure") == 0)
        {
            return procsym;
        }
        else if (strcmp(identifier_or_digit, "call") == 0)
        {
            return callsym;
        }

        return identsym; // string isn't a reserved word. It's an identifier
    }
    // recursive calls
    return state_identifier(futureCharacter, identifier_or_digit, i + 1);
}

int state_Digits(char identifier_or_digit[])
{
    // start from 1 because we already placed 0 before call so 4 spots available in array
    int i = 1;
    char futureCharacter = fgetc(file);

    // loop while future character is a digit and within the length constraint
    while (isdigit(futureCharacter))
    {
        identifier_or_digit[i] = futureCharacter;
        i++;
        if (isdigit(futureCharacter = fgetc(file)) && i >= 5)
        {
            printf("Number Too Long\n");
            exit(1);
        }
    }
    // case if future character is not a digit
    if (!isdigit(futureCharacter))
    {
        // null-terminate the string
        identifier_or_digit[i] = '\0';
        // moves file pointer back for future stuff i.e 1234a moves the file pointer from a to 4 so a can be used later
        if (futureCharacter != EOF)
        {
            fseek(file, -1, SEEK_CUR);
        }
        // returns the token type
        return numbersym;
    }
    return numbersym;
}

int program()
{
    char operand[6];

    // block
    block();

    // periodsym
    if (tokenList[token_list_index] != '1' || tokenList[token_list_index + 1] != '9') // periodsym
    {
        printf("Error: program must end with period\n");
        exit(1);
    }
    else
    {

        strcpy(operand, "EOP");
        emit(operand, 0, 3, 9);
    }
    return 1;
}

void block()
{
    char operand[6];
    int JMP_INDEX = ccx;
    strcpy(operand, "JMP");
    emit(operand, 0, ccx * 3, 7);

    const_declaration();
    int numVars = var_declaration();
    proc_declaration();
    opr_array[JMP_INDEX].M = ccx * 3;

    strcpy(operand, "INC");
    emit(operand, 0, 3 + numVars, 6);
    statements();
}

void const_declaration()
{

    char name[12];
    char digit[6];
    int digit_index = 0;
    int name_index = 0;

    if (tokenList[token_list_index] == '2' && tokenList[token_list_index + 1] == '8') // const =28
    {
        do
        {
            token_list_index += 3;                                                            // skips token and space value
            if (tokenList[token_list_index] != '2' || tokenList[token_list_index + 1] != ' ') // err
            {
                printf("Error: const, var, procedure, call and read keywords must be followed by identifier\n");
                exit(1);
            }
            token_list_index += 2; // is on the index for the indentifier name
            while (isalnum(tokenList[token_list_index]))
            {
                name[name_index++] = tokenList[token_list_index];
                token_list_index++;
            }
            name[name_index] = '\0';

            name_index = 0; // restarts index
            if (symbol_table_check(name) != -1)
            {
                printf("Error: Symbol name has already been declared");
                exit(1);
            }
            token_list_index++;
            if (tokenList[token_list_index] != '9' || tokenList[token_list_index + 1] != ' ')
            { // eqlsym = 9
                printf("Error: constants must be assigned with =\n");
                exit(1);
            }
            token_list_index += 2;

            if (tokenList[token_list_index] != '3' || tokenList[token_list_index + 1] != ' ')
            {
                printf("Error: constants must be assigned an integer value\n");
                exit(1);
            }
            token_list_index += 2;
            while (isdigit(tokenList[token_list_index]))
            {
                digit[digit_index++] = tokenList[token_list_index];
                token_list_index++;
            }
            digit[digit_index] = '\0';
            digit_index = 0;
            insert(name, 1, atoi(digit), 0, 1);
            token_list_index++;                                                                 // moves to the next token
        } while (tokenList[token_list_index] == '1' && tokenList[token_list_index + 1] == '7'); // commasym = 17
        if (tokenList[token_list_index] != '1' || tokenList[token_list_index + 1] != '8')       // semicolonsym = 18
        {
            printf("Error: constant, variable, and procedure declarations must be followed by a semicolon\n");
            exit(1);
        }
        token_list_index += 3;
    }
}

int var_declaration()
{

    char name[12];
    int name_index = 0;
    int numVars = 0;
    if (tokenList[token_list_index] == '2' && tokenList[token_list_index + 1] == '9') // token == varysym
    {
        do
        {

            token_list_index += 3; // gets the next token

            if (tokenList[token_list_index] != '2' || tokenList[token_list_index + 1] != ' ') // token != identsym
            {
                printf("Error: const, var, procedure, call and read keywords must be followed by identifier\n");
                exit(1);
            }
            token_list_index += 2; // gets the next token

            while (isalnum(tokenList[token_list_index])) // gets identifier name
            {
                name[name_index++] = tokenList[token_list_index];
                token_list_index++;
            }
            name[name_index] = '\0';
            name_index = 0;

            if (symbol_table_check(name) != -1)
            {
                printf("Error: Symbol name has already been declared\n");
                exit(1);
            }
            insert(name, 2, 0, (3 + numVars), 0); // changed mark to 0
            numVars++;
            token_list_index++;

        } while (tokenList[token_list_index] == '1' && tokenList[token_list_index + 1] == '7'); // commasym = 17

        if (tokenList[token_list_index] != '1' || tokenList[token_list_index + 1] != '8') // semicolonsym = 18
        {
            printf("constant and variable declarations must be followed by a semicolon");
            exit(1);
        }

        token_list_index += 3;
    }
    return numVars;
}

void insert(char identifier[12], int kind, int val, int addr, int mark)
{

    // inserts the new variable to the symbol table
    strcpy(symbol_table[table_index].name, identifier);
    symbol_table[table_index].kind = kind;
    symbol_table[table_index].level = lexographical_level;
    symbol_table[table_index].val = val;
    symbol_table[table_index].addr = addr;
    symbol_table[table_index].mark = mark;
    table_index++;
}
void statements()
{
    char name[12];
    int name_index = 0;
    char *operand = malloc(sizeof(char) * 6);

    if (tokenList[token_list_index] == '2' && tokenList[token_list_index + 1] == ' ') // identsym
    {
        token_list_index += 2; // gets the next token

        while (isalnum(tokenList[token_list_index])) // gets identifier name
        {
            name[name_index++] = tokenList[token_list_index];
            token_list_index++;
        }
        name[name_index] = '\0';
        name_index = 0;
        int symindex = symbol_table_check(name);
        if (symindex == -1)
        {
            printf("Error: undeclared identifier %s\n", name);
            exit(1);
        }
        if (symbol_table[symindex].kind != 2) // not a var, it's a constant
        {

            printf("Error: only variable values may be altered\n");
            exit(1);
        }
        token_list_index++; // get next token

        if (tokenList[token_list_index] != '2' || tokenList[token_list_index + 1] != '0') // becomesym
        {
            printf("Error: assignment statements must use :=\n");
            exit(1);
        }
        token_list_index += 3;
        expression();
        strcpy(operand, "STO");

        emit(operand, lexographical_level - symbol_table[symindex].level, symbol_table[symindex].addr, 4);
        return;
    }
    if (tokenList[token_list_index] == '2' && tokenList[token_list_index + 1] == '7')
    { // callsym
        token_list_index += 3;

        if (tokenList[token_list_index] != '2' || tokenList[token_list_index + 1] != ' ') // token != identsym
        {
            printf("Error: const, var, procedure, call and read keywords must be followed by identifier\n");
            exit(1);
        }
        token_list_index += 2; // gets the next token

        if (!(isalnum(tokenList[token_list_index])))
        { // check if its alnum
            printf("Error: identifer name for call is non-alphanumeric\n");
            exit(1);
        }

        while (isalnum(tokenList[token_list_index])) // gets identifier name
        {
            name[name_index++] = tokenList[token_list_index];
            token_list_index++;
        }
        name[name_index] = '\0';
        name_index = 0;

        int index = symbol_table_check(name);
        if (index == -1)
        {
            printf("Procedure doesn't exist\n");
            exit(1);
        }

        strcpy(operand, "CAL");

        emit(operand, lexographical_level, 3, 5);
        token_list_index += 1;
    }

    if (tokenList[token_list_index] == '2' && tokenList[token_list_index + 1] == '1') // beginsym
    {
        do
        {
            token_list_index += 3;
            statements();
        } while (tokenList[token_list_index] == '1' && tokenList[token_list_index + 1] == '8'); // semicolonsym
        if (tokenList[token_list_index] != '2' || tokenList[token_list_index + 1] != '2')       // endsym
        {
            printf("Error: begin must be followed by end\n");
            exit(1);
        }

        lexographical_level--;
        token_list_index += 3;

        if (tokenList[token_list_index] != '1' || tokenList[token_list_index + 1] != '9') // not periodsym
        {
            strcpy(operand, "RTN");
            emit(operand, 0, 0, 2);
        }

        return;
    }
    if (tokenList[token_list_index] == '2' && tokenList[token_list_index + 1] == '3') // ifsym
    {
        token_list_index += 3;
        conditions();
        int jpcIdx = ccx;
        strcpy(operand, "JPC");

        emit(operand, 0, 0, 8);                                                              // emit JPC
        if (tokenList[token_list_index] != '2' || tokenList[token_list_index + 1] != '4') // thensym
        {
            printf("Error: then expected.\n");
            exit(1);
        }
        token_list_index += 3;

        statements();

        opr_array[jpcIdx].M = ccx*3;
        return;
    }
    if (tokenList[token_list_index] == '2' && tokenList[token_list_index + 1] == '5') // whilesym
    {
        token_list_index += 3;
        int loopIdx = ccx;
        conditions();
        if (tokenList[token_list_index] != '2' || tokenList[token_list_index + 1] != '6') // dosym
        {
            printf("Error: while must be followed by do\n");
            exit(1);
        }
        token_list_index += 3;
        int jpcIdx = ccx;
        strcpy(operand, "JPC");

        emit(operand, 0, 0, 8); // emit JPC
        statements();
        strcpy(operand, "JMP");

        opr_array[jpcIdx].M = loopIdx;
        // code[jpcIdx].M = current code index

        free(operand);
        return;
    }
    if (tokenList[token_list_index] == '3' && tokenList[token_list_index + 1] == '2') // readysym
    {
        token_list_index += 3;
        if (tokenList[token_list_index] != '2' || tokenList[token_list_index + 1] != ' ') // identsym
        {
            printf("Error: read keyword must be followed by identifier\n");
            exit(1);
        }
        token_list_index += 2;
        while (isalnum(tokenList[token_list_index])) // gets identifier name
        {
            name[name_index++] = tokenList[token_list_index];
            token_list_index++;
        }
        name[name_index] = '\0';
        name_index = 0;
        int symIdx = symbol_table_check(name);
        if (symIdx == -1)
        {
            printf("Error: undefined identifier\n");
            exit(1);
        }
        if (symbol_table[symIdx].kind != 2) // not a var
        {
            printf("Error: read keyword is not a var\n");
            exit(1);
        }
        token_list_index++;
        strcpy(operand, "READ");

        emit(operand, 0, 0, 9);
        strcpy(operand, "STO");

        emit(operand, symbol_table[symIdx].level, symbol_table[symIdx].addr, 9);
        return;
    }
    if (tokenList[token_list_index] == '3' && tokenList[token_list_index + 1] == '1') // writesym
    {
        token_list_index += 3;
        expression();
        strcpy(operand, "WRITE");

        emit(operand, 0, 1, 9);
        return;
    }
}
void conditions()
{
    char *operand = malloc(sizeof(char) * 7);

    if (tokenList[token_list_index] == '1' && tokenList[token_list_index + 1] == ' ') // oddsym
    {
        token_list_index += 2;
        expression();
        strcpy(operand, "ODD");

        emit(operand, 0, 11, 2);
    }
    else
    {
        expression();
        if (tokenList[token_list_index] == '9' && tokenList[token_list_index + 1] == ' ') // eqsym in the pdf it says eqlsym
        {
            token_list_index += 2;
            expression();
            strcpy(operand, "EQL");

            emit(operand, 0, 5, 2);
        }
        else if (tokenList[token_list_index] == '1' && tokenList[token_list_index + 1] == '0') // neqsym
        {
            token_list_index += 3;
            expression();
            strcpy(operand, "NEQ");

            emit(operand, 0, 6, 2);
        }
        else if (tokenList[token_list_index] == '1' && tokenList[token_list_index + 1] == '1') // lessym
        {
            token_list_index += 3;
            expression();
            strcpy(operand, "LSS");
            emit(operand, 0, 7, 2);
        }

        else if (tokenList[token_list_index] == '1' && tokenList[token_list_index + 1] == '2') // leqsym
        {
            token_list_index += 3;
            expression();
            strcpy(operand, "LEQ");

            emit(operand, 0, 8, 2);
        }

        else if (tokenList[token_list_index] == '1' && tokenList[token_list_index + 1] == '3') // gtrsym
        {
            token_list_index += 3;
            expression();
            strcpy(operand, "GTR");

            emit(operand, 0, 9, 2);
        }

        else if (tokenList[token_list_index] == '1' && tokenList[token_list_index + 1] == '4') // geqsym
        {
            token_list_index += 3;
            expression();
            strcpy(operand, "GEQ");

            emit(operand, 0, 10, 2);
        }
        else
        {
            printf("Error: condition must contain comparison operator\n");
            exit(1);
        }
    }
    free(operand);
}
void factor()
{
    char name[12];
    int name_index = 0;
    char operand[6];
    int symbol_index;

    if (tokenList[token_list_index] == '2' && tokenList[token_list_index + 1] == ' ')
    {
        token_list_index += 2;                       // next token
        while (isalnum(tokenList[token_list_index])) // stores identifier name
        {
            name[name_index++] = tokenList[token_list_index];
            token_list_index++;
        }
        name[name_index] = '\0';
        name_index = 0;
        symbol_index = symbol_table_check(name); // looks for identifier name
        if (symbol_index == -1)
        {
            printf("Error: undeclared identifier %s\n", name);
            exit(1);
        }
        if (symbol_table[symbol_index].kind == 1) // const
        {
            strcpy(operand, "LIT");

            emit(operand, 0, symbol_table[symbol_index].val, 1);
        }
        else if (symbol_table[symbol_index].kind == 2) // var
        {
            strcpy(operand, "LOD");
            emit(operand, (lexographical_level - symbol_table[symbol_index].level), symbol_table[symbol_index].addr, 3);
        }
        else
        {
            printf("Error: cant do assignments on procedure");
            exit(1);
        }
        token_list_index++; // next token
    }
    else if (tokenList[token_list_index] == '3' && tokenList[token_list_index + 1] == ' ')
    {
        token_list_index += 2;
        char M[6];
        int index = 0;
        while (isdigit(tokenList[token_list_index]))
        {
            M[index++] = tokenList[token_list_index];
            token_list_index++;
        }
        M[index] = '\0';
        strcpy(operand, "LIT");
        int int_M = convert_string_to_int(M);

        emit(operand, 0, int_M, 1);
        token_list_index++; // next token
    }
    else if (tokenList[token_list_index] == '1' && tokenList[token_list_index + 1] == '5') // lparentsym
    {
        token_list_index += 3; // next token
        expression();
        if (tokenList[token_list_index] != '1' || tokenList[token_list_index + 1] != '6') // not right parenthese
        {
            printf("Error: right parenthesis must follow left parenthesis\n");
            exit(1);
        }
        token_list_index += 3; // next token
    }
    else
    {
        printf("Error: arithmetic equations must contain operands, parentheses, numbers, or symbols\n");
        exit(1);
    }
}

void term()
{
    char operand[6];
    char M[6];
    strcpy(M, "0");
    factor();
    while ((tokenList[token_list_index] == '6' && tokenList[token_list_index + 1] == ' ') || // multsym
           (tokenList[token_list_index] == '7' && tokenList[token_list_index + 1] == ' '))   // slashsym
    {
        if (tokenList[token_list_index] == '6' && tokenList[token_list_index + 1] == ' ')
        {
            token_list_index += 2; // get next token
            factor();
            strcpy(operand, "MUL");
            emit(operand, 0, 3, 2);
        }
        else if (tokenList[token_list_index] == '7' && tokenList[token_list_index + 1] == ' ')
        {
            token_list_index += 2; // get next token
            factor();
            strcpy(operand, "DIV");
            emit(operand, 0, 4, 2);
        }
    }
}

void expression()
{
    char operand[6];
    term();
    char M[7];
    strcpy(M, "0");
    if (tokenList[token_list_index] == '5' && tokenList[token_list_index + 1] == ' ') // minusym =5
    {
        token_list_index += 2; // get next token
        term();
        strcpy(operand, "SUB");
        emit(operand, 0, 2, 2);
        while ((tokenList[token_list_index] == '4' && tokenList[token_list_index + 1] == ' ') || // plussym =4
               (tokenList[token_list_index] == '5' && tokenList[token_list_index + 1] == ' '))   // minussym
        {
            if ((tokenList[token_list_index] == '4' && tokenList[token_list_index + 1] == ' '))
            {
                token_list_index += 2; // get next token
                term();
                strcpy(operand, "ADD");
                emit(operand, 0, 1, 2);
            }
            else
            {
                token_list_index += 2; // get next token
                term();
                strcpy(operand, "SUB");
                emit(operand, 0, 2, 2);
            }
        }
    }
    else
    {

        if (tokenList[token_list_index] == '4' && tokenList[token_list_index + 1] == ' ') // plussym = 4
        {
            token_list_index += 2; // get next token
            term();
            strcpy(operand, "ADD");
            emit(operand, 0, 1, 2);
        }
        while ((tokenList[token_list_index] == '4' && tokenList[token_list_index + 1] == ' ') || // plussym =4
               (tokenList[token_list_index] == '5' && tokenList[token_list_index + 1] == ' '))   // minussym
        {
            if ((tokenList[token_list_index] == '4' && tokenList[token_list_index + 1] == ' '))
            {
                token_list_index += 2; // get next token
                term();
                strcpy(operand, "ADD");
                emit(operand, 0, 1, 2);
            }
            else
            {
                token_list_index += 2; // get next token
                term();
                strcpy(operand, "SUB");
                emit(operand, 0, 2, 2);
            }
        }
    }
}


int symbol_table_check(char identifier[12])
{

    for (int i = table_index; i > 0; i--)
    {
        if (strcmp(symbol_table[i].name, identifier) == 0)
        {
            return i;
        }
    }
    return -1;
}

void emit(char operand[6], int L, int M, int O)
{

    strcpy(opr_array[ccx].operand, operand);
    opr_array[ccx].L = L;
    opr_array[ccx].M = M;
    opr_array[ccx].O = O;
    
   
    if (strcmp(operand, "WRITE") == 0)
    {
        strcpy(opr_array[ccx].operand, "SYS");
        // strcpy(opr_array[ccx].M, "1");
    }
    else if (strcmp(operand, "READ") == 0)
    {
        strcpy(opr_array[ccx].operand, "SYS");
        // strcpy(opr_array[ccx].M, "2");
    }
    // else
    // {

    //     // strcpy(opr_array[ccx].M, M);
    // }
    ccx++;
}

void proc_declaration()
{

    char name[12];
    int name_index = 0;

    while (tokenList[token_list_index] == '3' && tokenList[token_list_index + 1] == '0')
    {
        token_list_index += 3;
        if (tokenList[token_list_index] != '2' || tokenList[token_list_index + 1] != ' ')
        { // check identsym
            printf("const, var, procedure must be followed by identifier");
            exit(1);
        }

        tokenList += 2;

        while (isalnum(tokenList[token_list_index])) // stores identifier name
        {
            name[name_index++] = tokenList[token_list_index];
            token_list_index++;
        }
        name[name_index] = '\0';
        name_index = 0;
        token_list_index++;
        if (tokenList[token_list_index] != '1' || tokenList[token_list_index + 1] != '8') // semicolonsym = 18
        {
            printf("Error: constant, procedure and variable declarations must be followed by a semicolon\n");
            exit(1);
        }
        lexographical_level++;
        insert(name, 3, 0, procedure_address, 0);
        procedure_address += 3;
        token_list_index += 3;

        block();                                                                          //
        if (tokenList[token_list_index] != '1' || tokenList[token_list_index + 1] != '8') // semicolonsym = 18
        {
            printf("Error: constant, procedure and variable declarations must be followed by a semicolon\n");
            exit(1);
        }
        token_list_index += 3;
    }
}

// void search_symbol_table(char name[12]){

//     int index = symbol_table_check(name);

//     for(int i =0; i<index;i++){
//         if(symbol_table_check[index]==)
//     }

//     return -1;

// }

int convert_string_to_int(char M[])
{
    int num = 0;
    for (int i = 0; M[i] != '\0'; i++)
    {
        num = num * 10 + (M[i] - 48);
    }
    return num;
}
