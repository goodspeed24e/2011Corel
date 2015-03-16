#ifndef ASN1_DATA_STRUCTURE_H
#define ASN1_DATA_STRUCTURE_H

/*ASN1 ERROR*/
#define ASN1_FALSE				S_FALSE
#define ASN1_OK					S_OK

#define ASN1_ERROR_LENGTH		300
#define ASN1_ERROR_TAG_VALUE	301
#define ASN1_ERROR_CHECK_HEADER	302
#define ASN1_ERROR_USING_BER	303
#define ASN1_ERROR_NULL_DATA	304
#define ASN1_ERROR_NO_FOUND_OBJ	305
#define ASN1_ERROR_MISSTYPE		306
#define ASN1_ERROR_NO_IMPLEMENT	307
#define ASN1_ERROR_NOT_MATCH	308

/*Limit*/
#define STACK_num               3
#define INT_MAX				    2147483647

/* Four type of ASN.1 */
#define ASN1_UNIVERSAL		    0x00
#define	ASN1_APPLICATION		0x40
#define ASN1_CONTEXT_SPECIFIC	0x80
#define ASN1_PRIVATE			0xc0

/* Identifier octet */
#define ASN1_CONSTRUCTED		0x20
#define ASN1_PRIMITIVE			0x1f

#define ASN1_CONSTRUCTED_TAG	0x20
#define ASN1_PRIMITIVE_TAG		0x00

/* Tag value of ASN.1 */
#define ASN1_BOOLEAN_TAG			1	
#define ASN1_INTEGER_TAG			2
#define ASN1_NEGATIVE_INTEGER_TAG  (2|0x200)
#define ASN1_BIT_STRING_TAG		    3
#define ASN1_OCTET_STRING_TAG		4
#define ASN1_NULL_TAG			    5
#define ASN1_OBJECT_TAG			    6
#define ASN1_OBJECT_DESCRIPTOR_TAG	7
#define ASN1_EXTERNAL_TAG			8
#define ASN1_REAL_TAG			    9
#define ASN1_ENUMERATED_TAG		   10
#define ASN1_UTF8STRING_TAG		   12
#define ASN1_SEQUENCE_TAG		   16
#define ASN1_SET_TAG			   17
#define ASN1_NUMERICSTRING_TAG	   18	
#define ASN1_PRINTABLESTRING_TAG   19
#define ASN1_T61STRING_TAG		   20
#define ASN1_TELETEXSTRING_TAG	   20	
#define ASN1_VIDEOTEXSTRING_TAG	   21	
#define ASN1_IA5STRING_TAG		   22
#define ASN1_UTCTIME_TAG		   23
#define ASN1_GENERALIZEDTIME_TAG   24	
#define ASN1_GRAPHICSTRING_TAG	   25	
#define ASN1_ISO64STRING_TAG	   26	
#define ASN1_VISIBLESTRING_TAG	   26	
#define ASN1_GENERALSTRING_TAG	   27	
#define ASN1_UNIVERSALSTRING_TAG   28	
#define ASN1_BMPSTRING_TAG		   30

/* No tag value*/
#define ASN1_CHOICE (-1)
#define ASN1_ANY    (-2)

/* Optional */
#define ASN1_FIELD_OPTIONAL	(0x1)
#define ASN1_PASS_DATA	    (0x1 << 2)
#define ASN1_SEQUENCE_OF	(0x1 << 3)
#define ASN1_SET_OF		    (0x1 << 4)
#define ASN1_EXPLICIT		(0x1 << 5)
#define ASN1_IMPLICIT		(0x1 << 6)
#define ASN1_NAME           (0x1 << 7)
#define ASN1_SIGNERINFO		(0x1 << 8)

/* Free memory tag */
#define FREE_STRING_TAG		      0x1
#define FREE_OBJECT_TAG			  0x2
#define FREE_ANY_TAG			  0x3
#define FREE_STACK_TAG			  0x4
#define FREE_PASS_TAG			  0x5
#define FREE_X509_STACK_TAG		  0x6
#define FREE_NAME_STACK_TAG		  0x7
#define FREE_EXTENSIONS_STACK_TAG 0x8
#define FREE_UNSIGNED_CHAR_TAG    0x9
#define FREE_SIGNERINFO_STACK_TAG 0x10

typedef struct asn1_string_st
{
	int length;
	int type;
	unsigned char *data;
} ASN1_STRING;

typedef struct asn1_object_identifier_st
{
	char *name;
	char *mean;
	int ID;
	int length;
	unsigned char *data;
} ASN1_OBJECT;

typedef struct stack_st
{
	int num;
	int max_num;
	char **data;
} STACK;

typedef struct ASN1_TempData_st{
	int PC;	   /* Primitive or Constructed */
	long plen;	/* length */
	int ptag;	/* class value */
	int pclass;	/* "Universal" or "Application" or "Context-specific" or "Private" */
	int hdrlen;	/* header length */
	int definite;	   /* Definite form=1 or Indefinite form */
}ASN1_TEMPDATA;

typedef struct ASN1_Info_st
{
	int v_class;
	int v_tag;
	int v_flag;
	int v_opt;
	int address;
}ASN1_INFO;

typedef int ASN1_BOOLEAN;
typedef int ASN1_NULL;
typedef ASN1_STRING ASN1_INTEGER;
typedef ASN1_STRING ASN1_ENUMERATED;
typedef ASN1_STRING ASN1_BIT_STRING;
typedef ASN1_STRING ASN1_OCTET_STRING;
typedef ASN1_STRING ASN1_PRINTABLESTRING;
typedef ASN1_STRING ASN1_T61STRING;
typedef ASN1_STRING ASN1_IA5STRING;
typedef ASN1_STRING ASN1_GENERALSTRING;
typedef ASN1_STRING ASN1_UNIVERSALSTRING;
typedef ASN1_STRING ASN1_BMPSTRING;
typedef ASN1_STRING ASN1_UTCTIME;
typedef ASN1_STRING ASN1_TIME;
typedef ASN1_STRING ASN1_GENERALIZEDTIME;
typedef ASN1_STRING ASN1_VISIBLESTRING;
typedef ASN1_STRING ASN1_UTF8STRING;

typedef struct asn1_any_st
{
	int type;
	union	{
		ASN1_NULL				null;
		ASN1_BOOLEAN		    boolean;
		ASN1_STRING 		    string;
		ASN1_OBJECT 		    object;
	} value;
}ASN1_ANY_ST;

#endif