#ifndef DXLISTS
#include "dxlists.h"
#endif

#include <thirdparty/zipcuba/zip.h>

#define DXSPFUNCTIONS
/*
  DeusEx 2023. Nikolaos Mourgis. deus-ex.gr
  This module provides special cross platform functions.
  All this functions support and return ONLY utf8 style pdx_strings
*/

#define COPY_SECTION_SUCCESS     0
#define COPY_SECTION_STRAY_OPEN  -501
#define COPY_SECTION_STRAY_CLOSE -502
#define COPY_SECTION_EMPTY       -503
#define COPY_SECTION_NULL_P      -504


bool dxCreateDirectory(PDX_STRING dir);
/*#
 Creates the directory. If failed return -1 else returns 0
#*/


bool dxFileExists(PDX_STRING file) ;
/*#
 Check if the file (it does not check for a directory) exist.
 Return true if it does and false if it does not.
#*/

bool dxDirExists(PDX_STRING dir) ;
/*#
 Check if the directory exist.
 Return true if it does and false if it does not.
#*/

PDX_STRING  dx_IntToStr(DXLONG64 value);
/*#
 Converts a signed number to a string.
 The application must free the PDX_STRING that the function returns
#*/

PDX_STRING  dx_UintToStr(uint64_t value);
/*#
 Converts a unsigned number to a string.
 The application must free the PDX_STRING that the function returns
#*/

PDX_STRING dx_FloatToStr(long double value,int dig) ;
/*
 Converts a double value to a string. the dig[its]
 set the number of decimal digits that the string
 will have.
*/

DXLONG64 dx_StringToInt(PDX_STRING str,bool *error);
DXLONG64 dx_string_to_int(char *str,bool *error);
/*
 Converts a string to an integer
*/

double dx_StringToReal(PDX_STRING str , bool *error);
/*
 Converts a string to a double
*/

void printbitsbyte(unsigned char x);
/*#
 Prints one byte in binary form
#*/
void printbits2byte(uint16_t x);
/*#
 Prints two bytes in binary form
#*/

void printbits4byte(uint32_t x);
/*#
 Prints four bytes in binary form with spaces between bytes
#*/

DXLONG64 dxUtf8PosBinary(PDX_STRING source , PDX_STRING sstr , DXLONG64 *fr_indx);
/*#
 The function search for the sstr in the source starting from fr_indx.fr_index is in CHARACTERS.
 If the function find the sstr then returns the pos(in bytes not in characters) of the first byte of the sstr
 in the source or -1 if the sstr does not exists.
 The fr_indx will have the position of the sstr in CHARACTERS. 
 NOTE : The returned value is the position of the string in RELATION with the fr_indx,
 that means that if the fr_ind is 1 and the returned value is 5 then the actual position
 is 6
#*/

DXLONG64 dxUtf8CharPosBinary(PDX_STRING source , DXUTF8CHAR c , DXLONG64 *fr_indx);
/*#
 The function search for the sstr in the source starting from fr_indx.
 If the function find the sstr then returns the pos(in bytes not in characters) of the first byte of the sstr
 in the source .
 the fr_indx is set to the next character (utf8) after the c
 NOTE : The returned value is the position of the string in RELATION with the fr_indx,
 that means that if the fr_ind is 1 and the returned value is 5 then the actual position
 is 6
#*/

DXLONG64 dxFindPatternInBytes(char * data,DXLONG64 datalen,char * pattern,DXLONG64 from_indx) ;
/*#
  The function find thw pattern (word) in the data. If the pattern doesnot exists, the function returns -1 
  else returns the position of the byte
#*/

PDX_STRING dx_utf8_trim_right(PDX_STRING str,char *utf8characters);
/*#
 The function trims the characters from the left of the str and return the str.
 The str will be changed inside the function
#*/
PDX_STRING dx_utf8_trim_left(PDX_STRING str,char *utf8characters);
/*#
 The function trims the characters from the left of the str and return the str.
 The str will be changed inside the function
#*/

PDX_STRING dx_utf8_trim(PDX_STRING str,char *utf8characters);
/*#
 The function calls the dx_utf8_trim_left and the dx_string_trim_right
#*/

void dxUtf8TrimRight(char** str, char* utf8characters);
/*#
 The function trims the characters from the left of the str .
 The str will be changed inside the function
#*/
void dxUtf8TrimLeft(char** str, char* utf8characters);
/*#
 The function trims the characters from the left of the str.
 The str will be changed inside the function
#*/

void dxUtf8Trim(char** str, char* utf8characters);
/*#
 The function calls the dx_utf8_trim_left and the dx_string_trim_right
#*/

short dxUtf8CpointByteCount(DXUTF8CHAR uchar) ;
/*#
 The function returns how many bytes the uchar codepoint occupies 
#*/

short dxUtf8CharByteCount(char utfc_1byte) ;
/*#
 The function returns how many bytes the utf8 char that has the uchar as first byte occupies 
#*/

void dxRetrieveBytes(char *out_b , char *first_byte,int bc);
/*
 The function returns to the out_ch that MUST be a buffer already created 
 the bytes that find starting from the first_byte and copying bc bytes
 
*/

uint32_t dxConvertUint32ToUTF8(char *buffer, uint32_t utfc);
/*
  converts a utf code point to a utf8 byte sequence
  the buffer must be 5 bytes long and it will be zero terminated.
  the function returns the codepoint length in bytes

*/

uint32_t dxConvertUTF8ToInt32(char *buffer,int len);
/*
 converts the utf8 character to a uint32_t value (codepoint)

*/


void ExplodeEx(PDX_STRINGLIST parts,PDX_STRING source_string,PDX_STRING separator);
/*#
 Gets the parts of a string. The separator is the separator for the parts e.g "," or "->"
#*/

void ExplodeChar(PDX_STRINGLIST parts,PDX_STRING source_string,char separator);
/*#
 Gets the parts of a string. The separator is the separator for the parts , a single byte
#*/

PDX_STRING CopyIndxToChar(PDX_STRING str ,DXUTF8CHAR c, DXLONG64 *indx);
/*#
 Creates and return the string from [indx] until [c].
 If the [c] character is not present , all the remaining string is returned and the indx is equal to
 the string length-1.
 The indx will be set to the position AFTER the c character.
 if the c character is in the end of the string then the indx will be set to string length
 the index is in essence the first byte of the utf8character
#*/

PDX_STRING CopyIndxToIndx(PDX_STRING str , DXLONG64 indx,DXLONG64 to_indx);
/*#
 Creates and return the string from [indx] until [to_indx].
 If the [to_indx] is < 0 then the function will copy until the string end
#*/

PDX_STRING CopyIndx(PDX_STRING str , DXLONG64 indx,DXLONG64 len);
/*#
 Creates and returns [len] characters from the string starting with [indx].
 if len < 0 then the function will copy until the end of the string
#*/

DXLONG64 dxHexToDec(char *hex);
/*
 Convert a hexadecimal number to decimal
*/

PDX_STRING dxDecToHex(uint64_t num );
/*
 Converts a decimal to hexadecimal
*/

char* dxExtractPathFromFilename(char* filename);
/*
 Extract the path from the file name. The function returns the string 
 from the first / or \ it finds starting from the end of the string.
 For example the ./some/path/myfile.hydra it will return 
 ./some/path/

 The return value is a new created string the caller must free it when it not need it anymore.
 If the filename is NULL the function still creates an EMPTY (str[0]=0;) string and returns it 
*/

char* dxExtractFilename(char* filename);
/*
 Extract the file name from the full name. The function returns the string 
 after the last / or \ .
 For example the ./some/path/myfile.hydra it will return 
 myfile.hydra

 The return value is a new created string the caller must free it when it not need it anymore.
 If the filename is NULL the function still creates an EMPTY (str[0]=0;) string and returns it 
*/

DXLONG64 dxFindStringANSI(char* str,DXLONG64 start_from, char* search_for);
/*
  The function [search_for] the string in the str starting from the [start_from]. If it does found it then returns 
  the position of the first character in the str or -1 if the string does not been found
*/
char *dxReplaceStringANSI(char* str, char* search_for, char* replace_with,bool replace_all);
/*
 The function finds the [search] string in the [str] string and replace it with the replace_with. 
 The function returns the new string (malloced in the function so free it when appropriate) 
 if the [search_for] exists and NULL if the search does not exists in the str.
*/

bool dxCharIsLatinOrNumber(char c);
/*returns true if the character is in the latin alphabet set or is a numeric representation*/

bool dxCharIsLatinNumberOr_(char c);
/*returns true if the character is in the latin alphabet set or is a numeric representation or a _ or -*/

bool dxIsCharNumber(char c);
/*
 returns true if the character is a number
*/

bool dxIsCharNumberOrDot(char c);
/*
 returns true if the character is a number or a dot
*/

bool dxIsStrNameSafe(char* name);
/*
  returns true if the name is comprised of latin letters , numeric characters and the '_' '-' 
*/

bool dxIsStrInteger(char* str);
/* returns true if the string is an integer*/

bool dxIsStrReal(char* str);
/* Returns true if the string is a real number*/

bool dxIsStrEqual(char* str1, char* str2);
/*
 Makes a binary compare of all the bytes of the strings 
 and return true if they are equal else return false
*/

bool dxIsCharInSet(char c, char* set);
/*
 Returns true if the c is in the set 
*/

char *dxIsSingleOperator(char *source , char op, char* banned_chars);
/*
 The function gets a character (op) and checks if is neighbour with any of the banned_chars.
 If its is a neighbour of one ofe them the then function returns the char and its neighbour.
 If the character does not have a neighbour then returns the character.
 If the character does not exists at all the function return NULL 
 Keep in mind that you have to free the returned value if not NULL.

 Remarks : The function return after the first encounter of the [op]
*/

char* dxRemoveChar(char* source, char c);
/*
  creates internally a new string identical to source but with out the c character
*/

char* dxRemoveCharExt(char* source, char c,char *string_indents);
/*
  creates internally a new string identical to source but with out the c character
  BUT with a twist. In the string idents you can have the characters that enclose
  strings. Every c character that is in the enclosed strin will be left behind.
  for example the Hello "I'm Hydra" it will become Hello"I'm Hydra" and not
  Hello"I'mHydra" when in the string_indents is the ["] character and the char to remove is the space.
  If the c is one of the string_indents characters the function returns NULL as
  it see it as fatal error
*/

char* dxRemoveCharsExt(char* source, char* remove_c, char* string_indents);
/*
 Same as the dxRermoveCharEx but supports multiple characters for removing
*/

char* dxReplaceChar(char* source, char repl_it , char repl_with);
/*
  creates internally a new string identical to source but with the [repl_it] character replaced with th [repl_width]
*/

char* dxReplaceCharExt(char* source, char repl_it, char repl_with, char* string_indents);
/*
  creates internally a new string identical to source but with the [repl_it] character replaced with th [repl_width]
  BUT with a twist. In the string idents you can have the characters that enclose
  strings. Every [repl_it] character that is in the enclosed string will be left behind.
  for example the Hello "I'm Hydra" it will become Hello-"I'm Hydra" and not
  Hello-"I'm-Hydra" when in the string_indents is the ["] character and the repl = ' ' and the repl_width =  '-'.
  If the c is one of the string_indents characters the function returns NULL as
  it see it as fatal error
*/

char* dxReplaceChars(char* source, char *repl_it, char repl_with);
/*
  creates internally a new string identical to source but with the [repl_it] characters all replaced with th [repl_width] character
*/

char* dxReplaceCharsExt(char* source, char *repl_it, char repl_with, char* string_indents);
/*
  creates internally a new string identical to source but with the [repl_it] characters (can be mutiple like " \t\r\n") replaced with th [repl_width]
  BUT with a twist. In the string idents you can have the characters that enclose
  strings. Every [repl_it] character that is in the enclosed string will be left behind.
  for example the Hello "I'm Hydra" it will become Hello-"I'm Hydra" and not
  Hello-"I'm-Hydra" when in the string_indents is the ["] character and the repl_it = ' ' and the repl_width =  '-'.
  If the c is one of the string_indents characters the function returns NULL as
  it see it as fatal error
*/


char* dxCopySectionFromStr(char **source, char open, char close, char* str_indents,int *status);
/*
  A very usefull function when you have nested ((())) or {{{}}} or [[[]]] etc.
  The source is a string that can have  sections like [open_character] <some str> [close_character]
  and returns the first section that founds. The function ignores the open and close characters that are inside a string 
  (determined by the str_idents). 
  The function creates a new string inside it and return it and the status flag will have the value of
  COPY_SECTION_SUCCESS.
  If an error occurs, the function returns NULL
  and the status flag will have one of the following error codes :
  
  COPY_SECTION_STRAY_OPEN   : The string has more open characters than close ones
  COPY_SECTION_STRAY_CLOSE  : The string has more close characters than open ones
  COPY_SECTION_EMPTY        : A valid section found but has nothing in it e.g ()
  COPY_SECTION_NULL_P       : the source is passed as NULL in the parameters 

  Remarks : The function can parse expressions like (15+3)*(62+(12*1)).
  The parsing starts from left to the right. When a section is found then 
  the source will be point to the next character after the section. To the 
  above example the function will return the 15+3 and the source will point to the *

  If the function reaches the string end then source will point to the terminal zero
  If the function returns an error the source will be unmodified 

*/

char* dxCopySectionFromStrSmart(char** source, char open, char close, char* str_indents, int already_open,int* status);
/*
  A very usefull function when you have nested ((())) or {{{}}} or [[[]]] etc.
  The source is a string that can have  sections like [open_character] <some str> [close_character]
  and returns the first section that founds. The function ignores the open and close characters that are inside a string
  (determined by the str_idents).
  The function creates a new string inside it and return it and the status flag will have the value of
  COPY_SECTION_SUCCESS.
  If an error occurs, the function returns NULL
  and the status flag will have one of the following error codes :

  COPY_SECTION_STRAY_OPEN   : The string has more open characters than close ones
  COPY_SECTION_STRAY_CLOSE  : The string has more close characters than open ones
  COPY_SECTION_EMPTY        : A valid section found but has nothing in it e.g ()
  COPY_SECTION_NULL_P       : the source is passed as NULL in the parameters

  Remarks : The function can parse expressions like (15+3)*(62+(12*1)).
  The parsing starts from left to the right. When a section is found then
  the source will be point to the next character after the section. To the
  above example the function will return the 15+3 and the source will point to the *

  If the function reaches the string end then source will point to the terminal zero
  If the function returns an error the source will be unmodified

  The already_open variable tells the function how many open_char has been already pass.
  This is usefull if we have to copy the section but we KNOW that some openning characters
  have already passed

*/

char* dxRawStringCopy(char* str) ;
/*
 the function copy the str to a new created in the function string that it returns
*/

void dxGobackToCh(char** str_pos,char *str_start, char toc);
/*
  The function starts from the str_pos and go backwards until reach the str_start or find the toc
*/

void dxGoBackWhileChars(char **str_indx,char *str_start, char * chars);
/*
 The function will set the str_indx to the first character that will find
 different than ones of the chars starting from str_indx and procceed to the start of the string
*/

void dxGoForwardToCh(char** str_pos, char toc);
/*
  The function starts from the str_pos and go forward until reach the end of the string or find the toc
*/

void dxGoForwardWhileChars(char **str_indx, char * chars);
/*
 The function will set the str_indx to the first character that will find
 different than ones of the chars
*/
void dxRightTrimFastChars(char* str_indx, char* chars);
/*
 for a fast trim of characters the function will set to 0 the first character that will find
 different than ones of the chars starting from the end of the string
*/

bool dxCheckSectionPairing(char *str, char open_char, char close_char, char* str_indent);
/*
 The function checks if the section has the same amount of open and close characters.
 If the section does not close right the function return false else returns true .
 The function can determine if the character are in a string section and ignores it 
 (via str_indent)
*/

bool dxCheckSectionPairingSmart(char* str, char open_char, char close_char, char* str_indent,int already_open);
/*
 The function checks if the section has the same amount of open and close characters.
 If the section does not close right the function return false else returns true .
 The function can determine if the character are in a string section and ignores it
 (via str_indent)
 The already_open variable tells the function how many open_char has been already pass. 
 This is usefull if we have to check the section but we KNOW that some openning characters
 have already passed
*/

bool dxItsStrEncapsulated(char* str, char open_c, char close_c);
/*
 The function parses the string and returns true if its encapsulated 
 e.g (my(string)) is encapsulated but my(string) is not.

 Remarks : The function does not do error checking, before 
 call it, call the dxCheckSectionPairing first. 
  
*/

