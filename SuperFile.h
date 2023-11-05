#pragma once

#include <stdio.h>
#include "Supermarket.h"
typedef unsigned char BYTE;

int		saveSuperMarketToFile(SuperMarket* pMarket, char* ifCompress, const char* superMarketFile, const char* customersFileName);

int		loadSuperMarketFromFile(SuperMarket* pMarket, const char* fileName,
			const char* customersFileName);

//int		loadCustomerFromTextFile(SuperMarket* pMarket, const char* customersFileName);

int		readSuperMarketFromCompressFile(SuperMarket* pMarket, const char* filename1,const char* filename);
int		loadProductFromTextFile(SuperMarket* pMarket, const char* fileName);
