/*
 Support for utf8 strings operations

 Nikolaos Mourgis deus-ex.gr 2024

 Live Long and Prosper
*/


char *hdr_return_utf8_pos(PDX_STRING str , DXLONG64 from_indx, DXLONG64 *byte_pos )
{
    /*
      returns the utf8 char of the string (first byte position) and the binary_pos will be set to the first byte position in the string
    */

    if((from_indx > (str->len-1))||(from_indx < 0)) return NULL ;
    char *prev_char = NULL ;
    int char_len ;
    *byte_pos = 0 ;
    char *utf8 = str->stringa ;
    for(DXLONG64 i = 0 ; i<from_indx ; i++ )
    {
      dx_get_utf8_char_ex2(&utf8,&prev_char,&char_len) ;
      *byte_pos = *byte_pos + char_len ;
    }

  return utf8;
}

DXLONG64 hdr_Utf8CharPos(PDX_STRING source , DXUTF8CHAR c , DXLONG64 fr_indx)
{
    /*
     The function return the absolute position of the [c] in characters. For example in the string 12345 
     the character 4 is in the 3 position.
     The function starts the search from the fr_indx that is too in characters and not bytes.
    */
     if(source == NULL) return -1 ;
     if(source->len == 0) return -1 ;
     if(c == 0) return -1   ;

     DXUTF8CHAR cc1      ;
     char *utf8_1 , *utf8tmp ;
     int charlen ;
     /*get to the position*/
     utf8_1 = dx_string_return_utf8_pos(source,fr_indx) ;
     if(utf8_1 == NULL) return -1 ;
     
     DXLONG64 chpos = 0   ;
     while(true)
     {
         cc1 =dx_get_utf8_char_ex2(&utf8_1,&utf8tmp,&charlen) ;
         if (cc1 == 0) return -1     ; // end of the source string, the char was not found

         if(cc1 == c) return chpos ;

         chpos++ ;

    }

  return -1 ;
}


PDX_STRING hdr_Utf8CopyUntilChar(PDX_STRING source , DXUTF8CHAR c , DXLONG64 *fr_indx)
{
    /*
	 The function starts from the fr_index and copy the string until the [c]. If the [c] does not exists 
	 then all the string from fr_indx until the end is copied and the fr_indx will set to -1.
	 When the function return the $fr_indx will have the position of the 
	 next character after the [c] OR if the character is the last of the string , the fr_indx will
	 have the position of the [c]
     In error the function return an empty PDX_STRING
	*/
     if((*fr_indx < 0)||(*fr_indx > (source->len-1))) 
     {
         *fr_indx = -1 ;
         return dx_string_createU(NULL,"") ;
     }
    char *utf8tmp = NULL ;
    char *utf8_1  = NULL ;
    DXLONG64 start_indx  ;
    /*go to the position*/
    if(*fr_indx > 0 )
    {
       utf8_1  = hdr_return_utf8_pos(source,*fr_indx,&start_indx) ;
    }
    else
    {
        utf8_1 = source->stringa ;
        start_indx = 0 ;
    }

    if(utf8_1 == NULL) 
    {
        *fr_indx = -1 ;
        /*binary copy all the string*/
        start_indx = 0 ;
        return CopyIndxToIndx(source,start_indx,source->bcount-1) ;
    }

    /*parse all the string to find the positions for copyng*/
    DXUTF8CHAR cc1 = 0 ;
    int char_len   = 0 ;
    DXLONG64 byte_len = 0 ; 
    while(true)
    {
        cc1 = dx_get_utf8_char_ex2(&utf8_1,&utf8tmp,&char_len) ;
        if((cc1 == c)||(cc1==0)) 
        {
          break ;
        }
       
        byte_len  = byte_len + char_len ;
        (*fr_indx)++ ; 
    }

    if (byte_len == 0)
    {
        /*the character was not found OR found but its the first character of the search so nothing will be copy*/
        if(cc1 != c) *fr_indx = -1 ;
        return dx_string_createU(NULL,"") ;
    }

    if(*fr_indx == source->len)
    {
      /*the *fr_indx has count the terminating 0 too that means that the character was not found*/
      *fr_indx = -1 ;
      return CopyIndxToIndx(source,start_indx,source->bcount-1) ; 
    }

    /*now copy the string*/
    return CopyIndxToIndx(source,start_indx,start_indx+(byte_len-1)) ;

}


/*
 soooo the utf8 upper case / lower case is a complicated story so i will support for now only the ones
 that i will use (latin and greek) and later i will add more charset
*/


