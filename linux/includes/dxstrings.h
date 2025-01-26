/*
 This module has string manipulation routines.
 The module is designed to be portable between different windows and linux flavors.
 DeusEx 2023. Nikolaos Mourgis. deus-ex.gr

 Updated 03-06-2024 The new supported string type of dx_wstring was added.
 The string character is a uint32_t entity that can store all the UTF codepoints 
 as numeric values.
 NOTE: The wchar_t in windows is 2 bytes long , the DXCHAR is 4 bytes long ,
 in the case that the DXCHAR value needs 3 or 4 bytes to store the
 conversion will lose the data of the large characters. I do not
 mitigate this as i intend  the class to be used for "normal" text and not for text > of 2 bytes characters.
 Ofcourse if there is a need for 4 bytes characters  , the utf8 type is available or we can stay
 in the DXCHAR type and a third function can do the translation. Or whatever.

*/

 #ifndef DXULONG
 #define DXULONG uint32_t
 #endif

 #ifndef DXLONG
 #define DXLONG int32_t
 #endif

 #ifndef DXUINT
 #define DXUINT  uint16_t
 #endif

 #ifndef DXINT
 #define DXINT int16_t
 #endif

 #ifndef DXDWORD
 #define DXDWORD uint32_t
 #endif

 #ifndef DXLONG64
 #define DXLONG64 int64_t
 #endif


 #define DXBYTE     uint8_t
 #define DXUTF8CHAR uint32_t
 #define DXCHAR     uint32_t



#ifdef _WIN32
	//import the functions as for some reason the tcc does not have the headers for it
	//WINBASEAPI INT WINAPI WideCharToMultiByte (UINT CodePage, DWORD dwFlags, LPCWCH lpWideCharStr, INT cchWideChar, LPSTR lpMultiByteStr, INT cbMultiByte, LPCCH lpDefaultChar, LPBOOL lpUsedDefaultChar);
	//WINBASEAPI INT WINAPI MultiByteToWideChar (UINT CodePage, DWORD dwFlags, LPCCH lpMultiByteStr, INT cbMultiByte, LPWSTR lpWideCharStr, INT cchWideChar);

	/* add some flags that are needed for the string conversions */
	#ifndef CP_ACP
	#define CP_ACP	0	/* Default to ANSI code page.  */
	#endif

	#ifndef CP_OEMCP
	#define CP_OEMCP 1 /* Default to the code page of the system.  */
	#endif



	#ifndef CP_UTF8
	#define CP_UTF8 65001
	#endif

	#ifndef CP_UTF16
	#define CP_UTF16 -1
	#endif

	#ifndef MB_ERR_INVALID_CHARS
	#define MB_ERR_INVALID_CHARS 8
	#endif
#endif


/*PDX_STRING object and functions*****************************************************/

/*
Disclaimer : I know that my code has not many safety measures
as const pointers or functions to get values to avoid to access directly the
struct members. Thats it. I know.
*/

/*
 the following variable can be set with the SetConversionCodePage(unsigned long codepage)
 function and it is used for the convertion from Ansi to widechar
*/
#ifdef _WIN32
unsigned long DX_CP_CODE_PAGE = CP_THREAD_ACP ;

void SetConversionCodePage(unsigned long codepage)
/*#
 Set the code page for the convertion from Ansi to widechar
#*/
{
  DX_CP_CODE_PAGE = codepage ;
}

#endif

enum dx_compare_res {dx_equal , dx_not_equal , dx_left_bigger, dx_right_bigger , dx_error, dx_not_initialized};
/*#
 The return value for simple comparisons
#*/


enum dx_string_type {dx_null,dx_wide,dx_ansi,dx_utf8,dx_wstring};
/*#
 Determine if a string is encoded in one of the supported forms
 note that the type wchar_t is of 2 bytes in windows and 4 bytes in linux.
 this is very important as it can leads to very wrong calculation if
 is used a hard coded value for the char length.
 
 The dx_wstring stores the characters as UTF codepoints of uint32_t type 
#*/
typedef struct dx_string
/*#
{
  char * stringa		 ; // a string of utf8 or ansi type.
  wchar_t * stringw	     ; // a string of utf16(windows) or utf32 (linux)
  enum dx_string_type type  ; // what string type this string holds
  DXLONG64 len           ; // the length of the string in characters
  DXLONG64 bcount		 ; // the length of the string in bytes

} *PDX_STRING ;

The basic string object
#*/
{
  char    * stringa		 ; // a string of utf8 or ansi type.
  wchar_t * stringw		 ; // a string of utf16 (this is declared like this for convinience)
  DXCHAR  * stringx      ; // a string of utf codepoints of 32 bit values	
  enum dx_string_type type ; // what string type this string holds
  DXLONG64 len           ; // the length of the string in characters
  DXLONG64 bcount		 ; // the length of the string in bytes WITHOUT the terminating 0

} *PDX_STRING ;


PDX_STRING dx_string_free(PDX_STRING str) ;
/*#
 Frees the memory of the string.
 returns NULL
#*/
PDX_STRING dx_string_createA(PDX_STRING dxstr , char * str)    ;
/*#
 The function checks if the dxstr is NULL. If it is then creates and return a new PDX_STRING as ansi string.
 If the dxstr is not null then the function will freed the object and it will recreate it.
#*/
PDX_STRING dx_string_createW(PDX_STRING dxstr , wchar_t * str) ;
/*#
 The function checks if the dxstr is NULL. If it is then creates and return a new PDX_STRING as wide string.
 If the dxstr is not null then the function will freed the object and it will recreate it.
#*/
PDX_STRING dx_string_createU(PDX_STRING dxstr , const char * str) ;
/*#
 The function checks if the dxstr is NULL. If it is then creates and return a new PDX_STRING as utf8 string.
 If the dxstr is not null then the function will freed the object and it will recreate it.
#*/

PDX_STRING dx_string_createX(PDX_STRING dxstring, DXCHAR * str);
/*#
 The function creates a new dx_string in the form of the native DXCHAR ARRAY.
 returns NULL if an error occurs
#*/
PDX_STRING dx_string_createX_a(PDX_STRING dxstring, char * str);
/*#
 The function creates a new dx_string in the form of the native DXCHAR ARRAY.
 from an ansi string. This is cpu intensive as the str will be first convert to 
 utf8 and then to DXCHAR*
#*/
PDX_STRING dx_string_createX_w(PDX_STRING dxstring,  wchar_t * str);
/*#
 The function creates a new dx_string in the form of the native DXCHAR ARRAY.
 from an wide string
#*/
PDX_STRING dx_string_createX_u(PDX_STRING dxstring, char * str);
/*#
 The function creates a new dx_string in the form of the native DXCHAR ARRAY.
 from an ansi string. This is cpu intensive as the str is a utf8 string and every character must 
 be recognized first.
#*/

/*
The following functions create a PDX_STRING from a buffer but does not copy the buffer ,
it assign the buffer and the buffer MUST be not invalidated outside of the dx_string_free
*/

PDX_STRING dx_string_create_bA(char *buf) ;
PDX_STRING dx_string_create_bU(char *buf) ;
PDX_STRING dx_string_create_bW(wchar_t *buf) ;
PDX_STRING dx_string_create_bX(DXCHAR *buf) ;
/****************************************************************************************/

void dx_concat_codepoint(PDX_STRING dxstring,DXCHAR ch);
/*#
 The function concats a codepoint to the end of the dxstring ONLY if the dxstring type is dx_wstring.
 The function modifies the dxstring and does not return a new copy
#*/

/**** 
you can use the following functions in an already created string by 
the creation functions. This is for special cases but the internal buffers 
will be freed 
****/

PDX_STRING dx_string_setA(PDX_STRING dxstr , char * str)    ;
/*#
 The function assigns the stringa to the str and sets the appropriate flags for an ansi string
#*/
PDX_STRING dx_string_setW(PDX_STRING dxstr , wchar_t * str) ;
/*#
 The function assigns the stringw to the str and sets the appropriate flags for an wide string
#*/
PDX_STRING dx_string_setU(PDX_STRING dxstr , char * str)    ;
/*#
 The function assigns the stringa to the str and sets the apropriate flags for a utf8 string
#*/
PDX_STRING dx_string_setX(PDX_STRING dxstr , DXCHAR * str )    ;
/*#
 The function assigns the stringx to the str and sets the apropriate flags for a dx_wstring string
#*/

PDX_STRING dx_string_create_uninit(enum dx_string_type type , size_t len ) ;
/*#
 Creates and returns a string of the [type] and mallocs len+1 CHARACTERS for the string.
 In error returns NULL. Does not permints length <= 0.
 Remarks : When the function is used to create a utf8 string , the len must be in BYTES and not in characters
 as it is with widestring or ansi.
 The Function sets the terminating zero character
#*/

/****************************************************************************************************************************/

PDX_STRING dx_string_convertU(PDX_STRING str) ;
/*#
 The function check if the dxstr is NULL. If it is then returns NULL.
 If the dxstr is not NULL then the function will try to convert the string to a Utf8.
 In success it will return a new string. In failure NULL will be returned.
#*/
PDX_STRING dx_string_convertW(PDX_STRING str) ;
/*#
 The function check if the dxstr is NULL. If it is then returns NULL.
 If the dxstr is not NULL then the function will try to convert the string to a widestring.
 In success it will return a new string. In failure NULL will be returned.
#*/

PDX_STRING dx_string_convertX(PDX_STRING str) ;
/*#
 The function check if the dxstr is NULL. If it is then returns NULL.
 If the dxstr is not NULL then the function will try to convert the string to a dx_wstring.
 In success it will return a new string. In failure NULL will be returned.
#*/

PDX_STRING dx_string_concat(const PDX_STRING str1 , const PDX_STRING str2);
/*#
 The function will concat the strings and it will return the new one.
 Remarks : The returned string is a newly created string.
 There is some non permited concatenations between strings : ansi + wide , ansi + utf8
 as the convertion is difficult and quite frankly  this string class is
 created manly for manipulation of wide and utf8 strings
#*/
PDX_STRING dx_string_copy(PDX_STRING str1 , const PDX_STRING str2);
/*#
 The function copies ths str2 to the str1. The function returns the str1 . If the
 str1 is NULL then a new string will be created internally and will be returned. If the str2 is NULL then no operation is done.
 The str1 will be resized to fit inside the contents of the str2.
 The str1 will be cast to the same type as the str2
#*/

PDX_STRING dx_string_clone(PDX_STRING str) ;
/*#
 The function deep copies all the strings fields and returns a clone of the str
#*/
PDX_STRING dx_string_set_empty(PDX_STRING str) ;
/*#
 The function sets the internal string to one character with the 0 as value.
 It sets the len and b count too.
#*/


enum dx_compare_res dx_string_compare(PDX_STRING str1, PDX_STRING str2);
/*#
 This function compares two strings and return one of the following values :
 dx_equal        : The strings are the same.
 dx_left_bigger  : The str1 is bigger.
 dx_right_bigger : The str2 is bigger.
 dx_error		   : An error occured internally in the function.

 NOTES : The function can be very taxing in cpu resources when you try to
 compare non wide strings as the function will try to convert the
 strings to widestrings and then compare them.
 If you use this classes for another abstraction , then the better is to use
 always widestrings to do this operation.
 Keep in mind that the [Hello Molly] is bigger than the [Hella Molly]
 as the [a] has lower value in the characters table than the [o]

 Do not use this function for alphabetical sorting as this function is created for fast check for equality , use
 the function dx_string_lex_cmp for alphabetical sorting purposes
#*/

enum dx_compare_res dx_string_native_compare(PDX_STRING str1, PDX_STRING str2);
/*#
 This function compares two of the same encoding strings and return one of the following values :
 dx_equal        : The strings are the same.
 dx_left_bigger  : The str1 is bigger.
 dx_right_bigger : The str2 is bigger.
 dx_error		   : An error occured internally in the function.

 NOTES : Use this function only in same type strings e.g ansi to ansi or utf8 to utf8
 because the function DOES NOT make any conversion between strings.
#*/

enum dx_compare_res dx_string_lex_cmp(PDX_STRING str1, PDX_STRING str2);
/*#
 This function compares two strings and return one of the following values :
 dx_equal        : The strings are the same.
 dx_left_bigger  : The str1 is bigger.
 dx_right_bigger : The str2 is bigger.
 dx_error		   : An error occured internally in the function.

 NOTES : The function can be very taxing in cpu resources when you try to
 compare non wide strings as the function will try to convert the
 strings to widestrings and then compare them.
 If you use this classes for another abstraction , then better is to use
 always widestrings to do this operation.


 Use this function for alphabetical sorting purposes
#*/

enum dx_compare_res dx_string_native_lex_cmp(PDX_STRING str1, PDX_STRING str2);
/*#
 This function compares two strings and return one of the following values :
 dx_equal        : The strings are the same.
 dx_left_bigger  : The str1 is bigger.
 dx_right_bigger : The str2 is bigger.
 dx_error		   : An error occured internally in the function.

 NOTES : Use this function only in same type strings e.g ansi to ansi or utf8 to utf8
 because the function DOES NOT make any conversion between strings.


 Use this function for alphabetical sorting purposes
#*/

char *dx_string_return_utf8_pos(PDX_STRING str , DXLONG64 pos );
/*#
 returns the position of the first byte of the character in the pos,
 in error returns NULL
#*/

uint32_t dx_string_convert_uint32_to_utf8(char *buffer, uint32_t utfc);
/*#
	The function gets a utf codepoint and returns in the buffer (mist be atleast 5 characters long) the utf8 character
#*/


uint32_t dx_convert_utf8_to_int32(char *buffer,int len);
/*#
 The function gets a utf8 char and its length and convert it to a unicode codepoint
#*/

uint32_t dx_string_uint32_to_utf8_size(uint32_t utfc);
/*#
	The function gets a utf codepoint and returns the bytes that are needed to encode the character as a utf8
#*/



