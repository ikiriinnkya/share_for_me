#include <stdio.h>
#include <stdlib.h>

struct stack
{
	int *arr;
	int top;	//末尾のデータのある配列の要素番号
};

int push(struct stack *stk, int input,int n)
{
	if (stk->top <= n)
	{
		stk->top++;
		stk->arr[stk->top] = input;
		return 0;
	}
	else
	{
		printf("can't add data\n");
		return -1;
	}
}

int pop(struct stack *stk)
{
	if (stk->top != 0)
	{
		int temp = stk->arr[stk->top];
    stk->arr[stk->top] = 0;
		stk->top--;
		return temp;
	}
	else
	{
		printf("no data\n");
		return -1;
	}
}

void init_stack(struct stack *stk,int n)
{
	int i;
  stk->arr = (int *)malloc(sizeof(int)*n);
	for (i = 0; i < n; i++)
		stk->arr[i] = 0;
	stk->top = 0;
}

void show_stack(struct stack *stk,int n)
{
	int i;
	for (i = 0; i < n; i++)
		printf("%d ", stk->arr[i]);

	printf("\ntop:%d\n\n", stk->top);
}

int main(void)
{
	int i,d,N,e;
  char input[2];
	struct stack stk;
  printf("max_data?input number >>");
  scanf("%d",&N);
	init_stack(&stk,N);
  while(1){
    printf("\ninput mode which you want pushmode:p popmode:o >>");
    scanf("%s",input);
    if(input[0] == 'p'){
      printf("\npush mode!input number >>");
      scanf("%d",&e);
      push(&stk,e,N);
      show_stack(&stk,N);
    }

    else if(input[0] == 'o'){
      printf("\npop mode!");
      d = pop(&stk);
      printf("\n%d\n", d);
      show_stack(&stk,N);
    }
  }
	return 0;
}