PDX_STRING UpperCase(PDX_STRING str)
{
    /* We will iterate the string and if the character is a latin lowercase letter then we will transform it to an uppercase one*/
    
    if(str->len == 0) return dx_string_createU(NULL,"") ;
    
    char *buff        = (char*)malloc(str->bcount+1) ;
    buff[str->bcount] = 0                            ;

    memcpy(buff,str->stringa,str->bcount)            ;
    /*prepare the string*/
    PDX_STRING news   = dx_string_create_bU(buff)    ;
    
    char *indx   = news->stringa ;
    char *prevch = NULL          ; 
    int char_len = 0             ;
    
    while(true)
    {
      DXCHAR uchar = dx_get_utf8_char_ex2(&indx,&prevch,&char_len) ;
      if(char_len == 1)
      {
        char chr = *prevch ;

        if((chr > 96)&&(chr < 123))
        {
          *prevch = chr - 32 ;
        }

      }
      if(uchar == 0) break ;
    }

    return news ;
}

PDX_STRING LowerCase(PDX_STRING str)
{
    /* We will iterate the string and if the character is a latin uppercase letter then we will transform it 
       to a lowercase one
    */
    
    if(str->len == 0) return dx_string_createU(NULL,"") ;
    
    char *buff        = (char*)malloc(str->bcount+1) ;
    buff[str->bcount] = 0                            ;

    memcpy(buff,str->stringa,str->bcount)            ;
    /*prepare the string*/
    PDX_STRING news   = dx_string_create_bU(buff)    ;
    
    char *indx   = news->stringa ;
    char *prevch = NULL          ; 
    int char_len = 0             ;
    
    while(true)
    {
      DXCHAR uchar = dx_get_utf8_char_ex2(&indx,&prevch,&char_len) ;
      if(char_len == 1)
      {
        char chr = *prevch ;

        if((chr > 64)&&(chr < 91))
        {
          *prevch = chr + 32 ;
        }

      }
      if(uchar == 0) break ;
    }

    return news ;
}


DXCHAR lower_case_gr_ch(DXCHAR c)
{

    if (c == 902) return 940 ;
    if (c == 904) return 941 ;
    if (c == 905) return 942 ;
    if (c == 906) return 943 ;
    if (c == 908) return 972 ;
    if (c == 910) return 944 ;
    if (c == 911) return 974 ;
    if (c == 913) return 945 ;
    if (c == 914) return 946 ;
    if (c == 915) return 947 ;
    if (c == 916) return 948 ;
    if (c == 917) return 949 ;
    if (c == 918) return 950 ;
    if (c == 919) return 951 ;
    if (c == 920) return 952 ;
    if (c == 921) return 953 ;
    if (c == 922) return 954 ;
    if (c == 923) return 955 ;
    if (c == 924) return 956 ;
    if (c == 925) return 957 ;
    if (c == 926) return 958 ;
    if (c == 927) return 959 ;
    if (c == 928) return 960 ;
    if (c == 929) return 961 ;
    if (c == 931) return 963 ;
    if (c == 932) return 964 ;
    if (c == 933) return 965 ;
    if (c == 934) return 966 ;
    if (c == 935) return 967 ;
    if (c == 936) return 968 ;
    if (c == 937) return 969 ;
    if (c == 938) return 970 ;
    if (c == 939) return 971 ;
    return c ; /*no match*/
}

DXCHAR lower_case_gr_ch_no_tonos(DXCHAR c)
{

    if (c == 902) return 945 ;
    if (c == 904) return 949 ;
    if (c == 905) return 951 ;
    if (c == 906) return 953 ;
    if (c == 908) return 959 ;
    if (c == 910) return 965 ;
    if (c == 911) return 969 ;
    if (c == 913) return 945 ;
    if (c == 914) return 946 ;
    if (c == 915) return 947 ;
    if (c == 916) return 948 ;
    if (c == 917) return 949 ;
    if (c == 918) return 950 ;
    if (c == 919) return 951 ;
    if (c == 920) return 952 ;
    if (c == 921) return 953 ;
    if (c == 922) return 954 ;
    if (c == 923) return 955 ;
    if (c == 924) return 956 ;
    if (c == 925) return 957 ;
    if (c == 926) return 958 ;
    if (c == 927) return 959 ;
    if (c == 928) return 960 ;
    if (c == 929) return 961 ;
    if (c == 931) return 963 ;
    if (c == 932) return 964 ;
    if (c == 933) return 965 ;
    if (c == 934) return 966 ;
    if (c == 935) return 967 ;
    if (c == 936) return 968 ;
    if (c == 937) return 969 ;
    if (c == 938) return 953 ;
    if (c == 939) return 965 ;

    return c ; /*no match*/
}

