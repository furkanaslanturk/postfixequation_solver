#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>

#define stackSize 300

char unknownNames[10];
int unknownCount = 0; // burada a b c d gibi değişkenlerin ismini ve kaç tane olduğunu tutuyorum

enum dataType{
	numtok, //number tokens
	symtok // symbol tokens
};
typedef enum dataType dataType;

struct Token{
	dataType dataType;
	int value;
	char symbol; 
};
typedef struct Token Token;
struct myStack{
	int top; // son elemanı tutacağız
	Token *array;
	int size;
};
typedef struct myStack myStack;

myStack* Stack(int expLength){ // stack i verilen uzunluğa göre oluşturduk
	myStack* myExp= malloc(sizeof(myStack));
	myExp->size=expLength;
	myExp->top=-1;
	myExp->array = malloc(sizeof(Token)*expLength);
	return myExp;
}
void push(myStack* myExp,Token t){
	if(myExp->top==myExp->size-1){
		printf("cant be pushed onto stack, because its full");
		return;
	}
	myExp->array[++(myExp->top)]=t;
}
Token pop(myStack* myExp){
	Token empty;
	empty.dataType= symtok;
	empty.symbol= '\0';
	empty.value=0;
	if(myExp->top==-1){
		printf("stack is empty cant be popped");
		return empty;
	}
	Token popped = myExp->array[(myExp->top)--];
	return popped;
}

myStack* organizedExpression(myStack* myExp){
	myStack* myOrganizedExpression= Stack(myExp->size);
	int i;
    for (i = 0; i <= myExp->top; i++) {
        Token t = myExp->array[i];

        if (t.dataType == numtok) {
            // sayılar direkt push
            push(myOrganizedExpression, t);
        }
        else if (t.dataType == symtok &&(t.symbol == '+' || t.symbol == '-' || t.symbol == '*' || t.symbol == '/' || t.symbol == '^')) {

            // Operator ve ondan onceki 2 eleman number ise bu clause a gireriz
            if (myOrganizedExpression->top >= 1 &&
                myOrganizedExpression->array[myOrganizedExpression->top].dataType == numtok &&
                myOrganizedExpression->array[myOrganizedExpression->top - 1].dataType == numtok) {

                Token b = pop(myOrganizedExpression); // ikinci operand
                Token a = pop(myOrganizedExpression); // birinci operand
                int num1 = a.value;
                int num2 = b.value;
                int result = 0;

                if (t.symbol == '+')      result = num1 + num2;
                else if (t.symbol == '-') result = num1 - num2;
                else if (t.symbol == '*') result = num1 * num2;
                else if (t.symbol == '/') result = num1 / num2;
                else if (t.symbol == '^') result = (int)pow(num1, num2);

                Token r;
                r.dataType = numtok;
                r.value = result;
                r.symbol = 0;
                push(myOrganizedExpression, r);
            }
            else {
                // Sadeleştiremiyorsak operatorü olduğu gibi ekleyelim
                push(myOrganizedExpression, t);
            }
        }
        else if(t.dataType==symtok && isalpha(t.symbol)){
            push(myOrganizedExpression, t); // variable ise push ve unknown sayısını bir arttır
        }
        else{
        	push(myOrganizedExpression, t); // kalanlar push
		}
    }
	if(myExp->top==myOrganizedExpression->top){
		return myExp; // hiç sadeleştirme yapamıyorsak inputtaki expressionı geri döndürmeliyiz
	}
    return myOrganizedExpression;
}
enum unknownType{
	unkOperand,
	unkOperator
};
typedef enum unknownType unknownType; // burada bilinmeyenin tiplerine baktık

struct Solution{ //burada solution isimli bir struct oluşturduk ki variableların sayısal ve operatorsel değerleri kolayca saklanabilsin ve erişilebilsin 
	int unknownNum;
	unknownType role[10];
	int value[10];
	char op[10];
};
typedef struct Solution Solution;
Solution solutions[500];
int solutionCount=0; // burada gerçekçi düşünüp max 500 solution vardır dedim ve count belirledim ki output verirken sınırım belirli olsun

bool evaluateWithSolution(Solution *sol, Token *tokens, int tokenCount, int *finalResult); // prototip assignUnknownValues da kullanacağım

