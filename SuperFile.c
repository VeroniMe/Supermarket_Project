     #define  _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Address.h"
#include "General.h"
#include "fileHelper.h"
#include "SuperFile.h"



int		saveSuperMarketToFile(SuperMarket* pMarket, char* ifCompress, const char* superMarketFile, const char* customersFileName)
{
	
	FILE* fp;
	fp = fopen(superMarketFile, "wb");
	CHECK_MSG_RETURN_0(fp);
	if (!strcmp(ifCompress, "0"))
	{
		if (!writeStringToFile(pMarket->name, fp, "Error write supermarket name\n"))
		{
			CLOSE_RETURN_0(fp);
		}

		if (!saveAddressToFile(&pMarket->location, fp))
		{
			CLOSE_RETURN_0(fp);
		}
		int count = getNumOfProductsInList(pMarket);

		if (!writeIntToFile(count, fp, "Error write product count\n"))
		{
			CLOSE_RETURN_0(fp);
		}

		Product* pTemp;
		NODE* pN = pMarket->productList.head.next; //first Node
		while (pN != NULL)
		{
			pTemp = (Product*)pN->key;
			if (!saveProductToFile(pTemp, fp))
			{
				CLOSE_RETURN_0(fp);
			}
			pN = pN->next;
		}
	}
	else if (!strcmp(ifCompress, "1"))
	{
		if (!saveSuperMarketToCompressFlie(pMarket, fp))
		{
			CLOSE_RETURN_0(fp);
		}
	}
	else
	{
		CLOSE_RETURN_0(fp);
	}
	fclose(fp);
	saveCustomerToTextFile(pMarket->customerArr, pMarket->customerCount, customersFileName);
	return 1;

}
#define BIN
#ifdef BIN
int	loadSuperMarketFromFile(SuperMarket* pMarket, const char* fileName,
	const char* customersFileName)
{
	FILE* fp;
	fp = fopen(fileName, "rb");
	CHECK_MSG_RETURN_0(fp);
	pMarket->name = readStringFromFile(fp, "Error reading supermarket name\n");
	if (!pMarket->name)
	{
		CLOSE_RETURN_0(fp);
	}

	if (!loadAddressFromFile(&pMarket->location, fp))
	{
		FREE_CLOSE_FILE_RETURN_0(&pMarket->location, fp);
	}

	int count;
	if (!readIntFromFile(&count, fp, "Error reading product count\n"))
	{
		FREE_CLOSE_FILE_RETURN_0(pMarket->name, fp);
	}

	if (count > 0)
	{
		Product* pTemp;
		for (int i = 0; i < count; i++)
		{
			pTemp = (Product*)calloc(1, sizeof(Product));
			if (!pTemp)
			{
				printf("Allocation error\n");
				L_free(&pMarket->productList, freeProduct);
				FREE_CLOSE_FILE_RETURN_0(pMarket->name, fp);
			}
			if (!loadProductFromFile(pTemp, fp))
			{
				L_free(&pMarket->productList, freeProduct);
				FREE_CLOSE_FILE_RETURN_0(pMarket->name, fp);
			}
			if (!insertNewProductToList(&pMarket->productList, pTemp))
			{
				L_free(&pMarket->productList, freeProduct);
				FREE_CLOSE_FILE_RETURN_0(pMarket->name, fp);
			}
		}
	}


	fclose(fp);

	pMarket->customerArr = loadCustomerFromTextFile(customersFileName, &pMarket->customerCount);
	CHECK_MSG_RETURN_0(pMarket->customerArr);

	return	1;

}
#else
int	loadSuperMarketFromFile(SuperMarket* pMarket, const char* fileName,
	const char* customersFileName)
{
	FILE* fp;
	fp = fopen(fileName, "rb");
	if (!fp)
	{
		printf("Error open company file\n");
		return 0;
	}

	//L_init(&pMarket->productList);


	pMarket->name = readStringFromFile(fp, "Error reading supermarket name\n");
	if (!pMarket->name)
	{
		fclose(fp);
		return 0;
	}

	if (!loadAddressFromFile(&pMarket->location, fp))
	{
		free(pMarket->name);
		fclose(fp);
		return 0;
	}

	fclose(fp);

	loadProductFromTextFile(pMarket, "Products.txt");


	pMarket->customerArr = loadCustomerFromTextFile(customersFileName, &pMarket->customerCount);
	if (!pMarket->customerArr)
		return 0;

	return	1;

}
#endif