#ifdef _WIN32
void EchoDXSTRING(HANDLE console ,PDX_STRING str);
#endif
#ifdef _LINUX
void EchoDXSTRING(PDX_STRING str);
#endif // _LINUX
/*#
 For debuging purposes
#*/

/*PDX_STRING object and functions*****************************************************/



DXLONG64 StrLenA(const char *a_str) ;
/*#
 Returns the length of an ansi string in characters
#*/
DXLONG64 StrLenW(const wchar_t *w_str) ;
/*#
 Returns the length of a utf16 string in characters
#*/
DXLONG64 StrLenU(const char *u_str) ;
/*#
 Returns the length of a utf8 string in characters
#*/
DXLONG64 StrLenX(const DXCHAR *dx_str ) ;
/*#
 Returns the length of a dx_string in characters
#*/

wchar_t * dx_ConvertAnsiToW(const char *str);
/*#
 Converts an ansi string to a utf16 string.
 if the [str] is not an ansi string then the function return NULL.
 The resulting string has been malloced inside the function
#*/

char * dx_ConvertAnsiToUtf8(const char *str );
/*#
 Converts an ansi string to a utf8 string.
 if the [str] is not an ansi string then the function return NULL.
 The resulting string has been malloced inside the function
#*/

char * dx_ConvertWToUtf8(const wchar_t *str);
/*#
 Converts a widestring string to a utf8 string.
 The resulting string has been malloced inside the function
#*/

wchar_t * dx_ConvertUtf8ToW(const char *str) ;
/*#
 Converts a utf8 string to a widestring string.
 The resulting string has been malloced inside the function
#*/

DXCHAR * dx_ConvertAnsiToDXS(const char *str) ; 
/*#
 Converts an ansi string to a dx_string string.
 The resulting string has been malloced inside the function
#*/

DXCHAR * dx_ConvertUtf8ToDXS(const char *str) ; 
/*#
 Converts a utf8 string to a dx_string string.
 The resulting string has been malloced inside the function
#*/

DXCHAR * dx_ConvertWideToDXS(wchar_t *str) ; 
/*#
 Converts a widestring string to a dx_string string.
 The resulting string has been malloced inside the function
#*/

wchar_t * dx_ConvertDXSToWide(DXCHAR *str) ; 
/*#
 Converts a dx_string to a widestring.
 The resulting string has been malloced inside the function
#*/

char * dx_ConvertDXSToUtf8(DXCHAR *str) ; 
/*#
 Converts a dx_string to a utf8 string.
 The resulting string has been malloced inside the function
#*/

DXCHAR dx_ConvertUtf8ToDXSc(char *ch);
/*#
 converts a utf8 character to a  DXCHAR
#*/

int dx_ConvertDXSToUtf8c(char *out_c , DXCHAR ch);
/*#
 converts a DXCHAR character to a utf8 character. Returns the length in bytes of the utf8 character
#*/

short dx_utf8_char_byte_count(char utfc_1byte) ;
/*#
 The function returns the byte cound for the specific utf8 char given that the utfc_1byte is the first byte of the character
#*/

DXLONG64 dx_utf8_word_byte_count(char *word);
/*
 The function returns the bytes that the word needs to be stored as a ut8 string
*/

PDX_STRING dx_utf8_remove_char(PDX_STRING string,char *utf8chars);
/*#
 The function removes the urf8char from the string. The string is altered in the function and the string is the returning value
#*/

PDX_STRING dx_unicode_remove_char(PDX_STRING string, DXCHAR this_char );
/*#
 The function removes the DXCHAR from the string. The string is altered in the function and the string is the returning value
#*/

PDX_STRING dx_utf8_replace_char(PDX_STRING string,char *utf8char, char *with_char) ;
/*#
 The function replace the utf8char in the string with the with_char 
#*/

PDX_STRING dx_unicode_replace_char(PDX_STRING string,DXCHAR or_char, DXCHAR with_char) ;
/*#
 The function replace the or_char in the string with the with_char 
#*/

char * dx_utf8_find_word(PDX_STRING str , DXLONG64 from_index , char *word , DXLONG64 *char_indx);
/*#
 The function search for the word in the str starting from the from_index.
 Returns the words first character index. The char_indx will be set 
 to the character number in the string e.g if the first character of the word was 
 the 10th character of the string then char_indx will be 9.
 If the word is not found then the function returns NULL.
#*/

char * dx_binary_find_word(PDX_STRING str , DXLONG64 *from_index , char *word);
/*#
 The function search for the word in the str starting from the from_index.
 Returns the words first byte index. The from_indx will be set 
 to the byte number in the string e.g if the first byte of the word was 
 the 10th byte of the string then byte_indx will be 9.
 If the word is not found then the function returns NULL.
#*/


DXLONG64 dx_unicode_find_word(PDX_STRING str,DXLONG64 from_index, DXCHAR *word) ;
/*#
 The function search for the word in the str starting from the from_index. Returns the 
 words first character index. 
#*/


void memcpy_from_indx(char *dest,char *src, DXLONG64 fr_indx, DXLONG64 bcount) ;
/*
 copy into the dest data from src from fr_indx position bcount bytes
*/



/*-------------------Implementation------------------------------*/

uint8_t GetBit(DXBYTE byte , uint8_t bit)
{
 /*
  The function gets a byte , and the position of the bit that we want
  to read , and returns the value of the bit as 0 or 1
  Note that the bits index starting from 0
 */
 DXBYTE mask ;
 mask = 1 ;
 mask = mask << (7 - bit) ;

 uint8_t res = byte & mask  ;
 if (res > 0) return 1 ;

 return 0 ;
}



/*
 ************************* PDX_STRING ***********************************
*/

PDX_STRING dx_string_free(PDX_STRING str)
{
	if (str == NULL) return NULL;
	switch(str->type)
	{
		case dx_null : break ;
		case dx_wide :
		{
		 free(str->stringw);
		 free(str);
		}
		break ;
		case dx_ansi :
		case dx_utf8 :
		{
		 free(str->stringa);
		 free(str);
		}
		break ;
		case dx_wstring :
		{
			free(str->stringx)  ;
			free(str)			;
		}
	}

	return NULL ;
}


PDX_STRING dx_string_createA(PDX_STRING dxstr , char * str)
{
    if (str == NULL)return NULL ;
	if (dxstr != NULL)
	{
	  /*the string is already created , we will re allocated it*/

		dxstr->len     = StrLenA(str)  ;
       //release the memory
        if (dxstr->type == dx_wide)
        free(dxstr->stringw) ;
        else
		if(dxstr->type == dx_wstring)
			free(dxstr->stringx);
		else
          free(dxstr->stringa) ;

		dxstr->stringa =	(char*)malloc(dxstr->len+1) ; // c string style to be compatible with some C routines
		if (dxstr->stringa == NULL) return NULL ;
		memcpy(dxstr->stringa , str , dxstr->len)   ;
		dxstr->stringa[dxstr->len] = 0 ;

		dxstr->stringw = NULL      ;
		dxstr->stringx = NULL      ;
		dxstr->type   = dx_ansi   ;
		dxstr->bcount  = dxstr->len ;
		return dxstr ;
	}

	PDX_STRING nstr =  (PDX_STRING) malloc(sizeof(struct dx_string)) ;
	if (nstr == NULL) return NULL ;
	nstr->len     = StrLenA(str)  ;

	nstr->stringa =	(char*)malloc(nstr->len+1) ; // c string style to be compatible with some C routines
	memcpy(nstr->stringa , str , nstr->len)   ;
	nstr->stringa[nstr->len] = 0 ;

	nstr->stringw = NULL      ;
	nstr->stringx = NULL      ;
	nstr->type   = dx_ansi   ;
	nstr->bcount  = nstr->len ;
	return nstr ;
}



PDX_STRING dx_string_createW(PDX_STRING dxstr , wchar_t * str)
{
	if (str == NULL)return NULL ;
	if (dxstr != NULL)
	{
	  /*the string is already created , wee will re allocated it*/

        if (dxstr->type == dx_wide)
        free(dxstr->stringw) ;
        else
		if(dxstr->type == dx_wstring)
			free(dxstr->stringx);
		else
          free(dxstr->stringa) ;

		dxstr->len     = StrLenW(str)  ;
        dxstr->bcount  = dxstr->len * sizeof(wchar_t) ;

		dxstr->stringw =	(wchar_t*) malloc(dxstr->bcount+sizeof(wchar_t)) ; // c string style to be compatible with some C routines
		memcpy(dxstr->stringw , str , dxstr->bcount)   		    ;
		dxstr->stringw[dxstr->len] = 0 							;

		dxstr->stringa = NULL      ;
		dxstr->stringx = NULL      ;
		dxstr->type   = dx_wide    ;

		return dxstr ;
	}

	PDX_STRING nstr =  (PDX_STRING) malloc(sizeof(struct dx_string)) ;
	if (nstr == NULL) return NULL ;
	nstr->len     = StrLenW(str)  ;
	nstr->bcount  = nstr->len * sizeof(wchar_t) ;

	nstr->stringw =	(wchar_t*) malloc(nstr->bcount+sizeof(wchar_t)) ; // c string style to be compatible with some C routines
	memcpy(nstr->stringw , str , nstr->bcount)   		    ;
	nstr->stringw[nstr->len] = 0 							;

	nstr->stringa = NULL      ;
	nstr->stringx = NULL	  ;
	nstr->type   = dx_wide    ;
	return nstr ;
}

PDX_STRING dx_string_createU(PDX_STRING dxstr , const char * str)
{
    if (str == NULL)return NULL ;
  /*we will treat a utf8 string as a simple byte array*/

	if (dxstr != NULL)
	{
	  /*the string is already created , we will re allocated it*/

        if (dxstr->type == dx_wide)
        free(dxstr->stringw) ;
        else
		if(dxstr->type == dx_wstring)
			free(dxstr->stringx);
		else
          free(dxstr->stringa) ;

		dxstr->len     = StrLenU(str)  ;
		dxstr->bcount  = StrLenA(str) ;

		dxstr->stringa =	(char*)malloc(dxstr->bcount+1) ; // c string style to be compatible with the C routines
		if (dxstr == NULL) return NULL ;
		memcpy(dxstr->stringa , str , dxstr->bcount)   ;
		dxstr->stringa[dxstr->bcount] = 0 ;

		dxstr->stringw = NULL      ;
		dxstr->stringx = NULL	   ;
		dxstr->type    = dx_utf8   ;
		return dxstr ;
	}

	PDX_STRING nstr =  (PDX_STRING) malloc(sizeof(struct dx_string)) ;
	if (nstr == NULL) return NULL ;
	nstr->len     = StrLenU(str)  ;
	nstr->bcount  = StrLenA(str) ;

	nstr->stringa =	(char*)malloc(nstr->bcount+1) ; // c string style to be compatible with some C routines
	memcpy(nstr->stringa , str , nstr->bcount)   ;
	nstr->stringa[nstr->bcount] = 0 ;

	nstr->stringw = NULL      ;
	nstr->stringx = NULL	  ;
	nstr->type   = dx_utf8    ;
	return nstr ;
}

PDX_STRING dx_string_createX(PDX_STRING dxstring, DXCHAR * str)
{
   if (str == NULL) return NULL ;

	if (dxstring != NULL)
	{
	  /*the string is already created , we will re allocated it*/

        if (dxstring->type == dx_wide)
        free(dxstring->stringw) ;
        else
		 if(dxstring->type == dx_wstring)
		 free(dxstring->stringx);
		 else
          free(dxstring->stringa) ;

		dxstring->len     = StrLenX(str)  ;
		dxstring->bcount  = dxstring->len * sizeof(DXCHAR);

		dxstring->stringx =	(DXCHAR*)malloc(dxstring->bcount+sizeof(DXCHAR)) ; // NULL terminating
		if (dxstring == NULL) return NULL ;
		memcpy(dxstring->stringx , str , dxstring->bcount)   ;
		dxstring->stringx[dxstring->len] = 0 ;

		dxstring->stringw = NULL         ;
		dxstring->stringa = NULL	     ;
		dxstring->type    = dx_wstring   ;
		return dxstring					 ;
	}

	PDX_STRING nstr =  (PDX_STRING) malloc(sizeof(struct dx_string)) ;
	if (nstr == NULL) return NULL ;
	nstr->len     = StrLenX(str)  ;
	nstr->bcount  = nstr->len * sizeof(DXCHAR) ;

	nstr->stringx =	(DXCHAR*)malloc(nstr->bcount+sizeof(DXCHAR)) ; 
	memcpy(nstr->stringx , str , nstr->bcount)   ;
	nstr->stringx[nstr->len] = 0				 ;

	nstr->stringw = NULL      ;
	nstr->stringa = NULL	  ;
	nstr->type   = dx_wstring ;
	return nstr ;
}


