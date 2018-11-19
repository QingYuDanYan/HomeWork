#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_INEQ, TK_OP_AND, TK_NUM, TK_REG, TK_PL, TK_PR, TK_DEREF, TK_NEG, 
	TK_MUL, TK_DIV, TK_ADD, TK_SUB
  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {"0x[0-9a-f]+",	TK_NUM},		// a number in hex
	{"[0-9a-f]+"	,	TK_NUM},		
  {" +",					TK_NOTYPE},	// spaces
	{"\\$\\w+",			TK_REG},
  {"\\(",					TK_PL},			// left parentheses
  {"\\)",					TK_PR},			// right parentheses
  {"\\*",					TK_MUL},		// mult
  {"\\/",					TK_DIV},		// div
  {"\\+",					TK_ADD},		// plus
  {"\\-",					TK_SUB},		// sub
  {"==",					TK_EQ},			// equal
	{"=!",					TK_INEQ},
	{"&&",					TK_OP_AND},
	{""}
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
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);//standarize the rules and put it into re[].
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128); //throw the error if failed .
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[4096];
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
					default					:	printf("in pos %d regex type error", position); break;
					case TK_ADD			:	tokens[nr_token].type = TK_ADD; 
														memset(tokens[nr_token].str, 0, 32); nr_token++; break;
					case TK_SUB			:	tokens[nr_token].type = TK_SUB; 
														memset(tokens[nr_token].str, 0, 32); nr_token++; break;
				 	case TK_MUL			:	tokens[nr_token].type = TK_MUL;
														memset(tokens[nr_token].str, 0, 32); nr_token++; break;
				 	case TK_DIV			:	tokens[nr_token].type = TK_DIV; 
														memset(tokens[nr_token].str, 0, 32); nr_token++; break;
       	  case TK_PL			:	tokens[nr_token].type = TK_PL;
														memset(tokens[nr_token].str, 0, 32); nr_token++; break;
       	  case TK_PR			:	tokens[nr_token].type = TK_PR;
														memset(tokens[nr_token].str, 0, 32); nr_token++; break;
       	  case TK_NUM			:	tokens[nr_token].type = TK_NUM;
														memset(tokens[nr_token].str, 0, 32);
														strncpy(tokens[nr_token].str, substr_start, substr_len);
				 										nr_token++;
				 										break;
					case TK_REG			:	tokens[nr_token].type = TK_REG;
														strncpy(tokens[nr_token].str, substr_start+1, substr_len-1);
														nr_token++;
														break;
       	  case TK_NOTYPE	:	break;
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

bool check_parenthese(int p, int q){
	if (tokens[p].type != TK_PL || tokens[q].type != TK_PR)
		return false;

	int count = 0;

	for(int i = p; i < q; i++){
		if			(tokens[i].type == TK_PL) count++;//有左括号就+1，右括号—1，可以一定程度模拟栈操作
		else if	(tokens[i].type == TK_PR) count--;
		
		if(count <= 0) return false;
	}

	//现在只遍历到倒数第二位，因为经过第一个if的验证，最后一位一定只剩右括号，所以现在count必须是1，否则就是匹配失败
	if(count == 1){/*printf("parenthese matched!\n");*/	return true;}
	else					{/*printf("parenthese unmatched \n");*/	return false;}
	
}

uint32_t read_reg(char *reg){
	printf("reg: %s\n", reg);
	for (int k = 0; k <8 ; k++){
		//printf("reg name %d,1 : %s\n", k, reg_name(k,1));
		//printf("reg name %d,2 : %s\n", k, reg_name(k,2));
		//printf("reg name %d,4 : %s\n", k, reg_name(k,4));
				if (!strcmp(reg, reg_name(k, 1))){
					return reg_b(k);
				}
				else if (!strcmp(reg, reg_name(k, 2))){
					return reg_w(k);
				}
				else if (!strcmp(reg, reg_name(k, 4))){
					return reg_l(k);
				}
	}

	printf("reg name error");
	assert(0);
	return 0;
}

	