bool dxItsStrEncapsulatedExt(char* str, char open_c, char close_c,char *str_indent);
/*
 The function parses the string and returns true if its encapsulated
 e.g (my(string)) is encapsulated but my(string) is not.
 The function ignores the open and close characters when the characters are inside a section
 that is defined by the str_indent e.g for Hydra+ the str idents are the '`' and the '"' characters 

 Remarks : The function does not do error checking, before
 call it, call the dxCheckSectionPairing first.

*/


char* dxGetNextWord(char** str_start, char* seps, char* str_indent, bool ignore_parenthesis , bool ignore_indexes, char* sep_found);
/*
 The function parses the string and accumulate the string until found 
 one of the seps. The function is aware of strings and can check for string sections 
 based on the contents of the str_ident. eg str_ident = "`'".
 The function can be made aware of two sections that it will ignore in the same way as the seps 
 where in a string section. This is usefull if you want to retrieve sentences that are logically
 structure with [] or ()
 The sep_found will be set to the seperator that separated the word or 0 if the end of string was reached 
 The function returns the word or NULL and the str_start has the next valid character
 of the string or 0

 Remarks : If the function finds a seperator immediately after the previous separator, the
 return is NULL but the sep_found has the separator that the function found
*/

DXLONG64 dxCharExistsInStr(char* str, char c, char* str_indent);
/*
 the function searches for the c char into the str and if it finds it and its not
 in a string section (the str_ident has the string indentificators like ` or ")
 returns the position that it found it in the string. If the character was not found 
 then the function returns -1
*/

DXLONG64 dxCharExistsInStrExt(char* str, char c, char* str_indent);
/*
 the function searches for the c char into the str and if it finds it and its not
 in a string section (the str_ident has the string indentificators like ` or ")
 OR in a section od () or []
 returns the position that it found it in the string. If the character was not found 
 then the function returns -1
*/

char* dxCopyStrToChar(char** str_indx, char c, char* str_indent);
/*
 The function starts from str_indx and copy all the characters until c.
 It can detect when to ignore the character that is belonging in a string section 
 via the str_indent .
 If the function does not found the c , all the string is returned.
 The str_indx will point to the c character or in the terminating 0
*/

char* dxCopyStrToCharReverse(char** str_indx, char c, char* str_indent);
/*
 The function starts from str_indx and copy all the characters until c. going to the str_start
 It can detect when to ignore the character that is belonging in a string section 
 via the str_indent .
 If the function does not found the c , all the string is returned.
 The str_indx will point to the c character or in the str_start
*/


char* dxCopyStrToChars(char** str_indx, char *chars, char* str_indent);
/*
 The function starts from str and copy all the characters until a char in the [chars] is found .
 It can detect when to ignore the character that is belonging in a string section 
 via the str_indent .
 If the function does not found any character , all the string is returned.
 The str_indx will point to the c character or in the terminating 0
*/

void dxCopyStrToStr(char** dest, char* source);
/*
 the function copies the source into the dest, and when the copy is done
 the dest, points to the next character position after the last character copied.
 The function does not know about string length and do the copy blindly
 until the zero in the source 
*/

char dxConvertChar(char if_char, char to_char,char this_char);
/*
 The function gets this_char and if its the if_char returns the to_char 
*/

char dxGetNextValidChar(char* str, char* invalid_chars);
/*
 The function return the first char it founds that is not in the invalid_chars.
 If there is not any other char except invalid chars then the function returns 0
*/

bool dxIsTextAString(char* str, char str_ident);
/*
 if the str is encapsulate in str_ident the function return true else return false.
 the function does not understand control characters and must been trimmed, the function
 do not understand concatenations and syntax, it just check the first and the last character 
 (but it is fast!)
 */

bool dxIsStringValidEncaps(char* str, char* str_idents);
/*
 the function check if the str is encapsulated in one of the str_idents.
 the function will return true if all the string is correct encapsulated 
 and false if the string has a break in encapsulation or it is not encapsulated at all
*/


bool dxCharExists(char **str, char c, char* str_idents);
/*
 return true if the c char exists in the str string outside of a str_idents section
 the str will point to the character or to the string end if the character does not exists
*/

DXLONG64 dxPower(DXLONG64 base, DXLONG64 exp);
/*
 a function to calculate exponents for integers
*/


uint64_t dxGetTickCount();
/*
 a cross platform (linux,windows) function that return the current tickcount from the system start
 
*/

char *CopyStrToChar(char * str, DXLONG64 *from_indx,char to_char);
/*#
 Returns the substring from_indx until find the to_char. 
 Upon completion the routine returns the substring as a new created string
 and the from_indx will have the position of the to_char.
 if the to_char does not exists then the string from from_indx until end is returned and the
 from_indx is set to -1
#*/

char *utf8CopyIndexToIndex(char *utf8str , DXLONG64 from_index , DXLONG64 to_index);
/*#
  Copy the urf8 characters between (and including) from_index and to_index
#*/

char *utf8CopyIndex(char *utf8str , DXLONG64 from_index , DXLONG64 char_cnt);
/*#
  Copy the char_cnt urf8 characters from_index
#*/

char *utf8CopyToCh(char *utf8str , DXLONG64 *from_index , char *to_char);
/*#
 Returns the substring from_indx until find the to_char. 
 Upon completion the routine returns the substring as a new created string
 and the from_indx will have the position of the to_char.
 if the to_char does not exists then the string from from_indx until end is returned and the
 from_indx is set to -1
#*/


PDX_STRING dxHostNameToIp(char* hostname);
/*#
 Return the ip of the hostname , if the hostname cannot be resolved the 
 a PDX_STRING with the same string as the hostname is returned 
#*/


void dx_PaddWithZeros(PDX_STRING str , int total_len) ;
/*#
 The function checks the str length i its smaller than the total len and 
 if it is padds the str with zeros 
#*/

DXLONG64 dx_GetFileSize(FILE *f);
/*
 return the file size of the open file. THE FUNCTION REWIND the pointer of the file , so
 use this function before do any other action in the file to avoid misreading or miswritings  
 */

PDX_STRING dx_LoadUTF8TextFromFile(PDX_STRING filename,bool *error);
/*#
  The function loads the bytes from a file and stores thenm as a utf8 
  string. If the encoding is not utf8 then the string will be unreadable.
#*/

void dxSleep(int millis) ;
/*#
 wraper for Sleep() and sleep()
#*/

PDX_STRING dxUrlEncodeUTF8(PDX_STRING orstr,bool space_as_plus);
/*#
 Encodes for url the str 
#*/

PDX_STRING dxUrlDecodeUTF8(PDX_STRING orstr,bool plus_as_space);
/*#
 Decodes the url  
#*/

bool dxGetFiles(PDX_STRING dir,PDX_STRINGLIST list,bool recursive) ;
/*#
 Fills the string list with the files in the directory.
 If recursive is true then all the files in the nested directories will be returned.
 If an error occurs , false is returned else true
#*/

bool dxGetDirs(PDX_STRING dir,PDX_STRINGLIST list,bool recursive) ;
/*#
 Fills the string list with the directories in the directory.
 If recursive is true then all the directories in the nested directories will be returned
 If an error occurs , false is returned else true
#*/

bool dxGetFileDirs(PDX_STRING dir,PDX_STRINGLIST list,bool recursive) ;
/*#
 Fills the string list with the files and directories in the directory.
 If recursive is true then all the files and directories in the nested directories will be returned
 If an error occurs , false is returned.
#*/

bool dxDeleteDir(PDX_STRING dir,bool deleteRoot);
/*#
 The function deletes the directory if the deleteRoot is true and all its contents.
 or only its contents if the deleteRoot is false 
 Return true if all its ok or false otherwise
#*/


bool dxFilesToZip(PDX_STRING fileName, PDX_STRINGLIST fileList);
/*#
 The function zips the files in the list in one zip
#*/

bool dxDirToZip(PDX_STRING fileName, PDX_STRING dirName) ;
/*#
 The function zips the contents of the dir and create a file name with the contents
#*/

bool dxExtractZip(PDX_STRING zipName, PDX_STRING toDir) ;
/*#
 The function zips the contents of the dir and create a file name with the contents
#*/

PDX_STRING dxStrToXorHex(PDX_STRING text,PDX_STRING key);
/*#
  The function creates a hexadecimal encoded xored string
#*/

PDX_STRING dxStrUnXorHex(PDX_STRING text,PDX_STRING key);
/*#
  The function creates a hexadecimal encoded xored string
#*/

PDX_STRING dxStrReverse(PDX_STRING str);
/*#
 The function reverses the string (BINARY REVERSE the utf8 characters will be erroneus transform)
#*/

char * dxBytesXor(char * bytes,DXLONG64 bcount,PDX_STRING key) ;
/*#
 The function creates a buffer with the same byte count as the bytes 
 but every byte of the buffer is xored with the key
#*/



void dx_strcpy(char *dest,char *source) ;

/*****************************************************************************/


/* ****************** implementation ***************** */

bool dxCreateDirectory(PDX_STRING dir)
{
  #ifdef _LINUX
    struct stat st = {0};
    if (stat(dir->stringa, &st) == -1)
    {
      if (mkdir(dir->stringa,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)==0) return true;
      else
          return false ;
    }
     else return true ;
  #endif
  #ifdef _WIN32
    struct stat st = {0};
    if (stat(dir->stringa, &st) == -1) 
    {
      if(_mkdir(dir->stringa) == 0) return true ;
      else 
          return false ;
    }
    else return true ;
  #endif
}