PDX_STRING dx_string_createX_a(PDX_STRING dxstring, char * str)
{
   if (str == NULL) return NULL ;

	if (dxstring != NULL)
	{
	  /*the string is already created , we will re allocated it*/

        if (dxstring->type == dx_wide)
        free(dxstring->stringw) ;
        else
		 if(dxstring->type == dx_wstring)
		 free(dxstring->stringx);
		 else
          free(dxstring->stringa) ;

		dxstring->len     = StrLenA(str)  ;
		dxstring->bcount  = dxstring->len * sizeof(DXCHAR);

		dxstring->stringx =	(DXCHAR*)malloc(dxstring->bcount+sizeof(DXCHAR)) ; // NULL terminating
		if (dxstring == NULL) return NULL ;
		
		if(dxstring->len == 0)
		{
			dxstring->stringx[0]= 0 ;
			return dxstring ;
		}

        /*convert the ansi to widechars*/
	    wchar_t * buff = dx_ConvertAnsiToW(str) ;
		wchar_t *buffindx = buff ;
		DXCHAR * strindx = dxstring->stringx ;
		while(*buffindx != 0)
		{
		   *strindx = (DXCHAR)(*buffindx) ;
		   strindx++ ;
		   buffindx++;
		}
		free(buff); /*the memory is not needed any more*/

		dxstring->stringx[dxstring->len] = 0 ;/*zero terminating*/

		dxstring->stringw = NULL         ;
		dxstring->stringa = NULL	     ;
		dxstring->type    = dx_wstring   ;
		return dxstring					 ;
	}

	PDX_STRING nstr =  (PDX_STRING) malloc(sizeof(struct dx_string)) ;
	if (nstr == NULL) return NULL ;
	nstr->len     = StrLenA(str)  ;
	nstr->bcount  = nstr->len * sizeof(DXCHAR) ;

	nstr->stringx =	(DXCHAR*)malloc(nstr->bcount+sizeof(DXCHAR)) ; 
	
	if (nstr->stringx == NULL) return NULL ;
	if(nstr->len == 0)
	{
		nstr->stringx[0]= 0 ;
		return nstr ;
	}	

    /*convert the ansi to widechars*/
	wchar_t * buff = dx_ConvertAnsiToW(str) ;
	wchar_t *buffindx = buff ;
	DXCHAR * strindx = nstr->stringx ;
	while(*buffindx != 0)
	{
		*strindx = (DXCHAR)(*buffindx) ;
		strindx++ ;
		buffindx++;
	}
	free(buff); /*the memory is not needed any more*/


	nstr->stringx[nstr->len] = 0 ;

	nstr->stringw = NULL      ;
	nstr->stringa = NULL	  ;
	nstr->type   = dx_wstring ;
	return nstr				  ;
}

PDX_STRING dx_string_createX_w(PDX_STRING dxstring, wchar_t * str)
{
   if (str == NULL) return NULL ;

	if (dxstring != NULL)
	{
	  /*the string is already created , we will re allocated it*/

        if (dxstring->type == dx_wide)
        free(dxstring->stringw) ;
        else
		 if(dxstring->type == dx_wstring)
		 free(dxstring->stringx);
		 else
          free(dxstring->stringa) ;

		dxstring->len     = StrLenW(str)  ;
		dxstring->bcount  = dxstring->len * sizeof(DXCHAR);

		dxstring->stringx =	(DXCHAR*)malloc(dxstring->bcount+sizeof(DXCHAR)) ; // NULL terminating
		if (dxstring == NULL) return NULL ;

		if(dxstring->len == 0)
		{
			dxstring->stringx[0]= 0 ;
			return dxstring ;
		}

		wchar_t *buffindx = str ;
		DXCHAR * strindx = dxstring->stringx ;
		while(*buffindx != 0)
		{
		   *strindx = (DXCHAR)(*buffindx) ;
		   strindx++ ;
		   buffindx++;
		}
		
		dxstring->stringx[dxstring->len] = 0 ;/*zero terminating*/

		dxstring->stringw = NULL         ;
		dxstring->stringa = NULL	     ;
		dxstring->type    = dx_wstring   ;
		return dxstring					 ;
	}

	PDX_STRING nstr =  (PDX_STRING) malloc(sizeof(struct dx_string)) ;
	if (nstr == NULL) return NULL ;
	nstr->len     = StrLenW(str)  ;
	nstr->bcount  = nstr->len * sizeof(DXCHAR) ;

	nstr->stringx =	(DXCHAR*)malloc(nstr->bcount+sizeof(DXCHAR)) ; 
	
	if (nstr->stringx == NULL) return NULL ;
	if(nstr->len == 0)
	{
		nstr->stringx[0]= 0 ;
		return nstr ;
	}

	wchar_t *buffindx = str ;
	DXCHAR * strindx = nstr->stringx ;
	while(*buffindx != 0)
	{
		*strindx = (DXCHAR)(*buffindx) ;
		strindx++ ;
		buffindx++;
	}

	nstr->stringx[nstr->len] = 0 ;

	nstr->stringw = NULL      ;
	nstr->stringa = NULL	  ;
	nstr->type   = dx_wstring ;
	return nstr				  ;
}

PDX_STRING dx_string_createX_u(PDX_STRING dxstring, char * str)
{
  if (str == NULL) return NULL ;

	if (dxstring != NULL)
	{
	  /*the string is already created , we will re allocated it*/

        if (dxstring->type == dx_wide)
        free(dxstring->stringw) ;
        else
		 if(dxstring->type == dx_wstring)
		 free(dxstring->stringx);
		 else
          free(dxstring->stringa) ;

		dxstring->len     = StrLenU(str)  ;
		dxstring->bcount  = dxstring->len * sizeof(DXCHAR);

		dxstring->stringx =	(DXCHAR*)malloc(dxstring->bcount+sizeof(DXCHAR)) ; // NULL terminating
		if (dxstring == NULL) return NULL ;
		
		if(dxstring->len == 0)
		{
			dxstring->stringx[0]= 0 ;
			return dxstring ;
		}

        /*convert the utf8 to widechars*/
	    wchar_t * buff = dx_ConvertUtf8ToW(str) ;
		wchar_t *buffindx = buff ;
		DXCHAR * strindx = dxstring->stringx ;
		while(*buffindx != 0)
		{
		   *strindx = (DXCHAR)(*buffindx) ;
		   strindx++ ;
		   buffindx++;
		}
		free(buff); /*the memory is not needed any more*/

		dxstring->stringx[dxstring->len] = 0 ;/*zero terminating*/

		dxstring->stringw = NULL         ;
		dxstring->stringa = NULL	     ;
		dxstring->type    = dx_wstring   ;
		return dxstring					 ;
	}

	PDX_STRING nstr =  (PDX_STRING) malloc(sizeof(struct dx_string)) ;
	if (nstr == NULL) return NULL ;
	nstr->len     = StrLenU(str)  ;
	nstr->bcount  = nstr->len * sizeof(DXCHAR) ;

	nstr->stringx =	(DXCHAR*)malloc(nstr->bcount+sizeof(DXCHAR)) ; 
	
	if (nstr->stringx == NULL) return NULL ;
		
	if(nstr->len == 0)
	{
		nstr->stringx[0]= 0 ;
		return nstr ;
	}
    /*convert the utf8 to widechars*/
	wchar_t * buff = dx_ConvertUtf8ToW(str) ;
	wchar_t *buffindx = buff ;
	DXCHAR * strindx = nstr->stringx ;
	while(*buffindx != 0)
	{
		*strindx = (DXCHAR)(*buffindx) ;
		strindx++ ;
		buffindx++;
	}
	free(buff); /*the memory is not needed any more*/


	nstr->stringx[nstr->len] = 0 ;

	nstr->stringw = NULL      ;
	nstr->stringa = NULL	  ;
	nstr->type   = dx_wstring ;
	return nstr				  ;

}


PDX_STRING dx_string_create_bA(char *buf)
{
	PDX_STRING nstr =  (PDX_STRING) malloc(sizeof(struct dx_string)) ;
	if (nstr == NULL) return NULL ;
	nstr->len     = StrLenA(buf)  ;

	nstr->stringa =	buf; // c string style to be compatible with some C routines

	nstr->stringw = NULL      ;
	nstr->stringx = NULL      ;
	nstr->type   = dx_ansi    ;
	nstr->bcount  = nstr->len ;
	return nstr ;
}

PDX_STRING dx_string_create_bU(char *buf)
{
	PDX_STRING nstr =  (PDX_STRING) malloc(sizeof(struct dx_string)) ;
	if (nstr == NULL) return NULL ;
	nstr->len     = StrLenU(buf)  ;
	nstr->bcount  = StrLenA(buf) ;

	nstr->stringa =	buf ;

	nstr->stringw = NULL      ;
	nstr->stringx = NULL	  ;
	nstr->type   = dx_utf8    ;
	return nstr ;
}


PDX_STRING dx_string_create_bW(wchar_t *buf) 
{
	PDX_STRING nstr =  (PDX_STRING) malloc(sizeof(struct dx_string)) ;
	if (nstr == NULL) return NULL ;
	nstr->len     = StrLenW(buf)  ;
	nstr->bcount  = nstr->len * sizeof(wchar_t) ;

	nstr->stringw =	buf								;
	nstr->stringa = NULL      ;
	nstr->stringx = NULL	  ;
	nstr->type   = dx_wide    ;
	return nstr ;
}

PDX_STRING dx_string_create_bX(DXCHAR *buf)
{
	
	PDX_STRING nstr =  (PDX_STRING) malloc(sizeof(struct dx_string)) ;
	if (nstr == NULL) return NULL ;
	nstr->len     = StrLenX(buf)  ;
	nstr->bcount  = nstr->len * sizeof(DXCHAR) ;

	nstr->stringx =	buf ; 

	nstr->stringw = NULL      ;
	nstr->stringa = NULL	  ;
	nstr->type   = dx_wstring ;
	return nstr ;
}

void dx_concat_codepoint(PDX_STRING dxstring,DXCHAR ch)
{

	if(dxstring->type != dx_wstring) return ;
	DXCHAR * buff = (DXCHAR*)malloc((dxstring->len+2)*sizeof(DXCHAR)) ; /*+1 for the new codepoint +1 for the zero terminating */
	
	for(DXLONG i = 0 ; i < dxstring->len;i++)
	buff[i] = dxstring->stringx[i] ;
	
	buff[dxstring->len]   = ch ;
	buff[dxstring->len+1] = 0  ;
	
	free(dxstring->stringx)				;
	dxstring->len = dxstring->len + 1   ;
	dxstring->stringx = buff			;
	return ;
}



PDX_STRING dx_string_setA(PDX_STRING dxstr , char * str)
{
	if((dxstr == NULL)||(str == NULL)) return NULL ;
	dxstr->len     = StrLenA(str)   ;
	if (dxstr->stringa != NULL)
		free(dxstr->stringa)		;
	if (dxstr->stringw != NULL)
		free(dxstr->stringw)		;
	if (dxstr->stringx != NULL)
		free(dxstr->stringx)		;
	dxstr->stringa = str  			;
	dxstr->stringw = NULL      		;
	dxstr->stringx = NULL      		;
	dxstr->type   = dx_ansi   		;
	dxstr->bcount  = dxstr->len 	;
	return dxstr 					;
}

PDX_STRING dx_string_setW(PDX_STRING dxstr , wchar_t * str)
{
	if((dxstr == NULL)||(str == NULL)) return NULL ;
	dxstr->len     = StrLenW(str)   ;
	if (dxstr->stringa != NULL)
		free(dxstr->stringa)		;
	if (dxstr->stringw != NULL)
		free(dxstr->stringw)		;
	if (dxstr->stringx != NULL)
		free(dxstr->stringx)		;
	dxstr->stringw = str  			;
	dxstr->stringa = NULL      		;
	dxstr->stringx = NULL      		;
	dxstr->type    = dx_wide   		;
	dxstr->bcount  = dxstr->len * sizeof(wchar_t)  	;
	return dxstr 					;
}

PDX_STRING dx_string_setU(PDX_STRING dxstr , char * str)
{
	if((dxstr == NULL)||(str == NULL)) return NULL ;
	dxstr->len     = StrLenU(str)   ;
	if (dxstr->stringa != NULL)
	free(dxstr->stringa)			;
	if (dxstr->stringw != NULL)
		free(dxstr->stringw)		;
	if (dxstr->stringx != NULL)
		free(dxstr->stringx)		;
	dxstr->stringa = str  			;
	dxstr->stringw = NULL			;
	dxstr->stringx = NULL      		;
	dxstr->type    = dx_utf8   		;
	dxstr->bcount  = StrLenA(str) 	;
	return dxstr 					;
}

PDX_STRING dx_string_setX(PDX_STRING dxstr , DXCHAR * str )   
{
	if((dxstr == NULL)||(str == NULL)) return NULL ;
	dxstr->len     = StrLenX(str)   ;
	if (dxstr->stringa != NULL)
	free(dxstr->stringa)			;
	if (dxstr->stringw != NULL)
		free(dxstr->stringw)		;
	if (dxstr->stringx != NULL)
		free(dxstr->stringx)		;

	dxstr->stringx = str  			;
	dxstr->stringw = NULL      		;
	dxstr->stringa = NULL      		;
	dxstr->type    = dx_wstring 	;
	dxstr->bcount  = dxstr->len * sizeof(DXCHAR);
	return dxstr 					;
}


PDX_STRING dx_string_create_uninit(enum dx_string_type type , size_t len )
{
	if (len <= 0) return NULL ;
    PDX_STRING nstr =  (PDX_STRING) malloc(sizeof(struct dx_string)) ;
	if (nstr == NULL) return NULL ;
	switch(type)
	{
      case dx_ansi :
	  {
	    nstr->len     = len  ;
	    nstr->stringa =	(char*)malloc(nstr->len+1) ; // C string style to be compatible with some C routines
	    nstr->stringa[nstr->len] = 0 ;
	    nstr->stringw = NULL         ;
		nstr->stringx = NULL         ;
	    nstr->type    = dx_ansi      ;
	    nstr->bcount  = nstr->len    ;
	  }
	  break ;
	  case dx_utf8 :
	  {
		nstr->len     = len  ;
	    nstr->stringa =	(char*)malloc(nstr->len+1) ; // C string style to be compatible with some C routines
	    nstr->stringa[nstr->len] = 0 ;
	    nstr->stringw = NULL         ;
		nstr->stringx = NULL         ;
	    nstr->bcount  = nstr->len    ;
		nstr->type    = dx_utf8      ;
	  }
	  break ;
	  case dx_wide :
	  {
        nstr->len     = len  ;
	    nstr->stringw =	(wchar_t*)malloc(sizeof(wchar_t)*(nstr->len+1)) ; // C string style to be compatible with some C routines
	    nstr->stringw[nstr->len] = 0 ;
	    nstr->stringa = NULL         ;
		nstr->stringx = NULL         ;
	    nstr->type    = dx_wide      ;
	    nstr->bcount  = nstr->len * sizeof(wchar_t)    ;
	  }
	  break;
	   case dx_wstring :
	  {
        nstr->len     = len  ;
	    nstr->stringx =	(DXCHAR*)malloc(sizeof(DXCHAR)*(nstr->len+1)) ;
	    nstr->stringx[nstr->len] = 0 ;
	    nstr->stringa = NULL         ;
	    nstr->stringw = NULL         ;
		nstr->type    = dx_wstring   ;
	    nstr->bcount  = nstr->len * sizeof(DXCHAR)    ;
	  }
	  break;
	  case dx_null : ;break;
	}

	return nstr ;
}

