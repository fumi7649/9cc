#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

typedef struct Node Node;
typedef struct Token Token;

// NodeKind
typedef enum
{
  ND_ADD,    //+
  ND_SUB,    // -
  ND_MUL,    // *
  ND_DIV,    // /
  ND_EQ,     // ==
  ND_NE,     // !=
  ND_LT,     // <
  ND_LE,     // <=
  ND_ASSIGN, //=
  ND_LVAR,   // ローカル変数
  ND_NUM,    // Integer
} NodeKind;

struct Node
{
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;
  int offset;
};

// Token
typedef enum
{
  TK_RESERVED, // 記号
  TK_IDENT,    // 識別子
  TK_NUM,      // 数字
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

struct Token
{
  TokenKind kind;
  Token *next;
  int val;
  char *str;
  int len;
};

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *primary();
Node *unary();
bool consume(char *op);
Token *consume_ident(void);
void expect(char *op);
int expect_number();

Node *assign();
Node *stmt();
void program();

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_number(int val);

void gen(Node *node);

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool at_eof();

Token *tokenize();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);

extern Token *token;
extern char *user_input;
extern Node *code[100];