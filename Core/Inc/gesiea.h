/**
 * @file sys3046Template.h
 * @author  Name1
 * @author  Name2
 * @version 1.0.0
 * @brief my short description for sys3046Template.h
 * @details more details for this file
 */   
 
#ifndef SYS3046TEMPLATE_H_INCLUDE
#define SYS3046TEMPLATE_H_INCLUDE

/**
 * Description of MYCONFIG_VALUE
 */
#define MYCONFIG_VALUE  0x1234


/**
 * Description of myStruct
 */
 struct myStruct 
 { 
   /** member1 documentation */
   int member1; 
   /** member2 documentation */
   char member2; 
 };


 /**
 * This is the function 1 description
 *@param parameter1  Description of parameter1
 *@param parameter2  Description of parameter2
 *@return Description of returned value
 */ 
int myFirstFunction( char * parameter1, long parameter2);

 /**
 * This is the function 2 description
 *@param parameter1  Description of parameter1
 */ 
void mySecondFunction( char * parameter1, long parameter2);


#endif  //SYS3046TEMPLATE_H_INCLUDE
