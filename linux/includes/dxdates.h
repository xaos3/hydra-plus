/*
 Date time manipulation routines

 Nikos Mourgis deus-ex.gr 2024

 Live Long and Prosper !
*/


PDX_STRING dx_day_of_the_week(int day) ;
PDX_STRING dx_day_of_the_week_full(int day) ;
PDX_STRING dx_month_name(int month) ;
PDX_STRING dx_month_name_full(int month) ;

time_t dx_Now();
time_t dx_convert_to_time(char *str);    /*only in YYYY-MM-DD 00:00:00 form*/
time_t dx_convert_to_time_struct(char *str,struct tm *time);/*same as the dx_convert_to_time but fills the time struct too*/
PDX_STRING dx_time_to_date_str(time_t ttime,bool add_time) ;/*returns the time in the form of DD-MM-YYY 00:00:00 form*/

/*
 returns the time in the format that we set to.
 Valid format elements : %d , %m ,%y ,%D ,%M ,%Y %: #d,#m

 the %d is the day as one or two digits
 the %m is the month as one or two digits

 the %D is the day as two digits
 the %M is the month as two digits
 the %Y is the year as four digits

 when the %: is encountered it will be replaced by the time component as 00:00:00

 the #d,#m will be replaced by the names of the days and months

*/
PDX_STRING dx_convert_to_time_ex(char *format,time_t ttime) ;

/*
 returns the days between this two dates. The dates MUST be in the 
 format of 2024-01-28 00:00:00 
*/
DXLONG64 dx_days_diff(char *date1 , char *date2) ;
/*
 returns the seconds difference between the two days
*/
DXLONG64 dx_sec_diff(char *date1 , char *date2);
/*add the days_to_add to the date. To subtract days pass a negative value*/
time_t dx_add_days(char *date,DXLONG64 days_to_add) ;

enum dx_date_comp {dx_date_smaller,dx_date_bigger,dx_date_equal};
/*The function return dx_date_smaller if the date1 is smaller than date2 
  dx_date_bigger if the date1 is bigger , and dx_date_equal if the dates are equal 
*/
enum dx_date_comp dx_dates_compare(char *base_date , char *comp_date);


/****************************Implementation *******************************/

PDX_STRING dx_day_of_the_week(int day) 
{
  if((day < 1)||(day>7)) return dx_string_createU(NULL,"Unk");

  char * days[7] ;
  days[0] = "Sun";
  days[1] = "Mon";
  days[2] = "Tue";
  days[3] = "Wed";
  days[4] = "Thu";
  days[5] = "Fri";
  days[6] = "Sat";
  
  return dx_string_createU(NULL,days[day-1]) ;
}

PDX_STRING dx_day_of_the_week_full(int day) 
{
  if((day < 1)||(day>7)) return dx_string_createU(NULL,"Unknown");
  
  char * days[7] ;
  days[0] = "Sunday";
  days[1] = "Monday";
  days[2] = "Tuesday";
  days[3] = "Wednesday";
  days[4] = "Thursday";
  days[5] = "Friday";
  days[6] = "Saturday";

  return dx_string_createU(NULL,days[day-1]) ;
}


PDX_STRING dx_month_name(int month)
{
  if((month < 0)||(month>11)) return dx_string_createU(NULL,"Unk");
  char * months[12] ;
  months[0]  = "Jan";
  months[1]  = "Feb";
  months[2]  = "Mar";
  months[3]  = "Apr";
  months[4]  = "May";
  months[5]  = "Jun";
  months[6]  = "Jul";
  months[7]  = "Aug";
  months[8]  = "Sep";
  months[9]  = "Oct";
  months[10] = "Nov";
  months[11] = "Dec";

  return dx_string_createU(NULL,months[month]) ;
}

PDX_STRING dx_month_name_full(int month)
{
  if((month < 0)||(month > 11)) return dx_string_createU(NULL,"Unknown");
  char * months[12] ;
  months[0]  = "January";
  months[1]  = "February";
  months[2]  = "March";
  months[3]  = "April";
  months[4]  = "May";
  months[5]  = "June";
  months[6]  = "July";
  months[7]  = "August";
  months[8]  = "September";
  months[9]  = "October";
  months[10] = "November";
  months[11] = "December";

  return dx_string_createU(NULL,months[month]) ;

}


time_t dx_Now()
{
  return time(NULL);
}

