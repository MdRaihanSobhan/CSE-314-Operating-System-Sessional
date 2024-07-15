import java.util.*;  

class Main   
{  
	public static void main(String[] args)  
	{  
		Scanner sc = new Scanner(System.in);  
		int n = sc.nextInt();
		for(int i = 0; i < n; i++)
		{
			int a = sc.nextInt();
			if(a % 2 == 0) System.out.println("YES");
			else System.out.println("NO");
		}
	}  
}  
