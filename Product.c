#define  _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "Product.h"
#include "General.h"
#include "fileHelper.h"


#define MIN_DIG 3
#define MAX_DIG 5

void	initProduct(Product* pProduct)
{
	initProductNoBarcode(pProduct);
	getBorcdeCode(pProduct->barcode);
}

void	initProductNoBarcode(Product* pProduct)
{
	initProductName(pProduct);
	pProduct->type = getProductType();
	pProduct->price = getPositiveFloat("Enter product price\t");
	pProduct->count = getPositiveInt("Enter product number of items\t");
}

void initProductName(Product* pProduct)
{
	do {
		printf("enter product name up to %d chars\n", NAME_LENGTH);
		myGets(pProduct->name, sizeof(pProduct->name), stdin);
	} while (checkEmptyString(pProduct->name));
}

void	printProduct(const Product* pProduct)
{
	printf("%-20s %-10s\t", pProduct->name, pProduct->barcode);
	printf("%-20s %5.2f %10d\n", typeStr[pProduct->type], pProduct->price, pProduct->count);
}

int		saveProductToFile(const Product* pProduct, FILE* fp)
{
	if (fwrite(pProduct, sizeof(Product), 1, fp) != 1)
	{
		puts("Error saving product to file\n");
		return 0;
	}
	return 1;
}



int saveProductToCompressFile(Product* pProduct, FILE* fp)
{
	BYTE productData[3] = { 0 };
	
	char* temp = pProduct->barcode;
	checkBarcodeWrite(temp);

	productData[0] = ((temp[0] & 0x3F) << 2) | (temp[1] >> 4) & 0x3;
	productData[1] = (temp[1] & 0xF) << 4 | ((temp[2] >> 2)& 0xF);
	productData[2] = ((temp[2] & 0x3) << 6) | (temp[3] & 0x3f);

	if (fwrite(&productData, sizeof(BYTE), 3, fp) != 3)
	{
		return 0;
	}

	BYTE productData1[3] = { 0 };
	int nameLengthOfProduct = (int)strlen(pProduct->name);
	productData1[0] = (temp[4] & 0x3F) << 2 | ((temp[5] >> 4) & 0x3);
	productData1[1] = ((temp[5] & 0xF) << 4) | (temp[6] >> 2);
	productData1[2] = ((temp[6] & 0x3) << 6) | ((nameLengthOfProduct & 0xF) << 2) | ( pProduct->type & 0x3);
	if (fwrite(&productData1, sizeof(BYTE), 3, fp) != 3)
	{
		return 0;
	}

	if (fwrite(pProduct->name, sizeof(char), nameLengthOfProduct, fp) != nameLengthOfProduct)
	{
		return 0;
	}
	BYTE productData2[3] = { 0 };
	
	float reminder = (pProduct->price - (int)pProduct->price)*100;
	int price = (int)pProduct->price;
	productData2[0] = pProduct->count;
	productData2[1] = (int)reminder << 1 | price >> 8;
	productData2[2] = price & 0xFF;
	if (fwrite(&productData2, sizeof(BYTE), 3, fp) != 3)
	{
		return 0;
	}
	return 1;
}

int readProductFromCompressFile(Product* pProduct, FILE* fp)
{
	BYTE productData[6] = { 0 };

	if (fread(&productData, sizeof(BYTE), 6, fp) != 6)
	{
		return 0;
	}


	pProduct->barcode[0] = productData[0] >> 2;
	pProduct->barcode[1] = (((productData[1] >> 4) | ((productData[0]) << 4)) & 0x3F);
	pProduct->barcode[2] = ((((productData[1]) << 2) | (productData[2] >> 6)) & 0x3F);
	pProduct->barcode[3] = (productData[2] & 0x3F);
	pProduct->barcode[4] = ((productData[3] >> 2) & 0x3F);
	pProduct->barcode[5] = ((((productData[3]) << 4) | (productData[4] >> 4)) & 0x3F);
	pProduct->barcode[6] = (((productData[4]) << 2 | (productData[5] >> 6)) & 0x3F);
	pProduct->barcode[7] = '\0';
	
	checkBarcodeRead(pProduct->barcode);	
	
	int nameLengthOfProduct;
	nameLengthOfProduct = (productData[5]  >> 2) & 0x0F;

	if (fread(pProduct->name, sizeof(char), nameLengthOfProduct, fp) != nameLengthOfProduct)
	{
		return 0;
	}
	pProduct->type = productData[5] & 0x3;

	BYTE productData2[3];
	if (fread(&productData2, sizeof(BYTE), 3, fp) != 3)
	{
		return 0;
	}
	pProduct->count = productData2[0];
	int rem = (productData2[1] >> 1);
	float remainder = (float)((productData2[1] >> 1) / 100.000);
	int price = ((productData2[1] & 0x1) << 8) | productData2[2];
	pProduct->price = (float)(price + remainder );

	return 1;
}