PDX_STRING dx_string_convertU(PDX_STRING str)
{
	if (str == NULL) return NULL ;

	switch(str->type)
	{
		case dx_ansi :
		{
		  PDX_STRING nstr  = dx_string_createU(NULL, "");
		  if (nstr == NULL) return NULL ;
		  return dx_string_setU(nstr ,dx_ConvertAnsiToUtf8(str->stringa ));
		}
		break ;
		case dx_wide :
		{
		 PDX_STRING nstr  = dx_string_createU(NULL, "");
		 if (nstr == NULL) return NULL ;
		 return dx_string_setU(nstr ,dx_ConvertWToUtf8(str->stringw));
		}
		break;
		case dx_utf8 :
		{
		  /*just copy the string*/
		  return dx_string_createU(NULL,str->stringa);
		}break;
		case dx_wstring :
		{
		  PDX_STRING nstr  = dx_string_createU(NULL, "");
		  if (nstr == NULL) return NULL ;
		  return dx_string_setU(nstr ,dx_ConvertDXSToUtf8(str->stringx));
		}
		break ;
		case dx_null : ;break;
	}

	return NULL ;
}

PDX_STRING dx_string_convertW(PDX_STRING str)
{
	if (str == NULL) return NULL ;

	switch(str->type)
	{
		case dx_ansi :
		{
		  PDX_STRING nstr  = dx_string_createW(NULL, L"");
		  if (nstr == NULL) return NULL ;
		  return dx_string_setW(nstr ,dx_ConvertAnsiToW(str->stringa));
		}
		break ;
		case dx_utf8 :
		{
		 PDX_STRING nstr  = dx_string_createW(NULL,L"");
		 if (nstr == NULL) return NULL ;
		 return dx_string_setW(nstr ,dx_ConvertUtf8ToW(str->stringa));
		}
		break;
		case dx_wide :
		{
		  /*just copy the string*/
		  return dx_string_createW(NULL,str->stringw) ;
		}
		break ;
		case dx_wstring :
		{
		  PDX_STRING nstr  = dx_string_createW(NULL, L"");
		  if (nstr == NULL) return NULL ;
		  return dx_string_setW(nstr ,dx_ConvertDXSToWide(str->stringx));
		}
		break ;
		case dx_null : ;break;
	}

	return NULL ;
}

PDX_STRING dx_string_convertX(PDX_STRING str)
{
	if (str == NULL) return NULL ;
	DXCHAR buff[1] ;
	buff[0] = 0 ;
	switch(str->type)
	{
		case dx_ansi :
		{
		  PDX_STRING nstr  = dx_string_createX(NULL, buff);
		  if (nstr == NULL) return NULL ;
		  return dx_string_setX(nstr ,dx_ConvertAnsiToDXS(str->stringa));
		}
		break ;
		case dx_utf8 :
		{
		 PDX_STRING nstr  = dx_string_createX(NULL,buff);
		 if (nstr == NULL) return NULL ;
		 return dx_string_setX(nstr ,dx_ConvertUtf8ToDXS(str->stringa));
		}
		break;
		case dx_wide :
		{
			PDX_STRING nstr  = dx_string_createX(NULL,buff);
			if (nstr == NULL) return NULL ;
			return dx_string_setX(nstr ,dx_ConvertWideToDXS(str->stringw));
		}
		break ;
		case dx_wstring :
		{
		  /*just copy the string*/
		  return dx_string_createX(NULL,str->stringx) ;
		}
		break ;
		case dx_null : ;break;
	}

	return NULL ;
}

PDX_STRING dx_string_concat(const PDX_STRING str1 , const PDX_STRING str2)
{
 /*
  For transparency we will permit diferent types of string to be concatenated.
  The resulting string type is determined from the str1 type.
  An empty string e.g a string with the 0 character as the first character
  will be used as the one to set the encoding but of course will be ommited
 */

  if ((str1 == NULL)&&(str2 == NULL)) return NULL ;
  if ((str1 == NULL)||(str1->len ==0)) return dx_string_clone(str2) ;
  if ((str2 == NULL)||(str2->len ==0)) return dx_string_clone(str1) ;

  switch(str1->type)
  {
    case dx_ansi :
	{
		switch(str2->type)
		{
			case dx_ansi :
			{
				//ansi on ansi sweet!
				//create a big enough string
				PDX_STRING nstr = dx_string_create_uninit(dx_ansi , str1->len + str2->len ) ;
				//copy the first string
				char *tindx = nstr->stringa ;
				memcpy(tindx , str1->stringa , str1->len) ;
				//copy the second one
				tindx = &(nstr->stringa[str1->len]) ;
				memcpy(tindx,str2->stringa,str2->len);
				//the terminating 0 is already set by the dx_string_create_uninit
				return nstr ;
			}
			break;
			case dx_wide :
			{
				//not permited!
				printf("Warning : The concatenation between a widestring and an ansi string is not permited.\n");
				return NULL ;
			}
			break;
			case dx_utf8 :
			{
				//not permited
				printf("Warning : The concatenation between a utf8 string and an ansi string is not permited.\n");
				return NULL ;
			}
			break;
			case dx_wstring :
			{
				//not permited
				printf("Warning : The concatenation between a DXCHAR string and an ansi string is not permited.\n");
				return NULL ;
			}
			break;
			case dx_null : ;break;
		}
	}
	break;
	case dx_wide :
	{
		switch(str2->type)
		{
			case dx_ansi :
			{
				//wide + ansi
				//create a big enough string
				PDX_STRING nstr = dx_string_create_uninit(dx_wide , str1->len + str2->len ) ;
				//copy the first string
				char *tindx = (char*)nstr->stringw ;
				memcpy(tindx , (char*)str1->stringw , (str1->len * sizeof(wchar_t))) ;
				//convert the ansi to widestring

				wchar_t *twchar = dx_ConvertAnsiToW(str2->stringa) ;
				if (twchar == NULL)
				{
                  dx_string_free(nstr);
				  return NULL ;
				}
				//copy the second one
				tindx = (char*)&(nstr->stringw[str1->len]) ;
				memcpy(tindx,(char*)twchar,(str2->len)*sizeof(wchar_t));
				//the terminating 0 is already set by the dx_string_create_uninit
				free(twchar);
				return nstr ;
			}
			break;
			case dx_wide :
			{
				//wide on wide sweet!
				//create a big enough string
				PDX_STRING nstr = dx_string_create_uninit(dx_wide , str1->len + str2->len ) ;
				//copy the first string
				char *tindx = (char*)nstr->stringw        ;
				memcpy(tindx , (char*)str1->stringw , (str1->len)*sizeof(wchar_t)) ;
				//copy the second one
				tindx = (char*)&(nstr->stringw[str1->len]) ;
				memcpy(tindx,(char*)str2->stringw,(str2->len*sizeof(wchar_t)));
				//the terminating 0 is already set by the dx_string_create_uninit
				return nstr ;
			}
			break;
			case dx_utf8 :
			{
				//wide + utf8
				//first convert utf8 to wide string
				wchar_t *wtchar = dx_ConvertUtf8ToW(str2->stringa) ;
				if (wtchar == NULL) return NULL ;

				//create a big enough string
				PDX_STRING nstr = dx_string_create_uninit(dx_wide , str1->len + str2->len ) ;
				//copy the first string
				char *tindx = (char*)nstr->stringw        ;
				memcpy(tindx , (char*)str1->stringw , (str1->len)*sizeof(wchar_t)) ;
				//copy the second one
				tindx = (char*)&(nstr->stringw[str1->len]) ;
				memcpy(tindx,(char*)wtchar,(str2->len*sizeof(wchar_t)));
				//the terminating 0 is already set by the dx_string_create_uninit
				free(wtchar) ;
				return nstr  ;
			}
			break ;
			case dx_wstring :
			{
			    //wide on wstring 
				//create a big enough string
				PDX_STRING nstr = dx_string_create_uninit(dx_wide , str1->len + str2->len ) ;
				//copy the first string
				char *tindx = (char*)nstr->stringw        ;
				memcpy(tindx , (char*)str1->stringw , (str1->len)*sizeof(wchar_t)) ;
				//copy the second one
				wchar_t * nindx   = (wchar_t*)&(nstr->stringx[str1->len]) ;
				DXCHAR  *sec_indx = str2->stringx ;  
				while(*sec_indx != 0)
				{
					*nindx = (wchar_t)*sec_indx ;
					nindx++;
					sec_indx++ ;
				}
				//the terminating 0 is already set by the dx_string_create_uninit
				return nstr ;
			}
			break ;
            case dx_null : ;break;
		}
	}
	break ;
	case dx_utf8 :
	{
		switch(str2->type)
		{
			case dx_ansi :
			{
				//utf8 + ansi

			    //convert the ansi to utf8
				char *tuchar = dx_ConvertAnsiToUtf8(str2->stringa) ;
				if (tuchar == NULL) return NULL ;

				//create a big enough string
				//for the utf8 we have to get the bcount
				PDX_STRING nstr = dx_string_create_uninit(dx_utf8 , str1->bcount + StrLenA(tuchar) ) ;
				//copy the first string
				char *tindx = nstr->stringa ;
				memcpy(tindx , str1->stringa , str1->bcount) ;

				//copy the second one
				tindx = &(nstr->stringa[str1->bcount]) ;
				memcpy(tindx,tuchar,StrLenA(tuchar)); // just count bytes until 0
				//the terminating 0 is already set by the dx_string_create_uninit
				free(tuchar);
				return nstr ;
			}
			break;
			case dx_wide :
			{
				//utf8 + wide
				//create a big enough string
				//first we have to convert the wide to utf8

				//convert the wide to utf8
				char *tuchar = dx_ConvertWToUtf8(str2->stringw) ;
				if (tuchar == NULL) return NULL ;


				//for the utf8 we have to get the bcount
				PDX_STRING nstr = dx_string_create_uninit(dx_utf8 , str1->bcount + StrLenA(tuchar)) ;
				//copy the first string
				char *tindx = nstr->stringa ;
				memcpy(tindx , str1->stringa , str1->bcount) ;

				//copy the second one
				tindx = &nstr->stringa[str1->bcount] ;
				memcpy(tindx,tuchar,StrLenA(tuchar)); // just count bytes until 0
				//the terminating 0 is already set by the dx_string_create_uninit
				free(tuchar);
				return nstr ;
			}
			break;
			case dx_utf8 :
			{
				//utf8 to utf8 sweet!

				//create a big enough string
				//for the utf8 we have to get the bcount
				PDX_STRING nstr = dx_string_create_uninit(dx_utf8 , str1->bcount + str2->bcount ) ;
				//copy the first string
				char *tindx = nstr->stringa ;
				memcpy(tindx , str1->stringa , str1->bcount) ;

				//copy the second one
				tindx = &nstr->stringa[str1->bcount] ;
				memcpy(tindx,str2->stringa,str2->bcount);
				//the terminating 0 is already set by the dx_string_create_uninit
				return nstr ;
			}
			break ;
			case dx_wstring :
			{
				//utf8 + wstring
				//create a big enough string
				//first we have to convert the DXCHAR string to utf8

				//convert the wstring to utf8
				char *tuchar = dx_ConvertDXSToUtf8(str2->stringx) ;
				if (tuchar == NULL) return NULL ;


				//for the utf8 we have to get the bcount
				PDX_STRING nstr = dx_string_create_uninit(dx_utf8 , str1->bcount + StrLenA(tuchar)) ;
				//copy the first string
				char *tindx = nstr->stringa ;
				memcpy(tindx , str1->stringa , str1->bcount) ;

				//copy the second one
				tindx = &nstr->stringa[str1->bcount] ;
				memcpy(tindx,tuchar,StrLenA(tuchar)); // just count bytes until 0
				//the terminating 0 is already set by the dx_string_create_uninit
				free(tuchar);
				return nstr ;

			}
			case dx_null : ;break;
		}
	}
	break ;
	case dx_wstring :
	{
		switch(str2->type)
		{
			case dx_ansi :
			{
				//wstring + ansi
				//create a big enough string
				PDX_STRING nstr = dx_string_create_uninit(dx_wstring , str1->len + str2->len ) ;
				//copy the first string
				char *tindx = (char*)nstr->stringx ;
				memcpy(tindx , (char*)str1->stringx , (str1->len * sizeof(DXCHAR))) ;
				//convert the ansi to widestring

				wchar_t *twchar = dx_ConvertAnsiToW(str2->stringa) ;
				if (twchar == NULL)
				{
                  dx_string_free(nstr);
				  return NULL ;
				}
				//copy the second one
				DXCHAR *dxindx = (DXCHAR*)(char*)&(nstr->stringx[str1->len]) ;
				for(DXLONG i = 0 ; i < str2->len;i++)
				{
					*dxindx =  (DXCHAR)twchar[i] ;
					dxindx++ ;
				}
				//the terminating 0 is already set by the dx_string_create_uninit
				free(twchar);
				return nstr ;
			}
			break;
			case dx_wide :
			{
				//wstring on wide
				//create a big enough string
				PDX_STRING nstr = dx_string_create_uninit(dx_wstring , str1->len + str2->len ) ;
				//copy the first string
				char *tindx = (char*)nstr->stringx        ;
				memcpy(tindx , (char*)str1->stringx , (str1->len)*sizeof(DXCHAR)) ;
				//copy the second one
				DXCHAR *dxindx = (DXCHAR*)(char*)&(nstr->stringx[str1->len]) ;
				for(DXLONG i = 0 ; i < str2->len;i++)
				{
					*dxindx =  (DXCHAR)str2->stringw[i] ;
					dxindx++ ;
				}
				//the terminating 0 is already set by the dx_string_create_uninit
				return nstr ;
			}
			break;
			case dx_utf8 :
			{
				//wide + utf8
				//first convert utf8 to wide string
				wchar_t *wtchar = dx_ConvertUtf8ToW(str2->stringa) ;
				if (wtchar == NULL) return NULL ;

				//create a big enough string
				PDX_STRING nstr = dx_string_create_uninit(dx_wstring , str1->len + str2->len ) ;
				//copy the first string
				char *tindx = (char*)nstr->stringx        ;
				memcpy(tindx , (char*)str1->stringx , (str1->len)*sizeof(DXCHAR)) ;
				//copy the second one
				DXCHAR *dxindx = (DXCHAR*)(char*)&(nstr->stringx[str1->len]) ;
				for(DXLONG i = 0 ; i < str2->len;i++)
				{
					*dxindx =  (DXCHAR)wtchar[i] ;
					dxindx++ ;
				}
				//the terminating 0 is already set by the dx_string_create_uninit
				free(wtchar) ;
				return nstr  ;
			}
			break ;
			case dx_wstring :
			{
			    //wstring on wstring 
				//create a big enough string
				PDX_STRING nstr = dx_string_create_uninit(dx_wstring , str1->len + str2->len ) ;
				//copy the first string
				char *tindx = (char*)nstr->stringx        ;
				memcpy(tindx , (char*)str1->stringx , (str1->len)*sizeof(DXCHAR)) ;
				//copy the second one
				DXCHAR * nindx   = (DXCHAR*)&(nstr->stringx[str1->len]) ;
				DXCHAR  *sec_indx = str2->stringx ;  
				while(*sec_indx != 0)
				{
					*nindx = *sec_indx ;
					nindx++;
					sec_indx++ ;
				}
				//the terminating 0 is already set by the dx_string_create_uninit
				return nstr ;
			}
			break ;
            case dx_null : ;break;
		}
	}
	case dx_null : ;break;
  }

 return NULL ;
}