int		loadProductFromTextFile(SuperMarket* pMarket, const char* fileName)
{
	FILE* fp;
	//L_init(&pMarket->productList);
	fp = fopen(fileName, "r");
	CLOSE_RETURN_0(fp);
	int count;
	fscanf(fp, "%d\n", &count);


	//Product p;
	Product* pTemp;
	for (int i = 0; i < count; i++)
	{
		pTemp = (Product*)calloc(1, sizeof(Product));
		myGets(pTemp->name, sizeof(pTemp->name), fp);
		myGets(pTemp->barcode, sizeof(pTemp->barcode), fp);
		fscanf(fp, "%d %f %d\n", &pTemp->type, &pTemp->price, &pTemp->count);
		insertNewProductToList(&pMarket->productList, pTemp);
	}

	fclose(fp);
	return 1;
}

int saveSuperMarketToCompressFlie(SuperMarket* pMarket, FILE* fp)
{
	BYTE byteNameAndProdCount[2] = { 0 };
	int ProductCount = getNumOfProductsInList(pMarket);
	int NameLength = (int)strlen(pMarket->name);
	byteNameAndProdCount[0] = ProductCount >> 2;
	byteNameAndProdCount[1] = ProductCount << 6 | NameLength;

	if (fwrite(&byteNameAndProdCount, sizeof(BYTE), 2, fp) != 2)
	{
		return 0;
	}

	if (fwrite(pMarket->name, sizeof(char), NameLength, fp) != NameLength)
	{
		return 0;
	}

	if (!saveAddressToCompressFile(&pMarket->location, fp))
		return 0;

	Product* pTemp;
	NODE* pN = pMarket->productList.head.next; //first Node
	while (pN != NULL)
	{
		pTemp = (Product*)pN->key;
		if (!saveProductToCompressFile(pTemp, fp))
		{
			CLOSE_RETURN_0(fp);
		}
		pN = pN->next;
	}
	return 1;

}


int readSuperMarketFromCompressFile(SuperMarket* pMarket, const char* filename1, const char* filename)
{
	FILE* fp = fopen(filename1, "rb");
	CHECK_MSG_RETURN_0(fp);
	BYTE byteNameAndProdCount[2] = { 0 };
	if (fread(byteNameAndProdCount, sizeof(BYTE), 2, fp) != 2)
		return 0;
	int numOfProducts = (byteNameAndProdCount[0] << 2 | (byteNameAndProdCount[1] >> 6) & 0x3);
	int nameLen = byteNameAndProdCount[1] & 0x3F;

	pMarket->name = (char*)calloc(nameLen+1, sizeof(char));
	CHECK_RETURN_0(pMarket->name);

	if (fread(pMarket->name, sizeof(char), nameLen, fp) != nameLen)
		return 0;

	if (!readAddressFromCompressFile(&pMarket->location, fp))
		return 0;

	NODE* pN = &pMarket->productList.head; //first Node
	for (int i = 0; i < numOfProducts; i++)
	{
		Product* pTemp = (Product*)calloc(1, sizeof(Product));
		CHECK_RETURN_0(pTemp);
		if (!readProductFromCompressFile(pTemp, fp))
		{
			CLOSE_RETURN_0(fp);
		}
		L_insert(pN, pTemp);
		pN = pN->next;
	}
	pMarket->customerArr = loadCustomerFromTextFile(filename, &pMarket->customerCount);
	CHECK_MSG_RETURN_0(pMarket->customerArr);
	fclose(fp);
	return 1;
}