void checkBarcodeRead(char* barcode)
{

	for (int i = 0; i < 7; i++) { 

		if (isdigit(barcode[i] + 48)) {
			barcode[i] += 48;
		}
		else {
			barcode[i] += 55;
		}

	}
}

void checkBarcodeWrite(char* barcode)
{
	for (int i = 0; i < 7; i++) {
		if (isdigit(barcode[i])) {

			barcode[i] -= 48;
		}
		else {

			barcode[i] -= 55;
		}
	}
}



int		loadProductFromFile(Product* pProduct, FILE* fp)
{
	if (fread(pProduct, sizeof(Product), 1, fp) != 1)
	{
		puts("Error reading product from file\n");
		return 0;
	}
	return 1;
}

void getBorcdeCode(char* code)
{
	char temp[MAX_STR_LEN];
	char msg[MAX_STR_LEN];
	sprintf(msg, "Code should be of %d length exactly\n"
		"UPPER CASE letter and digits\n"
		"Must have %d to %d digits\n"
		"First and last chars must be UPPER CASE letter\n"
		"For example A12B40C\n",
		BARCODE_LENGTH, MIN_DIG, MAX_DIG);
	int ok = 1;
	int digCount = 0;
	do {
		ok = 1;
		digCount = 0;
		printf("Enter product barcode ");
		getsStrFixSize(temp, MAX_STR_LEN, msg);
		if (strlen(temp) != BARCODE_LENGTH)
		{
			puts(msg);
			ok = 0;
		}
		else {
			//check and first upper letters
			if (!isupper(temp[0]) || !isupper(temp[BARCODE_LENGTH - 1]))
			{
				puts("First and last must be upper case letters\n");
				ok = 0;
			}
			else {
				for (int i = 1; i < BARCODE_LENGTH - 1; i++)
				{
					if (!isupper(temp[i]) && !isdigit(temp[i]))
					{
						puts("Only upper letters and digits\n");
						ok = 0;
						break;
					}
					if (isdigit(temp[i]))
						digCount++;
				}
				if (digCount < MIN_DIG || digCount > MAX_DIG)
				{
					puts("Incorrect number of digits\n");
					ok = 0;
				}
			}
		}

	} while (!ok);

	strcpy(code, temp);
}


eProductType getProductType()
{
	int option;
	printf("\n\n");
	do {
		printf("Please enter one of the following types\n");
		for (int i = 0; i < eNofProductType; i++)
			printf("%d for %s\n", i, typeStr[i]);
		scanf("%d", &option);
	} while (option < 0 || option >= eNofProductType);
	getchar();
	return (eProductType)option;
}

const char* getProductTypeStr(eProductType type)
{
	if (type < 0 || type >= eNofProductType)
		return NULL;
	return typeStr[type];
}

int		isProduct(const Product* pProduct, const char* barcode)
{
	if (strcmp(pProduct->barcode, barcode) == 0)
		return 1;
	return 0;
}

int		compareProductByBarcode(const void* var1, const void* var2)
{
	const Product* pProd1 = (const Product*)var1;
	const Product* pProd2 = (const Product*)var2;

	return strcmp(pProd1->barcode, pProd2->barcode);
}


void	updateProductCount(Product* pProduct)
{
	int count;
	do {
		printf("How many items to add to stock?");
		scanf("%d", &count);
	} while (count < 1);
	pProduct->count += count;
}


void	freeProduct(Product* pProduct)
{
	//nothing to free!!!!
}