bool dxFileExists(PDX_STRING file)
{
  if(file->type != dx_utf8)
  {
    printf("[dxFileExists] The [file] must be in a utf8 string form.");
    return false ;
  }

  #ifdef _LINUX
   struct stat   buffer;
   return (stat (file->stringa, &buffer) == 0);

  #endif // _LINUX

  #ifdef _WIN32
   /* windows need the string to be utf16 to work ... */
   PDX_STRING str = dx_string_convertW(file) ;
   DWORD dwAttrib = GetFileAttributes(str->stringw);
   dx_string_free(str);
   if(dwAttrib == INVALID_FILE_ATTRIBUTES)
   {
     return false ;
   }
   return (dwAttrib != INVALID_FILE_ATTRIBUTES &&!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
  #endif // _WIN32
  //printf("Define _WIN32 or _LINUX for the compilation of the routine [dxFileExists]");
  return false ;

}


bool dxDirExists(PDX_STRING dir) 
{
    if( access( dir->stringa, 0 ) == 0 )
    {

        struct stat status;
        stat( dir->stringa, &status );
        return (status.st_mode & S_IFDIR) != 0;
    }
    return false;

#ifdef _LINUX

#endif
#ifdef _WIN32

    PDX_STRING dirw = dx_string_convertW(dir) ;
    if(dirw == NULL) 
    {
        printf("Error -> dxDirExists : The convertion to WIDESTRING failed. %s\n",dir->stringa);
        return false ;
    }
    DWORD dwAttrib = GetFileAttributes(dirw->stringw);
    dx_string_free(dirw);
   return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
         (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#endif

}


PDX_STRING dx_IntToStr(DXLONG64 value)
{

 char val[33];

 if (value==0)
 {
   val[0] = 48 ;
   val[1] = 0  ;
   return dx_string_createU(NULL,val)  ;
 }
 char m;
 bool neg=false;

 if (value<0)
 {
   value=value*(-1);
   neg=true;
 }

 /*
  for ommiting the reversing of the string we will
  do a litle alchemy!
 */

 char * vindx = &val[31] ;
 val[32] = 0  ; // terminate the string ;
 while (value!=0)
  {

    m      = value % 10 ;
    value  = value / 10 ;
    *vindx = m + 48 ; // get the right digit

    vindx--  ;

  }
    vindx++ ; // restore the right position
    //check if we need a prefix
    if (neg == true)
    {
     vindx--;
     *vindx = '-' ;
    }

    PDX_STRING str = dx_string_createU(NULL,vindx);

    return str;
}

PDX_STRING dx_UintToStr(uint64_t value)
{

 char val[33];

 if (value==0)
 {
   val[0] = 48 ;
   val[1] = 0  ;
   return dx_string_createU(NULL,val)  ;
 }
 char m;

 /*
  for ommiting the reversing of the string we will
  do a litle alchemy!
 */

 char * vindx = &val[31] ;
 val[32] = 0  ; // terminate the string ;
 while (value!=0)
  {

    m      = value % 10 ;
    value  = value / 10 ;
    *vindx = m + 48 ; // get the right digit

    vindx--  ;

  }
    vindx++ ; // restore the right position

    PDX_STRING str = dx_string_createU(NULL,vindx);

    return str;
}

PDX_STRING dx_FloatToStr(long double value,int dig)
{

    if ( dig > 9 )dig = 9;

    DXLONG64 body  = value ; // get the integer part

    long double dek = value - body ;
    if(dek < 0)dek = dek * -1 ;

    PDX_STRING bdy = dx_IntToStr(body);

    if ( dek == 0 )
    {
        char decim[10] ;
        for(int i = 0;i<dig;i++)
         decim[i]='0';

        decim[dig] = 0 ; // terminate string

        PDX_STRING dec  = dx_string_createU(NULL,decim);
        PDX_STRING dot  = dx_string_createU(NULL,".")  ;
        PDX_STRING res = NULL  ;
        PDX_STRING midd = NULL ;
        if(dig != 0)
        {
         midd = dx_string_concat(bdy,dot)    ;
         res  = dx_string_concat(midd,dec)              ;
        }
        else
            res = dx_string_createU(NULL,bdy->stringa) ;

        dx_string_free(bdy);
        dx_string_free(dec);
        dx_string_free(dot);
        dx_string_free(midd);
        return res;
    }


    /* find the decimal part */

    char decim[10] ;
    int i ;
    for (i = 1; i <= dig ; i++ )
    {
      long double base = (1 / pow(10,i)) ;
      char prt =((unsigned int)(dek / base)) +48 ;
      decim[i-1] = (char)prt;
      dek = dek - ((unsigned int)(dek / base)) * base ;
    }

    decim[i-1] = 0 ;


    /* -------------------------------- */
    PDX_STRING res = NULL ;
    if(dig > 0)
    {
        PDX_STRING dekad = dx_string_createU(NULL,decim) ;
        PDX_STRING dot   = dx_string_createU(NULL,".") ;
        PDX_STRING midd  = dx_string_concat(bdy,dot);
        res   = dx_string_concat(midd,dekad);
    
        dx_string_free(bdy)    ;
        dx_string_free(dekad ) ;
        dx_string_free(dot)    ;
        dx_string_free(midd)   ;
    }
    else
    {
      res   = bdy ;
    }

    return res;
}

DXLONG64 dx_StringToInt(PDX_STRING str,bool *error)
{
    *error = false ; 
    if( str->len == 0 )
    {
        *error = true ;
        return 0 ;
    }
    if(dxIsStrInteger(str->stringa) == false) 
    {
     *error = true ;
     return 0;
    }
    DXLONG64 c = 0, sign = 0, x = 0;
    const char *p = str->stringa;

    for(c = *(p++); (c < 48 || c > 57); c = *(p++)) {if (c == 45) {sign = 1; c = *(p++); break;}};
    for(; c > 47 && c < 58; c = *(p++)) x = (x << 1) + (x << 3) + c - 48;
    return sign ? -x : x;
}

DXLONG64 dx_string_to_int(char *str,bool *error)
{
    *error = false ; 
    if(str == NULL)
    {
        *error = true ;
        return 0 ;
    }
    
    if( str[0] == 0)
    {
        *error = true ;
        return 0 ;
    }

    if(dxIsStrInteger(str) == false) 
    {
     *error = true ;
     return 0;
    }
    DXLONG64 c = 0, sign = 0, x = 0;
    const char *p = str;

    for(c = *(p++); (c < 48 || c > 57); c = *(p++)) {if (c == 45) {sign = 1; c = *(p++); break;}};
    for(; c > 47 && c < 58; c = *(p++)) x = (x << 1) + (x << 3) + c - 48;
    return sign ? -x : x;
}

double dx_StringToReal(PDX_STRING str , bool *error)
{
    *error = false ;
    if(dxIsStrReal(str->stringa) == false) 
    {
     *error = true ;
     return 0;
    }

    return atof(str->stringa) ;
}

void printbitsbyte(unsigned char x)
{
    for(int i=sizeof(x)<<3; i; i--)
        putchar('0'+((x>>(i-1))&1));
}

void printbits2byte(uint16_t x)
{
    for(int i=sizeof(x)<<3; i; i--)
        putchar('0'+((x>>(i-1))&1));
}

void printbits4byte(uint32_t x)
{
    for(int i=sizeof(x)<<3; i; i--)
    {
        if ((i % 8) == 0 ) putchar(' ');
       putchar('0'+((x>>(i-1))&1));
    }

}


DXLONG64 dxUtf8PosBinary(PDX_STRING source , PDX_STRING sstr , DXLONG64 *fr_indx)
{
     if(source == NULL) return -1 ;
     if(sstr == NULL)   return -1 ;
     if(source->len == 0) return -1 ;
     if(sstr->len == 0) return -1   ;

     DXUTF8CHAR cc1,cc2       ;
     char *utf8_1 , *utf8_2  , *utf8tmp ;
     int charlen ;
     utf8_1 = dx_string_return_utf8_pos(source,*fr_indx) ;
     if(utf8_1 == NULL) return -1 ;
     utf8_2 = sstr->stringa   ;
     DXLONG64 binary_pos = 0   ;

     /*
       we will parse all the characters of the source and for every one we will check against
       the sstr characters. if we find a match of the first character we will set the flag onit = true .
       we will check the other characters. If the sstr reach its end and the [onit] is true the
       the function will return the current character position in the source.
       If the next character does not match then the onit will be set to false , and the sstr character index
       will be reset.
     */
     bool onit = false ;
     do
     {
         cc1 =dx_get_utf8_char_ex2(&utf8_1,&utf8tmp,&charlen) ; 
         if (cc1 == 0)
         {
             if(onit == true) return binary_pos ;
             else
             return -1     ; // end of the source string
         }
         cc2 = dx_get_utf8_char(&utf8_2); // get next character of the separator
         if (cc2 == 0)
         {
             /*the end of the sstring check if the flag onit is true or not*/
             if(onit == true)
             {
                 return binary_pos ; // all ok! we found one
             }
             else
             {
                 //reset and fetch the first character
                 utf8_2 = sstr->stringa ;
                 cc2 = dx_get_utf8_char(&utf8_2);
             }
         }

         if(cc1 != cc2)
         {
           /*reset the sstr*/
           utf8_2 = sstr->stringa ;
           onit   = false         ;
         }
          else
          {
              /*make the onit true*/
              if(onit == false) onit = true ;
          }

         //the binary_pos is the binary position of the character as the utf8 characters are variable in byte size
         binary_pos = binary_pos + charlen ;
         (*fr_indx)++; /*update the character pos*/
     }while(true) ;


  return -1 ;
}

DXLONG64 dxUtf8CharPosBinary(PDX_STRING source , DXUTF8CHAR c , DXLONG64 *fr_indx)
{
     if(source == NULL) return -1 ;
     if(source->len == 0) return -1 ;
     if(c == 0) return -1   ;

     DXUTF8CHAR cc1      ;
     char *utf8_1 , *utf8tmp ;
     int charlen ;
     utf8_1 = dx_string_return_utf8_pos(source,*fr_indx) ;
     DXLONG64 chpos = 0   ;

     while(true)
     {
         cc1 =dx_get_utf8_char_ex2(&utf8_1,&utf8tmp,&charlen) ;
         if (cc1 == 0)
         {
             return -1     ; // end of the source string
         }

         if(cc1 == c)
          {
            chpos = chpos + charlen ;
            //check if the next character is not 0
            cc1 = dx_get_utf8_char_ex2(&utf8_1,&utf8tmp,&charlen);
            if(cc1 == 0) *fr_indx = source->len -1 ;
            else
            {
                *fr_indx = *fr_indx + chpos; // effectively point to the next character
            }
            return chpos-charlen ;
          }

       chpos = chpos + charlen ;

    }



  return -1 ;

}


DXLONG64 dxFindPatternInBytes(char * data,DXLONG64 datalen,char * pattern,DXLONG64 from_indx) 
{

     if(data == NULL)      return -1 ;
     if(pattern == NULL)   return -1 ;
     if(datalen == 0)      return -1 ;
     DXLONG64 patlen = strlen(pattern) ;
     if(patlen == 0)       return -1   ;
     if(from_indx < 0 )    return -1   ;
     if((datalen-1) < from_indx) return -1 ;

     char cc1,cc2       ;
     DXLONG64 pos = 0   ;

     /*
       we will parse all the characters of the source and for every one we will check against
       the pattern characters. if we find a match of the first character we will set the flag onit = true .
       we will check the other characters. If the pattern reach its end and the [onit] is true the
       the function will return the current character position in the source.
       If the next character does not match then the onit will be set to false , and the sstr character index
       will be reset.
     */
     bool onit = false ;
     DXLONG64 cur_pos = from_indx ;
     DXLONG64 pat_pos = 0 ;
 
     while(cur_pos < datalen)
     {
         cc1 = data[cur_pos]    ;/*next byte in the data*/
         cc2 = pattern[pat_pos] ;/*next byte in the pattern*/

         if (cc2 == 0)
         {
             /*the end of the pattern, check if the flag onit is true or not*/
             if(onit == true)
             {
                 return pos ; // all ok! we found it
             }
             else
             {
                 //reset and fetch the first character
                 pat_pos = 0 ;
                 cur_pos++ ;
                 continue ;
             }
         }

         if(cc1 != cc2)
         {
           /*reset the pattern*/
           pat_pos = 0            ;
           onit   = false         ;
           cur_pos++              ;
           continue               ;
         }
          else
          {
              /*make the onit true*/
              if(onit == false) 
              {
                  onit = true ;
                  //set the pos
                  pos = cur_pos ; 
              }
          }

         pat_pos++ ;
         cur_pos++ ;
     }

  return -1 ;
}


DXLONG64 dxCompareBytes(char * data,DXLONG64 datalen,char * pattern,DXLONG64 from_indx) 
{
     /*return 0 if the patterns bytes are analogous to the datas bytes startinf from indx */

     if(data == NULL)      return 0 ;
     if(pattern == NULL)   return 0 ;
     if(datalen == 0)      return 0 ;
     DXLONG64 patlen = strlen(pattern) ;
     if(patlen == 0)       return 0   ;
     if(from_indx < 0 )    return 0   ;
     if((datalen-1) < from_indx) return 0 ;

     char cc1,cc2       ;
     DXLONG64 pos = 0   ;

     /*
       we will parse all the characters of the source and for every one we will check against
       the pattern characters. if we find a match of the first character we will set the flag onit = true .
       we will check the other characters. If the pattern reach its end and the [onit] is true the
       the function will return 1.
       If the next character does not match then the function will return 0 .
     */
     DXLONG64 cur_pos = from_indx ;
     DXLONG64 pat_pos = 0 ;
 
     while(cur_pos < datalen)
     {
         cc1 = data[cur_pos]    ;/*next byte in the data*/
         cc2 = pattern[pat_pos] ;/*next byte in the pattern*/

         if (cc2 == 0)
         {
             /*the end of the pattern*/ 
            return 1 ; // all ok! we found it
         }

         if(cc1 != cc2)
         {
           return 0 ;
         }
         pat_pos++ ;
         cur_pos++ ;
     }

  return 0 ;
}

bool dx_utf8_is_char_in(DXUTF8CHAR c,const char *utf8characters)
{
    DXUTF8CHAR utf8c = 0 ;
    while(true)
    {
      utf8c = dx_get_utf8_char( (char**) (&utf8characters));
      if(utf8c == 0) break ;
      if (utf8c == c) return true ;

    }

   return false ;
}


PDX_STRING dx_utf8_trim_right(PDX_STRING str,char *utf8characters)
{

  /*
   This is trickier than a left trim ,
   we will parse all the characters from the right. if we found trimable characters
   we will continue , if we find the end of the string and the right_trim is false we will return an empty string ,
   no usable characters inside the string. If we find a usable character , we
   set the flag right_trim = true .
   if we found the end of the string the we return the string as is.
   if we found a trimable character we set the flag on_trim = true
   and store the byte_cnt to the previous character.
   if we found the end of the string then we return the string until the byte_cnt
   if we found a non trimable character then we set the flag on_trim = false
   if we reach the end of the string and the on_trim is false then
   we return all the string , else we return the string from start until the
   byte_cnt.
  */

  if(str == NULL) return NULL  ;
  if(str->len == 0) return str ;

  char * prev_chr ;
  char * stru = str->stringa ;
  int blen ;
  DXLONG64 byte_cnt = 0 ;
  bool right_trim = false ;
  bool on_trim    = false ;
  DXLONG64 tmp_b_count = 0 ;

  DXUTF8CHAR utf8c ;
  do
    {
      utf8c = dx_get_utf8_char_ex2(&stru,&prev_chr,&blen);
      if(utf8c == 0) break ;
      if (dx_utf8_is_char_in(utf8c,utf8characters) == true)
      {
        if (on_trim == false) on_trim = true ;
        tmp_b_count = tmp_b_count + blen ;
      }
       else
       {
           if(right_trim == false) right_trim = true ; // nevertheless we found a usable character so we are ready for right trim
           else
           {
               //we are in the right trim , check if the on_trim is true and make it false
               if(on_trim == true)
               {
                 on_trim = false ; // the characters are "protected" by usable characters after them
                 byte_cnt = byte_cnt+tmp_b_count ; // add this characters to the string by calculate them bytes for the copy later
                 tmp_b_count = 0 ;
               }

           }
           byte_cnt = byte_cnt+blen ; //accumulate the bytes that are valid for the string
       }
  }
  while(true);

  /*
   create a new trim based of the flags
  */
  if (right_trim == false)
  {
      /*no usefull characters in all the string return empty string*/
      dx_string_createU(str,"");
      return str ;
  }
  else
   if(on_trim == false)
    {
     //return the string as is , no trimable characters in the end
     return str ;
    }
     else
     {
         /*now we have trimable characters , so we will copy only the bytes we need*/
         if(byte_cnt > str->bcount ) // for catching bugs
         {
             printf("Fatal Error in byte_cnt. The value is bigger than the length of the string : %d > %d ",(int)byte_cnt,(int)str->len);
             return str ;
         }

         DXLONG64 siz = byte_cnt+1 ;
         char *data = (char*)malloc(siz) ;
         memcpy(data,str->stringa,byte_cnt) ;
         data[siz-1] = 0 ; // terminate with zero
         dx_string_setU(str,data) ;
     }


  return str ;

}

PDX_STRING dx_utf8_trim_left(PDX_STRING str,char *utf8characters)
{
  if(str == NULL) return NULL  ;
  if(str->len == 0) return str ;

  char * prev_chr ;
  char * stru = str->stringa ;

  DXUTF8CHAR utf8c ;
  do
    {
      utf8c = dx_get_utf8_char_ex(&stru,&prev_chr);
      if(utf8c == 0) break ;
      if (dx_utf8_is_char_in(utf8c,utf8characters) != true)
      {
        break ; // if the character is not in the for trimming characters then we break the operation
      }
  } while(true);

  /*
  create a new string starting from the prev_chr as the call of the
  dx_get_utf8_char_ex reposition the pointer to the next character.
  if utf8c is 0 then the string is empty as all the other characters are to being trimed
  */
  if (utf8c == 0)dx_string_set_empty(str) ;
  else
  {
    DXLONG64 siz = strlen(prev_chr)+1 ;
    char *data = (char*)malloc(siz) ;
    if (data == NULL) return str;
    memcpy(data,prev_chr,siz) ; // copy the terminating zero too
    dx_string_setU(str,data) ;
  }

  return str ;

}

PDX_STRING dx_utf8_trim(PDX_STRING str,char *utf8characters)
{
  dx_utf8_trim_left(str,utf8characters)  ;
  dx_utf8_trim_right(str,utf8characters) ;
  return str ;
}

void dxUtf8TrimRight(char** str, char* utf8characters)
{
    /*
  This is trickier than a left trim ,
  we will parse all the characters from the left. if we found trimable characters
  we will continue , if we find the end of the string and the right_trim is false we will return an empty string ,
  no usable characters inside the string. If we find a usable character , we
  set the flag right_trim = true .
  if we found the end of the string the we return the string as is.
  if we found a trimable character we set the flag on_trim = true
  and store the byte_cnt to the previous character.
  if we found the end of the string then we return the string until the byte_cnt
  if we found a non trimable character then we set the flag on_trim = false
  if we reach the end of the string and the on_trim is false then
  we return all the string , else we return the string from start until the
  byte_cnt.
 */

    if (str == NULL) return ;
    if (*str == NULL) return;
    if (strlen(*str) == 0) return ;

    char* prev_chr;
    char* stru = *str;
    int blen;
    DXLONG64 byte_cnt = 0;
    bool right_trim = false;
    bool on_trim = false;
    DXLONG64 tmp_b_count = 0;

    DXUTF8CHAR utf8c;
    do
    {
        utf8c = dx_get_utf8_char_ex2(&stru, &prev_chr, &blen);
        if (utf8c == 0) break;
        if (dx_utf8_is_char_in(utf8c, utf8characters) == true)
        {
            if (on_trim == false) on_trim = true;
            tmp_b_count = tmp_b_count + blen;
        }
        else
        {
            if (right_trim == false) right_trim = true; // nevertheless we found a usable character so we are ready for right trim
            else
            {
                //we are in the right trim , check if the on_trim is true and make it false
                if (on_trim == true)
                {
                    on_trim = false; // the characters are "protected" by usable characters after them
                    byte_cnt = byte_cnt + tmp_b_count; // add this characters to the string by calculate them bytes for the copy later
                    tmp_b_count = 0;
                }

            }
            byte_cnt = byte_cnt + blen; //accumulate the bytes that are valid for the string
        }
    } while (true);

    /*
     create a new trim based of the flags
    */
    if (right_trim == false)
    {
        /*no usefull characters in all the string return empty string*/
        free(*str);
        *str = (char*)malloc(1);
        *str[0] = 0;
        return;
    }
    else
        if (on_trim == false)
        {
            //return the string as is , no trimable characters in the end
            return ;
        }
        else
        {
            /*now we have trimable characters , so we will copy only the bytes we need*/
            if (byte_cnt > strlen(*str)) // for catching bugs
            {
                printf("Fatal Error in byte_cnt. The value is bigger than the length of the string : %d > %d ", (int)byte_cnt, (int)strlen(*str));
                return ;
            }

            DXLONG64 siz = byte_cnt + 1;
            char* data = (char*)malloc(siz);
            memcpy(data, *str, byte_cnt);
            data[siz - 1] = 0; // terminate with zero
            free(*str);
            *str = data;
        }


    return ;

}

void dxUtf8TrimLeft(char** str, char* utf8characters)
{
    if (str == NULL)  return ;
    if (*str == NULL) return ;
    if (strlen(*str) == 0) return ;

    char* prev_chr;
    char* stru = *str;

    DXUTF8CHAR utf8c;
    do
    {
        utf8c = dx_get_utf8_char_ex(&stru, &prev_chr);
        if (utf8c == 0) break;
        if (dx_utf8_is_char_in(utf8c, utf8characters) != true)
        {
            break; // if the character is not in the for trimming characters then we break the operation
        }
    } while (true);

    /*
    create a new string starting from the prev_chr as the call of the
    dx_get_utf8_char_ex reposition the pointer to the next character.
    if utf8c is 0 then the string is empty as all the other characters are to being trimed
    */
    if (utf8c == 0)
    {
        free(*str);
        *str = (char*)malloc(1) ;
        *str[0] = 0;
    }
    else
    {
        DXLONG64 siz = strlen(prev_chr) + 1;
        char* data = (char*)malloc(siz);
        if (data == NULL) return;
        memcpy(data, prev_chr, siz); // copy the terminating zero too
        free(*str);
        *str = data;
    }

    return;

}

void dxUtf8Trim(char** str, char* utf8characters)
{
    dxUtf8TrimLeft(str, utf8characters);
    dxUtf8TrimRight(str, utf8characters);
    return;
}

short dxUtf8CpointByteCount(DXUTF8CHAR utfc)
{
    if (utfc <= 0x7F)   return 1;
    else 
    if (utfc <= 0x07FF)  return 2;
    else 
    if (utfc <= 0xFFFF)  return 3;
    if (utfc <= 0x10FFFF) return 4;
    else return 0;   /*error unknown codepoint*/
}

short dxUtf8CharByteCount(char utfc_1byte)
{
   if (GetBit(utfc_1byte,0) == 0) return 1 ;

   if ((GetBit(utfc_1byte,0) == 1)&&(GetBit(utfc_1byte,1) == 1)&&(GetBit(utfc_1byte,2) == 0)) return 2 ;
   if ((GetBit(utfc_1byte,0) == 1)&&(GetBit(utfc_1byte,1) == 1)&&(GetBit(utfc_1byte,2) == 1)&&(GetBit(utfc_1byte,3) == 0))
       return 3 ;
  if ((GetBit(utfc_1byte,0) == 1)&&(GetBit(utfc_1byte,1) == 1)&&(GetBit(utfc_1byte,2) == 1)&&(GetBit(utfc_1byte,3) == 1)&&
      (GetBit(utfc_1byte,4) == 0 )) return 4 ;

        return 0 ;
}

void dxRetrieveBytes(char *out_b , char *first_byte,int bc)
{
 	if(bc == 1)
	{
	  out_b[0] = *first_byte ;
	  out_b[1] = 0 ;
	}
	else if(bc == 2)
	{
		out_b[0] = *first_byte ;
		out_b[1] = *(first_byte+1);
		out_b[2] = 0 ;
	}
	else if(bc == 3)
	{
	  	out_b[0] = *first_byte ;
		out_b[1] = *(first_byte+1);
		out_b[3] = *(first_byte+2) ;
		out_b[4] = 0 ;
	}
	else if(bc == 4)
	{
	  	out_b[0] = *first_byte ;
		out_b[1] = *(first_byte+1) ;
		out_b[3] = *(first_byte+2) ;
		out_b[4] = *(first_byte+3) ;
		out_b[5] = 0 ;
	}

    return ;
}

uint32_t dxConvertUint32ToUTF8(char *buffer, uint32_t utfc)
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

uint32_t dxConvertUTF8ToInt32(char *buffer,int len)
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

void ExplodeEx(PDX_STRINGLIST parts,PDX_STRING source_string,PDX_STRING separator)
{

 DXLONG64 indx     = 0   ;
 DXLONG64 seppos   = 0   ;
 DXLONG64 char_pos = 0   ;

 if(parts == NULL)          return ;
 if(source_string == NULL)  return ;
 if(separator == NULL)      return ;

 if(source_string->len == 0 ) return ;
 if(separator->len == 0 )
 {
     dx_stringlist_add_raw_string(parts,source_string->stringa) ;
     return ;
 }

  //if the separator was not found then return all the string 
  seppos = dxUtf8PosBinary(source_string,separator,&char_pos);
  if (seppos == -1)
  {
      // there is not any part in the string
     dx_stringlist_add_raw_string(parts,source_string->stringa) ;
     return ;
  }

 
 char_pos = 0 ;
 seppos   = 0 ;
 while (indx <= source_string->bcount)
 {

  seppos = dxUtf8PosBinary(source_string,separator,&char_pos);
  if (seppos == -1)
  {
      if(indx == source_string->bcount) return ;
      // there is not any other part in the string

      char * data = (char*)malloc((source_string->bcount-indx)+1);
      memcpy_from_indx(data,source_string->stringa,indx,source_string->bcount-indx) ;
      data[source_string->bcount-indx] = 0 ; //remove the separator from the data and terminate the string
      indx = -1  ; // we handle the string positions in the byte level and not in the character level

      PDX_STRING part = dx_string_create_bU(data);
      dx_stringlist_add_string(parts,part) ;

     return ;
  }
   else
    {
      char * data = (char*)malloc(seppos+1);
      memcpy_from_indx(data,source_string->stringa,indx,seppos) ;
      data[seppos-(separator->bcount)] = 0 ; //remove the separator from the data terminate the string
      indx = indx + seppos  ; // we handle the string positions in the byte level and not in the character level


      PDX_STRING part = dx_string_create_bU(data)      ;
      dx_stringlist_add_string(parts,part) ;
    }
 }

  return;
}


/*###############################################################*/

PDX_STRING _get_line_to_char(char **buff_indx , char term )
{
    /*
     helper function for the ExplodeChar.
     returns the string until the term. The buff_indx will point to the next character after the 
     term. If the first character that the function finds is the term then returns a NULL PDX_STRING.
     To determine the caller the end of the string it can check the buff_indx, it must be 0
     If the term does not exists then the function returns the string until the end.
    */
    char * str_indx   = *buff_indx   ;
    if((*str_indx == 0)||(*str_indx == term)) return NULL ;
    /*find the term*/
    DXLONG64 term_pos = 0            ;
    while ((*str_indx != term)&&(*str_indx != 0))
    {
        str_indx++ ;
        term_pos++ ;
    }

    PDX_STRING nstr = NULL ;
    /*check the state*/
    if(*str_indx == term)
    {
        /*the terminator was found, copy the string*/
        
        char * str = malloc(term_pos+1) ; /*we will not copy the terminator*/
        str[term_pos] = 0               ; /*terminate the string*/
        memcpy(str,*buff_indx,term_pos) ; /*use the indx as count so the terminator will not be copied*/
        nstr = dx_string_create_bU(str) ;
        str_indx++ ; /*pass the termnator!*/
    }

    if(*str_indx == 0)
    {
        /*the terminator was not found, copy the string*/
        
        char * str = malloc(term_pos+1) ; /*copy the last byte too*/ 
        str[term_pos] = 0             ; /*terminate the string*/
        memcpy(str,*buff_indx,term_pos);
        nstr = dx_string_create_bU(str) ;
    }

    *buff_indx = str_indx ;
    return nstr ;
}

void ExplodeChar(PDX_STRINGLIST parts,PDX_STRING source_string,char separator)
{

     if(parts == NULL)          return ;
     if(source_string == NULL)  return ;
     /*get the strings*/
     char *buff_indx  = source_string->stringa ;
     while(true)
     {
       
       PDX_STRING str = _get_line_to_char(&buff_indx , separator ) ;
       /*check the state*/
       if(str != NULL) dx_stringlist_add_string(parts,str) ;
       if(*buff_indx == 0) break ;
     }
      return;
}

/*################################################################*/


PDX_STRING CopyIndxToChar(PDX_STRING str ,DXUTF8CHAR c, DXLONG64 *indx)
{
/*#
 Creates and return the string from [indx] until [c].
 If the [c] character is not present , all the remainig string is returned and the indx is equal to
 the string length.
 The indx will be set to the position AFTER the c character.
 if the c character is in the end of the string then the indx will be set to string length
#*/

  //get the position of the character
  DXLONG64 tindx = *indx ;
  DXLONG64 seppos = dxUtf8CharPosBinary(str,c,&tindx);
  if (seppos == -1)
  {
      // the character is not present
     if (*indx >= str->len) return dx_string_createU(NULL,"") ;

     char *utf8str = dx_string_return_utf8_pos(str,tindx);
     tindx = str->len - 1;
     *indx = tindx ;
     return dx_string_createU(NULL,utf8str) ;
  }

  //copy the string
  char * buf = (char*)malloc(seppos+1) ;
  memcpy(buf,dx_string_return_utf8_pos(str,*indx),seppos) ;
  buf[seppos] = 0 ;//terminate the string and remove the separator character simultaneously
  *indx = tindx ;
  return dx_string_create_bU(buf) ;
}

PDX_STRING CopyIndxToIndx(PDX_STRING str , DXLONG64 indx,DXLONG64 to_indx)
{
  //get the position of the character
  if(indx < 0) return NULL ;
  if( (to_indx > str->bcount-1)||(to_indx < 0) ) to_indx = str->bcount - 1 ;
 
  if(to_indx < indx ) return NULL ;

  //copy the string
  DXLONG64 blen = (to_indx - indx) + 1 ; /*we want the [indx] and the [to_indx] characters too*/
  char * buf = (char*)malloc(blen+1) ;

  memcpy(buf,&(str->stringa[indx]),blen) ;
  buf[blen] = 0 ;//terminate the string 
  return dx_string_create_bU(buf) ;
}

PDX_STRING CopyIndx(PDX_STRING str , DXLONG64 indx,DXLONG64 len)
{
  //get the position of the character
  if(indx < 0) return NULL ;
  if( ((indx+len) > str->bcount-1)||(len < 0) ) len = str->bcount ;

  //copy the string
  char * buf = (char*)malloc(len+1) ;

  memcpy(buf,&(str->stringa[indx]),len) ;
  buf[len] = 0 ;//terminate the string
  return dx_string_create_bU(buf) ;
 
}


DXLONG64 dxHexToDec(char *hex)
{
    DXLONG64 len = strlen(hex)-1;
    DXLONG64 i = 0 ;
    DXLONG rem,decnum = 0  ;
    while(len>=0)
    {
        rem = hex[len];
        if(rem>=48 && rem<=57)
            rem = rem-48;
        else if(rem>=65 && rem<=70)
            rem = rem-55;
        else if(rem>=97 && rem<=102)
            rem = rem-87;
        else return -1 ;
        decnum = decnum + (rem*pow(16, i));
        len--;
        i++;
    }

    return decnum ;
}


PDX_STRING dxDecToHex(uint64_t num)
{
    char * hex = (char*)malloc(128) ;
    char * hex_indx = hex ;
    if (num == 0) 
    {
        *hex_indx = '0';
        hex_indx++;
        *hex_indx = 0 ;
        return dx_string_create_bU(hex) ;
    }

    while (num > 0) 
    {
        DXLONG64 remainder = num % 16;

        if (remainder < 10)
            *hex_indx = remainder + '0'; /*proceed to find the character*/
        else
            *hex_indx = remainder + 'A' - 10;

        hex_indx++ ;

        num = num / 16 ;
    }
    
    *hex_indx = 0 ;
    PDX_STRING hexs = dx_string_create_bU(hex) ;
    PDX_STRING hexstr = dxStrReverse(hexs)     ;
    dx_string_free(hexs);
    return hexstr ;
}

char *dxExtractPathFromFilename(char *filename)
{

    if (filename == NULL)
    {
        char* str = (char*)malloc(1);
        str[0] = 0; 
        return str;
    }
    int len = strlen(filename) ;
    char *path = (char*)malloc(len+1) ;
    memcpy(path, filename, len);
    path[len] = 0 ; /*terminate it*/
    /*search for a / or \ */
    char* findx = &path[len-1] ; 
    while (findx != path)
    {
        if ((*findx == '\\') || (*findx == '/'))
        {
            *(findx + 1) = 0; /*terminate the string effectivelly cut the filename part*/
            break;
        }

        findx--;
    }

    /*check if there is not any / or \*/

    if ((findx == path) && ((*findx == '\\') || (*findx == '/')))
    {
        *(findx + 1) = 0; /* the filename it was like /hello.hydra */
    }
    else
        if (findx == path)
        {
            *findx = 0;
        }
        else
        {
            *(findx+1) = 0;
        }

    return path;

}

char* dxExtractFilename(char* filename)
{

    if (filename == NULL)
    {
        char* str = (char*)malloc(1);
        str[0] = 0;
        return str;
    }
    int len = strlen(filename);
    /*search for a / or \ */
    char* findx  = &filename[len - 1];
    int fpos = len-1 ;
    while (findx != filename)
    {
        if ((*findx == '\\') || (*findx == '/'))
        {   
            fpos++;
            break;
        }
        fpos--;
        findx--;
    }

    /*check if there is not any file name*/
    if (fpos == len - 1)
    {
        char* fname = (char*)malloc(1);
        fname[0] = 0;
        return fname ;
    }

    if (fpos < 0) fpos = 0; /*well Im paranoic!*/
   
    int clen = len - fpos ;
    

    char* fname = (char*)malloc(clen+1);
    for (int i = fpos; i < fpos+clen; i++)
    {
        fname[i - fpos] = filename[i];
    }
    fname[clen] = 0;
    return fname;

}

DXLONG64 dxFindStringANSI(char* str, DXLONG64 start_from, char* search_for)
{
    if ((str == NULL) || (search_for == NULL)) return -1  ;
    if (start_from >= strlen(str)) return -1          ;

    bool in_found = false ;
    DXLONG64 pos  = 0     ;

    char *strindx = &str[start_from];
    char *sindx   = search_for;
    DXLONG64 indx = start_from;
    while (*strindx != 0)
    {
        if ((*sindx == 0) && (in_found == true)) return pos; /*The string exists!!*/

        if ((*strindx == *sindx) && (in_found == false))
        {
            /*the first character of the searched string is found*/
            pos       = indx;
            in_found = true; 
        }
        else
            if (*strindx != *sindx)
            {
                /*oops the searched string must be reset*/
                pos = 0          ;
                sindx = search_for   ;
                in_found = false ;
            }

        indx++;
        strindx++;
        if(in_found == true) sindx++  ; /*next character*/
    }

    if (in_found == true) return pos ;

    return -1;
}

char *dxReplaceStringANSI(char* str, char* search_for, char* replace_with,bool replace_all)
{
  /*find the first occurence*/
    DXLONG64  pos = dxFindStringANSI(str, 0, search_for) ;
    if (pos == -1) return NULL;

    DXLONG64 sflen  = strlen(search_for)  ;
    DXLONG64 stlen  = strlen(str)          ;
    DXLONG64 rwlen  = strlen(replace_with) ;
    /*we will create a list that we will use to save the individual buffers for the replace */

    PDX_LIST buf_list = dx_list_create_list();
    if (buf_list == NULL) return NULL ;

    char* buf = (char*)malloc(pos + 1) ; /*+1 for the terminating 0 the pos points to the first character of the search_for so we will
                                           use its value as a length for the string before the search_for */
    memcpy(buf, str, pos) ; /*copy the string before the search_for*/
    buf[pos] = 0 ;
    PDXL_OBJECT obj = dxl_object_create() ;

    obj->flags   = 0 ; /*0 means normal text*/
    obj->obj     = buf ;
    obj->int_key = pos ;
    dx_list_add_node_direct(buf_list, obj) ;
    /*add the object represent the [replace_with] string*/
    obj = dxl_object_create();
    obj->flags = 1; /*1 means the [replace_with] text*/
    obj->int_key = rwlen ;
    obj->obj = replace_with;
    dx_list_add_node_direct(buf_list, obj);

    /*check if we will replace all the occurences of the [search_for]*/
    if (replace_all == true)
    {
        while (true)
        {
            pos = pos + sflen; /*position to the character after the [search_for]*/
            DXLONG64 prev_pos = pos ;
            pos = dxFindStringANSI(str, pos, search_for) ;
            if (pos == -1)
            {
                /*copy the remainder string*/
                if((stlen -prev_pos) <= 0) break;

                buf = malloc((stlen - prev_pos) + 1) ; /*allocate the memory for the remainder string*/
                if (buf == NULL) break ; /*fails silently*/

                memcpy(buf, &str[prev_pos], (stlen - prev_pos));
                buf[stlen- prev_pos] = 0;
                
                PDXL_OBJECT obj = dxl_object_create();
                obj->flags = 0; /*0 means normal text*/
                obj->obj = buf;
                obj->int_key = (stlen  - prev_pos) ;
                dx_list_add_node_direct(buf_list, obj)  ;
                break;
            }

            /*get the string before the pos*/

            buf = malloc((pos - prev_pos)+1); /*allocate the memory for the string*/
            if (buf == NULL) break; /*fails silently*/
            memcpy(buf, &str[prev_pos], pos-prev_pos);
            buf[pos-prev_pos] = 0;
            /*add the buffer in the list*/
            PDXL_OBJECT obj = dxl_object_create();

            obj->flags = 0; /*0 means normal text*/
            obj->obj = buf;
            obj->int_key = (pos - prev_pos);
            dx_list_add_node_direct(buf_list, obj);
            /*add the object represent the [replace_with] string*/
            obj = dxl_object_create();
            obj->flags = 1; /*1 means the [replace_with] text*/
            obj->obj = replace_with  ;
            obj->int_key = rwlen     ;
            dx_list_add_node_direct(buf_list, obj);


        }


    } /*replace_all == true*/
    else
    {
        pos = pos + sflen+1; /*position to the character after the [search_for]*/
       
            /*copy the remainder string*/
            if ((stlen - pos)+1 > 0)
            {

                buf = malloc(((stlen - pos)+1) + 1); /*allocate the memory for the remainder string*/
                if (buf != NULL)
                {

                    memcpy(buf, &str[pos-1], (stlen - pos)+1);
                    buf[(stlen - pos)+1] = 0;

                    PDXL_OBJECT obj = dxl_object_create();
                    obj->flags = 0; /*0 means normal text*/
                    obj->obj = buf;
                    obj->int_key = (stlen - pos)+1;
                    dx_list_add_node_direct(buf_list, obj);
                }
            }
        
    }

    /*construct the buffer*/
    /*get the total length*/
    DXLONG64 tot_len = 0;
    PDXL_NODE  node = buf_list->start;
    while (node != NULL)
    {
        PDXL_OBJECT obj = node->object   ;
        tot_len = tot_len + obj->int_key ;
        node = node->right;
    }

    /*allocate the memory*/
    buf = (char*)malloc(tot_len + 1);
    if (buf == NULL)
    {
        /*free the memory and return NULL*/
        PDXL_NODE  node = buf_list->start;
        while (node != NULL)
        {
            PDXL_OBJECT obj = node->object;
            if (obj->flags == 0) free((char*)obj->obj) ;
            free(obj) ;
            node = node->right;
        }

        dx_list_delete_list(buf_list);
        return NULL;
    }

    /*copy the memory*/
    char* buf_indx = buf ;
    node = buf_list->start;
    while (node != NULL)
    {
        PDXL_OBJECT obj = node->object;
        char *str_indx = (char*)obj->obj ;
        while (*str_indx != 0)
        {
            *buf_indx = *str_indx ;
            str_indx++ ;
            buf_indx++ ;
        }
        node = node->right;
    }

    buf[tot_len] = 0  ; /*terminate*/

    /*free memory*/
    node = buf_list->start;
    while (node != NULL)
    {
        PDXL_OBJECT obj = node->object;
        if (obj->flags == 0) free((char*)obj->obj);
        free(obj);
        node = node->right;
    }

    dx_list_delete_list(buf_list);

    return buf;
}

bool dxCharIsLatinOrNumber(char c)
{

    char* validc = "qwertyuioplkjhgfdsazxcvbnmQWERTYUIOPLKJHGFDSAZXCVBNM1234567890";
    int len = strlen(validc);
    for (int i = 0; i < len; i++)
    {
        if (validc[i] == c) return true;
    }

    return false;
}

bool dxCharIsLatinNumberOr_(char c)
{

    char* validc = "qwertyuioplkjhgfdsazxcvbnmQWERTYUIOPLKJHGFDSAZXCVBNM1234567890_-";
    int len = strlen(validc);
    for (int i = 0; i < len; i++)
    {
        if (validc[i] == c) return true;
    }

    return false;
}


bool dxIsCharNumber(char c)
{
    char* nums = "1234567890";
    int len = strlen(nums);
    for (int i = 0; i < len; i++)
    {
        if (nums[i] == c) return true;
    }

    return false;
}

bool dxIsCharNumberOrDot(char c)
{
    char* validc = "1234567890.";
    int len = strlen(validc);
    for (int i = 0; i < len; i++)
    {
        if (validc[i] == c) return true;
    }

    return false;
}

bool dxIsCharNumberDotOrSign(char c)
{
    char* validc = "+-1234567890.";
    int len = strlen(validc);
    for (int i = 0; i < len; i++)
    {
        if (validc[i] == c) return true;
    }

    return false;
}

bool dxIsStrNameSafe(char* name)
{
    while (*name != 0)
    {
        if (dxCharIsLatinNumberOr_(*name) == false) return false;
        name++ ;
    }

    return true;
}


bool dxIsStrInteger(char* str)
{
    if (str == NULL) return false;
    int len = strlen(str);
    /*check for sign first*/
    if ((str[0] == '+') || (str[0] == '-')) str++; /*ommit the first character*/
    while(*str != 0)
    {
        if (dxIsCharNumber(*str) == false) return false;
        str++ ;
    }

    return true;
}

bool dxIsStrReal(char* str)
{
    if (str == NULL) return false;
    int len = strlen(str);
    /*check for sign first*/
    if ((str[0] == '+') || (str[0] == '-')) str++; /*ommit the first character*/
    while (*str != 0)
    {
        if (dxIsCharNumberOrDot(*str) == false) return false;
        str++;
    }

    return true;
}


bool dxIsStrEqual(char* str1, char* str2)
{
    if (strlen(str1) != strlen(str2)) return false;
    while (*str1 != 0)
    {
        if (*str1 != *str2) return false;
        str1++;
        str2++;
    }

    return true;
}


bool dxIsCharInSet(char c, char* set)
{
    while (*set != 0)
    {
        if (*set == c) return true;
        set++;
    }

    return false;
}

char* dxIsSingleOperator(char *source, char op, char* banned_chars)
{

    char* indx_src = source ;
    char* ret = NULL ;
    bool found = false;
    while (*indx_src != 0)
    {
        if (*indx_src == op)
        {
            ret = (char*)malloc(3);
            ret[0] = op ;
            ret[1] = 0  ;
            if (dxIsCharInSet(*(indx_src - 1), banned_chars) == true)
            {
                ret[0] = *(indx_src - 1);
                ret[1] = op ;
                ret[2] = 0  ;
                return ret;
            }

            if (dxIsCharInSet(*(indx_src + 1), banned_chars) == true)
            {
                ret[0] = op;
                ret[1] = *(indx_src + 1);
                ret[2] = 0;
                return ret;
            }

            return ret;
        }

        indx_src++ ;
    }

    return NULL ;
}


char* dxRemoveChar(char* source, char c)
{
    if (source == NULL) return NULL ;
    /*create a buffer as large as the original string*/
    char* nstr = (char*)malloc(strlen(source) + 1);
    char* nstr_indx = nstr;
    while (*source != 0)
    {
        if (*source != c)
        {
            *nstr_indx = *source;
            nstr_indx++;
        }
        source++;
    }

    *nstr_indx = 0; /*terminate string*/

    return nstr;
}

char *dxRemoveCharExt(char* source, char c, char* string_indents)
{

    if (source == NULL) return NULL;
    if (dxIsCharInSet(c, string_indents) == true) return NULL;

    char str_ident_c = 0;

    /*create a buffer as large as the original string*/
    char* nstr = (char*)malloc(strlen(source) + 1);
    char* nstr_indx = nstr;
    while (*source != 0)
    {
        /*check if we have to enter the no remove state*/
        if ((str_ident_c == 0) && (dxIsCharInSet(*source, string_indents) == true))
            str_ident_c = *source;
        else
            if ((str_ident_c != 0) && (*source == str_ident_c))
                str_ident_c = 0 ;
        
        if ((*source == c) && (str_ident_c == 0))
        {
            //do nothing :D
        }
        else
        {
            //add the character in the new string
            *nstr_indx = *source;
            nstr_indx++;
        }
        source++;
    }

    *nstr_indx = 0; /*terminate string*/

    return nstr;
}


char* dxRemoveCharsExt(char* source, char* remove_c, char* string_indents)
{

    if (source == NULL) return NULL;
    char* rem_indx = remove_c;
    while (*rem_indx != 0)
    {
        if (dxIsCharInSet(*rem_indx, string_indents) == true) return NULL;
        rem_indx++;
    }
    char str_ident_c = 0;

    /*create a buffer as large as the original string*/
    char* nstr = (char*)malloc(strlen(source) + 1);
    char* nstr_indx = nstr;
    while (*source != 0)
    {
        /*check if we have to enter the no remove state*/
        if ((str_ident_c == 0) && (dxIsCharInSet(*source, string_indents) == true))
            str_ident_c = *source;
        else
            if ((str_ident_c != 0) && (*source == str_ident_c))
                str_ident_c = 0;

        if ((dxIsCharInSet(*source, remove_c) == true) && (str_ident_c == 0))
        {
            /*ommit the chars*/
            source++;
            continue;
        }
        else
        {
            //add the character in the new string
            *nstr_indx = *source;
        }
        nstr_indx++;
        source++;
    }

    *nstr_indx = 0; /*terminate string*/

    return nstr;
}


char* dxReplaceChar(char* source, char repl_it , char repl_with)
{
    if (source == NULL) return NULL;
    /*create a buffer as large as the original string*/
    char* nstr = (char*)malloc(strlen(source) + 1);
    char* nstr_indx = nstr;
    while (*source != 0)
    {
        if (*source != repl_it)
        {
            *nstr_indx = *source;
        }
        else
        {
            *nstr_indx = repl_with;
        }

        nstr_indx++;
        source++;
    }

    *nstr_indx = 0; /*terminate string*/

    return nstr;
}

char* dxReplaceCharExt(char* source, char repl_it, char repl_with , char* string_indents)
{

    if (source == NULL) return NULL;
    if (dxIsCharInSet(repl_it, string_indents) == true) return NULL;

    char str_ident_c = 0;

    /*create a buffer as large as the original string*/
    char* nstr = (char*)malloc(strlen(source) + 1);
    char* nstr_indx = nstr;
    while (*source != 0)
    {
        /*check if we have to enter the no remove state*/
        if ((str_ident_c == 0) && (dxIsCharInSet(*source, string_indents) == true))
            str_ident_c = *source;
        else
            if ((str_ident_c != 0) && (*source == str_ident_c))
                str_ident_c = 0;

        if ((*source == repl_it) && (str_ident_c == 0))
        {
            *nstr_indx = repl_with;
        }
        else
        {
            //add the character in the new string
            *nstr_indx = *source;
        }
        nstr_indx++;
        source++;
    }

    *nstr_indx = 0; /*terminate string*/

    return nstr;
}

char* dxReplaceChars(char* source, char* repl_it, char repl_with)
{
    if (source == NULL) return NULL  ;
    if (repl_it == NULL) return NULL ;
    /*create a buffer as large as the original string*/
    char* nstr = (char*)malloc(strlen(source) + 1);
    char* nstr_indx = nstr;
    while (*source != 0)
    {
        if (dxIsCharInSet(*source , repl_it) == false)
        {
            *nstr_indx = *source;
        }
        else
        {
            *nstr_indx = repl_with;
        }

        nstr_indx++;
        source++;
    }

    *nstr_indx = 0; /*terminate string*/

    return nstr;
}

char* dxReplaceCharsExt(char* source, char *repl_it, char repl_with, char* string_indents)
{

    if (source == NULL) return NULL;
    char *repl_indx = repl_it;
    while (*repl_indx != 0)
    {
        if (dxIsCharInSet(*repl_indx, string_indents) == true) return NULL;
        repl_indx++;
    }
    char str_ident_c = 0;

    /*create a buffer as large as the original string*/
    char* nstr = (char*)malloc(strlen(source) + 1);
    char* nstr_indx = nstr;
    while (*source != 0)
    {
        /*check if we have to enter the no remove state*/
        if ((str_ident_c == 0) && (dxIsCharInSet(*source, string_indents) == true))
            str_ident_c = *source;
        else
            if ((str_ident_c != 0) && (*source == str_ident_c))
                str_ident_c = 0;

        if ((dxIsCharInSet(*source, repl_it) == true) && (str_ident_c == 0))
        {
            *nstr_indx = repl_with;
        }
        else
        {
            //add the character in the new string
            *nstr_indx = *source;
        }
        nstr_indx++;
        source++;
    }

    *nstr_indx = 0; /*terminate string*/

    return nstr;
}


char* dxCopySectionFromStr(char** source, char open_ch, char close_ch, char* str_indents, int* status)
{
   
    *status = COPY_SECTION_NULL_P ;
    if (source == NULL) return NULL;
    if (*source == NULL) return NULL;

    int   open_chars   = -1 ;
    char  str_indent = 0 ;
    char* start_copy = NULL;
    char* end_copy   = NULL;
    char* source_indx = *source;
    
    int   chars_in_section = 0 ;

    while (*source_indx != 0)
    {

        /*check if we close a string*/
        if ((str_indent != 0) && (*source_indx == str_indent))
        {
            str_indent = 0 ;
            if (open_chars > 0) chars_in_section++;
            source_indx++  ;
            continue;
        }
        /*check if we are inside a string */
        if (str_indent != 0)
        {
            /*do not count open close characters*/
            if (open_chars > 0) chars_in_section++;
            source_indx++;
            continue;
        }
        /*check if we found a string*/
        if ((str_indent == 0) && (dxIsCharInSet(*source_indx, str_indents) == true))
        {
            str_indent = *source_indx ;
            if (open_chars > 0) chars_in_section++;
            source_indx++;
            continue;
        }

        /*count the open/close characters*/
        if (*source_indx == open_ch)
        {
            if (open_chars == -1)
            {
                start_copy = source_indx + 1; /*start of the expression*/
                open_chars = 1 ;
            }
            else
            {
                /*this char belongs in the section so we add it in the chars_in_section */
                open_chars++;
                chars_in_section++;
            }
            source_indx++;
            continue;
        }
        else
            if (*source_indx == close_ch)
            {
                if (open_chars == -1)
                {
                    /*too many closing section characters*/
                    *status = COPY_SECTION_STRAY_CLOSE ;
                    return NULL;
                }

                open_chars--;
                /*check if we found the section*/
                if (open_chars == 0)
                {
                    source_indx--;
                    end_copy = source_indx;
                    break ;
                }
            }

        if (open_chars > 0) chars_in_section++ ;
        source_indx++;
    }

    /*check if we found a valid section*/
    if (open_chars > 0)
    {
        *status = COPY_SECTION_STRAY_OPEN ;
        return NULL;
    }

    if (chars_in_section == 0)
    {
        *status = COPY_SECTION_EMPTY ;
        *source = start_copy + 1; /*if the section is empty then next valid character is the start_copy+1 */
        return NULL ;
    }

    *status = COPY_SECTION_SUCCESS;
    /*all good return section*/
    char* nstr = (char*)malloc(chars_in_section+1);

    if (nstr == NULL)
    {
        *status = COPY_SECTION_NULL_P;
        return NULL;
    }

    char* nstr_indx = nstr;
    while (start_copy != end_copy)
    {
        *nstr_indx = *start_copy;
        nstr_indx++;
        start_copy++;
    }
    
    *nstr_indx = *start_copy ; /*copy last character*/
    nstr_indx++;
    *nstr_indx = 0;/*terminate string*/

    /*set the source*/
    *source = (end_copy + 2);// pass the last close character and onto the next one 

    return nstr ;

}


char* dxCopySectionFromStrSmart(char** source, char open_ch, char close_ch, char* str_indents, int already_open, int* status)
{
    *status = COPY_SECTION_NULL_P;
    if (source == NULL) return NULL;
    if (*source == NULL) return NULL;

    int   open_chars = -1 ;
    if (already_open != 0) open_chars = already_open;
    char* start_copy = NULL;
    if (open_chars > -1) start_copy = *source;/*we do not wait to find the first { as it is already being calculated*/
    
    char  str_indent = 0;
    char* end_copy = NULL;
    char* source_indx = *source;

    int   chars_in_section = 0;
    int comment_type = 0 ;


    while (*source_indx != 0)
    {

        /*check for the comments --------------------*/
        if(str_indent == 0) /*if we are not in a string section*/
        {
            if(comment_type == 1)
            {
              if((*source_indx == '*')&&(*(source_indx+1) == ']'))
              {
                /*exit comment section*/
                comment_type = 0 ;
                source_indx      = source_indx + 2 ; /*next valid character*/
                chars_in_section = chars_in_section + 2 ;
              }
            }
            else
            if(comment_type == 2)
            {
              if(*source_indx == 10)
              {
                  /*exit comment , new line*/
                  comment_type = 0 ;
                  source_indx = source_indx + 1; /*next valid character*/
                  chars_in_section = chars_in_section + 1 ;
              }
            }

            if(comment_type == 0)
            {
                if((*source_indx == '[')&&(*(source_indx+1) == '*'))
                {
                  /*enter comment section*/
                  comment_type = 1 ;
                  source_indx = source_indx + 2 ; /*next valid character*/
                  chars_in_section = chars_in_section + 2;
                }
                else
                    if((*source_indx == '#')||(*source_indx == '?'))
                    {
                        comment_type = 2 ;
                        source_indx++ ; /*next valid character*/
                        chars_in_section++ ;
                    }
            }

            /*if we are in a comment section then ommit any character*/
            if(comment_type != 0) 
            {
                /*next character*/
                source_indx++ ;
                chars_in_section++ ;
                continue ;
            }
        }
        /*-------------------------------------------*/

        /*check if we close a string*/
        if ((str_indent != 0) && (*source_indx == str_indent))
        {
            str_indent = 0;
            if (open_chars > 0) chars_in_section++;
            source_indx++;
            continue;
        }
        /*check if we are inside a string */
        if (str_indent != 0)
        {
            /*do not count open close characters*/
            if (open_chars > 0) chars_in_section++;
            source_indx++;
            continue;
        }
        /*check if we found a string*/
        if ((str_indent == 0) && (dxIsCharInSet(*source_indx, str_indents) == true))
        {
            str_indent = *source_indx;
            if (open_chars > 0) chars_in_section++;
            source_indx++;
            continue;
        }

        /*count the open/close characters*/
        if (*source_indx == open_ch)
        {
            if (open_chars == -1)
            {
                start_copy = source_indx + 1; /*start of the expression*/
                open_chars = 1;
            }
            else
            {
                /*this char belongs in the section so we add it in the chars_in_section */
                open_chars++;
                chars_in_section++;
            }
            source_indx++;
            continue;
        }
        else
            if (*source_indx == close_ch)
            {
                if (open_chars == -1)
                {
                    /*too many closing section characters*/
                    *status = COPY_SECTION_STRAY_CLOSE;
                    return NULL;
                }

                open_chars--;
                /*check if we found the section*/
                if (open_chars == 0)
                {
                    source_indx--;
                    end_copy = source_indx;
                    break;
                }
            }

        if (open_chars > 0) chars_in_section++;
        source_indx++;
    }

    /*check if we found a valid section*/
    if (open_chars > 0)
    {
        *status = COPY_SECTION_STRAY_OPEN;
        return NULL;
    }

    if (chars_in_section == 0)
    {
        *status = COPY_SECTION_EMPTY;
        *source = start_copy + 1; /*if the section is empty then next valid character is the start_copy+1 */
        return NULL;
    }

    *status = COPY_SECTION_SUCCESS;
    /*all good return section*/
    char* nstr = (char*)malloc(chars_in_section + 1);

    if (nstr == NULL)
    {
        *status = COPY_SECTION_NULL_P;
        return NULL;
    }

    char* nstr_indx = nstr;
    while (start_copy != end_copy)
    {
        *nstr_indx = *start_copy;
        nstr_indx++;
        start_copy++;
    }

    *nstr_indx = *start_copy; /*copy last character*/
    nstr_indx++;
    *nstr_indx = 0;/*terminate string*/

    /*set the source*/
    *source = (end_copy + 2);// pass the last close character and onto the next one 

    return nstr;
}

char* dxRawStringCopy(char* str)
{

    char* nstr = malloc(strlen(str) + 1);
    if (nstr == NULL) return NULL ;
    char* nstr_indx = nstr ;
    while (*str != 0)
    {
        *nstr_indx = *str ;
        nstr_indx++       ;
        str++             ;
    }

    *nstr_indx = 0;

    return nstr ;
}



void dxGobackToCh(char** str_pos, char* str_start, char toc)
{
    if ((str_pos == NULL) || (str_start == NULL)) return; /*nothing to do*/
    while (*str_pos != str_start)
    {
        if (*(*str_pos) == toc) return;
        (*str_pos)--;
    }

}

void dxGoForwardToCh(char** str_pos, char toc)
{
    if (str_pos == NULL) return; /*nothing to do*/
    while (*(*str_pos) != 0)
    {
        if (*(*str_pos) == toc) return;
        (*str_pos)++;
    }
}

void dxGoForwardWhileChars(char** str_indx, char* chars)
{
    if ((str_indx == NULL) ||(chars == NULL)) return; /*nothing to do*/

    while (*(*str_indx) != 0)
    {
        if ( dxIsCharInSet(*(*str_indx),chars) == false) return;
        (*str_indx)++;
    }
}

void dxGoBackWhileChars(char **str_indx,char *str_start, char * chars)
{
    if ((str_indx == NULL) ||(chars == NULL)) return; /*nothing to do*/

    while (*str_indx != str_start)
    {
        if ( dxIsCharInSet(*(*str_indx),chars) == false) return;
        (*str_indx)--;
    }    
}


void dxRightTrimFastChars(char* str_indx, char* chars)
{
    if ((str_indx == NULL) || (chars == NULL)) return; /*nothing to do*/
    /*go to the end*/
    char* start = str_indx ;
    while (*str_indx != 0) str_indx++;
    /*reset the position to the first valid character*/
    if(start!=str_indx)  str_indx--;
    while (str_indx != start)
    {
        if (dxIsCharInSet(*str_indx, chars) == false) break;
        str_indx--;
    }
    /*check the case that the string has only one character that is not valid*/
    if (dxIsCharInSet(*str_indx, chars) == true) *str_indx = 0;
    else
        *(str_indx + 1) = 0;
}

bool dxCheckSectionPairing(char *str, char open_char, char close_char, char* str_indent)
{
    /*
      the function checks if the pairings are valid. If for any reason the open_chars become < 0
      then the expression has not correct pairings
    */
    int open_chars = 0 ;
    char str_i = 0     ;
    char* strint = ""  ;
    if (str_indent == NULL) str_indent = strint;

    while (*str != 0)
    {

        if (str_i != 0)
        {
            if (str_i == *str) str_i = 0;
        }
        else
        {
            if (dxIsCharInSet(*str, str_indent) == true)
                str_i = *str;
            else
            {
                if (*str == open_char) open_chars++;
                else
                    if (*str == close_char) open_chars--;

                if (open_chars < 0) return false;
            }
        }

        str++;
    }

    if (open_chars != 0) return false;
    else
        return true;
}

bool dxCheckSectionPairingSmart(char* str, char open_char, char close_char, char* str_indent, int already_open)
{
    /*
      the function checks if the pairings are valid. If for any reason the open_chars become < 0
      then the expression has not correct pairings

      the function will check for comment sections like [**] # or ?

    */
    int open_chars = already_open;
    char str_i     = 0;
    char* strint   = "";
    if (str_indent == NULL) str_indent = strint;

    int comment_type = 0 ; // 0 - not in comment 1 in multiline comment 2 in one line comment  

    while (*str != 0)
    {
        /*check for the comments --------------------*/
        if(str_i == 0) /*if we are not in a string section*/
        {
                if(comment_type == 1)
                {
                  if((*str == '*')&&(*(str+1) == ']'))
                  {
                    /*exit comment section*/
                    comment_type = 0 ;
                    str = str + 2 ; /*next valid character*/
                  }
                }
                else
                if(comment_type == 2)
                {
                  if(*str == 10)
                  {
                      /*exit comment , new line*/
                      comment_type = 0 ;
                      str = str + 1; /*next valid character*/
                  }
                }

                if(comment_type == 0)
                {
                    if((*str == '[')&&(*(str+1) == '*'))
                    {
                      /*enter comment section*/
                      comment_type = 1 ;
                      str = str + 2 ; /*next valid character*/
                    }
                    else
                        if((*str == '#')||(*str == '?'))
                        {
                            comment_type = 2 ;
                            str++ ; /*next valid character*/
                        }
                }

                /*if we are in a comment section then ommit any character*/
                if(comment_type != 0) 
                {
                    /*next character*/
                    str++ ;
                    continue ;
                }
        } /*if not in string section */
        /*-------------------------------------------*/

        if(*str == 0) break ; /*check if the comments are ending the string, this error will be handled in the caller*/

        if (str_i != 0)
        {
            if (str_i == *str) str_i = 0;
        }
        else
        {
            if (dxIsCharInSet(*str, str_indent) == true) /*check if the character is in a string */
                str_i = *str;
            else
            {
                if (*str == open_char) open_chars++;
                else
                    if (*str == close_char) open_chars--;

                if (open_chars < 0) return false;
            }
        }

        str++;
    }

    if (open_chars != 0) return false;
    else
        return true;
}

bool dxItsStrEncapsulated(char* str, char open_c, char close_c)
{
    if (str == NULL) return false ;
    int len = strlen(str) ;
    if ((*str != open_c) || (str[len-1] != close_c)) return false;/*fast check*/
    
    int sections = 0 ;  
    /*for the string to be encapsulated the sections var must become 0 */
    for (int i = 1; i < (len - 1); i++) /*for the second character until one before last*/
    {
        if (str[i] == open_c) sections++;
        else
            if (str[i] == close_c) sections--;
        /*if the sections go below 0 then the string is not encapsulated or an error in the pairs occured*/
        if (sections < 0) return false;
    }

    if (sections == 0) return true ;
    else
        return false ;
}


bool dxItsStrEncapsulatedExt(char* str, char open_c, char close_c, char* str_indent)
{
    if ((str == NULL)||(str_indent==NULL)) return false;
    int len = strlen(str);
    if ((*str != open_c) || (str[len - 1] != close_c)) return false;/*fast check*/

    int sections = 0;
    char str_ind = 0;
    /*for the string to be encapsulated the sections var must become 0 */
    for (int i = 1; i < (len - 1); i++) /*for the second character until one before last*/
    {
        if(str_ind != 0)
        { 
            if (str[i] == str_ind) str_ind = 0;
            continue;
        }
        
        if (dxIsCharInSet(str[i], str_indent) == true)
        {
            str_ind = str[i];
            continue;
        }

        if (str[i] == open_c) sections++;
        else
            if (str[i] == close_c) sections--;
        /*if the sections go below 0 then the string is not encapsulated or an error in the pairs occured*/
        if (sections < 0) return false;
    }

    if (sections == 0) return true;
    else
        return false;
}


char* dxGetNextWord(char** str_start, char* seps, char* str_indent , bool ignore_parenthesis, bool ignore_indexes, char  *sep_found)
{
    if (str_start == NULL) return NULL;
    if (seps == NULL) return NULL;
    char* tind = "";

    if (str_indent == NULL)
    {
        str_indent = tind ;
    }

    char null_sect[1];
    null_sect[0] = 0;

    int sect_par = 0 ; /*if this is > 0 then we are inside a section of () */
    int sect_ind = 0 ; /*if this is > 0 then we are inside a section of [] */

    *sep_found = 0;
    char* accum = (char*)malloc(strlen(*str_start)+1); 
    accum[0] = 0;
    char* accum_indx = accum;
    char str_indent_char = 0 ;

    while (*(*str_start) != 0)
    {

        if (dxIsCharInSet(*(*str_start), str_indent) == true)
        {
            if (str_indent_char == *(*str_start)) str_indent_char = 0;
            else
                if (str_indent_char == 0)
                    str_indent_char = *(*str_start);
        }
        else if (str_indent_char == 0) /*check the sections*/
        {
            if (*(*str_start) == '[') sect_ind++;
            else
                if (*(*str_start) == ']') sect_ind--;
                else
                    if (*(*str_start) == '(') sect_par++;
                    else
                        if (*(*str_start) == ')') sect_par--;
        }

        if (dxIsCharInSet(*(*str_start), seps) == false)
        {
            /*this is not a separator*/
            *accum_indx = *(*str_start) ;
            accum_indx++;
        }
        else
        {
            if (str_indent_char != 0)
            {
                /*we are inside a string section!*/ 
                *accum_indx = *(*str_start);
                accum_indx++;
            }
            else
            {
                if ((ignore_parenthesis == true) && (sect_par > 0))
                {
                    *accum_indx = *(*str_start);
                    accum_indx++;
                    goto next;
                }
                
                if ((ignore_indexes == true) && (sect_ind > 0))
                {
                    *accum_indx = *(*str_start);
                    accum_indx++;
                    goto next;
                }

                /*the separator stands!*/
                *sep_found = *(*str_start);
                (*str_start)++; /*next valid character*/
                break;
                
            }
        }
        next:
        (*str_start)++ ;
    }

    *accum_indx = 0 ; /*terminate string*/
    return accum;
}


DXLONG64 dxCharExistsInStr(char* str, char c, char* str_indent)
{
    DXLONG64 retpos = -1;
    DXLONG64 cpos   = 0 ;
    char str_i = 0;
    while (*str != 0)
    {
        if (str_i == 0)
        {
            if (dxIsCharInSet(*str, str_indent) == true) str_i = *str ;
            else
                if (*str == c)
                {
                    retpos = cpos;
                    break;
                }

        } 
        else
        {
            if (*str == str_i) str_i = 0;
        }
        cpos++;
        str++;
    }


    return retpos;
}

DXLONG64 dxCharExistsInStrExt(char* str, char c, char* str_indent)
{
    int parenthesis = 0 ;
    int brackets    = 0 ;

    DXLONG64 retpos = -1;
    DXLONG64 cpos   = 0 ;
    char str_i = 0;
    while (*str != 0)
    {
        if (str_i == 0)
        {
            if (dxIsCharInSet(*str, str_indent) == true) str_i = *str ;
            else
                if ((*str == c)&&(parenthesis == 0)&&(brackets == 0))
                {
                    retpos = cpos;
                    break;
                }
                else
                {
                    if(*str == '(') parenthesis++ ;
                    else
                    if(*str == ')') parenthesis-- ;
                    else
                    if(*str == '[') brackets++    ;
                    else
                    if(*str == ']') brackets--    ;
                }

        } 
        else
        {
            if (*str == str_i) str_i = 0;
        }
        cpos++;
        str++;
    }


    return retpos;
}

char* dxCopyStrToChar(char **str, char c, char *str_indent)
{

    if (str == NULL) return NULL;
    char* stri = "";
    if (str_indent == NULL) str_indent = stri;

    /*allocate the memory for the return*/

    char* accum = (char*)malloc(strlen(*str)+1);
    accum[0] = 0;
    char* accum_i = accum;
    char str_i = 0;
    while (*(*str) != 0)
    {
        if (str_i == 0)
        {
            if (dxIsCharInSet(*(*str), str_indent) == true) str_i = **str;
            else
                if (*(*str) == c)
                {
                    break;
                }
        }
        else
        {
            if (*(*str) == str_i) str_i = 0;
        }
        *accum_i = *(*str);
        accum_i++;
        (*str)++;
    }

    *accum_i = 0;

    return accum ;
}


char* dxCopyStrToCharReverse(char** str , char c, char* str_indent)
{
    if (str == NULL) return NULL;
    char* stri = "";
    if (str_indent == NULL) str_indent = stri;

    /*calculate the memory we need*/
    DXLONG64 len = strlen(*str) ;
    char *str_indx = &((*str)[len-1]) ;
    DXLONG64 bcount = 0 ;
    while(true)
    {
      if(*str_indx == c)
      {
       break ;
      }
      else
       if(str_indx == *str) 
       {
           bcount++ ;
           break ;
       }
       else
       {
         bcount++ ;
       }
      
      str_indx-- ; 
    }

    /*allocate the memory for the return*/
    char* accum = (char*)malloc(bcount+1);
    accum[bcount] = 0;
    char* accum_i = &(accum[bcount-1]);
    char str_i = 0;
    str_indx = &((*str)[len-1]) ;
    while (true)
    {
        if (str_i == 0)
        {
            if (dxIsCharInSet(*str_indx, str_indent) == true) str_i = *str_indx;
            else
                if (*str_indx == c)
                {
                    break;
                }
        }
        else
        {
            if (*str_indx == str_i) str_i = 0;
        }
        *accum_i = *str_indx;
        if(str_indx == *str) break ;
        
        accum_i--;
        str_indx--;
    }
    *str = str_indx ;
    return accum ;
}

char* dxCopyStrToChars(char **str, char *chars, char *str_indent)
{

    if (str == NULL) return NULL;
    char* stri = "";
    if (str_indent == NULL) str_indent = stri;

    /*allocate the memory for the return*/

    char* accum = (char*)malloc(strlen(*str)+1);
    accum[0] = 0;
    char* accum_i = accum;
    char str_i = 0;
    while (*(*str) != 0)
    {
        if (str_i == 0)
        {
            if (dxIsCharInSet(*(*str), str_indent) == true) str_i = **str;
            else
                if (dxIsCharInSet(*(*str) , chars))
                {
                    break;
                }
        }
        else
        {
            if (*(*str) == str_i) str_i = 0;
        }
        *accum_i = *(*str);
        accum_i++;
        (*str)++;
    }

    *accum_i = 0;

    return accum ;
}


void dxCopyStrToStr(char** dest, char* source)
{
    if ((dest == NULL) || (source == NULL)) return;
    while (*source != 0)
    {
        *(*dest) = *source;
        (*dest)++;
        source++;
    }

    return;
}

char dxConvertChar(char if_char, char to_char, char this_char)
{
    if (this_char == if_char) return to_char;
    return this_char;
}

char dxGetNextValidChar(char* str, char* invalid_chars)
{

    while (dxIsCharInSet(*str, invalid_chars) == true)
    {
        if (*str == 0) return 0;/*habit is habit*/
        str++;
    }

   return *str;
}


bool dxIsTextAString(char* str, char str_ident)
{
    if (*str != str_ident) return false;
    if (str[strlen(str)-1] != str_ident) return false;

    return true;
}

bool dxIsStringValidEncaps(char* str, char* str_idents)
{
    if (str == NULL) return false ;
    if (str[0] == 0) return false ;

    char str_ident = 0;
    if (dxIsCharInSet(*str, "`\"") == false) return false;
    if (dxIsCharInSet(str[strlen(str)-1], "`\"") == false) return false;
    
    str_ident = *str;
    str++;
    while (*str != 0)
    {
        if (*str != str_ident) str++;
        else
        {
            if (*(str + 1) == 0) return true;
            else
                return false;
        }
    }


    return false ;

}

bool dxCharExists(char ** str, char c, char* str_idents)
{
    char str_ident = 0;

    while (*(*str) != 0)
    {
        if (str_ident != 0)
        {
            if (*(*str) == str_ident) str_ident = 0; /*exit the text section*/
        }
        else
        {
            if (dxIsCharInSet(*(*str), str_idents) == true) /*the *str is one of the identations for the text sections*/
            {
                str_ident = *(*str);
            }
            else
                if (*(*str) == c) return true; /*reqular character check if it is the one we search for*/
        }

        (*str)++;
    }

    return false;

}

DXLONG64 dxPower(DXLONG64 base, DXLONG64 exp)
{
    DXLONG64 i           ;
    DXLONG64 result = 1  ;
    for (i = 0; i < exp; i++) result *= base;
    return result;
}


uint64_t dxGetTickCount()
{
  #ifdef _WIN32
    return GetTickCount64();
  #endif

  #ifdef _LINUX

    struct timespec tm;
    clock_gettime(CLOCK_MONOTONIC_RAW, &tm);
    return (tm.tv_sec * 1000) + (tm.tv_nsec / 1000000);

  #endif

}


char *CopyStrToChar(char * str, DXLONG64 *from_indx,char to_char)
{
  

  if(*from_indx < 0) *from_indx = 0 ;
  DXLONG64 chlen = 0 ;
  char *str_indx = &str[*from_indx];
  while((*str_indx != to_char)&&(*str_indx != 0 ))
  {
   chlen++ ; 
   str_indx++ ;
  }

  char * buff = (char*)malloc(chlen+1) ;
  buff[chlen] = 0 ;
  memcpy(buff,&(str[*from_indx]),chlen) ;

  if(*str_indx == 0) *from_indx = - 1 ;
  else 
      *from_indx = *from_indx + chlen ;

  return buff ;
}


char *utf8CopyIndexToIndex(char *utf8str , DXLONG64 from_index , DXLONG64 to_index)
{
    /*go to the from_index*/
    if(from_index < 0) from_index = 0 ;
    DXCHAR ch = 0 ; 
    for(DXLONG64 i = 0 ; i < from_index ; i++)
    {
      ch = dx_get_utf8_char(&utf8str) ;
      if(ch == 0) return NULL ; /*the from_index is bigger than the string length*/
    }
    char     *chindx = utf8str ;
    char     *prevch  ;
    int      char_len = 0 ;
    DXLONG64 buf_size = 0 ; 
    for(DXLONG64 i = from_index;i <= to_index ;i++ )
    {
      DXCHAR ch = dx_get_utf8_char_ex2(&chindx,&prevch,&char_len) ;
      if(ch == 0) break ;
      buf_size = buf_size + char_len ;
    }
 
    char *buf = (char*)malloc(buf_size+1); 
    buf[buf_size] = 0 ;

    memcpy(buf,utf8str,buf_size) ;

    return buf ;

}

char *utf8CopyIndex(char *utf8str , DXLONG64 from_index , DXLONG64 char_cnt)
{
    /*go to the from_index*/
    if(from_index < 0) from_index = 0 ;
    DXCHAR ch = 0 ; 
    for(DXLONG64 i = 0 ; i < from_index ; i++)
    {
      ch = dx_get_utf8_char(&utf8str) ;
      if(ch == 0) return NULL ; /*the from_index is bigger than the string length*/
    }

    char     *chindx = utf8str ;
    char     *prevch  ;
    int      char_len = 0 ;
    DXLONG64 buf_size = 0 ; 
    for(DXLONG64 i = 0 ; i < char_cnt ;i++ )
    {
      DXCHAR ch = dx_get_utf8_char_ex2(&chindx,&prevch,&char_len) ;
      if(ch == 0) break ;
      buf_size = buf_size + char_len ;
    }
 
    char *buf = (char*)malloc(buf_size+1); 
    buf[buf_size] = 0 ;

    memcpy(buf,utf8str,buf_size) ;

    return buf ;

}


char *utf8CopyToCh(char *utf8str , DXLONG64 *from_index , char *to_char)
{
  

    if(*from_index < 0) *from_index = 0 ;
    /*go to the index*/
    DXCHAR ch = 0 ; 
    for(DXLONG64 i = 0 ; i < *from_index ; i++)
    {
        ch = dx_get_utf8_char(&utf8str) ;
        if(ch == 0) return NULL ; /*the from_index is bigger than the string length*/
    }

    char     *chindx = utf8str ;
    char     *prevch  ;
    int      char_len = 0 ;
    DXLONG64 buf_size = 0 ; 
    DXCHAR the_ch = dx_convert_utf8_to_int32(to_char,dxUtf8CharByteCount(*to_char)) ;
    while(true)
    {
      DXCHAR ch = dx_get_utf8_char_ex2(&chindx,&prevch,&char_len) ;
      if((ch == 0)||(ch == the_ch)) break ;
      buf_size = buf_size + char_len ;
      (*from_index)++ ;
    }
 
    if(*from_index == 0) *from_index = - 1 ;

    char *buf = (char*)malloc(buf_size+1); 
    buf[buf_size] = 0 ;

    memcpy(buf,utf8str,buf_size) ;
    return buf ;

}


void dx_strcpy(char *dest,char *source) 
{
     while(*source != 0 )
     {
       *dest = *source ;
       dest++   ;
       source++ ;
     }
 
     *dest = 0 ;
}



PDX_STRING dxHostNameToIp(char *hostname)
{
    struct addrinfo* result = NULL;
    struct addrinfo* ptr = NULL;
    struct addrinfo hints;

    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;


    int res = getaddrinfo(hostname, NULL, &hints, &result);
    
    if (res != 0)
    {
        return dx_string_createU(NULL, hostname);
    }

    struct sockaddr_in* sockaddr_ipv4 = NULL;

    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        // get the ipv4 address , for now we support only ipv4
        if (ptr->ai_family == AF_INET)
        {
            sockaddr_ipv4 = (struct sockaddr_in*)ptr->ai_addr;
            break;

        }

    }

    if (sockaddr_ipv4 == NULL)
    {   
        freeaddrinfo(result);
        return dx_string_createU(NULL, hostname);
    }

    char ip[32];
    inet_ntop(AF_INET, (void*)&(sockaddr_ipv4->sin_addr), ip, 32);

    freeaddrinfo(result);
	PDX_STRING dxip = dx_string_createU(NULL,ip);
	return dxip ;
}


void dx_PaddWithZeros(PDX_STRING str , int total_len)
{
 if(total_len == 0) return ;
 if(total_len <= str->len) return ;

 char *tmp = malloc(total_len+1);
 tmp[total_len] = 0 ;
 int i = total_len -1 ;
 int indx = str->len -1 ;
 while(true)
 {
   tmp[i] = str->stringa[indx] ;
   i-- ;
   indx-- ;
   if(indx == - 1) break ;
 }

 while(i > -1)
 {
   tmp[i] = '0' ;
   i--;
 }
 
 str = dx_string_setU(str,tmp) ;

 return ;
}

DXLONG64 dx_GetFileSize(FILE *f)
{
    fseek(f, 0L, SEEK_END);
    DXLONG64 sz = ftell(f);
    rewind(f);
    return sz ;
}

PDX_STRING dx_LoadUTF8TextFromFile(PDX_STRING filename,bool *error)
{
    FILE * f = fopen(filename->stringa,"rb") ; /*open for reading*/
    if(f == NULL)
    {
     *error = true ; 
     return dx_string_createU(NULL,"");
    }
    DXLONG64 sz = dx_GetFileSize(f);
    /*create a buffer big enough*/
    char * buff = (char*)malloc(sz+1);
    buff[sz] = 0 ;

   if (fread(buff,sz,1,f)!=1)  
   {
     *error = false ; /*an empty file is not an error*/
     free(buff);
     fclose(f) ;
     return dx_string_createU(NULL,"") ;
   }
   fclose(f);
   return dx_string_create_bU(buff) ;
}


void dxSleep(int millis)
{
#ifdef _LINUX 
    struct timespec ts;
    int res;

    if (millis < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = millis / 1000;
    ts.tv_nsec = (millis % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);


#endif

#ifdef _WIN32 
  Sleep(millis) ;
#endif
 
}

PDX_STRING dxUrlEncodeUTF8(PDX_STRING orstr,bool space_as_plus)
{
    // allocate memory for the worst possible case (all characters need to be encoded)
    if(orstr == NULL) return NULL ;
    char * str = orstr->stringa ;
    DXLONG64 pos   = 0 ;
    DXLONG   len   = orstr->bcount ; 
    char *enc = (char *)malloc(sizeof(char)*len*3+1);
    char *enc_indx = enc ;
    const char *hex = "0123456789abcdef";


  while(*str!=0)
  {
    //Check if we have a safe char
    if (((*str>='A')&&(*str<='Z'))||((*str>='a')&&(*str<='z')||((*str>='0')&&(*str<='9')))
        ||(*str=='@')||(*str=='.')||(*str=='_')||(*str=='-'))
    {
     //add it
     *enc_indx = *str;
     //next byte
     enc_indx++;
     str++ ;
    }
    else
    if (*str == ' ') 
    {
      //add space
      if (space_as_plus == 1)
      {
        *enc_indx = '+';
        enc_indx++ ;
      }
      else
      {
        memcpy(enc_indx,"%20",3) ;
        enc_indx = enc_indx+3    ;
      }

      str++ ; /*next byte*/
    }
    else
        if(*str == '+') /*the + is special in the url encoding as it can be decoded as a space*/
        {
          memcpy(enc_indx,"%2B",3) ;
          enc_indx = enc_indx+3    ;
        }
        else
        if(*str == '%') /*the % is special in the url encoding , its value is %25*/
        {
          memcpy(enc_indx,"%25",3) ;
          enc_indx = enc_indx+3    ;
        }
     else
        {
           *enc_indx = '%' ;
           enc_indx = enc_indx + 2 ; /*to place the digits correctly*/
           //Convert byte to hex, a byte needs two characters to be described 
           DXLONG64 num = (unsigned char)(*str) ; 
           while (num > 0) 
            {
                DXLONG64 remainder = num % 16;

                if (remainder < 10)
                    *enc_indx = remainder + '0'; /*proceed to find the character*/
                else
                    *enc_indx = remainder + 'A' - 10;

                enc_indx-- ;

                num = num / 16 ;
            }
           /*reposition the enc_indx*/
           enc_indx = enc_indx +3 ;
           str++ ;
        }

  }
    *enc_indx = 0 ; 
    return dx_string_create_bU(enc) ;
}


PDX_STRING dxUrlDecodeUTF8(PDX_STRING orstr,bool plus_as_space)
{
  char *str_indx = orstr->stringa ;
  char *dec_str = (char*)malloc(orstr->bcount+1);
  char *dec_str_indx = dec_str ;

  DXLONG64 len = strlen(str_indx);

  while(*str_indx != 0)
  {
    //check token
    if (*str_indx == '+')
    {
      if(plus_as_space == true)
      *dec_str_indx = ' ' ;
      else
          *dec_str_indx = '+' ;
      dec_str_indx++ ;
      str_indx++     ;
    }
    else 
    if (*str_indx != '%')
    {
     *dec_str_indx = *str_indx ;
      dec_str_indx++ ;
      str_indx++    ;
    }
    else
    {
      if(*(str_indx+1)=='%')
      {
       *dec_str_indx='%' ;
       dec_str_indx++ ;  
       str_indx++ ;
       continue ;
      }
      str_indx++ ;
      /*safe guard agains corrupted strings*/
      if(*str_indx == 0) break ;
      //following is two characters to convert to a byte except if another % is in succession. 
      //if it is the the current % is a real character of the string and we add it to the new string
      char tbyte[3];
      tbyte[0] = *str_indx ;
      str_indx++ ;
      if(*str_indx == 0) break ;
      tbyte[1] = *str_indx ;
      tbyte[2] = 0         ;
      str_indx++           ;
      *dec_str_indx = (unsigned char)dxHexToDec(tbyte);
      dec_str_indx++ ;
    }

  }

  *dec_str_indx = 0 ; 
  
  return dx_string_create_bU(dec_str) ;
}


/*########################################################*/

#ifdef _LINUX
bool dxGetFiles(PDX_STRING strdir,PDX_STRINGLIST list,bool recursive) 
{
  DIR * d = opendir(strdir->stringa) ; // open the path
  if(d==NULL) return false        ; // if was not able, return
  
  PDX_STRING cdir = NULL ;
  if(strdir->stringa[strdir->bcount-1] !='/')
  {
     PDX_STRING sep = dx_string_createU(NULL,"/") ;
     cdir = dx_string_concat(strdir,sep) ;       
     dx_string_free(sep) ;
  }
  else
      cdir = dx_string_createU(NULL,strdir->stringa) ;       

  struct dirent * dir  ; // for the directory entries

  while ((dir = readdir(d)) != NULL) // if we were able to read something from the directory
  {
      PDX_STRING dname = dx_string_createU(NULL,dir->d_name) ;
      PDX_STRING fname = dx_string_concat(cdir,dname)        ;
      dx_string_free(dname);
      if(dir->d_type != DT_DIR) dx_stringlist_add_string(list,fname) ; /*this is a file*/
      else
      if(dir -> d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 ) // if it is a directory
      {
        if(recursive == true)  dxGetFiles(fname,list,true); // recall with the new path
        dx_string_free(fname);
      }
  }
    closedir(d); // finally close the directory

  dx_string_free(cdir);

  return true ;
}
#endif

#ifdef _WIN32
bool dxGetFiles(PDX_STRING dir,PDX_STRINGLIST list,bool recursive) 
{
    WIN32_FIND_DATA fdFile; 
    HANDLE hFind = NULL; 

    /*create the new string that is dir\*.* */
    PDX_STRING mask = dx_string_createW(NULL,L"\\*.*")  ;
    PDX_STRING wdir = NULL ;
    if((dir->stringa[dir->bcount-1] != '\\')&&(dir->stringa[dir->bcount-1] != '/'))
    {
      PDX_STRING tempwd = dx_string_convertW(dir) ;
      PDX_STRING sep    = dx_string_createW(NULL,L"\\") ;
      wdir = dx_string_concat(tempwd,sep) ;
      dx_string_free(tempwd) ;
      dx_string_free(sep)    ;
    }
    else
    {
     wdir = dx_string_convertW(dir)  ; 
    }

    PDX_STRING root_dir = dx_string_concat(wdir,mask)   ;
    dx_string_free(mask) ;

    if((hFind = FindFirstFile(root_dir->stringw, &fdFile)) == INVALID_HANDLE_VALUE) 
    { 
        dx_string_free(wdir) ;
        dx_string_free(root_dir) ;
        return false; 
    } 

    do
    { 
        //Find first file will always return "."
        //    and ".." as the first two directories. 
        if((wcscmp(fdFile.cFileName, L".") != 0) && (wcscmp(fdFile.cFileName, L"..") != 0)) 
        { 
            PDX_STRING nfname = dx_string_createW(NULL,fdFile.cFileName) ; 
            PDX_STRING wfile   = dx_string_concat(wdir,nfname) ;
            PDX_STRING file = dx_string_convertU(wfile) ;
            dx_string_free(wfile) ;
            dx_string_free(nfname) ;

            //Is the entity a File or Folder? 
            if(fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY) 
            { 
                if(recursive == true) dxGetFiles(file,list,true); //Recursion, I love it!
                dx_string_free(file);
            } 
            else
               { 
                 /*add the string to the list*/
                 dx_stringlist_add_string(list,file) ; /*the file will be freed in the stringlist destruction*/
               } 
        }

    }while(FindNextFile(hFind, &fdFile)); //Find the next file. 

    FindClose(hFind); //Always, Always, clean things up! 

    dx_string_free(wdir)     ;
    dx_string_free(root_dir) ;

    return  true ;
}
#endif


/*########################################################*/

#ifdef _LINUX
bool dxGetDirs(PDX_STRING strdir,PDX_STRINGLIST list,bool recursive) 
{
      DIR * d = opendir(strdir->stringa) ; // open the path
      if(d==NULL) return false        ; // if was not able, return
  
      PDX_STRING cdir = NULL ;
      if(strdir->stringa[strdir->bcount-1] !='/')
      {
         PDX_STRING sep = dx_string_createU(NULL,"/") ;
         cdir = dx_string_concat(strdir,sep) ;       
         dx_string_free(sep) ;
      }
      else
          cdir = dx_string_createU(NULL,strdir->stringa) ;       

      struct dirent * dir  ; // for the directory entries

      while ((dir = readdir(d)) != NULL) // if we were able to read something from the directory
      {
          PDX_STRING dname = dx_string_createU(NULL,dir->d_name) ;
          PDX_STRING fname = dx_string_concat(cdir,dname)        ;
          dx_string_free(dname);
          if(dir->d_type != DT_DIR) dx_string_free(fname); /*this is a file*/
          else
          if(dir -> d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 ) // if it is a directory
          {
            PDX_STRING sep    = dx_string_createU(NULL,"/") ;
            PDX_STRING tfname = dx_string_concat(fname,sep) ;
            dx_string_free(sep);
            dx_stringlist_add_string(list,tfname) ;
            if(recursive == true)  dxGetDirs(fname,list,true); // recall with the new path
            dx_string_free(fname);
          }
      }
        closedir(d); // finally close the directory

      dx_string_free(cdir);

      return true ;
}
#endif

#ifdef _WIN32
bool dxGetDirs(PDX_STRING dir,PDX_STRINGLIST list,bool recursive) 
{
    WIN32_FIND_DATA fdFile; 
    HANDLE hFind = NULL; 

    /*create the new string that is dir\*.* */
    PDX_STRING mask = dx_string_createW(NULL,L"\\*.*")  ;
    PDX_STRING wdir = NULL ;
    if((dir->stringa[dir->bcount-1] != '\\')&&(dir->stringa[dir->bcount-1] != '/'))
    {
      PDX_STRING tempwd = dx_string_convertW(dir) ;
      PDX_STRING sep    = dx_string_createW(NULL,L"\\") ;
      wdir = dx_string_concat(tempwd,sep) ;
      dx_string_free(tempwd) ;
      dx_string_free(sep)    ;
    }
    else
    {
     wdir = dx_string_convertW(dir)  ; 
    }

    PDX_STRING root_dir = dx_string_concat(wdir,mask)   ;
    dx_string_free(mask) ;

    if((hFind = FindFirstFile(root_dir->stringw, &fdFile)) == INVALID_HANDLE_VALUE) 
    { 
        dx_string_free(wdir) ;
        dx_string_free(root_dir) ;
        return false; 
    } 

    do
    { 
        //Find first file will always return "."
        //    and ".." as the first two directories. 
        if((wcscmp(fdFile.cFileName, L".") != 0) && (wcscmp(fdFile.cFileName, L"..") != 0)) 
        { 
            PDX_STRING nfname = dx_string_createW(NULL,fdFile.cFileName) ; 
            PDX_STRING wfile   = dx_string_concat(wdir,nfname) ;
            PDX_STRING file = dx_string_convertU(wfile) ;
            dx_string_free(wfile) ;            dx_string_free(nfname) ;

            //Is the entity a File or Folder? 
            if(fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY) 
            { 
                PDX_STRING sep   = dx_string_createU(NULL,"\\") ;
                PDX_STRING tfile = dx_string_concat(file,sep)   ;
                dx_string_free(sep);
                dx_stringlist_add_string(list,tfile) ; /*the file will be freed in the stringlist destruction*/
                if(recursive == true) dxGetDirs(file,list,true); //Recursion, I love it! 
                dx_string_free(file);
            } else
            {
                dx_string_free(file);
            }
        }
    } 
    while(FindNextFile(hFind, &fdFile)); //Find the next file. 

    FindClose(hFind); //Always, Always, clean things up! 

    dx_string_free(wdir)     ;
    dx_string_free(root_dir) ;

    return  true ;
}
#endif

#ifdef _LINUX
bool dxGetFileDirs(PDX_STRING strdir,PDX_STRINGLIST list,bool recursive) 
{
      DIR * d = opendir(strdir->stringa) ; // open the path
      if(d==NULL) return false        ; // if was not able, return
  
      PDX_STRING cdir = NULL ;
      if(strdir->stringa[strdir->bcount-1] !='/')
      {
         PDX_STRING sep = dx_string_createU(NULL,"/") ;
         cdir = dx_string_concat(strdir,sep) ;       
         dx_string_free(sep) ;
      }
      else
          cdir = dx_string_createU(NULL,strdir->stringa) ;       

      struct dirent * dir  ; // for the directory entries

      while ((dir = readdir(d)) != NULL) // if we were able to read something from the directory
      {
          PDX_STRING dname = dx_string_createU(NULL,dir->d_name) ;
          PDX_STRING fname = dx_string_concat(cdir,dname)        ;
          dx_string_free(dname);
          if(dir->d_type != DT_DIR) dx_stringlist_add_string(list,fname) ; /*this is a file*/
          else
          if(dir -> d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 ) // if it is a directory
          {
            PDX_STRING sep    = dx_string_createU(NULL,"/") ;
            PDX_STRING tfname = dx_string_concat(fname,sep) ;
            dx_string_free(sep);
            dx_stringlist_add_string(list,tfname) ;
            if(recursive == true)  dxGetFileDirs(fname,list,true); // recall with the new path
            dx_string_free(fname);
          }
      }
        closedir(d); // finally close the directory

      dx_string_free(cdir);

      return true ;
}
#endif
#ifdef _WIN32
bool dxGetFileDirs(PDX_STRING dir,PDX_STRINGLIST list,bool recursive) 
{
    WIN32_FIND_DATA fdFile; 
    HANDLE hFind = NULL; 

    /*create the new string that is dir\*.* */
    PDX_STRING mask = dx_string_createW(NULL,L"\\*.*")  ;
    PDX_STRING wdir = NULL ;
    if((dir->stringa[dir->bcount-1] != '\\')&&(dir->stringa[dir->bcount-1] != '/'))
    {
      PDX_STRING tempwd = dx_string_convertW(dir) ;
      PDX_STRING sep    = dx_string_createW(NULL,L"\\") ;
      wdir = dx_string_concat(tempwd,sep) ;
      dx_string_free(tempwd) ;
      dx_string_free(sep)    ;
    }
    else
    {
     wdir = dx_string_convertW(dir)  ; 
    }

    PDX_STRING root_dir = dx_string_concat(wdir,mask)   ;
    dx_string_free(mask) ;

    if((hFind = FindFirstFile(root_dir->stringw, &fdFile)) == INVALID_HANDLE_VALUE) 
    { 
        dx_string_free(wdir) ;
        dx_string_free(root_dir) ;
        return false; 
    } 

    do
    { 
        //Find first file will always return "."
        //    and ".." as the first two directories. 
        if((wcscmp(fdFile.cFileName, L".") != 0) && (wcscmp(fdFile.cFileName, L"..") != 0)) 
        { 
            PDX_STRING nfname = dx_string_createW(NULL,fdFile.cFileName) ; 
            PDX_STRING wfile   = dx_string_concat(wdir,nfname) ;
            PDX_STRING file = dx_string_convertU(wfile) ;
            dx_string_free(wfile) ;
            dx_string_free(nfname) ;

            //Is the entity a File or Folder? 
            if(fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY) 
            { 
                PDX_STRING sep   = dx_string_createU(NULL,"\\") ;
                PDX_STRING tfile = dx_string_concat(file,sep)   ;
                dx_string_free(sep);
                dx_stringlist_add_string(list,tfile) ; /*the file will be freed in the stringlist destruction*/
                if(recursive == true) dxGetFileDirs(file,list,true); //Recursion, I love it! 
                dx_string_free(file);
            } 
            else
               { 
                 /*add the string to the list*/
                 dx_stringlist_add_string(list,file) ; /*the file will be freed in the stringlist destruction*/
               } 
        }
    } 
    while(FindNextFile(hFind, &fdFile)); //Find the next file. 

    FindClose(hFind); //Always, Always, clean things up! 

    dx_string_free(wdir)     ;
    dx_string_free(root_dir) ;

    return  true ;
}
#endif


bool dxDeleteDir(PDX_STRING dir,bool deleteRoot)
{
    if(dxDirExists(dir) == false) return true ; /*anyway is gone xD*/
    /*get all the files and all the directories*/
    PDX_STRINGLIST flist = dx_stringlist_create() ;
    dxGetFileDirs(dir,flist,true) ;
    PDX_STRINGLIST dlist = dx_stringlist_create() ;
    dxGetDirs(dir,dlist,true)                     ;

    /*delete files*/
    PDXL_NODE node = flist->start ;
    while(node != NULL)
    {
      PDXL_OBJECT obj  = node->object ;
      PDX_STRING fname = obj->key     ; 
      remove(fname->stringa)          ;
      node = node->right              ;
    }

    /*delete dirs*/
    node = dlist->start ;
    while(node != NULL)
    {
      PDXL_OBJECT obj  = node->object ;
      PDX_STRING dname = obj->key     ;
#ifdef _WIN32
      _rmdir(dname->stringa)           ;
#endif
#ifdef _LINUX
      rmdir(dname->stringa)           ;
#endif
      node = node->right              ;
    }

    dx_stringlist_free(flist) ;
    dx_stringlist_free(dlist) ;
 
    if(deleteRoot == true)
    {
      int ret = rmdir(dir->stringa)     ;
    
      if (ret == 0) return true ;
      return false              ;
    }

    return true ;
}


bool dxFilesToZip(PDX_STRING fileName, PDX_STRINGLIST list) 
{

    /*Create a char * array  with the files for the zip_create */
    char **fnames = malloc(sizeof(char*)*list->count) ;
    DXLONG64 fnames_count = 0 ;
    PDXL_NODE node = list->start ;
    while(node != NULL)
    {
        PDXL_OBJECT obj = node->object ;
        PDX_STRING str  = obj->key     ; 
        if((str->stringa[str->bcount-1] != '\\')&&(str->stringa[str->bcount-1] != '/'))
        {
           fnames[fnames_count] = str->stringa ; /*for now the fnames_count is like an index :D*/
           fnames_count++ ;
        }

        node = node->right ;
    }

   if(zip_create(fileName->stringa,fnames,fnames_count) != 0 )
   {
       free(fnames) ;
       return false ;
   }

   free(fnames) ; 
   return true ;
}



bool dxDirToZip(PDX_STRING fileName, PDX_STRING dirName) 
{
    bool res = true ;
    PDX_STRINGLIST list = dx_stringlist_create() ;
    if (dxGetFileDirs(dirName,list,true) == false) 
    {
        dx_stringlist_free(list) ;
        return false ; 
    }

    struct zip_t *zip = zip_open(fileName->stringa, ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');

    if(zip == NULL) 
    {
        dx_stringlist_free(list) ;
        return false ; 
    }

    PDXL_NODE node = list->start ;
    while(node != NULL)
    {
        PDXL_OBJECT obj = node->object ;
        PDX_STRING str  = obj->key     ; 
        DXLONG64 skip_c = dirName->bcount ;
        if((str->stringa[skip_c] == '\\')||(str->stringa[skip_c] == '/')) skip_c++ ;
        if((str->stringa[str->bcount-1] != '\\')&&(str->stringa[str->bcount-1] != '/'))
        {
              /*create the entry for the zip file , remove the dir structure until (and) this folder*/
              zip_entry_open(zip, &(str->stringa[skip_c]))   ;
              zip_entry_fwrite(zip, str->stringa) ;
              zip_entry_close(zip)                ;
        }
        else
        {
          
          if(zip_entry_open(zip, &(str->stringa[skip_c])) != 0)
          {
              res = false ;
              break ;
          }

          if(zip_entry_write(zip, NULL, 0) != 0) 
          {
            res = false ;
            break ;
          }
          zip_entry_close(zip);
          
        }

        node = node->right ;
    }

   zip_close(zip);
   dx_stringlist_free(list);
   return res ;
}


bool dxExtractZip(PDX_STRING zipName, PDX_STRING toDir) 
{
   int arg = 0;
   if(zip_extract(zipName->stringa, toDir->stringa,NULL, &arg) == 0) return true ;
   else
       return false ;
 
}

PDX_STRING dxStrToXorHex(PDX_STRING text,PDX_STRING key)
{
    if(text->len == 0) return dx_string_createU(NULL,"")            ;
    if(key->len == 0) return dx_string_createU(NULL,text->stringa)  ;
    
    /*create the buffer as double the size because every byte will be encoded as a hexadecimal value.*/
    DXLONG64 blen = (text->bcount*2)+1  ;  /*+1 for terminating zero*/ 
    char * buff   = (char*)malloc(blen) ;
    if(buff == NULL) return NULL  ;
    buff[blen-1] = 0              ;
    DXLONG64 len  = text->bcount  ;
    DXLONG64 klen = key->bcount   ;
    char *buffindx = buff ;
    for (DXLONG64 i = 0; i < len; ++i)
    {
        DXLONG64   bt   = text->stringa[i] ^ key->stringa[i % klen];
        PDX_STRING hexb = dxDecToHex(bt)     ;
        
        if(hexb == NULL) 
        {
         free(buff)  ;
         return NULL ;
        }
        if((hexb->bcount != 2)&&(hexb->bcount != 1) ) 
        {
          free(buff)  ;
          dx_string_free(hexb);
          return NULL ;
        }

        /*we need to pad with a zero as the dectohex returns the hex of the number not as a byte*/
        if(hexb->bcount == 1)
        {
          char * hx = (char*)malloc(3)        ;
          hx[0] = '0';
          hx[1] = hexb->stringa[0] ;
          hx[2] = 0;
          dx_string_free(hexb);
          hexb = dx_string_create_bU(hx);
        }

        *buffindx = hexb->stringa[0] ;
        buffindx++;
        *buffindx = hexb->stringa[1] ;
      
        dx_string_free(hexb) ;
        buffindx++ ; /*next byte*/
    }
    
    return dx_string_create_bU(buff) ;

}


PDX_STRING dxStrUnXorHex(PDX_STRING text,PDX_STRING key)
{
    if(text->len == 0) return dx_string_createU(NULL,"")            ;
    if(key->len == 0) return dx_string_createU(NULL,text->stringa)  ;
    
    /*create the buffer the same size for safety.*/
    DXLONG64 blen = (text->bcount)+1  ;  /*+1 for terminating zero*/ 
    char * buff = malloc(blen) ;
    if(buff == NULL) return NULL  ;
    buff[blen-1] = 0              ; /*for safety this will change later*/
    
    DXLONG64 len  = text->bcount  ;
    DXLONG64 klen = key->bcount   ;
    char *buffindx = buff         ;
    char *textindx = text->stringa;
    char hex[3]                   ;
    char *keyindx  = key->stringa ;  



    while(*textindx != 0)
    {

        /*get the numerical value of the byte*/
        hex[0] = *textindx ;
        textindx++;
        hex[1] = *textindx ;
        hex[2] = 0 ;
        textindx++ ; /*next byte*/

        DXLONG64 trbyte = dxHexToDec(hex) ;
        

        char bt = ((char)trbyte) ^ (*keyindx);
       /*adnvance the keyindex to be ready for the next xor*/
        keyindx++ ;
        if(*keyindx == 0)
        {
          /*reset the key ;*/
          keyindx = key->stringa ;
        }

        /*add the bt to the buffer*/
        *buffindx = bt ;
        buffindx++;  
    }
    
    /*set the termination*/
    *buffindx = 0 ;

    return dx_string_create_bU(buff) ;
}

PDX_STRING dxStrReverse(PDX_STRING str)
{
    char *ostr = str->stringa ;
    char *nstr = (char*)malloc(str->bcount+1) ;
    nstr[str->bcount] = 0 ;
    char *nstrindx    = &nstr[str->bcount-1] ;
    for(DXLONG64 i=0;i<str->bcount;i++)
    {
      *nstrindx = str->stringa[i] ;
      nstrindx-- ;
    
    }

    return dx_string_create_bU(nstr) ;
}

char * dxBytesXor(char * bytes,DXLONG64 bcount,PDX_STRING key) 
{
    if(bytes == NULL) return NULL ;
    if(bcount == 0 ) return NULL  ;
    if(key->bcount == 0 ) return NULL ;
    char * buff = (char*)malloc(bcount) ;
    if(buff == NULL) return NULL ;
    for (DXLONG64 i = 0; i < bcount; ++i)
    {
        buff[i] = bytes[i] ^ key->stringa[i % key->bcount] ;
    }

    return buff ;
}

/*################################################################*/