PDX_STRING dx_string_copy(PDX_STRING str1 , const PDX_STRING str2)
{
	if (str2 == NULL) return NULL ;

	if (str1 == NULL)
	{
		str1 = (PDX_STRING)malloc(sizeof(struct dx_string)) ;
		str1->type = dx_null ;
		str1->stringa = NULL ;
		str1->stringw = NULL ;
		str1->stringx = NULL ;
	}

		str1->len    = 0 ;
		str1->bcount = 0 ;

		switch(str1->type)
		{
			case dx_ansi : {free(str1->stringa);str1->stringa = NULL;}
			break ;
			case dx_utf8 : {free(str1->stringa);str1->stringa = NULL;}
			break ;
			case dx_wide : {free(str1->stringw);str1->stringw = NULL;}
			break ;
			case dx_wstring : {free(str1->stringx);str1->stringx = NULL;}
			break ;
			case dx_null : ;break;
		}

		str1->type = str2->type ;
		switch(str1->type)
		{
			case dx_ansi :
			{
			 	if (str2->len == 0) {str1->stringa = (char *)malloc(1);str1->stringa[0]=0;return str1;}
			    /*if not zero then do the actual copy*/
			    str1->stringa = (char *)malloc(str2->len+1) ;
				memcpy(str1->stringa,str2->stringa,str2->len);
				str1->stringa[str2->len] = 0 ;
				str1->len    = str2->len ;
				str1->bcount = str2->len ;
				return str1 ;
			}
			break ;
			case dx_wide :
			{
				if (str2->len == 0) {str1->stringw = (wchar_t*)malloc(sizeof(wchar_t));str1->stringw[0]=0;return str1;}
				/*if not zero then do the actual copy*/
				str1->stringw = (wchar_t *)malloc(sizeof(wchar_t)*(str2->len+1)) ;
				memcpy(str1->stringw,str2->stringw,str2->bcount);
				str1->stringw[str2->len] = 0 ;
				str1->len    = str2->len ;
				str1->bcount = str2->bcount ;
				return str1 ;
			}
			break;
			case dx_utf8 :
			{
				if (str2->len == 0) {str1->stringa = (char*)malloc(1);str1->stringa[0]=0;return str1;}
				/*if not null then do the actual copy*/
				str1->stringa = (char *)malloc(str2->bcount+1) ;
				memcpy(str1->stringa,str2->stringa,str2->bcount);
				str1->stringa[str2->bcount] = 0 ;
				str1->len    = str2->len ;
				str1->bcount = str2->bcount ;
				return str1 ;
			}
			break ;
			case dx_wstring :
			{
				if (str2->len == 0) {str1->stringx = (DXCHAR*)malloc(sizeof(DXCHAR));str1->stringx[0]=0;return str1;}
				/*if not zero then do the actual copy*/
				str1->stringx = (DXCHAR *)malloc(sizeof(DXCHAR)*(str2->len+1)) ;
				memcpy(str1->stringx,str2->stringx,str2->bcount);
				str1->stringx[str2->len] = 0 ;
				str1->len    = str2->len ;
				str1->bcount = str2->bcount ;
				return str1 ;
			}
			break;
			case dx_null : ;break;

		}

	return NULL ;
}


PDX_STRING dx_string_clone(PDX_STRING str)
{
	/*just a copy in an empty string*/
	PDX_STRING nstr = (PDX_STRING)malloc(sizeof(struct dx_string)) ;
	nstr->stringa = NULL ;
	nstr->stringw = NULL ;
	nstr->stringx = NULL ;
	nstr->type    = dx_null ;
	return dx_string_copy(nstr , str);
}

PDX_STRING dx_string_set_empty(PDX_STRING str)
{
	if(str == NULL) return NULL ;
	str->len=0;
	str->bcount=0;
	switch(str->type)
	{
	 case dx_ansi :  {free(str->stringa);str->stringa = (char*)malloc(1);str->stringa[0]=0;}
     break;
	 case dx_wide :  {free(str->stringw);str->stringw = (wchar_t*)malloc(1*sizeof(wchar_t));str->stringw[0]=0;}
     break;
	 case dx_utf8 :  {free(str->stringa);str->stringa = (char*)malloc(1);str->stringa[0]=0;}
	 break ;
	 case dx_wstring :  {free(str->stringx);str->stringx = (DXCHAR*)malloc(1*sizeof(DXCHAR));str->stringx[0]=0;}
     break;
	 case dx_null : ;break;
	}

	return str ;
}

enum dx_compare_res dx_string_compare(PDX_STRING str1, PDX_STRING str2)
{
	if ((str1 == NULL)||(str2 == NULL)) return dx_error ;
	/*test the easy first*/
	if (str1->len > str2->len) return dx_left_bigger  ;
	if (str1->len < str2->len) return dx_right_bigger ;

	/*apparently the strings have the same character count. Now check if they need to be converted to widestring*/
	PDX_STRING nstr1,nstr2 ;
	if(str1->type != dx_wide)
	{
		/*convert the string to widestring*/
		nstr1 = dx_string_convertW(str1) ;
		if(nstr1 == NULL) return dx_error ;
	}
	else nstr1 = str1 ; // just to make our life easy!

	if(str2->type != dx_wide)
	{
		/*convert the string to widestring*/
		nstr2 = dx_string_convertW(str2) ;

		if(nstr2 == NULL)
		{
		  if(nstr1 != str1) dx_string_free(nstr1) ;
		  return dx_error ;
		}
	}
	else nstr2 = str2 ; // just to make our life easy!

	//check the strings
	wchar_t *cc1 = nstr1->stringw ;
	wchar_t *cc2 = nstr2->stringw ;
	while(*cc1 != 0) // use the str1 as control doesn't  matter because we have permited only the equal size strings in this stage
	{
	  if(*cc1 != *cc2)
	  {
	    if (*cc1 > *cc2)
		{
		  if(nstr1 != str1) dx_string_free(nstr1) ;
		  if(nstr2 != str2) dx_string_free(nstr2) ;
		  return dx_left_bigger ;
		}
		 else
			 {
			   if(nstr1 != str1) dx_string_free(nstr1);
			   if(nstr2 != str2) dx_string_free(nstr2) ;
			   return dx_right_bigger ;
			 }
	  }
      cc1++ ;
	  cc2++ ;
	}

	if(nstr1 != str1) dx_string_free(nstr1);
	if(nstr2 != str2) dx_string_free(nstr2) ;

	return dx_equal ;
}


DXUTF8CHAR dx_get_utf8_char(char **char_indx)
{
     /*
      The function gets a pointer to the start of a utf8 character and
      return the character arithmetic value. The char_indx will be advanced to the
      start of the next character.
      The utf8 string MUST be zero terminated
     */

    char *chind = *char_indx ;

    if(*chind == 0) return 0 ; // string end

   if (GetBit(*chind,0) == 0) // 0
   {
     //this is a normal ansi char no need to do anything more
     (*char_indx)++ ; // next byte
     return *chind ;
   }

   /* check for the 2-bytes characters */
   if ((GetBit(*chind,0) == 1)&&(GetBit(*chind,1) == 1)&&(GetBit(*chind,2) == 0)) // 110
   {
        *char_indx = *char_indx + 2 ;  // next character byte
        /* construct the arithmetic value of the character */
        uint32_t utf8char = 0;
        uint32_t mask     = 0 ;

        /* first (control) byte */
        ((char*)&utf8char)[0] = (*chind << 3) ;
        utf8char = utf8char << 3 ;
        /* second control byte */
        chind++ ;
        ((char*)&mask)[0] = (*chind << 2)  ;
        /*position the bits*/
        mask = mask >> 2 ;
        /*construct the utf8 byte*/
        utf8char = utf8char | mask ;

        return utf8char ;
   }

    if ((GetBit(*chind,0) == 1)&&(GetBit(*chind,1) == 1)&&(GetBit(*chind,2) == 1)&&(GetBit(*chind,3) == 0))//1110
    {
        *char_indx = *char_indx + 3 ;  // next character byte
        /* construct the arithmetic value of the character */
        uint32_t utf8char = 0;
        uint32_t mask     = 0 ;

        /* first (control) byte */
        ((char*)&utf8char)[0] = (*chind << 4) ;
        utf8char = utf8char << 8 ;
        /* second byte */
        chind++ ;
        ((char*)&mask)[0] = (*chind << 2)  ;
        /*position the bits*/
        mask = mask << 4 ;
        /*construct the utf8 byte*/
        utf8char = utf8char | mask ;

         /* third byte */
        mask     = 0 ;
        chind++ ;
        ((char*)&mask)[0] = (*chind << 2)  ;
        /*position the bits*/
        mask = mask >> 2 ;
        /*construct the utf8 byte*/
        utf8char = utf8char | mask ;

        return utf8char ;

    }

      if ((GetBit(*chind,0) == 1)&&(GetBit(*chind,1) == 1)&&(GetBit(*chind,2) == 1)&&(GetBit(*chind,3) == 1)&&
      (GetBit(*chind,4) == 0 )) //11110
      {

       *char_indx = *char_indx + 4 ;  // next character byte
        /* construct the arithmetic value of the character */
        uint32_t utf8char = 0;
        uint32_t mask     = 0 ;

        /* first (control) byte */
        ((char*)&utf8char)[0] = (*chind << 5) ;
        utf8char = utf8char << 13 ;

        /* second byte */
        chind++ ;
        ((char*)&mask)[0] = (*chind << 2)  ;
        /*position the bits*/
        mask = mask << 10 ;

        /*construct the utf8 byte*/
        utf8char = utf8char | mask ;

         /* third byte */
        mask = 0 ;
        chind++ ;
        ((char*)&mask)[0] = (*chind << 2)  ;
        /*position the bits*/
        mask = mask << 4 ;

        /*construct the utf8 byte*/
        utf8char = utf8char | mask ;

         /* fourth byte */
        mask = 0 ;
        chind++ ;
        ((char*)&mask)[0] = (*chind << 2)  ;
        /*position the bits*/
        mask = mask >> 2 ;
        /*construct the utf8 byte*/
        utf8char = utf8char | mask ;

        return utf8char ;

      }

   return 0 ;
}


DXUTF8CHAR dx_get_utf8_char_ex(char **char_indx,char **prev_char)
{
     /*
      The function gets a pointer to the start of a utf8 character and
      return the character arithmetic value. The char_indx will be advanced to the
      start of the next character.
      The utf8 string MUST be zero terminated
      the prev_char will be have the previous character position
     */
    (*prev_char) = *char_indx ;
    char *chind = *char_indx ;

    if(*chind == 0) return 0 ; // string end

   if (GetBit(*chind,0) == 0) // 0
   {
     //this is a normal ansi char no need to do anything more
     (*char_indx)++ ; // next byte
     return *chind ;
   }

   /* check for the 2-bytes characters */
   if ((GetBit(*chind,0) == 1)&&(GetBit(*chind,1) == 1)&&(GetBit(*chind,2) == 0)) // 110
   {
        *char_indx = *char_indx + 2 ;  // next character byte
        /* construct the arithmetic value of the character */
        uint32_t utf8char = 0;
        uint32_t mask     = 0 ;

        /* first (control) byte */
        ((char*)&utf8char)[0] = (*chind << 3) ;
        utf8char = utf8char << 3 ;
        /* second control byte */
        chind++ ;
        ((char*)&mask)[0] = (*chind << 2)  ;
        /*position the bits*/
        mask = mask >> 2 ;
        /*construct the utf8 byte*/
        utf8char = utf8char | mask ;

        return utf8char ;
   }

    if ((GetBit(*chind,0) == 1)&&(GetBit(*chind,1) == 1)&&(GetBit(*chind,2) == 1)&&(GetBit(*chind,3) == 0))//1110
    {
        *char_indx = *char_indx + 3 ;  // next character byte
        /* construct the arithmetic value of the character */
        uint32_t utf8char = 0;
        uint32_t mask     = 0 ;

        /* first (control) byte */
        ((char*)&utf8char)[0] = (*chind << 4) ;
        utf8char = utf8char << 8 ;
        /* second byte */
        chind++ ;
        ((char*)&mask)[0] = (*chind << 2)  ;
        /*position the bits*/
        mask = mask << 4 ;
        /*construct the utf8 byte*/
        utf8char = utf8char | mask ;

         /* third byte */
        mask     = 0 ;
        chind++ ;
        ((char*)&mask)[0] = (*chind << 2)  ;
        /*position the bits*/
        mask = mask >> 2 ;
        /*construct the utf8 byte*/
        utf8char = utf8char | mask ;

        return utf8char ;

    }

      if ((GetBit(*chind,0) == 1)&&(GetBit(*chind,1) == 1)&&(GetBit(*chind,2) == 1)&&(GetBit(*chind,3) == 1)&&
      (GetBit(*chind,4) == 0 )) //11110
      {

       *char_indx = *char_indx + 4 ;  // next character byte
        /* construct the arithmetic value of the character */
        uint32_t utf8char = 0;
        uint32_t mask     = 0 ;

        /* first (control) byte */
        ((char*)&utf8char)[0] = (*chind << 5) ;
        utf8char = utf8char << 13 ;

        /* second byte */
        chind++ ;
        ((char*)&mask)[0] = (*chind << 2)  ;
        /*position the bits*/
        mask = mask << 10 ;

        /*construct the utf8 byte*/
        utf8char = utf8char | mask ;

         /* third byte */
        mask = 0 ;
        chind++ ;
        ((char*)&mask)[0] = (*chind << 2)  ;
        /*position the bits*/
        mask = mask << 4 ;

        /*construct the utf8 byte*/
        utf8char = utf8char | mask ;

         /* fourth byte */
        mask = 0 ;
        chind++ ;
        ((char*)&mask)[0] = (*chind << 2)  ;
        /*position the bits*/
        mask = mask >> 2 ;
        /*construct the utf8 byte*/
        utf8char = utf8char | mask ;

        return utf8char ;

      }
   /*something  very strange is happening. lets check the byte*/
   if(*char_indx == 0) return 0 ;
   (*char_indx)++ ; /*encoding error, next byte*/
   return (DXUTF8CHAR)*char_indx ; 
}

