#pragma once

#define CHECK_RETURN_0(some) {\
if(!some)\
return 0;\
}

#define CHECK_MSG_RETURN_0(some) {\
	if(!some){\
		printf("Error in open file\n");\
		return 0;\
	}\
}\



#define FREE_CLOSE_FILE_RETURN_0(some,fp) {\
if(!some){\
free(some);\
fclose(fp);\
return 0;\
}\
}

#define CLOSE_RETURN_0(fp) {\
fclose(fp);\
return 0;\
}
