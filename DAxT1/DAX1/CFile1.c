#include <stdio.h>

int main()
{
	UInt16 X=0, Y=0, Z=0, R1=0, R2=0;

	UInt8 array1[300];
	UInt8 array2[300];
	UInt8 array3[300];
	
	for(UInt8 i=0; i<300; i++)		// fill up the stack with 300 numbers
	{
		array1[i] = i;
		X=i;
	}
	
	// this is part 2
	for(UInt16 i =X;  i>=0; i--)
	{
		if(array1[i] %5 ==0)
		{
			array2[Y]=array1[i];
		}
		else
		{
			array3[Z] = array1[i];
		}
	}
	
	UInt8 yFlag=0;
	
	// this is part 3
	for(UInt16 i =0; i<300; i++)
	{
		if(!yFlag && Y!=0)
		{
			R1 += array2[Y];
			Y--;
		}
		elseif(Y==0)
		{
			yFlag=1;
		}
		
		if(Z!=0)
		{
			R2 += array3[Z];
			Z--;
		}
		else if(Z==0 && yFlag)
		{
			i=300;
		}
	}
	
	printf("summ of div by 5 numbers = %d\n", R1);
	printf("Sum of not div by 5 numbers = %d\n", R2);
	
	
	
	
	
	
	
}