DXUTF8CHAR dx_get_utf8_char_ex2(char **char_indx,char **prev_char,int *char_len)
{
     /*
      The function gets a pointer to the start of a utf8 character and
      return the character arithmetic value. The char_indx will be advanced to the
      start of the next character.
      The utf8 string MUST be zero terminated
      the prev_char will be have the previous character position
      the char_len will have the byte count of the character
     */
    (*prev_char) = *char_indx ;
    char *chind = *char_indx ;

    if(*chind == 0) return 0 ; // string end

   if (GetBit(*chind,0) == 0) // 0
   {
     //this is a normal ansi char no need to do anything more
     (*char_indx)++ ; // next byte
     *char_len = 1 ;
     return *chind ;
   }

   /* check for the 2-bytes characters */
   if ((GetBit(*chind,0) == 1)&&(GetBit(*chind,1) == 1)&&(GetBit(*chind,2) == 0)) // 110
   {
        *char_indx = *char_indx + 2 ;  // next character byte
        /* construct the arithmetic value of the character */
        uint32_t utf8char = 0;
        uint32_t mask     = 0 ;

        /* first (control) byte */
        ((char*)&utf8char)[0] = (*chind << 3) ;
        utf8char = utf8char << 3 ;
        /* second control byte */
        chind++ ;
        ((char*)&mask)[0] = (*chind << 2)  ;
        /*position the bits*/
        mask = mask >> 2 ;
        /*construct the utf8 byte*/
        utf8char = utf8char | mask ;
        *char_len = 2 ;
        return utf8char ;
   }

    if ((GetBit(*chind,0) == 1)&&(GetBit(*chind,1) == 1)&&(GetBit(*chind,2) == 1)&&(GetBit(*chind,3) == 0))//1110
    {
        *char_indx = *char_indx + 3 ;  // next character byte
        /* construct the arithmetic value of the character */
        uint32_t utf8char = 0;
        uint32_t mask     = 0 ;

        /* first (control) byte */
        ((char*)&utf8char)[0] = (*chind << 4) ;
        utf8char = utf8char << 8 ;
        /* second byte */
        chind++ ;
        ((char*)&mask)[0] = (*chind << 2)  ;
        /*position the bits*/
        mask = mask << 4 ;
        /*construct the utf8 byte*/
        utf8char = utf8char | mask ;

         /* third byte */
        mask     = 0 ;
        chind++ ;
        ((char*)&mask)[0] = (*chind << 2)  ;
        /*position the bits*/
        mask = mask >> 2 ;
        /*construct the utf8 byte*/
        utf8char = utf8char | mask ;
        *char_len = 3 ;
        return utf8char ;

    }

      if ((GetBit(*chind,0) == 1)&&(GetBit(*chind,1) == 1)&&(GetBit(*chind,2) == 1)&&(GetBit(*chind,3) == 1)&&
      (GetBit(*chind,4) == 0 )) //11110
      {

       *char_indx = *char_indx + 4 ;  // next character byte
        /* construct the arithmetic value of the character */
        uint32_t utf8char = 0;
        uint32_t mask     = 0 ;

        /* first (control) byte */
        ((char*)&utf8char)[0] = (*chind << 5) ;
        utf8char = utf8char << 13 ;

        /* second byte */
        chind++ ;
        ((char*)&mask)[0] = (*chind << 2)  ;
        /*position the bits*/
        mask = mask << 10 ;

        /*construct the utf8 byte*/
        utf8char = utf8char | mask ;

         /* third byte */
        mask = 0 ;
        chind++ ;
        ((char*)&mask)[0] = (*chind << 2)  ;
        /*position the bits*/
        mask = mask << 4 ;

        /*construct the utf8 byte*/
        utf8char = utf8char | mask ;

         /* fourth byte */
        mask = 0 ;
        chind++ ;
        ((char*)&mask)[0] = (*chind << 2)  ;
        /*position the bits*/
        mask = mask >> 2 ;
        /*construct the utf8 byte*/
        utf8char = utf8char | mask ;

        *char_len = 4 ;
        return utf8char ;

      }

    /*something  very strange is happening. lets check the byte*/
   if(*char_indx == 0) 
   {
	   *char_len = 1 ;
	   return 0 ;
   }
   (*char_indx)++ ; /*encoding error, next byte*/
   *char_len = 1 ;
   return (DXUTF8CHAR)*char_indx ; 
}



enum dx_compare_res dx_string_native_compare(PDX_STRING str1, PDX_STRING str2)
{
  	if ((str1 == NULL)||(str2 == NULL)) return dx_error ;
  	if (str1->type != str2->type) return dx_error       ;
	/*test the easy first*/
	if (str1->len > str2->len) return dx_left_bigger  ;
	if (str1->len < str2->len) return dx_right_bigger ;

   switch(str1->type)
   {
     case dx_ansi :
     {
        char *cc1 = str1->stringa ;
        char *cc2 = str2->stringa ;
        while(*cc1 != 0) // use the str1 as control doesn't  matter because we have permited only the equal size strings in this stage
        {
          if(*cc1 != *cc2)
          {
            if (*cc1 > *cc2)
              return dx_left_bigger ;
             else
                return dx_right_bigger ;
          }
          cc1++ ;
          cc2++ ;
        }
     } break;
     case dx_wide :
     {
        wchar_t *cc1 = str1->stringw ;
        wchar_t *cc2 = str2->stringw ;
        while(*cc1 != 0) // use the str1 as control doesn't  matter because we have permited only the equal size strings in this stage
        {
          if(*cc1 != *cc2)
          {
            if (*cc1 > *cc2)
              return dx_left_bigger ;
             else
                return dx_right_bigger ;
          }
          cc1++ ;
          cc2++ ;
        }
     } break ;

     case dx_utf8 :
     {

       DXUTF8CHAR cc1,cc2 ;
       char *utf8_1 , *utf8_2 ;
       utf8_1 = str1->stringa ;
       utf8_2 = str2->stringa ;

       while(true)
       {
         // get the characters for comparison and advance the pointer to the next character
         cc1 = dx_get_utf8_char(&utf8_1) ;
         cc2 = dx_get_utf8_char(&utf8_2) ;
         if (cc1 != cc2)
         {
             if (cc1 > cc2)
                return dx_left_bigger ;
             else
                return dx_right_bigger ;
         }

         if (cc1 == 0) break ; //end string , we assume [we have make sure] that the strings are valid zero terminated strings
       }

     } break ;
	  case dx_wstring :
     {
        DXCHAR *cc1 = str1->stringx ;
        DXCHAR *cc2 = str2->stringx ;
        while(*cc1 != 0) // use the str1 as control doesn't  matter because we have permited only the equal size strings in this stage
        {
          if(*cc1 != *cc2)
          {
            if (*cc1 > *cc2)
              return dx_left_bigger ;
             else
                return dx_right_bigger ;
          }
          cc1++ ;
          cc2++ ;
        }
     } break ;
     case dx_null : ;break;
   }

   return dx_equal ;
}

enum dx_compare_res dx_string_native_lex_cmp(PDX_STRING str1, PDX_STRING str2)
{
  	if ((str1 == NULL)||(str2 == NULL)) return dx_error ;
  	if (str1->type != str2->type) return dx_error       ;

   switch(str1->type)
   {
     case dx_ansi :
     {
            DXLONG64 base_len = 0 ;
            if (str1->len < str2->len) base_len = str1->len ;
                 else
                    base_len = str2->len ; // either this is the smaller length or the length are equal

            char *cc1 = str1->stringa ;
            char *cc2 = str2->stringa ;

            for(DXLONG64 i = 0 ; i < base_len ; i++) // for all the characters of the smaller string
            {
              if(*cc1 != *cc2)
              {
                if (*cc1 > *cc2) return dx_left_bigger ;
                  else
                    return dx_right_bigger ;
              }
              cc1++ ;
              cc2++ ;
            }
     } break;

     case dx_wide :
     {
            DXLONG64 base_len = 0 ;
            if (str1->len < str2->len) base_len = str1->len ;
                 else
                    base_len = str2->len ; // either this is the smaller length or the length are equal

            wchar_t *cc1 = str1->stringw ;
            wchar_t *cc2 = str2->stringw ;

            for(DXLONG64 i = 0 ; i < base_len ; i++) // for all the characters of the smaller string
            {
              if(*cc1 != *cc2)
              {
                if (*cc1 > *cc2) return dx_left_bigger ;
                  else
                    return dx_right_bigger ;
              }
              cc1++ ;
              cc2++ ;
            }

     } break ;

     case dx_utf8 :
     {

       DXUTF8CHAR cc1,cc2 ;
       char *utf8_1 , *utf8_2 ;

       if (str1->len < str2->len)
       {
         utf8_1 = str1->stringa ;
         utf8_2 = str2->stringa ;
       }
        else
        {
          utf8_1 = str2->stringa ;
          utf8_2 = str1->stringa ;
        }// either this is the smaller length or the length are equal

       while(true)
       {
         // get the characters for comparison and advance the pointer to the next character
         cc1 = dx_get_utf8_char(&utf8_1) ;
         cc2 = dx_get_utf8_char(&utf8_2) ;
         if(cc1 != cc2)
         {
            if (cc1 > cc2) return dx_left_bigger ;
             else
                return dx_right_bigger ;
        }

         if (cc1 == 0) break ; // the cc1 is the smaller string (or the strings are equal) so we will base the termination in it
       }

     } break ;
	  case dx_wstring :
     {
            DXLONG64 base_len = 0 ;
            if (str1->len < str2->len) base_len = str1->len ;
                 else
                    base_len = str2->len ; // either this is the smaller length or the length are equal

            DXCHAR *cc1 = str1->stringx ;
            DXCHAR *cc2 = str2->stringx ;

            for(DXLONG64 i = 0 ; i < base_len ; i++) // for all the characters of the smaller string
            {
              if(*cc1 != *cc2)
              {
                if (*cc1 > *cc2) return dx_left_bigger ;
                  else
                    return dx_right_bigger ;
              }
              cc1++ ;
              cc2++ ;
            }

     } break ;

     case dx_null : ;break;
   }

    /*
    if the strings are equal until now we will check if they have different lengths and the
    one with the bigger length will be the bigger string
    */

	if (str1->len > str2->len) return dx_left_bigger ;
	else
	 if (str1->len < str2->len) return dx_right_bigger ;
      else return dx_equal ;
}

enum dx_compare_res dx_string_lex_cmp(PDX_STRING str1, PDX_STRING str2)
{
	if ((str1 == NULL)||(str2 == NULL)) return dx_error ;

	PDX_STRING nstr1,nstr2 ;
	if(str1->type != dx_wide)
	{
		/*convert the string to widestring*/
		nstr1 = dx_string_convertW(str1) ;
		if(nstr1 == NULL) return dx_error ;
	}
	else nstr1 = str1 ; // just to make our life easy!

	if(str2->type != dx_wide)
	{
		/*convert the string to widestring*/
		nstr2 = dx_string_convertW(str2) ;

		if(nstr2 == NULL)
		{
		  if(nstr1 != str1) dx_string_free(nstr1) ;
		  return dx_error ;
		}
	}
	else nstr2 = str2 ; // just to make our life easier!


	wchar_t *cc1 = nstr1->stringw ;
	wchar_t *cc2 = nstr2->stringw ;

    DXLONG64 base_len = 0 ;
	if (nstr1->len < nstr2->len)
		base_len = nstr1->len ;
		 else
			base_len = nstr2->len ; // either this is the smaller length or the length are equal

	for(DXLONG64 i = 0 ; i < base_len ; i++) // for all the characters of the smaller string
	{
	  if(*cc1 != *cc2)
	  {
	    if (*cc1 > *cc2)
		{
		  if(nstr1 != str1) dx_string_free(nstr1) ;
		  if(nstr2 != str2) dx_string_free(nstr2) ;
		  return dx_left_bigger ;
		}
		 else
			 {
			   if(nstr1 != str1) dx_string_free(nstr1);
			   if(nstr2 != str2) dx_string_free(nstr2) ;
			   return dx_right_bigger ;
			 }
	  }
      cc1++ ;
	  cc2++ ;
	}