time_t dx_convert_to_time(char *str)
{
  /*only in YYYY-MM-DD 00:00:00 form*/
	struct tm time = {0} ;
	
	char *strindx = str ;

	/*fill the time fields */
	
	/*year*/
	char *elem = dxCopyStrToChar(&strindx,'-',"") ;
	bool error = false ;
	time.tm_year = dx_string_to_int(elem,&error) - 1900 ;

	free(elem);
	if((error == true)||(*strindx == 0) ){return -1 ;}
	strindx++ ;
	if(*strindx == 0) {return -1;}
	
	/*month*/
	elem = dxCopyStrToChar(&strindx,'-',"") ;
	error = false ;
	time.tm_mon = dx_string_to_int(elem,&error) - 1 ;

	free(elem);
	if((error == true)||(*strindx == 0) ){return -1 ;}
	strindx++ ;
	if(*strindx == 0) {return -1;}

	/*day*/
	elem   = dxCopyStrToChar(&strindx,' ',"") ;
	error   = false ;
	time.tm_mday = dx_string_to_int(elem,&error)      ;

	free(elem);
	if((error == true)||(*strindx == 0) ){return -1 ;}
	strindx++ ;
	if(*strindx == 0) {return -1;}

	/*get hours*/
	elem   = dxCopyStrToChar(&strindx,':',"") ;
	error   = false ;
	time.tm_hour = dx_string_to_int(elem,&error)      ;

	free(elem);
	if((error == true)||(*strindx == 0) ){return -1 ;}
	strindx++ ;
	if(*strindx == 0) {return -1;}

	/*get minutes*/
	elem   = dxCopyStrToChar(&strindx,':',"") ;
	error   = false ;
	time.tm_min = dx_string_to_int(elem,&error)      ;

	free(elem);
	if((error == true)||(*strindx == 0) ){return -1 ;}
	strindx++ ;
	if(*strindx == 0) {return -1;}

	/*get seconds*/
	elem   = dxCopyStrToChar(&strindx,' ',"") ; /*until the end actually*/
	error   = false ;
	time.tm_sec = dx_string_to_int(elem,&error)      ;

	free(elem);
	if(error == true){return -1 ;}
	/*every other element after that is uselless for us*/
	/*do the magic of the mktime!!!*/

	return mktime(&time);
}

void dx_set_chars(char **strindx,PDX_STRING elem)
{
	for(int i = 0 ; i < elem->len;i++)
	{
	  *(*strindx) = elem->stringa[i] ;
	  (*strindx)++ ;
	}
 return ;
}

PDX_STRING dx_time_to_date_str(time_t ttime, bool add_time) 
{
  /*return date time in YYYY-MM-DD 00:00:00 form*/
  struct tm *tims = localtime(&ttime);

  int len =  10 ;
  if(add_time == true) len = len + 9 ;

  char * datestr = malloc(len+1) ;
  datestr[len] = 0 ;
  char *strindx = datestr ;
  /*year*/
  PDX_STRING elem = dx_IntToStr(tims->tm_year + 1900) ;
  dx_PaddWithZeros(elem,4) ;
  dx_set_chars(&strindx,elem) ;
  dx_string_free(elem)     ;
  *strindx = '-';
  strindx++ ;

  /*month*/
  elem = dx_IntToStr(tims->tm_mon + 1) ;
  dx_PaddWithZeros(elem,2) ;
  dx_set_chars(&strindx,elem) ;
  dx_string_free(elem)     ;
  *strindx = '-';
  strindx++ ;

  /*day*/
  elem = dx_IntToStr(tims->tm_mday) ;
  dx_PaddWithZeros(elem,2) ;
  dx_set_chars(&strindx,elem) ;
  dx_string_free(elem)     ;

  if(add_time == true)
  {
	  *strindx = ' ';
	  strindx++ ;
	  /*hours*/
	  elem = dx_IntToStr(tims->tm_hour) ;
	  dx_PaddWithZeros(elem,2) ;
	  dx_set_chars(&strindx,elem) ;
	  dx_string_free(elem)     ;
	  *strindx = ':';
	  strindx++ ;

	  /*minutes*/
	  elem = dx_IntToStr(tims->tm_min) ;
	  dx_PaddWithZeros(elem,2) ;
	  dx_set_chars(&strindx,elem) ;
	  dx_string_free(elem)     ;
	  *strindx = ':';
	  strindx++ ;

	  /*seconds*/
	  elem = dx_IntToStr(tims->tm_sec) ;
	  dx_PaddWithZeros(elem,2) ;
	  dx_set_chars(&strindx,elem) ;
	  dx_string_free(elem)     ;
  }

  return dx_string_create_bU(datestr) ;
}