struct Assigning{
	int unknownNum;
	unknownType type[100];
};
typedef struct Assigning Assigning;


int findUnknownIndex(char symbol) {  //burada bilinmeyenin indexini bulduk ki solution veyahut operator, operand olduğunu kolayca ayırt edebilelim
	int i;
    for (i = 0; i < unknownCount; i++) {
        if (unknownNames[i] == symbol)
			return i;
    }
    return -1;
}

bool checkValidity(Assigning *assign,Token *tokens,int tokenCount){
	myStack * dummy = Stack(tokenCount);
	int i;
	for(i=0;i<tokenCount;i++){
		if (tokens[i].dataType == symtok && tokens[i].symbol == '=') {
        break;
    }
		if (tokens[i].dataType == numtok) {
            // sayılar direkt push
        	push(dummy, tokens[i]);
        }
        else if (tokens[i].dataType == symtok && (tokens[i].symbol == '+' || tokens[i].symbol == '-' || tokens[i].symbol == '*' || tokens[i].symbol == '/' || tokens[i].symbol == '^')) {

            // Operator ve ondan onceki 2 eleman number ise bu clause a gireriz
            if (dummy->top >= 1 ) {

				pop(dummy); // burada üstteki gibi (organizedExpression) result bulmaya çalışmadım çünkü şuan sadece symbol mu number mı onu öğrendik
				pop(dummy); // gerçek alabileceği değerleri solving kısmında bulacağız

                Token r;
                r.dataType = numtok;
                r.value = 0;
                r.symbol = 0;
                push(dummy, r);
            }
            else{
            	return false;
			}
        
	}
	else if(isalpha(tokens[i].symbol)){
		 int idx = findUnknownIndex(tokens[i].symbol);
    if (idx == -1) { //böyle bir variableın yok direkt yanlış dönder
        return false;
    }
    unknownType role = assign->type[idx];

    if (role == unkOperand) {
        // bilinmeyen operand gibi davranır yani direkt push
        Token tmp;
        tmp.dataType = numtok;
        tmp.value = 0;
        tmp.symbol = 0;
        push(dummy, tmp);
    } else { // unkOperator
        // bilinmeyen operator gibi davranır yani 2 pop 1 push
        if (dummy->top < 1) {
            return false; // 2 tane poplayacak eleman yoksa bu operator olamaz direkt yanlış dönder
        }
        Token b = pop(dummy); // değer bulmaya çalışmıyoruz sadece böyle bir ifade valid olabilir mi (değer bulma solutionsda)
        Token a = pop(dummy);

        Token res;
        res.dataType = numtok;
        res.value = 0;
        res.symbol = 0;
        push(dummy, res);
    }	
	}
	
}
		if (dummy->top == 0 && dummy->array[0].dataType == numtok) { // burada aslında son bir kontrol yapıyoruz son kalan numbertoken olmalı yani sayısal bir değer 
    		return true;
		} else {
    		return false;
		}
}
void assignUnknownValues(int idx,Assigning *assign,Solution *cur,Token *tokens,int lhsCount,int rhsValue); //prototip


void solving(Assigning* assign, Token *tokens, int tokenCount){
	Solution cur;
    cur.unknownNum = assign->unknownNum;

    for (int i = 0; i < cur.unknownNum; i++) {
        cur.role[i] = assign->type[i];
        cur.value[i] = 0;
        cur.op[i] = 0;
    }

    int eqPos = -1;
    for (int i = 0; i < tokenCount; i++) {
        if (tokens[i].dataType == symtok && tokens[i].symbol == '=') {
            eqPos = i;
            break;
        }
    }

    int leftCount = tokenCount;
    int rightValue = 0;

    if (eqPos != -1) {
        leftCount = eqPos;
        if (eqPos + 1 < tokenCount && tokens[eqPos + 1].dataType == numtok) {
            rightValue = tokens[eqPos + 1].value;
        }
    }

    assignUnknownValues(0, assign, &cur, tokens, leftCount, rightValue);
	
}


