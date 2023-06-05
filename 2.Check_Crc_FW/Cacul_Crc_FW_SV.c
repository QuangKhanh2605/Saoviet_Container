#include <conio.h>
#include <stdio.h>

unsigned int 	crc, crc_boot = 0;
unsigned char 	Temp = 0;
unsigned char	Crc_u8, Crc_u8_boot = 0;
FILE 			*fp, *fp2;
unsigned int 	Total, total_boot = 0;

int main ()
{
	fp = fopen("SVCOS_P_V1_1_3.bin","rb");   
	
	if(fp == NULL)
	{
		printf("Error: Could not open file.");
		return 0;
	}

	while (!feof(fp))
	{
		if(fread(&Temp, 1, 1, fp) < 1)
			break;
		crc += Temp;
		Crc_u8 = (unsigned char) crc;
		Total++;
	}
	
	
	printf("Tong phan tu: %d \r\n", Total);
	printf("Gia tri crc: %d \r\n",crc);
	printf("Gia tri crc_u8 : %d", Crc_u8);
	
	fclose(fp);
	
	fp2 = fopen("BOOT_DCU_L4_V2.bin","rb");
	
	if(fp2 == NULL)
	{
		printf("Error: Could not open file.");
		return 0;
	}

	while (!feof(fp2))
	{
		if(fread(&Temp, 1, 1, fp2) < 1)
			break;
		crc_boot += Temp;
		Crc_u8_boot = (unsigned char) crc_boot;
		total_boot++;
	}
	
	
	printf("\r\nTong phan tu boot: %d \r\n", total_boot);
	printf("Gia tri crc boot : %d \r\n",crc_boot);
	printf("Gia tri crc_u8 boot  : %d", Crc_u8_boot);
	
	fclose(fp);
	
	
	return 1;
}