PDX_STRING dx_convert_to_time_ex(char *format,time_t ttime) 
{
  /*%d , %m ,%D ,%M ,%Y %: #d,#m #D #M*/
  struct tm *tm = localtime(&ttime);
  /*we opt for the easy way xD so .. replace , replace ,replace!*/
  PDX_STRING nstr = dx_string_createU(NULL,format) ;

  /*#d*/
  PDX_STRING reple = dx_string_createU(NULL,"#d") ;
  PDX_STRING elem  = dx_day_of_the_week(tm->tm_wday+1) ; 
  dx_replace_word_binary(nstr,0,reple,elem,true) ;
  dx_string_free(elem) ;

  /*#M*/
  dx_string_createU(reple,"#M") ;
  elem  = dx_month_name_full(tm->tm_mon) ; 
  dx_replace_word_binary(nstr,0,reple,elem,true) ;
  dx_string_free(elem) ;

   /*#D*/
  reple = dx_string_createU(reple,"#D") ;
  elem  = dx_day_of_the_week_full(tm->tm_wday+1) ; 
  dx_replace_word_binary(nstr,0,reple,elem,true) ;
  dx_string_free(elem) ;

  /*#m*/
  reple = dx_string_createU(reple,"#m") ;
  elem  = dx_month_name(tm->tm_mon) ; 
  dx_replace_word_binary(nstr,0,reple,elem,true) ;
  dx_string_free(elem) ;


  /*d*/
  reple = dx_string_createU(reple,"%d") ;
  elem  = dx_IntToStr(tm->tm_mday) ; 
  dx_replace_word_binary(nstr,0,reple,elem,true) ;
  dx_string_free(elem) ;

  /*m*/
  reple = dx_string_createU(reple,"%m") ;
  elem  = dx_IntToStr(tm->tm_mon+1) ; 
  dx_replace_word_binary(nstr,0,reple,elem,true) ;
  dx_string_free(elem) ;

  /*D*/
  reple = dx_string_createU(reple,"%D") ;
  elem  = dx_IntToStr(tm->tm_mday) ; 
  dx_PaddWithZeros(elem,2) ;
  dx_replace_word_binary(nstr,0,reple,elem,true) ;
  dx_string_free(elem) ;

  /*M*/
  reple =dx_string_createU(reple,"%M") ;
  elem  = dx_IntToStr(tm->tm_mon+1) ; 
  dx_PaddWithZeros(elem,2) ;
  dx_replace_word_binary(nstr,0,reple,elem,true) ;
  dx_string_free(elem) ;

  /*Y*/
  reple = dx_string_createU(reple,"%Y") ;
  elem  = dx_IntToStr(tm->tm_year+1900) ; 
  dx_PaddWithZeros(elem,4) ;
  dx_replace_word_binary(nstr,0,reple,elem,true) ;
  dx_string_free(elem) ;

  /*%:*/
  reple = dx_string_createU(reple,"%:") ;
  elem  = dx_IntToStr(tm->tm_hour) ;
  dx_PaddWithZeros(elem,2) ;
  PDX_STRING min   = dx_IntToStr(tm->tm_min) ;
  dx_PaddWithZeros(min,2)  ;
  PDX_STRING sec   = dx_IntToStr(tm->tm_sec) ;
  dx_PaddWithZeros(sec,2) ;
  PDX_STRING semi  = dx_string_createU(NULL,":") ;
  PDX_STRING nelem = dx_string_concat(elem,semi);
  dx_string_free(elem) ;
  elem			   = dx_string_concat(nelem,min);
  dx_string_free(nelem) ;
  nelem			   = dx_string_concat(elem,semi);
  dx_string_free(elem) ;
  elem			   = dx_string_concat(nelem,sec);
  dx_replace_word_binary(nstr,0,reple,elem,true) ;
  dx_string_free(elem) ;
  dx_string_free(min)  ;
  dx_string_free(sec)  ;
  dx_string_free(semi) ;
  dx_string_free(nelem);
  dx_string_free(reple);

  return nstr ;
}


DXLONG64 dx_days_diff(char *date1 , char *date2)
{
    time_t t1 = dx_convert_to_time(date1);
    time_t t2 = dx_convert_to_time(date2);
    double dt = difftime(t1, t2);
    return abs(round(dt / 86400)) ;
}

