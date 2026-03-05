#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKEN_LEN 256
#define MAX_LINE_LEN 1024

const char* keywords[] = {
    "number", "write", "newline", "repeat", "times", "and"
};

int is_keyword(const char* word) {
    for (int i = 0; i < 6; i++) {
        if (strcmp(word, keywords[i]) == 0) return 1;
    }
    return 0;
}

int is_identifier(const char* word) {
    if (!isalpha(word[0])) return 0;
    for (int i = 1; word[i]; i++) {
        if (!isalnum(word[i]) && word[i] != '_') return 0;
    }
    return strlen(word) <= 20;
}

int is_int_constant(const char* word) {
    int i = 0;
    if (word[0] == '-') i = 1;
    if (!isdigit(word[i])) return 0;
    for (; word[i]; i++) {
        if (!isdigit(word[i])) return 0;
    }
    return 1;
}

void remove_comments(char* line) {
    char* start = strstr(line, "*");
    while (start) {
        char* end = strstr(start + 1, "*");
        if (!end) {
            printf("Error: Unclosed comment.\n");
            exit(1);
        }
        memmove(start, end + 1, strlen(end + 1) + 1);
        start = strstr(line, "*");
    }
}

void tokenize_line(FILE* out, char* line) {
    int i = 0, start = 0;
    int len = strlen(line);

    while (i < len) {
        char c = line[i];

     
        if (c == '"') {
            int str_start = i;
            i++; 

            while (i < len && line[i] != '"') {
                i++;
            }

            if (i >= len) {
                printf("Error: Unclosed string constant.\n");
                exit(1);
            }

      
            int str_len = i - str_start + 1;
            char token[MAX_TOKEN_LEN];
            if (str_len >= MAX_TOKEN_LEN) {
                printf("Error: String constant too long.\n");
                exit(1);
            }
            strncpy(token, line + str_start, str_len);
            token[str_len] = '\0';

            fprintf(out, "StringConstant(%s)\n", token);

            i++;         
            start = i;   

            continue;      
        }

        int is_token_end = 0;
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            is_token_end = 1;
        } else if (c == ';' || c == '{' || c == '}' || c == ':' || c == '+' || c == '-') {
            is_token_end = 2;
        }
        
        

        if (is_token_end) {
   
            if (start < i) {
                char token[MAX_TOKEN_LEN];
                int token_len = i - start;
                if (token_len >= MAX_TOKEN_LEN) {
                    printf("Error: Token too long.\n");
                    exit(1);
                }
                strncpy(token, line + start, token_len);
                token[token_len] = '\0';

                if (is_keyword(token)) {
                    fprintf(out, "Keyword(%s)\n", token);
                } else if (is_int_constant(token)) {
                    fprintf(out, "IntConstant(%s)\n", token);
                } else if (is_identifier(token)) {
                    fprintf(out, "Identifier(%s)\n", token);
                } else {
                    printf("Error: Unrecognized token '%s'.\n", token);
                    exit(1);
                }
            }

        
            if (is_token_end == 2) {
                if (c == ':') {
                    if (line[i + 1] == '=') {
                        fprintf(out, "Operator(:=)\n");
                        i++; 
                    } else {
                        printf("Error: Unrecognized token ':' without '='.\n");
                        exit(1);
                    }
                } else if (c == '+') {
                    if (line[i + 1] == '=') {
                        fprintf(out, "Operator(+=)\n");
                        i++;
                    } else {
                        printf("Error: Unrecognized token '+' without '='.\n");
                        exit(1);
                    }
                } else if (c == '-') {
                    if (line[i + 1] == '=') {
                        fprintf(out, "Operator(-=)\n");
                        i++;
                    } else {
                        printf("Error: Unrecognized token '-'.\n");
                        exit(1);
                    }
                } else if (c == ';') {
                    fprintf(out, "EndOfLine\n");
                } else if (c == '{') {
                    fprintf(out, "OpenBlock\n");
                } else if (c == '}') {
                    fprintf(out, "CloseBlock\n");
                }
             
            }

            start = i + 1;
        }

        i++;
    }

    if (start < len) {
        char token[MAX_TOKEN_LEN];
        int token_len = len - start;
        if (token_len >= MAX_TOKEN_LEN) {
            printf("Error: Token too long.\n");
            exit(1);
        }


        while (token_len > 0 && (line[start + token_len -1] == '\n' || line[start + token_len -1] == '\r')) {
            token_len--;
        }
        strncpy(token, line + start, token_len);
        token[token_len] = '\0';

        if (strlen(token) > 0) {
            if (is_keyword(token)) {
                fprintf(out, "Keyword(%s)\n", token);
            } else if (is_int_constant(token)) {
                fprintf(out, "IntConstant(%s)\n", token);
            } else if (is_identifier(token)) {
                fprintf(out, "Identifier(%s)\n", token);
            } else {
                printf("Error: Unrecognized token '%s'.\n", token);
                exit(1);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: la <scriptname>\n");
        return 1;
    }

    char filename[256];
    snprintf(filename, sizeof(filename), "%s.plus", argv[1]);

    FILE* in = fopen(filename, "r");
    if (!in) {
        printf("Error: File '%s' not found.\n", filename);
        return 1;
    }

    char outname[256];
    snprintf(outname, sizeof(outname), "%s.lx", argv[1]);
    FILE* out = fopen(outname, "w");
    if (!out) {
        printf("Error: Could not create output file '%s'.\n", outname);
        fclose(in);
        return 1;
    }

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), in)) {
        remove_comments(line);
        tokenize_line(out, line);
    }

    fclose(in);
    fclose(out);

    printf("Lexical analysis complete. Output in '%s'.\n", outname);
    return 0;
}