void makeAssignments(int index,Assigning *cur,Token *tokens, int tokenCount){
	if(index==cur->unknownNum){
		if(checkValidity(cur, tokens, tokenCount)){ // burada aslında 2. adımı tamamlamış oluyoruz yani operator veya operand olma olasılıklarına göre bunu kontrol ediyoruz burada
			solving(cur, tokens, tokenCount); // burada da solutionları denetip buluyoruz yani 3.adım
		}
		return;
	}
	cur->type[index] = unkOperand; //recursion kullanarak bir algoritma oluşturdum. Bu ne işe yarar örneğin a b c variable ım olsun. Bu assignments da önce a ya operand atar sonra rec ile girip b ye oradan c ye atar.
	makeAssignments(index+1, cur,tokens, tokenCount); // daha sonra recursionla geri gelerek c yi operator yapar diğerleri operand kalır aynı mantık recursion ile tum olasılıkları tarattım
	
	cur->type[index]= unkOperator;
	makeAssignments(index+1,cur,tokens,tokenCount);
}
void assignUnknownValues(int idx,Assigning *assign,Solution *cur,Token *tokens,int leftCount, int rightValue) {
    // tüm bilinmeyenler dolmuş mu diye her recursionda kontrol
    if (idx == assign->unknownNum) {
        // dolmuşsa bu atamayı test ediyor ve gerçekten böyle bir solution çıkabiliyorsa public solutions arrayime ekliyorum
        int result;
        if (evaluateWithSolution(cur, tokens, leftCount, &result)) {
            if (result == rightValue) {
                if (solutionCount < 500) {
                    solutions[solutionCount++] = *cur;
                }
            }
        }
        return;
    }
    // burada makeAssignments içinde operator operand atamalarında kullandığıma benzer bir recursion algoritması kullandım
    // kullanıcı max deneteceği sayıyı kendisi belirleyebilir (ben 30 a kadar denettim) her bir bilinmeyen değeri bilinmeyenle atanıyor + recursion sayesinde kolayca tüm olasılıkları denettirebiliyorum
    if (cur->role[idx] == unkOperand) {
        // operandlar 1 den 30 a kadar deneniyor
        for (int val = 1; val <= 30; val++) {
            cur->value[idx] = val;
            assignUnknownValues(idx + 1, assign, cur, tokens, leftCount, rightValue);
        }
    } else { 
        // Operator ise: +, -, *, / , ^ değerlerini alabilir
        char ops[] = { '+', '-', '*', '/','^' };
        int opCount = 5;

        for (int k = 0; k < opCount; k++) {
            cur->op[idx] = ops[k];
            assignUnknownValues(idx + 1, assign, cur, tokens, leftCount, rightValue);
        }
    }
}
bool evaluateWithSolution(Solution *sol,Token *tokens,int tokenCount,int *finalResult){ // burada artık variablelara değerleri atamışız ve bu postfix ifadesinin gerçekten doğru sonucu mu verdiğini kontrol ediyoruz
    myStack *st = Stack(tokenCount);

    for (int i = 0; i < tokenCount; i++) {
        Token t = tokens[i];

        if (t.dataType == numtok) {
            // dümdüz bir sayı pushla
            push(st, t);
        }
        else if (t.dataType == symtok && isalpha(t.symbol)) { //burada bilinmeyen variablesa (a b c gibi) giriyor
            int idx = findUnknownIndex(t.symbol);
            if (idx == -1) return false;

            if (sol->role[idx] == unkOperand) {
                // bilinmeyen operand ise sayısal değerini al ve pushla
                Token tmp;
                tmp.dataType = numtok;
                tmp.value = sol->value[idx];
                tmp.symbol = 0;
                if (tmp.value <= 0) return false; // pdfte variableların pozitif olması gerektiği söylendiği için böyle bir if clause kullandım
                push(st, tmp);
            } else {
                // bilinmeyen operator ise 
                if (st->top < 1) return false;
                Token b = pop(st);
                Token a = pop(st);
                int x = a.value;
                int y = b.value;
                int res;

                char op = sol->op[idx];
                if (op == '+') {
                    res = x + y;
                } else if (op == '-') {
                    res = x - y;
                } else if (op == '*') {
                    res = x * y;
                } else if (op == '/') {
                    if (y == 0) return false;
                    if (x % y != 0) return false;
                    res = x / y;
                } else if (op== '^'){
                	res = 1;
                	int i;
    				for (i = 1; i <= y; i++)
						res *= x;
				}
				 else {
                    return false;
                }

            if (i != tokenCount - 1&& res<=0){
				return false;} // son adım harici tüm işlemler pozitif olmalı
					

                Token tmp;
                tmp.dataType = numtok;
                tmp.value = res;
                tmp.symbol = 0;
                push(st, tmp);
            }
        }
        else if (t.dataType == symtok &&(t.symbol == '+' || t.symbol == '-' || t.symbol == '*' || t.symbol == '/'|| t.symbol=='^')) {
			//token direkt operator ise
            if (st->top < 1) return false;
            Token b = pop(st);
            Token a = pop(st);
            int x = a.value;
            int y = b.value;
            int res;

            if (t.symbol == '+') {
                res = x + y;
            } else if (t.symbol == '-') {
                res = x - y;
            } else if (t.symbol == '*') {
                res = x * y;
            } else if (t.symbol == '/') {
        		if (y == 0) return false;
        		if (x % y != 0) return false;
        		res = x / y;
    		} else if (t.symbol == '^') {
        		res = 1;
        		for (int k = 0; k < y; k++) res *= x;
    		}

    	if (i != tokenCount - 1 && res <= 0){ //son adım harici tüm işlemler pozitif olmalı
			return false;}

    Token tmp;
    tmp.dataType = numtok;
    tmp.value = res;
    tmp.symbol = 0;
    push(st, tmp);
        }
        
        else {
            return false;
        }
    }
    *finalResult = st->array[0].value;
    return true;
}




