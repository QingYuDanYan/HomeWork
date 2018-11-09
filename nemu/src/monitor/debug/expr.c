#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NO, TK_HEX, TK_REG, TK_VAR, TK_LE, TK_GE, TK_NE, TK_AND_AND, TK_OR_OR, TK_AND, TK_OR

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {" +", TK_NOTYPE},    // spaces
  {"0x[a-f|A-F|0-9]+", TK_HEX},  // hex
  {"\\$[a-z|A-Z]+", TK_REG},  //reg
  {"[a-z|A-Z|_]+[a-z|A-Z|_|0-9]+", TK_VAR},
  {"\\(", '('},         // left bracket
  {")", ')'},           // right bracket
  {"\\+", '+'},         // plus
  {"\\*", '*'},         // multipy 
  {"\\-", '-'},         // sub
  {"/", '/'},           // divide
  {"[0-9]+", TK_NO},    // number
  {"==", TK_EQ},        // equal
  {"<=", TK_LE},        // less equal
  {">=", TK_GE},        // greater equal
  {"!=", TK_NE},        // not equal
  {"&&", TK_AND_AND},   // &&
  {"||", TK_OR_OR},     // ||
  {"&", '&'},        // &
  {"|", '|'},         // |
  {"<", '<'},           // <
  {">", '>'}            // >
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[65536];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;
          case 0 :
            break;
          default: 
            tokens[nr_token].type = rules[i].token_type;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            ++nr_token;
        }
        break; 
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


int check_bracket_balance(int p, int q) {
  int layer = 0;
  for (int i = p; i <= q; i++) {
    if (tokens[i].type == '(') layer ++;
    if (tokens[i].type == ')') layer --;
  }

  if (layer == 0) return 0;
  else return 1;
}


int checkparentheses(int p, int q) {
  if (check_bracket_balance(p, q) != 0){
    Assert(0, "bracket imbalance\n");
  }

  if (tokens[p].type != '(' || tokens[q].type != ')') {
    return 1;
  }
  else {
    int layer = 0;
    for (int i = p + 1; i <= q - 1; i++) {
      int type = tokens[i].type;
      if (type == '(') layer ++;
      if (type == ')') layer --;
      if (layer < 0) return 1;
    }
  }

  return 0;
}


int op_find(int p, int q) {
  int layer = 0, rightmost = -1;
  bool add_sub_exist = false;
  for (int i = p; i <= q; i++) {
    int type = tokens[i].type;
    if (type == '(') {
      ++layer;
    }
    if (type == ')'){
      --layer;
    }
    if (layer == 0)  {
      if ( (type == '*' || type == '/') && (add_sub_exist == false) ) {
        rightmost = i;        
      }
      if (type == '+' || type == '-'){
        if (i - 1 >= p){
          int pre_type = tokens[i-1].type;
          if (pre_type == '+' || pre_type == '-' || pre_type == '*' || pre_type == '/') {
            continue;
          }

        }
        rightmost = i;
        add_sub_exist = true;
      }
    }
  }
  return rightmost;  
}



int eval(int p, int q, bool *success) {
  if (p > q) {
    /* bad expression */
    Assert(0, "bad expression\n");
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    int type = tokens[p].type;
    if (type == TK_NO)
      return atoi(tokens[p].str);
    if (type == TK_HEX) {
      uint32_t ret = 0;
      sscanf(tokens[p].str+2, "%x", &ret);
      return ret;
    }
    if (type == TK_REG) {
      int i = 0;
      for (i = 0; i < 8; i++){
        if ((strcmp(tokens[p].str + 1, regsl[i])) == 0) {
          break;
        }
      }
      return reg_l(i);
    }
  }
  else if (checkparentheses(p, q) == 0) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses
     */
    return eval(p + 1, q - 1, success);
  }
  else {
    int op = op_find(p, q); /* the position of main op in the token expression */
    uint32_t val1, val2;
    if ( p == op ) {
      val2 = eval(op + 1, q, success); 
      switch(tokens[p].type) {
        case '*':
          return vaddr_read(val2, 4); 
        case '+':
          return val2;
        case '-':
          return -val2;
        default:
          Assert(0, "Unknown Typen");
      }
    }

    val1 = eval(p, op - 1, success);
    val2 = eval(op + 1, q, success);

    switch (tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': if (val2 == 0) *success = false; return val1 / val2;
      case '<': return val1 < val2;
      case '>': return val1 > val2;
      case '&': return val1 & val2;
      case '|': return val1 | val2;
      case TK_EQ: return val1 == val2;
      case TK_GE: return val1 >= val2;
      case TK_LE: return val1 <= val2;
      case TK_NE: return val1 != val2;
      case TK_AND_AND: return val1 && val2;
      case TK_OR_OR: return val1 || val2;
      default: Assert(0, "unknown token type\n");
    }
  }

  return 0;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  uint32_t res = eval(0, nr_token - 1, success);

  return res;
}