	if(nstr1 != str1) dx_string_free(nstr1);
	if(nstr2 != str2) dx_string_free(nstr2) ;

    /*
	  if the strings are equal until now we will check if they have different lengths and the
	  one with the bigger length will be the bigger string
	*/

	if (str1->len > str2->len) return dx_left_bigger ;
	else
	 if (str1->len < str2->len) return dx_right_bigger ;
	     else return dx_equal ;
}

#ifdef _WIN32

void EchoDXSTRING(HANDLE console , PDX_STRING str)
{
	if (str == NULL)
	{
		printf("NULL");
		return ;
	}
	if ((str->type == dx_ansi)||(str->type == dx_utf8)) printf("%s",str->stringa);
	else
	if (str->type == dx_wide)
	{
	  unsigned long written = 0 ;
	  if (console != NULL)
      {
          WriteConsoleW(console,str->stringw,str->len,&written,NULL);
         // WriteConsoleW(console,L"\n",StrLenA("\n"),&written,NULL);
      }
	  else
	     printf("%ls",str->stringw) ;
	}
}
#endif

#ifdef _LINUX

void EchoDXSTRING(PDX_STRING str)
{
	if (str == NULL)
	{
		printf("NULL\n");
		return ;
	}
	if ((str->type == dx_ansi)||(str->type == dx_utf8)) printf("%s\n",str->stringa);
	else
	 if (str->type == dx_wide) printf("%ls\n",str->stringw) ;
}
#endif

/***********************************************************************/

DXLONG64 StrLenA(const char *a_str)
{
 DXLONG64 len = 0;
 if (a_str == NULL) return 0 ;
 while(*a_str!=0)
 {
   len++  ;
   a_str++ ; // next character
 }
  return len;
}

DXLONG64 StrLenW(const wchar_t *w_str)
{
 DXLONG64 len = 0;
 if (w_str == NULL) return 0 ;
 while(*w_str!=0)
 {
   len++  ;
   w_str++ ; // next character
 }
  return len;
}

DXLONG64 StrLenU(const char *s)
{
  DXLONG64 len = 0 ;
  while (*s != 0)
  {
    if ((*s & 0xC0) != 0x80) len++ ;
    s++;
  }
  return len;
}

DXLONG64 StrLenX(const DXCHAR *dx_str )
{
	 DXLONG64 len = 0;
	 if (dx_str == NULL) return 0 ;
	 while(*dx_str!=0)
	 {
	   len++  ;
	   dx_str++ ; // next character
	 }
	  return len;
}

DXLONG64 StrLenoldU(const char *u_str)
{
 /*
  the StrLen for the ansi and utf16 string is trivial , but for the utf8
  we have to employe more advanced techniques , this function is
  just for show it was my first attempt and frankly im embarased
  that after 22 years of programming i did write this! I will blame
  my lazines to read the utf8 properties and so to understand what bytes
  imply that are stand alone or the head of a codepoint :'(
 */
 DXLONG64 len  = 0 ;
 DXLONG64 indx = 0 ;
 /* To get the length of the buffer in byte we will treat the string as an ansi string
    as is very easy for the user to pass a non comformat utf8 string in the
	function we will always check if we are out of the string bounds when we check its bytes
 */
 DXLONG64 blen = StrLenA(u_str);
 /*for all the bytes of the utf8 string */

 while ( (*u_str != 0) && (indx < blen) ) //until the string's end (utf8 strings are being terminated with a zero byte like the ansi strings)
 {										  //we check for an error in the indexing too if the string is not a valid utf8 string
   if (GetBit(*u_str,0) == 0) // 0
   {
	 //this is a normal ansi char no need to do anything more
     len++   ;
     indx++  ;
	 u_str++ ; // next byte
   }
    else
      if ((GetBit(*u_str,0) == 1)&&(GetBit(*u_str,1) == 1)&&(GetBit(*u_str,2) == 0)) // 110
      {
        len++ ;
        indx  = indx  + 2 ; // the character is constructed with 2 bytes
		u_str = u_str + 2 ; // next character byte
	  }
       else
        if ((GetBit(*u_str,0) == 1)&&(GetBit(*u_str,1) == 1)&&(GetBit(*u_str,2) == 1)&&(GetBit(*u_str,3) == 0))//1110
        {
	     len++ ;
         indx  = indx  + 3 ;// the character is constructed with 3 bytes
         u_str = u_str + 3 ;
        }
		else
		  if ((GetBit(*u_str,0) == 1)&&(GetBit(*u_str,1) == 1)&&(GetBit(*u_str,2) == 1)&&(GetBit(*u_str,3) == 1)&&
		  (GetBit(*u_str,4) == 0 )) //11110
		  {
			len++ ;
			indx  = indx  + 4  ;// the character is constructed with 4 bytes
			u_str = u_str + 4  ;
		  }

 }

 /* now we have the correct length in characters for the utf8 string*/
return len ;

}



wchar_t * dx_ConvertAnsiToW(const char *str)
{
	if ( str == NULL ) return NULL ;
	size_t len = StrLenA(str) +1;

	wchar_t *tch=(wchar_t*) malloc(sizeof(wchar_t)*(len));

	#ifdef _LINUX
	  mbstowcs(tch , str , len);
	#endif

	#ifdef _WIN32
	 MultiByteToWideChar(DX_CP_CODE_PAGE,0,str,len-1,tch,len);
	#endif

	tch[len-1]=0;

	return tch;
}

char * dx_ConvertAnsiToUtf8(const char *str )
{
 /*
  Convert an ansi string to a utf8 string.
  Windows require the ansi string to be converted first to widestring so double the work!
  */
  if ( str == NULL ) return NULL ;
  wchar_t * wstr = dx_ConvertAnsiToW(str);
  if (wstr == NULL) return NULL ;

  /*convert to utf8*/
 #ifdef _WIN32
  //get the required size
  int nsize = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL)+1;
  char *nstr = (char *) malloc(nsize);
  WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nstr, nsize, NULL, NULL);
  //free the memory of the temporary buffer
  free(wstr);
  nstr[nsize-1]=0;
  return nstr ;
 #endif

 #ifdef _LINUX
  size_t nsize = wcstombs(NULL , wstr , 0);
  if (nsize == -1)
  {
   free(wstr);
   return NULL ;
  }
  nsize = nsize + 1;
  char *nstr = (char *) malloc(nsize);
  wcstombs(nstr , wstr , nsize);
  free(wstr);
  nstr[nsize-1]=0;
  return nstr ;
 #endif

  return NULL;

}

char * dx_ConvertWToUtf8(const wchar_t *str)
{
  /*convert to utf8*/
  //get the required size
  if ( str == NULL ) return NULL ;

  #ifdef _WIN32

    int nsize = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL)+1;
    char *nstr = (char *) malloc(nsize);
    WideCharToMultiByte(CP_UTF8, 0, str, -1, nstr, nsize, NULL, NULL);
	nstr[nsize-1]=0;
    return nstr ;

  #endif

  #ifdef _LINUX

    size_t nsize = wcstombs(NULL,str,0);
	if (nsize == -1) return NULL ;
	nsize = nsize + 1 ;
    char *nstr = (char *) malloc(nsize);
	wcstombs(nstr,str,nsize);
	nstr[nsize-1]=0;
    return nstr ;

  #endif

  return NULL ;
}


wchar_t * dx_ConvertUtf8ToW(const char *str)
{
   #ifdef _WIN32

	   if ( str == NULL ) return NULL ;
	   size_t len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	   if (len > 1 )
	   {
          len = len + 1 ;
		  wchar_t *wstr = (wchar_t*)malloc(len*sizeof(wchar_t))          ;
		  MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, len)  ;
		  wstr[len-1]=0;
		  return wstr ;
	   }
   #endif

  #ifdef _LINUX

	  size_t nsize = mbstowcs(NULL , str , 0);
	  if (nsize == -1) return NULL ;
	  nsize = nsize + 1 ;
	  wchar_t *wstr = malloc(nsize*sizeof(wchar_t)) ;
	  mbstowcs(wstr , str , nsize);
	  wstr[nsize-1]=0;
	  return wstr ;
 #endif

  return NULL ;
}


DXCHAR * dx_ConvertAnsiToDXS(const char *str)
{
	if (str == NULL) return NULL ;
	DXLONG64 len = StrLenA(str)  ;

	DXCHAR *stringx = (DXCHAR*)malloc((len+1)*sizeof(DXCHAR)) ; 
	
	if (stringx == NULL) return NULL ;
		
    /*convert the ansi to widechars*/
	wchar_t * buff = dx_ConvertAnsiToW(str) ;
	wchar_t *buffindx = buff ;
	DXCHAR * strindx = stringx ;
	/*copy the data*/
	while(*buffindx != 0)
	{
		*strindx = (DXCHAR)(*buffindx) ;
		strindx++ ;
		buffindx++;
	}
	free(buff); /*the memory is not needed any more*/

	stringx[len] = 0 ;
	return stringx	 ;
}

DXCHAR * dx_ConvertUtf8ToDXS(const char *str)
{
 	if (str == NULL) return NULL ;
	DXLONG64 len = StrLenU(str)  ;

	DXCHAR *stringx = (DXCHAR*)malloc((len+1)*sizeof(DXCHAR)) ; 
	
	if (stringx == NULL) return NULL ;
		
    /*convert the ansi to widechars*/
	wchar_t * buff = dx_ConvertUtf8ToW(str) ;
	wchar_t *buffindx = buff ;
	DXCHAR * strindx = stringx ;
	/*copy the data*/
	while(*buffindx != 0)
	{
		*strindx = (DXCHAR)(*buffindx) ;
		strindx++ ;
		buffindx++;
	}
	free(buff); /*the memory is not needed any more*/

	stringx[len] = 0 ;
	return stringx	 ;
}

DXCHAR * dx_ConvertWideToDXS(wchar_t *str) 
{
	if (str == NULL) return NULL ;
	DXLONG64 len = StrLenW(str)  ;

	DXCHAR *stringx = (DXCHAR*)malloc((len+1)*sizeof(DXCHAR)) ; 
	
	if (stringx == NULL) return NULL ;
		
	wchar_t *buffindx = str ;
	DXCHAR * strindx = stringx ;
	/*copy the data*/
	while(*buffindx != 0)
	{
		*strindx = (DXCHAR)(*buffindx) ;
		strindx++ ;
		buffindx++;
	}

	stringx[len] = 0 ;
	return stringx	 ;
}


wchar_t * dx_ConvertDXSToWide(DXCHAR *str)
{
    if (str == NULL) return NULL ;
	DXLONG64 len = StrLenX(str)  ;

	wchar_t *stringx = (wchar_t*)malloc((len+1)*sizeof(wchar_t)) ; 
	if (stringx == NULL) return NULL ;
		
	DXCHAR *buffindx = str ;
	wchar_t * strindx = stringx ;
	/*copy the data*/
	while(*buffindx != 0)
	{
		*strindx = (wchar_t)(*buffindx) ;
		strindx++ ;
		buffindx++;
	}

	stringx[len] = 0 ;
	return stringx	 ;
}

char * dx_ConvertDXSToUtf8(DXCHAR *str)
{
	if (str == NULL) return NULL ;
	DXLONG64 len = 0  ;

	/*Get the required memory length for the utf8 string*/
	DXCHAR *buffindx = str ;
	while(*buffindx != 0)
	{
		len = len + dx_string_uint32_to_utf8_size(*buffindx) ;
		buffindx++ ;
	}
	/*we got the length*/
	char *stringx = (char*)malloc(len+1) ; 
	if (stringx == NULL) return NULL ;
		
	buffindx = str ;
	char * strindx = stringx ;
	/*copy the data*/
	char buffer[5] ;
	while(*buffindx != 0)
	{
		uint32_t clen = dx_string_convert_uint32_to_utf8(buffer, *buffindx) ;
		memcpy(strindx,buffer,clen) ;
		strindx  = strindx + clen ;
		buffindx++;
	}
	stringx[len] = 0 ;
	return stringx	 ;
}

DXCHAR dx_ConvertUtf8ToDXSc(char *ch)
{
	int len = StrLenA(ch) ;
	if(len ==1)
	{
		return *ch ; /*Simple 1 byte character*/
	}
	if(len == 2)
	{
		uint16_t int1 = ((uint16_t)(ch[0] & 0b00011111)) << 6 ;
		uint16_t int2 =((uint16_t)(ch[1]  & 0b00111111)) ;
		return int1 + int2 ;
	}
	if(len == 3)
	{
		uint16_t int1 = ((uint16_t)(ch[0] & 0b00001111)) << 12  ;
		uint16_t int2 = ((uint16_t)(ch[1] & 0b00111111)) << 6   ;
		uint16_t int3 = ((uint16_t)(ch[2] & 0b00111111));
		return int1 + int2 + int3 ;
	}
	if(len == 4)
	{
		uint32_t int1 = ((uint32_t)(ch[0] & 0b00000111)) << 18;
		uint16_t int2 = ((uint16_t)(ch[1] & 0b00111111)) << 12;
		uint16_t int3 = ((uint16_t)(ch[2] & 0b00111111)) << 6;
		uint16_t int4 = ((uint16_t)(ch[3] & 0b00111111)) ;
		return int1 + int2 + int3 + int4;
	}

	return -1 ;
}


