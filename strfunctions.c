/*
	ICS 53: Principles of System Design
	Winter Quarter 2015/2016
	Rewritten string functions from the 
	string library
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define require(e) if (!(e)) fprintf(stderr, "FAILED line %d        %s: %s       %s\n", __LINE__, __FILE__, __func__, #e)
#define SIZE 1000

char * val;
int index = 0;

int strlen(const char * s)
{
	return !*s ? 0: 1+ strlen(s+1);
}

char * strcpy (char * dst, const char *src)
{
	char *p = dst;  	// dst would then point at the null in while loop
    while (*p++ = *src++);
	return dst;
}

char * strncpy(char * dst, const char * src, size_t n)
{
	char *p = dst;
	for(int i = 0; i < n; i++)
	{
		if (!*src)
			break;
		*p++ = *src++;
	}
	return dst;
}
char * strchr(const char * str, int c)
{
	unsigned int i;
	for(i = 0; str[i]; ++i)
	{
		if(str[i] == c)
			return &str[i];
	}
	return NULL;
}

char * strrchr(const char * str, int c)
{
	char * index = NULL;
	for (int i = 0; str[i]; ++i)
	{	
		if(str[i] == c)
			index =&str[i];
	}
	return index;
}

char * strcat( char * dst, const char * src)
{
	strcpy(dst + strlen(dst), src);	
	return dst;
}

char * strncat (char * dest, const char * src, size_t n)
{
	int i;
	int len = strlen(dest);
	for(i=0; i< n; i++)
	{
		*(dest+len+i) = src[i];
	}	
	return dest;
}

int strcmp ( const char * str1, const char* str2 )
{
	while(*str1 && *str2)
	{
		if (*str1 != *str2)
			break;
		str1++;
		str2++;
	}
	return *str1 - *str2;
}
int strncmp(const char * str1, const char * str2, int index)
{
	int i = 0;
	while ((*str1 && *str2) && (i++ < index-1))
	{
		if (*str1++ != *str2++)
			break;
	}
	return *str1 - *str2;
}

char * strpbrk( char *str1, char * str2)
{
	for(int i = 0; str1[i]; ++i)
	{
		if(strchr(str2, str1[i]) != NULL){
			return &str1[i]; 
		}
	}
	return NULL;
}
int strspn( const char * str1, const char * str2)
{
	int index = 0;
	for (int i = 0; str1[i]; ++i)
	{
		if(strchr(str2, str1[i]) == NULL)
			return index;
		index++;
	}
	return index;

}

int strcspn( const char * str1, const char * str2)
{
	int index = 0;
	for (int i = 0; str1[i]; ++i)
	{
		if(strchr(str2, str1[i])!= NULL)
			return index;
		index++;
	}
	return index;
}

char * strstr(const char *haystack, const char *needle)
{
	char val[SIZE];
	for (int i = 0; i < strlen(haystack); ++i)
	{
		if(haystack[i] == needle[0])
		{
			if (strncmp(&haystack[i], needle, strlen(needle)) == 0)
				return &haystack[i];
		}
	}
	return NULL;
}


char * strtok(char * str, const char * delim)
{
	if (str != NULL)
	{
		val = strchr(str, delim[0]);
		str[strcspn(str,delim)] = NULL;
		return str;

	}
	else
		return  (val++ != NULL) ? strtok(val++, delim) : NULL;
}

	

// ----------------- TEST FUNCTIONS -------------------------

bool _using_strcmp( const char* str1, const char* str2)
{
	return strcmp(str1,str2) == 0 ? true : false;
}

void test_strlen()
{
    require(strlen("") == 0);
    require(strlen("hello") == 5);
}
void test_strcpy()
{
	char a[SIZE], b[SIZE] = "copy", c[SIZE] = "new";
	strcpy(a,b);
	require(_using_strcmp(a,b));
	strcpy(a,c);
	require(_using_strcmp(a,c));
}

void test_strncpy()
{
	char a[SIZE], b[SIZE] = "copy", c[SIZE] = "new";
	strncpy(a,b,3);
	require(_using_strcmp(a,"cop"));
	strncpy(a,c,3);
	require(_using_strcmp(a,"new"));
}
void test_strchr()
{
	char test[] = "make this work", test2[] = "ayy lmao", test3[] = "superman";
	require(_using_strcmp(strchr(test, 'e'), "e this work"));
	require(_using_strcmp(strchr(test2, 'l'), "lmao"));
	require(_using_strcmp(strchr(test3, 'm'), "man"));

}
void test_strrchr()
{
	char test[] = "aaaaaaabee", test2[] = "plsnononoLOL";
	require(_using_strcmp(strrchr(test, 'a'), "abee"));
	require(_using_strcmp(strrchr(test2, 'n'), "noLOL"));

}
void test_strcmp()
{
    char *r = "hello", *s = "hello", *t = "he"; 
    require(strcmp(r,s) == 0);
    require(strcmp(t,r) < 0);
    require(strcmp(s,t) > 0);
    require(strcmp(r, "hallo") > 0);
}

void test_strncmp()
{
	char *r = "hello", *s = "heyya", *t = "helios";
	require(strncmp(r,s,2) == 0);
	require(strncmp(r,t,3) == 0);
	require(strncmp(r,s,5) < 0);
	require(strncmp(r,s,3) < 0);
	require(strncmp(r,"helloh", 5) == 0);
	require(strncmp(t, "helas", 4) > 0);
	require(strncmp(t, "helas", 3) == 0);
}
void test_strcat()
{
	char a[SIZE], b[SIZE], t[SIZE] = "hellothere";
	strcpy(a,"hello");
	strcpy(b,"there");
	strcat(a,b);
	require(_using_strcmp(a,"hellothere"));
}

void test_strncat()
{
	char a[SIZE] = "hello", b[SIZE] = "there";
	strncat(a,b,3);
	require(_using_strcmp(a, "hellothe"));
	strncat(b,a,3);
	require(_using_strcmp(b,"therehel"));

}
void test_strspn()
{
	char a[SIZE] = "ABCDEFG019874";
	char set[SIZE] = "ABCD";
	char b[SIZE] = "12384110A";
	char setb[SIZE] = "1234567890";
	require(strspn(a,set) == 4);
	require(strspn(b,setb) == 8);
}
void test_strcspn()
{
	char a[SIZE] = "fcba73";
	char b[SIZE] = "thr3ee";
	char num[SIZE] = "123456789";
	char str1[SIZE] = "ABCDEF4960910";
	char str2[SIZE] = "013";
	require(strcspn(a,num) == 4);
	require(strcspn(b,num) == 3);
	require(strcspn(str1, str2) == 9);
}

void test_strbrk()
{
	char a[SIZE] = "this is a sample string";
	require(_using_strcmp(strpbrk(a, "aeiou"), "is is a sample string"));
	require(_using_strcmp(strpbrk(a,"mp"), "mple string"));
}

void test_strstr()
{
	const char haystack[20] = "TutorialsPoint";
	const char needle[10] = "Point";
	const char hay[SIZE] = "thislineworks";
	const char nee[SIZE] = "line";
	const char bad[SIZE] = "inter";
	char *ret;
	char *second;
	char * third;
	ret = strstr(haystack, needle);
	second = strstr(hay, nee);
	third = strstr(haystack, bad);
	require(_using_strcmp(ret, "Point"));
	require(_using_strcmp(second, "lineworks"));
	require(third == NULL);
}
void test_strtok()
{
	char str[SIZE] = "blue-cat-dude-ayy";
	const char s[2] = "-";
	char t[SIZE];
	char * token = t;
	token = strtok(str,s);
	require(_using_strcmp(token, "blue"));
	token = strtok(NULL, s);
	require(_using_strcmp(token, "cat"));
	token = strtok(NULL, s);
	require(_using_strcmp(token, "dude"));
	token = strtok(NULL, s);
	require(_using_strcmp(token, "ayy"));
}

int main()
{
    test_strlen();
    test_strcmp();
    test_strncmp();
    test_strcpy();
    test_strncpy();
    test_strchr();
    test_strrchr();
    test_strcat();
    test_strncat();
    test_strspn();
    test_strcspn();
    test_strbrk();
    test_strstr();
    test_strtok();

    return 0;
}