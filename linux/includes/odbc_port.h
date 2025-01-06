/*
 This module offers some functions to convert the 2 bytes sqlwchar
 of odbc to diferent wchar_t C types.

*/


PDX_STRING odbc_string_create(PDX_STRING,SQLWCHAR *str) ;
/*
 return a PDX_STRING of widechar type
*/

SQLWCHAR * dx_odbc_wchar_to_sqlwchar(wchar_t *str);
/*
 in windows is a simple type casting as the two types are identical,
 in linux the wchar_t is 4 bytes so we have to fit them in the 2 bytes of the
 SQLWCHAR
*/



/*--------- implementation ---------------*/

DXLONG64 dx_odbc_sqlwchar_len(SQLWCHAR *str)
{

    DXLONG64 cnt = 0 ;
    while(*str != 0)
    {
        cnt++ ;
        str++ ;
    }

    return cnt ;
}


wchar_t *dx_odbc_sqlwchar_convert(SQLWCHAR *str)
{

    wchar_t *wstr = malloc((dx_odbc_sqlwchar_len(str)+1)*sizeof(wchar_t)); // calculate the terminating 0 too
    wchar_t *wstr_indx = wstr ;
    while(*str != 0)
    {
      *wstr_indx = *str ;
      wstr_indx++ ;
      str++  ;
    }

    *wstr_indx = 0 ;

    return wstr ;
}

SQLWCHAR * dx_odbc_wchar_to_sqlwchar(wchar_t *str)
{

    #ifdef _WIN32
    return (SQLWCHAR*)str ;
    #endif // _WIN32

    #ifdef _LINUX
        SQLWCHAR *odbc_str = malloc((StrLenW(str)+1) *sizeof(SQLWCHAR)  ) ;// calculate the terminating 0 too
        SQLWCHAR * odbc_indx = odbc_str ;
        while(*str != 0)
        {
          *odbc_indx = *str ;
          odbc_indx++ ;
          str++  ;
        }

        *odbc_indx = 0 ;

        return odbc_str ;

    #endif

}


PDX_STRING odbc_string_create(PDX_STRING dxstr,SQLWCHAR *str)
{

 #ifdef _WIN32
  if (str == NULL)
  return dx_string_createW(dxstr,L"");
  else
     return dx_string_createW(dxstr,str);
 #endif // _WIN32

 #ifdef _LINUX
 if (str == NULL)
  return dx_string_createW(dxstr,L"");
  else
  {
   wchar_t *tmp = dx_odbc_sqlwchar_convert(str);
   PDX_STRING ns = dx_string_createW(dxstr,tmp);
   free(tmp)  ;
   return ns  ;
  }
 #endif // _LINUX

}
