uint32_t eval(int p, int q){
	if (p > q){
	// p>q is impossible!
		printf("p > q error!\n");
		return 0;
	}

	else if (p == q){
	// return number or content of reg.
		if (tokens[p].str != NULL){
			if (tokens[p].type == TK_NUM)
				return strtol(tokens[p].str, NULL, 16);
			else
				return read_reg(tokens[p].str);
		}
		else{
			printf("syntax error in expression, near position %d.", p);
			return 0;
		}
	}

/*	else if (tokens[p].type == TK_DEREF){
		uint32_t res = vaddr_read(eval(p+1,q),1);
		printf("res of %d to %d is %x\n", p+1, q, res);
		return res;
	}

	else if (tokens[p].type == TK_NEG){
		int resneg = -eval(p+1,q);
		printf("res of %d to %d is %x\n",p+1, q, resneg);
		return resneg;
	}
*/
	else if (check_parenthese(p,q) == true ){
	//delete the parenthese
		return eval(p + 1, q - 1);
	}

	//find main op.
	else{
		int op = p; //position of main op
		int pos = p;
		int BG,EN;
		while(pos < q){
			switch (tokens[pos].type){
				case TK_PL		:BG = pos;   //In this case, skip the content between parenthese
											 EN = pos+1;
											 while(EN < q){
												 EN++;
												 //printf("Got TK_PL when finding main op.\n");
												 int check = check_parenthese(BG, EN);
												 if( check == true){
													 pos = EN; 
													 break;
												 }
											 }
											 break;
				case TK_PR		:pos++;break;
				case TK_NUM		:pos++;break;
				case TK_DEREF :if (tokens[op].type < TK_DEREF )
												 op = pos;
											 pos++;break;
				case TK_NEG		:if (tokens[op].type < TK_DEREF )
												 op = pos;
											 pos++;break;
				case TK_ADD		:op = pos; pos++; break;
				case TK_SUB		:op = pos; pos++; break;
				case TK_MUL		:if(tokens[op].type < TK_ADD)
												 op = pos;
											 pos++;
											 break;
				case TK_DIV		:if(tokens[op].type < TK_ADD) 
												 op = pos; 
											 pos++; 
											 break;
				default				:printf("op not found!\n");pos++;break;
			}
		}
		printf("Main op founded : %d at %d \n", tokens[op].type, op);

		// when op is one of '+-*\'.
		if (tokens[op].type >= TK_MUL){
			int val1 = eval (p, op-1);
			int val2 = eval (op+1, q);

			switch (tokens[op].type){
				case TK_ADD	: return val1 + val2;
				case TK_SUB	: return val1 - val2;
				case TK_MUL	: return val1 * val2;
				case TK_DIV	: return val1 / val2;
				default			:	printf("op format doesn't match to oprand!\n");assert(0);return 0;
			}
		}

		// when op is minus or deref.
		else if (tokens[op].type >= TK_DEREF){
			int val = eval (op+1, q);
			switch (tokens[op].type){
				case TK_DEREF	:return vaddr_read(val,1);
				case TK_NEG		:return -val;
				default				:printf("op format doesn't match to oprand!\n");assert(0);return 0;
			}
		}
		else {
			printf("un predicted error!\n");
			return -1;
		}
	}
	
}


uint32_t expr(char *e, bool *success) {
  uint32_t result;
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

	//To mark the dereference and the negative .
	for (int i = 0; i < nr_token; i++){
		if ((tokens[i].type == TK_MUL || tokens[i].type == TK_SUB)
			 	&& (i == 0 || (tokens[i-1].type != TK_NUM && tokens[i-1].type != TK_PR))){
			if (tokens[i].type == TK_MUL)
				tokens[i].type = TK_DEREF;
			else
				tokens[i].type = TK_NEG;
		}
	}

  /* TODO: Insert codes to evaluate the expression. */
  for (int i = 0; i < nr_token ; i++){
    printf("No:%d, Type: %d; content: %s\n", i, tokens[i].type, tokens[i].str);
  } 
  printf("Hello expr!\n");
  result = eval(0, nr_token - 1);
	printf("result is : 0x%x, %d \n", result, result);
  return result;
}