int main(){
	char Expression[stackSize];
	FILE* file = fopen("postfixexpression.txt","r");
	
	if(!file){
		printf("File isnt exist or wrong calling in fopen \n");
		return 1;
	}
	fgets(Expression,stackSize,file); 
	fclose(file);
	// text dosyasindan expression alimi tamamlandi
	int length = strlen(Expression);
	if (length > 0 && Expression[length - 1] == '\n') { //burada stringin sonunda otomatik gelen \n i kaldırıyorum
    	Expression[length - 1] = '\0';
    	length--;
	}

	int i=0;
	myStack* myExp = Stack(length);
	while (i < length) {
    	if (Expression[i] == ' ') { // boşlukları atla
        	i++;
        	continue;
    		}

    	if (isdigit(Expression[i])) {
        	// Birden fazla basamağı birleştir
        	int value = 0;
        	while (i < length && isdigit(Expression[i])) {
            	value = value * 10 + (Expression[i] - '0');
            	i++;
        		}

        	Token t;
        	t.dataType = numtok;
        	t.value    = value;
        	t.symbol   = 0;
        	push(myExp, t);
    	}
    	else {
        	// Operatör veya değişken
        	Token t;
        	t.dataType = symtok;
        	t.symbol   = Expression[i];
        	t.value    = 0;

        	if (isalpha(Expression[i])) { 
            	// variablesa buraya giriyor, unknownNames dizisinde var mı bak
            	int k, isTrue = 0;
            	for (k = 0; k < unknownCount; k++) {
                	if (Expression[i] == unknownNames[k]) {
                    	isTrue = 1;
                    	break;
                	}
            	}
            	if (isTrue == 0) {
                	unknownNames[unknownCount++] = Expression[i];
            	}
        	}
        	push(myExp, t);
        	i++; // tek karakterlik token olduğu için burada sadece 1 arttırıyoruz
    	}
	}
	myStack* myOrg = organizedExpression(myExp); // expression organized edildi 1.step
	Token *tokens = myOrg->array;
	int tokenCount = myOrg->top+1;
	Assigning cur;
	cur.unknownNum = unknownCount;
	makeAssignments(0,&cur,tokens,tokenCount); // burada tüm ihtimallerden hangisinin operand operator olacağı kombinasyonlarını çekeceğiz, aynı zamanda solutionları da bulup tüm adımları tamamlayacağız
	
	printf("Solution Number: %d\n", solutionCount);

for (int s = 0; s < solutionCount; s++) { //pdfteki output gibi solutionsları veriyorum
    printf("(");
    for (int i = 0; i < solutions[s].unknownNum; i++) {
        if (solutions[s].role[i] == unkOperand) {
            printf("%d", solutions[s].value[i]);
        } 
		else {
            printf("%c", solutions[s].op[i]);
        }
        if (i < solutions[s].unknownNum - 1) {
            printf(", ");
        }
    }
    printf(")\n");
}
}