time_t dx_add_days(char *date,DXLONG64 days_to_add) 
{

	 /*only in YYYY-MM-DD 00:00:00 form*/
	struct tm time = {0} ;
	
	char *strindx = date ;

	/*fill the time fields */
	
	/*year*/
	char *elem = dxCopyStrToChar(&strindx,'-',"") ;
	bool error = false ;
	time.tm_year = dx_string_to_int(elem,&error) - 1900 ;

	free(elem);
	if((error == true)||(*strindx == 0) ){return -1 ;}
	strindx++ ;
	if(*strindx == 0) {return -1;}
	
	/*month*/
	elem = dxCopyStrToChar(&strindx,'-',"") ;
	error = false ;
	time.tm_mon = dx_string_to_int(elem,&error) - 1 ;

	free(elem);
	if((error == true)||(*strindx == 0) ){return -1 ;}
	strindx++ ;
	if(*strindx == 0) {return -1;}

	/*day*/
	elem   = dxCopyStrToChar(&strindx,' ',"") ;
	error   = false ;
	time.tm_mday = dx_string_to_int(elem,&error)      ;

	free(elem);
	if((error == true)||(*strindx == 0) ){return -1 ;}
	strindx++ ;
	if(*strindx == 0) {return -1;}

	/*get hours*/
	elem   = dxCopyStrToChar(&strindx,':',"") ;
	error   = false ;
	time.tm_hour = dx_string_to_int(elem,&error)      ;

	free(elem);
	if((error == true)||(*strindx == 0) ){return -1 ;}
	strindx++ ;
	if(*strindx == 0) {return -1;}

	/*get minutes*/
	elem   = dxCopyStrToChar(&strindx,':',"") ;
	error   = false ;
	time.tm_min = dx_string_to_int(elem,&error)      ;

	free(elem);
	if((error == true)||(*strindx == 0) ){return -1 ;}
	strindx++ ;
	if(*strindx == 0) {return -1;}

	/*get seconds*/
	elem   = dxCopyStrToChar(&strindx,' ',"") ; /*until the end actually*/
	error   = false ;
	time.tm_sec = dx_string_to_int(elem,&error)      ;

	free(elem);
	if(error == true){return -1 ;}
	/*every other element after that is uselless for us*/

	time.tm_mday = time.tm_mday + days_to_add ;
	/*do the magic of the mktime!!!*/

	return mktime(&time);

}

DXLONG64 dx_sec_diff(char *date1 , char *date2)
{
    time_t t1 = dx_convert_to_time(date1);
    time_t t2 = dx_convert_to_time(date2);
   
    return abs(t1-t2) ;
}

enum dx_date_comp dx_dates_compare(char *base_date , char *comp_date)
{
   time_t t1 = dx_convert_to_time(base_date);
   time_t t2 = dx_convert_to_time(comp_date);
   
   DXLONG64 dif =  t1-t2 ;

   if(dif < 0) return dx_date_smaller;
   if(dif > 0) return dx_date_bigger;
   return dx_date_equal ;
}


time_t dx_convert_to_time_struct(char *str,struct tm *time)
{
  /*only in YYYY-MM-DD 00:00:00 form*/
	memset(time,0,sizeof(struct tm)) ;
	
	char *strindx = str ;

	/*fill the time fields */
	
	/*year*/
	char *elem = dxCopyStrToChar(&strindx,'-',"") ;
	bool error = false ;
	time->tm_year = dx_string_to_int(elem,&error) - 1900 ;

	free(elem);
	if((error == true)||(*strindx == 0) ){return -1 ;}
	strindx++ ;
	if(*strindx == 0) {return -1;}
	
	/*month*/
	elem = dxCopyStrToChar(&strindx,'-',"") ;
	error = false ;
	time->tm_mon = dx_string_to_int(elem,&error) - 1 ;

	free(elem);
	if((error == true)||(*strindx == 0) ){return -1 ;}
	strindx++ ;
	if(*strindx == 0) {return -1;}

	/*day*/
	elem   = dxCopyStrToChar(&strindx,' ',"") ;
	error   = false ;
	time->tm_mday = dx_string_to_int(elem,&error)      ;

	free(elem);
	if((error == true)||(*strindx == 0) ){return -1 ;}
	strindx++ ;
	if(*strindx == 0) {return -1;}

	/*get hours*/
	elem   = dxCopyStrToChar(&strindx,':',"") ;
	error   = false ;
	time->tm_hour = dx_string_to_int(elem,&error)      ;

	free(elem);
	if((error == true)||(*strindx == 0) ){return -1 ;}
	strindx++ ;
	if(*strindx == 0) {return -1;}

	/*get minutes*/
	elem   = dxCopyStrToChar(&strindx,':',"") ;
	error   = false ;
	time->tm_min = dx_string_to_int(elem,&error)      ;

	free(elem);
	if((error == true)||(*strindx == 0) ){return -1 ;}
	strindx++ ;
	if(*strindx == 0) {return -1;}

	/*get seconds*/
	elem   = dxCopyStrToChar(&strindx,' ',"") ; /*until the end actually*/
	error   = false ;
	time->tm_sec = dx_string_to_int(elem,&error)      ;

	free(elem);
	if(error == true){return -1 ;}
	/*every other element after that is uselless for us*/
	/*do the magic of the mktime!!!*/

	return mktime(time);
}