DXCHAR upper_case_gr_ch(DXCHAR c)
{
  if (c == 940) return 902 ;
  if (c == 941) return 904 ;
  if (c == 942) return 905 ;
  if (c == 943) return 906 ;
  if (c == 972) return 908 ;
  if (c == 973) return 910 ; 
  if (c == 944) return 939 ;
  if (c == 974) return 911  ;
  if (c == 945) return 913  ;
  if (c == 946) return 914  ;
  if (c == 947) return 915  ;
  if (c == 948) return 916  ;
  if (c == 949) return 917  ;
  if (c == 950) return 918  ;
  if (c == 951) return 919  ;
  if (c == 952) return 920  ;
  if (c == 953) return 921  ;
  if (c == 954) return 922  ;
  if (c == 955) return 923  ;
  if (c == 956) return 924  ;
  if (c == 957) return 925  ;
  if (c == 958) return 926  ;
  if (c == 959) return 927  ;
  if (c == 960) return 928  ;
  if (c == 961) return 929  ;
  if ((c == 963)||(c==962)) return 931 ;
  if (c == 964) return 932  ;
  if (c == 965) return 933  ;
  if (c == 966) return 934  ;
  if (c == 967) return 935  ;
  if (c == 968) return 936  ;
  if (c == 969) return 937  ;
  if (c == 970) return 938  ;
  if (c == 971) return 939  ;
  if (c == 912) return 938  ;
  
   return c ;
}

DXCHAR upper_case_gr_ch_no_tonos(DXCHAR c)
{
  if (c == 940) return 913 ;
  if (c == 941) return 917 ;
  if (c == 942) return 919 ;
  if (c == 943) return 921 ;
  if (c == 972) return 927 ;
  if (c == 973) return 933 ;
  if (c == 944) return 933 ;
  if (c == 974) return 937 ;
  if (c == 945) return 913 ;
  if (c == 946) return 914  ;
  if (c == 947) return 915  ;
  if (c == 948) return 916  ;
  if (c == 949) return 917  ;
  if (c == 950) return 918  ;
  if (c == 951) return 919  ;
  if (c == 952) return 920  ;
  if (c == 953) return 921  ;
  if (c == 954) return 922  ;
  if (c == 955) return 923  ;
  if (c == 956) return 924  ;
  if (c == 957) return 925  ;
  if (c == 958) return 926  ;
  if (c == 959) return 927  ;
  if (c == 960) return 928  ;
  if (c == 961) return 929  ;
  if ((c == 963)||(c==962)) return 931 ;
  if (c == 964) return 932  ;
  if (c == 965) return 933  ;
  if (c == 966) return 934  ;
  if (c == 967) return 935  ;
  if (c == 968) return 936  ;
  if (c == 969) return 937  ;
  if (c == 970) return 921  ;
  if (c == 971) return 933  ;
  if (c == 912) return 921  ;

   return c ;
}



PDX_STRING UpperCaseGr(PDX_STRING str,bool no_tonos)
{
    /* We will iterate the string and if the character is a greek lowercase letter then we will transform it 
       to a uppercase one
    */
    
    if(str->len == 0) return dx_string_createU(NULL,"") ;
    
    char *buff        = (char*)malloc(str->bcount+1) ;
    buff[str->bcount] = 0                            ;

    memcpy(buff,str->stringa,str->bcount)            ;
    /*prepare the string*/
    PDX_STRING news   = dx_string_create_bU(buff)    ;
    
    char *indx   = news->stringa ;
    char *prevch = NULL          ; 
    int char_len = 0             ;
    
    while(true)
    {
      DXCHAR uchar = dx_get_utf8_char_ex2(&indx,&prevch,&char_len) ;
      if(char_len == 2) /*greek chars needs two bytes*/
      {
        DXCHAR pchar = 0 ;
        if(no_tonos == false)
        pchar = upper_case_gr_ch(uchar);
        else
            pchar = upper_case_gr_ch_no_tonos(uchar);
        
        if(pchar!=uchar) 
        {
          /*set the new byte characters*/
          char temp[5] ;
          dxConvertUint32ToUTF8(temp, pchar);
          prevch[0] = temp[0];
          prevch[1] = temp[1];
        }
         
      }
      if(uchar == 0) break ;
    }

    return news ;
}


PDX_STRING LowerCaseGr(PDX_STRING str,bool no_tonos)
{
    /* We will iterate the string and if the character is a greek uppercase letter then we will transform it 
       to a lowercase one
    */
    
    if(str->len == 0) return dx_string_createU(NULL,"") ;
    
    char *buff        = (char*)malloc(str->bcount+1) ;
    buff[str->bcount] = 0                            ;

    memcpy(buff,str->stringa,str->bcount)            ;
    /*prepare the string*/
    PDX_STRING news   = dx_string_create_bU(buff)    ;
    
    char *indx   = news->stringa ;
    char *prevch = NULL          ; 
    int char_len = 0             ;
    
    while(true)
    {
      DXCHAR uchar = dx_get_utf8_char_ex2(&indx,&prevch,&char_len) ;
      if(char_len == 2) /*greek chars needs two bytes*/
      {
        DXCHAR pchar = 0 ;
        if(no_tonos == false)
        pchar = lower_case_gr_ch(uchar);
        else
            pchar = lower_case_gr_ch_no_tonos(uchar);
        
        if(pchar!=uchar) 
        {
          /*set the new byte characters*/
          char temp[5] ;
          dxConvertUint32ToUTF8(temp, pchar);
          prevch[0] = temp[0];
          prevch[1] = temp[1];
        }
         
      }
      if(uchar == 0) break ;
    }

    return news ;
}















