
#include "9cc.h"

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}
// NodeKindがvalだけの時に使用する
Node *new_node_number(int val)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

// ---------------生成文法---------------

Node *code[100];

Node *assign()
{
  Node *node = equality();
  if (consume("="))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}

Node *expr()
{
  return assign();
}

Node *stmt()
{
  Node *node = expr();
  expect(";");
  return node;
}

void program()
{
  int i = 0;
  while (!at_eof())
  {
    code[i++] = stmt();
  }
  code[i] = NULL;
}

Node *equality()
{
  Node *node = relational();

  for (;;)
  {
    if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

Node *relational()
{
  Node *node = add();
  for (;;)
  {
    if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume("<="))
      node = new_node(ND_LE, node, add());
    else if (consume(">"))
      node = new_node(ND_LT, add(), node);
    else if (consume(">="))
      node = new_node(ND_LE, add(), node);
    else
      return node;
  }
}

Node *add()
{
  Node *node = mul();
  for (;;)
  {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *mul()
{
  Node *node = unary();

  for (;;)
  {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *primary()
{
  if (consume("("))
  {
    Node *node = expr();
    consume(")");
    return node;
  }
  Token *tok = consume_ident(); // Nodeを作る上でtokenの情報が必要なのでtokenが返り値
  if (tok)
  {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    node->offset = (tok->str[0] - 'a' + 1) * 8;
    return node;
  }
  return new_node_number(expect_number());
}

Node *unary()
{
  if (consume("+"))
  {
    return primary();
  }
  if (consume("-"))
  {
    return new_node(ND_SUB, new_node_number(0), primary());
  }
  return primary();
}

void error(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt); // 引数を一つにまとめる

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s\n", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

bool consume(char *op)
{
  if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

Token *consume_ident()
{
  if (token->kind != TK_IDENT)
  {
    return NULL;
  }
  Token *t = token;
  token = token->next;
  return t;
}

void expect(char *op)
{
  if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
    error_at(token->str, "expected %s", op);
  token = token->next;
}

int expect_number()
{
  if (token->kind != TK_NUM)
    error("数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof()
{
  return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

bool startswith(char *p, char *q)
{
  return memcmp(p, q, strlen(q)) == 0;
}

Token *tokenize()
{
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p)
  {
    if (isspace(*p))
    {
      p++;
      continue;
    }

    if ('a' <= *p && *p <= 'z')
    {
      cur = new_token(TK_IDENT, cur, p++, 1);
      continue;
    }

    if (startswith(p, "=") || startswith(p, ";")) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">="))
    {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (strchr("+-*/()<>", *p))
    {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (isdigit(*p))
    {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }
    error_at(p, "invalid token");
  }
  new_token(TK_EOF, cur, p, 0);
  return head.next;
}