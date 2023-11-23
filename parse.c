
#include "9cc.h"

LVar *locals;

char *strndup(const char *str, size_t n)
{
  char *new_str = (char *)malloc(n + 1);
  if (new_str)
  {
    strncpy(new_str, str, n);
    new_str[n] = '\0';
  }
  return new_str;
}

Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs)
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

LVar *new_lvar(char *name)
{
  LVar *var = calloc(1, sizeof(LVar));
  var->name = name;
  var->next = locals;
  locals = var;
  return var;
}

Node *new_var_node(LVar *var)
{
  Node *node = new_node(ND_LVAR);
  node->var = var;
  return node;
}

// ---------------生成文法---------------

Node *code[100];

Node *assign()
{
  Node *node = equality();
  if (consume("="))
    node = new_binary(ND_ASSIGN, node, assign());
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
      node = new_binary(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_binary(ND_NE, node, relational());
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
      node = new_binary(ND_LT, node, add());
    else if (consume("<="))
      node = new_binary(ND_LE, node, add());
    else if (consume(">"))
      node = new_binary(ND_LT, add(), node);
    else if (consume(">="))
      node = new_binary(ND_LE, add(), node);
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
      node = new_binary(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_binary(ND_SUB, node, mul());
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
      node = new_binary(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_binary(ND_DIV, node, unary());
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
    Node *node = new_node(ND_LVAR); 

    if (tok->kind == TK_IDENT)
    {
      LVar *lvar = find_lvar(tok);
      if (!lvar)
        lvar = new_lvar(strndup(tok->str, tok->len));
      return new_var_node(lvar);
    }
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
    return new_binary(ND_SUB, new_node_number(0), primary());
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

LVar *find_lvar(Token *tok)
{
  for (LVar *var = locals; var; var = var->next)
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
}
