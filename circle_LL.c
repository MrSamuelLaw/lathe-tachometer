
#include<stdint.h>
#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<math.h>


#define LIST_SIZE 10  // don't forget to adjust this
#define UNIDIRECTIONAL_LIMIT 20


typedef struct Node
{
	uint16_t value;
	struct Node* next;
} Node;


Node* Head = NULL;  // don't forget to set it
uint16_t Sum = 0;


void __TEST__print(Node* head, int number_of_nodes)
{
	printf("---------VALUES---------");
	for(int x = 0; x < number_of_nodes; x++)
	{
		printf("Node %d: Value: %d\n", x, head->value);
		head = head->next;
	}
}


int pulseIn()
{
	int value;
	printf("Next value read in?\t");
	scanf("%d", &value);
	return value;
}



bool is_bidirectional()
{
	int32_t raw_sums = 0;
	int32_t sum_of_absolutes = 0;
	for(int x = 1; x < LIST_SIZE; x++, Head = Head->next)
	{
		register int16_t delta = Head->next->value - Head->value;
		sum_of_absolutes += abs(delta);
		raw_sums += delta;
	}

	Head = Head->next;  // return head to original position
	return sum_of_absolutes != abs(raw_sums);
}


int standard_deviation()
{
	int mean = Sum / LIST_SIZE;

	int variance = 0;
	for(int x = 0; x < LIST_SIZE; x++, Head = Head->next) variance += pow(Head->value - mean, 2);

	printf("std = %f\n",sqrt(variance));
	return sqrt(variance);
}


void update_sum(int old_value, int new_value)
{
	Sum -= old_value;  // remove previous value
	Sum += new_value;
}


void update_linked_list(int new_value)
{
	Head->value = new_value;
	Head = Head->next;
}




int main()
{
	Node node10;
	Head = &node10;

	Node node9 = {0, &node10};
	Node node8 = {0, &node9};
	Node node7 = {0, &node8};
	Node node6 = {0, &node7};
	Node node5 = {0, &node6};
	Node node4 = {0, &node5};
	Node node3 = {0, &node4};
	Node node2 = {0, &node3};
	Node node1 = {0, &node2};
	node10 = (Node){0, &node1};

	while(true)
	{
		int new_value = pulseIn();

		update_sum(Head->value, new_value);
		update_linked_list(new_value);

		if(UNIDIRECTIONAL_LIMIT < standard_deviation() && is_bidirectional())
		{
			// shit is going back code
			printf("\033[0;31mERROR: bidirectional\033[0;37m\n");
		}
		else printf("\033[0;32mUnidirectional\033[0;37m\n");


	}

	return 0;
}