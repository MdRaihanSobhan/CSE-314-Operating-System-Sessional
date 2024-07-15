#include <stdio.h>

int main()
{
	int n, a;

	scanf("%d", &n);
	
	while(n--)
	{
		scanf("%d", &a);
		if(a % 2 == 1)
		{
			printf("YES\n");
		}
		else
		{
			printf("NO\n");
		}
	}
}