int dx_ConvertDXSToUtf8c(char *out_c , DXCHAR ch)
{
   if (ch <= 0x7F) 
    {
        out_c[0] = (char) ch;
        out_c[1] = 0;
        return 1;
     }
      else 
      if (ch <= 0x07FF) 
      {
        
        out_c[0] = (char) (((ch >> 6) & 0x1F) | 0xC0);
        out_c[1] = (char) (((ch >> 0) & 0x3F) | 0x80);
        out_c[2] = 0;
        return 2;
      }
      else 
      if (ch <= 0xFFFF) 
      {
        out_c[0] = (char) (((ch >> 12) & 0x0F) | 0xE0);
        out_c[1] = (char) (((ch >>  6) & 0x3F) | 0x80);
        out_c[2] = (char) (((ch >>  0) & 0x3F) | 0x80);
        out_c[3] = 0;
        return 3;
      }
      else if (ch <= 0x10FFFF) 
      {
        out_c[0] = (char) (((ch >> 18) & 0x07) | 0xF0);
        out_c[1] = (char) (((ch >> 12) & 0x3F) | 0x80);
        out_c[2] = (char) (((ch >>  6) & 0x3F) | 0x80);
        out_c[3] = (char) (((ch >>  0) & 0x3F) | 0x80);
        out_c[4] = 0;
        return 4;
      }
      else 
      { 
        /*error unknown codepoint*/
        out_c[0] = 0;  
        return 0;
      }

    return 0 ;
}



char *dx_string_return_utf8_pos(PDX_STRING str , DXLONG64 pos )
{
    if((pos > (str->len-1))||(pos < 0)) return NULL ;

    char *utf8 = str->stringa ;
    for(DXLONG64 i = 0 ; i<pos ; i++ )
    {
      dx_get_utf8_char(&utf8);
    }

  return utf8;
}

uint32_t dx_string_convert_uint32_to_utf8(char *buffer, uint32_t utfc)
{
    if (utfc <= 0x7F) 
    {
        buffer[0] = (char) utfc;
        buffer[1] = 0;
        return 1;
     }
      else 
      if (utfc <= 0x07FF) 
      {
        
        buffer[0] = (char) (((utfc >> 6) & 0x1F) | 0xC0);
        buffer[1] = (char) (((utfc >> 0) & 0x3F) | 0x80);
        buffer[2] = 0;
        return 2;
      }
      else 
      if (utfc <= 0xFFFF) 
      {
        buffer[0] = (char) (((utfc >> 12) & 0x0F) | 0xE0);
        buffer[1] = (char) (((utfc >>  6) & 0x3F) | 0x80);
        buffer[2] = (char) (((utfc >>  0) & 0x3F) | 0x80);
        buffer[3] = 0;
        return 3;
      }
      else if (utfc <= 0x10FFFF) 
      {
        buffer[0] = (char) (((utfc >> 18) & 0x07) | 0xF0);
        buffer[1] = (char) (((utfc >> 12) & 0x3F) | 0x80);
        buffer[2] = (char) (((utfc >>  6) & 0x3F) | 0x80);
        buffer[3] = (char) (((utfc >>  0) & 0x3F) | 0x80);
        buffer[4] = 0;
        return 4;
      }
      else 
      { 
        /*error unknown codepoint*/
        buffer[0] = 0;  
        return 0;
      }

    return 0 ;
}

uint32_t dx_convert_utf8_to_int32(char *buffer,int len)
{
    if(len ==1)
    {
      return *buffer ; /*Simple 1 byte character*/
    }
    if(len == 2)
    {
        uint16_t int1 = ((uint16_t)(buffer[0] & 0b00011111)) << 6 ;
        uint16_t int2 =((uint16_t)(buffer[1]  & 0b00111111)) ;
        return int1 + int2 ;
    }
    if(len == 3)
    {
        uint16_t int1 = ((uint16_t)(buffer[0] & 0b00001111)) << 12  ;
        uint16_t int2 = ((uint16_t)(buffer[1] & 0b00111111)) << 6   ;
        uint16_t int3 = ((uint16_t)(buffer[2] & 0b00111111));
        return int1 + int2 + int3 ;
    }
    if(len == 4)
    {
        uint32_t int1 = ((uint32_t)(buffer[0] & 0b00000111)) << 18;
        uint16_t int2 = ((uint16_t)(buffer[1] & 0b00111111)) << 12;
        uint16_t int3 = ((uint16_t)(buffer[2] & 0b00111111)) << 6;
        uint16_t int4 = ((uint16_t)(buffer[3] & 0b00111111)) ;
        return int1 + int2 + int3 + int4;
    }

    return -1 ;
}

uint32_t dx_string_uint32_to_utf8_size(uint32_t utfc)
{
    if (utfc <= 0x7F)  return 1;
    else if (utfc <= 0x07FF) return 2;
    else if (utfc <= 0xFFFF) return 3;
    else if (utfc <= 0x10FFFF) return 4;
    else  return 0;

    return 0 ;
}



void memcpy_from_indx(char *dest,char *src,DXLONG64 fr_indx,DXLONG64 bcount)
{
   src = &src[fr_indx] ;
   memcpy(dest,src,bcount) ;
}


/********************UTF8 STRING MANIPULATION FUNCTIONS******************/

short dx_utf8_char_byte_count(char utfc_1byte)
{
   if (GetBit(utfc_1byte,0) == 0) return 1 ;

   if ((GetBit(utfc_1byte,0) == 1)&&(GetBit(utfc_1byte,1) == 1)&&(GetBit(utfc_1byte,2) == 0)) return 2 ;
   if ((GetBit(utfc_1byte,0) == 1)&&(GetBit(utfc_1byte,1) == 1)&&(GetBit(utfc_1byte,2) == 1)&&(GetBit(utfc_1byte,3) == 0))
       return 3 ;
  if ((GetBit(utfc_1byte,0) == 1)&&(GetBit(utfc_1byte,1) == 1)&&(GetBit(utfc_1byte,2) == 1)&&(GetBit(utfc_1byte,3) == 1)&&
      (GetBit(utfc_1byte,4) == 0 )) return 4 ;

        return 0 ;
}

DXLONG64 dx_utf8_word_byte_count(char *word)
{
  char * cc = NULL;	 
  int char_len = 0 ;
  DXLONG64 cnt = 0 ;
  while(*word != 0)
  {
    dx_get_utf8_char_ex2(&word,&cc,&char_len);
	cnt = cnt + char_len ;
  }

  return cnt ;
}

PDX_STRING dx_utf8_remove_char(PDX_STRING string,char *utf8char)
{
	bool no_change = true ; 
    DXCHAR this_char =dx_convert_utf8_to_int32(utf8char,dx_utf8_char_byte_count(*utf8char)) ;
    char * ustr = string->stringa ;
	char * c_char ;
    DXLONG64 buflen = 0 ; 
    while(*ustr != 0)
    {
      int char_len ;
      DXCHAR temp_char = dx_get_utf8_char_ex2(&ustr,&c_char,&char_len) ;
      if(temp_char != this_char)
	  {
		  buflen = buflen + char_len ;
		  if(no_change == true) no_change = false ;
	  }
    }

	if(no_change == true) return string ;

    char *buf = (char*)malloc(buflen+1); /*+1 zero terminated string*/
    char *buf_indx = buf ;
    ustr = string->stringa ;
    while(*ustr != 0)
    {
		int char_len ;
		DXCHAR temp_char = dx_get_utf8_char_ex2(&ustr,&c_char,&char_len) ;
        if(temp_char != this_char)
        {
          for(int i=0;i<char_len;i++) 
          {
           *buf_indx = *c_char ;
           buf_indx++ ;/*next byte*/
           c_char++     ; /*next byte*/
          }
        
        } 
    }
    
    /*free the previous memory and set the new one*/
	*buf_indx = 0 ;
    dx_string_setU(string,buf) ;

    return string ;
}

PDX_STRING dx_unicode_remove_char(PDX_STRING string, DXCHAR this_char)
{
    DXLONG64 strlen = 0 ;

	DXCHAR * str = string->stringx ;
    while(*str != 0)
    {
      if(*str != this_char) strlen++  ;
	  str++ ;
    }

	if(strlen == string->len) /*no character will be removed, exit*/
	 return string ;

    DXCHAR *buff = (DXCHAR*)malloc((strlen+1)*sizeof(DXCHAR)); /*+1 zero terminated string*/
    DXCHAR *buf_indx = buff ;
    str = string->stringx   ;
    while(*str != 0)
    {
        if(*str != this_char)
        {
			 *buf_indx = *str ;
			 buf_indx++ ;
        } 

		str++ ;
    }
    
    /*free the previous memory and set the new one*/
	*buf_indx = 0 ;
    dx_string_setX(string,buff) ;

    return string ;
}


PDX_STRING dx_utf8_replace_char(PDX_STRING string,char *utf8char, char *with_char)
{
	bool no_change = true ;
    DXCHAR this_char =dx_convert_utf8_to_int32(utf8char,dx_utf8_char_byte_count(*utf8char)) ;
    char * ustr = string->stringa ;
	char * c_char ;
	int with_char_len = dx_utf8_char_byte_count(*with_char) ;
   
	DXLONG64 buflen = 0 ; 
    while(*ustr != 0)
    {
      int char_len = 0 ; 
      DXCHAR temp_char = dx_get_utf8_char_ex2(&ustr,&c_char,&char_len) ;
      if(temp_char != this_char) buflen = buflen + char_len ;
	  else
	  {
		  if(no_change == true) no_change = false ;
		  buflen = buflen + with_char_len ; /*we have to calculate the bytes of the new character that it will replace the old one*/
	  }
    }

	if(no_change == true) return string ;

    char *buf = (char*)malloc(buflen+1); /*+1 zero terminated string*/
    char *buf_indx = buf ;
    ustr = string->stringa ;

    while(*ustr != 0)
    {
		int char_len = 0 ;
		DXCHAR temp_char = dx_get_utf8_char_ex2(&ustr,&c_char,&char_len) ;
        if(temp_char != this_char)
        {
          for(int i=0;i<char_len;i++) 
          {
           *buf_indx = *c_char ;
           buf_indx++ ;/*next byte*/
           c_char++     ; /*next byte*/
          }
        
        } 
		else
		{
			  char * with_char_indx = with_char ;
			  for(int i=0;i<with_char_len;i++) 
			  {
			   *buf_indx = *with_char_indx ;
			   buf_indx++ ;/*next byte*/
			   with_char_indx++     ; /*next byte*/
			  }
		}

    }

    /*free the previous memory and set the new one*/
	*buf_indx = 0 ;
    dx_string_setU(string,buf) ;

    return string ;

}

PDX_STRING dx_unicode_replace_char(PDX_STRING string,DXCHAR or_char, DXCHAR with_char)
{
	DXCHAR * str    = string->stringx ;

    while(*str != 0)
    {
        if(*str == or_char) 
		{
		   *str = with_char ;
		}
		str++ ;
    }
    
    return string ;

}

char * dx_utf8_find_word(PDX_STRING str , DXLONG64 from_index , char *word , DXLONG64 *char_indx )
{
    /*utf8 is good and dandy but the variable character size kills me*/
	*char_indx = - 1 ;
	if(str == NULL)  return NULL ;
	if(word == NULL) return NULL ;
	if((from_index < 0 )||(from_index>=str->len)) return NULL ;
	
	/*go to the rignt index*/
	char *str_indx  = str->stringa ;
	for(DXLONG64 i = 0 ; i < from_index ;i++ ) dx_get_utf8_char(&str_indx) ;

	*char_indx = -1 ;

	char *word_indx     = word  ;
	DXLONG64 indx	    = 0  ;
	DXLONG64 first_char = -1 ; 
	char *cur_char = NULL    ;
	int char_len   = 0		 ;
	while(*str_indx != 0)
	{
		DXCHAR word_char = dx_get_utf8_char(&word_indx) ;
		/*check if the character is valid*/
		/*check if the word has reached its end*/
		if(word_char == 0)
		{
		 /*found it*/
		  *char_indx =  first_char ;
		  return cur_char ;
		}

		DXCHAR base_char = dx_get_utf8_char_ex2(&str_indx,&cur_char,&char_len) ;

		if(base_char != word_char)
		{
			if(first_char != -1) first_char = -1;
			word_indx = word ;/*reset*/
		}
		else
		{
			if(first_char == -1) first_char = indx + from_index ;
			/*the next character is being in line by the dx_get_utf8_char*/
		}

		indx++;
	}

	*char_indx = -1 ;

	return NULL     ;

}

char * dx_binary_find_word(PDX_STRING str , DXLONG64 *from_index , char *word)
{
	
	if(str == NULL)  return NULL ;
	if(word == NULL) return NULL ;
	if((*from_index < 0 )||(*from_index>=str->bcount)) return NULL ;
	
	/*go to the rignt index*/
	char *str_indx  = &(str->stringa[*from_index]) ;

	int word_len = strlen(word); /*length in bytes*/
	char *word_indx     = word  ;
	while(*str_indx != 0)
	{
		/*check if the character is valid*/
		/*check if the word has reached its end*/
		if(*word_indx == 0)
		{
		 /*found it*/
			*from_index = *from_index - word_len ;
		    return (str_indx - word_len) ;
		}

		if(*str_indx != *word_indx) 
		{
			word_indx = word ;/*reset*/
			str_indx++ ;
			(*from_index)++ ;
			continue ;
		}

		word_indx++ ;
		str_indx++;
		(*from_index)++ ;
	}


	if(*word_indx == 0 )
	{
		*from_index = *from_index - word_len ;
		return str_indx - word_len ;
	}
	return NULL     ;
}

DXLONG64 dx_unicode_find_word(PDX_STRING str,DXLONG64 from_index, DXCHAR *word )
{
	if(str == NULL)  return -1 ;
	if(word == NULL) return -1 ;
	if((from_index < 0 )||(from_index>=str->len)) return -1 ;

	DXCHAR *str_indx = &(str->stringx[from_index]) ;
	DXCHAR *word_indx  = word  ;
	DXLONG64 indx = 0 ;
	DXLONG64 first_char = -1 ; 
	while(*str_indx != 0)
	{
		/*check if the character is valid*/
		if(*str_indx != *word_indx)
		{
			if(first_char != -1) first_char = -1;
			word_indx = word ;/*reset*/
		}
		else
		{
			if(first_char == -1) first_char = indx+from_index ;
			word_indx++ ;/*next character*/
		}
		/*check if the word has reached its end*/
		if(*word_indx == 0)
		{
		 /*found it*/
		  return first_char ; 	  
		}

		str_indx++ ;
		indx++;
	}

	return -1 ;
}















