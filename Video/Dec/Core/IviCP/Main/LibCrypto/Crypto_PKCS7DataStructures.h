#ifndef PKCS7_DATA_STRUCTURE_H
#define PKCS7_DATA_STRUCTURE_H

/**********************************/
/*       ASN.1 DATA STRUCTURE     */
/**********************************/
/*ASN1 ERROR*/
#define ASN1_FALSE				S_FALSE
#define ASN1_OK					S_OK

#define ASN1_ERROR_LENGTH		   300
#define ASN1_ERROR_TAG_VALUE	   301
#define ASN1_ERROR_CHECK_HEADER	   302
#define ASN1_ERROR_USING_BER	   303
#define ASN1_ERROR_NULL_DATA	   304
#define ASN1_ERROR_NO_FOUND_OBJ	   305
#define ASN1_ERROR_MISSTYPE		   306
#define ASN1_ERROR_NO_IMPLEMENT	   307
#define ASN1_ERROR_NOT_MATCH	   308
#define ASN1_ERROR_END_OF_CONTENTS 309

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

/**********************************/
/*       X509 DATA STRUCTURE     */
/**********************************/
#define asn1_info_X509_num 28
#define asn1_info_X509_Extensions_num 4
#define asn1_info_X509_AuthorityID_num 7
#define free_X509_num 18

const ASN1_INFO asn1_info_X509[asn1_info_X509_num]={
 {ASN1_UNIVERSAL,ASN1_SEQUENCE_TAG,0,0,0},		 //Certificate::=SEQUENCE
 {ASN1_UNIVERSAL,ASN1_SEQUENCE_TAG,0,0,0},		 //tbsCertificate::=SEQUENCE
 {ASN1_CONTEXT_SPECIFIC, 0, ASN1_EXPLICIT,1,0},  // version [0] EXPLICIT Version DEFAULT v1
 {ASN1_UNIVERSAL, ASN1_INTEGER_TAG, 0,0,1},		 // Version (INTEGER)
 {ASN1_UNIVERSAL, ASN1_INTEGER_TAG, 0,0,1},		 // serialNumber (INTEGER)
 {ASN1_UNIVERSAL,ASN1_SEQUENCE_TAG,0,0,0},		 // signature AlgorithmIdentifier (SEQUENCE)
 {ASN1_UNIVERSAL,ASN1_OBJECT_TAG,0,0,1},		 //algorithm (OBJECT IDENTIFIER)
 {ASN1_UNIVERSAL,ASN1_ANY,0,1,1},				 //parameters ANY DEFINED BY algorithm OPTIONAL
 {ASN1_UNIVERSAL,ASN1_CHOICE,ASN1_NAME,1},		 //issuer NAME(CHOICE)
 {ASN1_UNIVERSAL,ASN1_SEQUENCE_TAG,0,0,0},		 // validity::=SEQUENCE
 {ASN1_UNIVERSAL,ASN1_CHOICE,0,0,1},			 //notBefore (CHOICE) 
 {ASN1_UNIVERSAL,ASN1_CHOICE,0,0,1},		     //notAfter  (CHOICE) 	
 {ASN1_UNIVERSAL,ASN1_CHOICE,ASN1_NAME,1},		 //subject NAME(CHOICE)
 {ASN1_UNIVERSAL,ASN1_SEQUENCE_TAG,0,0,0},		 // subjectPublicKeyInfo::=SEQUENCE
 {ASN1_UNIVERSAL,ASN1_SEQUENCE_TAG,0,0,0},		 // AlgorithmIdentifier (SEQUENCE)
 {ASN1_UNIVERSAL,ASN1_OBJECT_TAG,0,0,1},		 //algorithm (OBJECT IDENTIFIER)
 {ASN1_UNIVERSAL,ASN1_ANY,0,1,1},				 //parameters ANY DEFINED BY algorithm OPTIONAL
 {ASN1_UNIVERSAL,ASN1_BIT_STRING_TAG,0,0,1},	 //subjectPublicKey (BIT STRING)
 {ASN1_CONTEXT_SPECIFIC, 1, ASN1_IMPLICIT, 1,0}, // issuerUniqueID [1] IMPLICIT UniqueIdentifier OPTIONAL
 {ASN1_UNIVERSAL, ASN1_BIT_STRING_TAG, 0, 0,1},  // UniqueIdentifier (BIT STRING)
 {ASN1_CONTEXT_SPECIFIC, 2, ASN1_IMPLICIT, 1,0}, // subjectUniqueID [2] IMPLICIT UniqueIdentifier OPTIONAL
 {ASN1_UNIVERSAL, ASN1_BIT_STRING_TAG, 0, 0,1},  // UniqueIdentifier (BIT STRING)
 {ASN1_CONTEXT_SPECIFIC, 3, ASN1_IMPLICIT|ASN1_SEQUENCE_OF, 1,0}, // extensions [3] EXPLICIT Extensions OPTIONAL
 {ASN1_UNIVERSAL,ASN1_SEQUENCE_TAG, ASN1_SEQUENCE_OF,0,0},
 {ASN1_UNIVERSAL,ASN1_SEQUENCE_TAG,0,0,0},		 //signatureAlgorithm  AlgorithmIdentifier(SEQUENCE)
 {ASN1_UNIVERSAL,ASN1_OBJECT_TAG,0,0,1},		 //algorithm (OBJECT IDENTIFIER)
 {ASN1_UNIVERSAL,ASN1_ANY,0,1,1},				 //extnValue (OCTET STRING)
 {ASN1_UNIVERSAL,ASN1_BIT_STRING_TAG,0,0,1}		 //signatureValue (BIT STRING)
};

const ASN1_INFO asn1_info_X509_Extensions[asn1_info_X509_Extensions_num]={
 {ASN1_UNIVERSAL,ASN1_SEQUENCE_TAG, 0,0,0},
 {ASN1_UNIVERSAL,ASN1_OBJECT_TAG,0,0,1},
 {ASN1_UNIVERSAL,ASN1_BOOLEAN_TAG,0,1,1},
 {ASN1_UNIVERSAL,ASN1_OCTET_STRING_TAG,0,0,1}
};

const ASN1_INFO asn1_info_X509_AuthorityID[asn1_info_X509_AuthorityID_num]={
 {ASN1_UNIVERSAL,ASN1_SEQUENCE_TAG,0,0,0},
 {ASN1_CONTEXT_SPECIFIC,0,ASN1_IMPLICIT,1,0},
 {ASN1_UNIVERSAL,ASN1_OCTET_STRING_TAG,0,0,1},
 {ASN1_CONTEXT_SPECIFIC,1,ASN1_IMPLICIT,1,0},
 {ASN1_UNIVERSAL,ASN1_CHOICE,ASN1_NAME,0,1},
 {ASN1_CONTEXT_SPECIFIC,2,ASN1_IMPLICIT,1,0},
 {ASN1_UNIVERSAL,ASN1_INTEGER_TAG,0,0,1}
};

const int free_X509[free_X509_num]={
	FREE_STRING_TAG, FREE_STRING_TAG, FREE_OBJECT_TAG, FREE_ANY_TAG,
	FREE_NAME_STACK_TAG, FREE_STRING_TAG, FREE_STRING_TAG,
	FREE_NAME_STACK_TAG, FREE_OBJECT_TAG, FREE_ANY_TAG,FREE_STRING_TAG,
	FREE_STRING_TAG, FREE_STRING_TAG, FREE_EXTENSIONS_STACK_TAG,
	FREE_OBJECT_TAG, FREE_ANY_TAG, FREE_STRING_TAG, FREE_UNSIGNED_CHAR_TAG};

typedef struct X509_algor_st
{
	ASN1_OBJECT algorithm;
	ASN1_ANY_ST parameter;
}X509_ALGOR;

typedef struct X509_AttributeTypeAndValue_st
{
	ASN1_OBJECT type;
	ASN1_ANY_ST value;
}X509_ATTRIBUTE_TYPE_AND_VALUE;

typedef struct X509_name_st
{
	STACK* RelativeDistinguishedName;
}X509_NAME_st;

typedef struct X509_validity_st
{
	ASN1_TIME notBefore;
	ASN1_TIME notAfter;
}X509_VALIDITY;

typedef struct X509_extension_st
{
	ASN1_OBJECT extnID;
	ASN1_BOOLEAN critical;
	ASN1_OCTET_STRING extnValue;
}X509_EXTENSION;

typedef struct X509_pubkey_st
{
	X509_ALGOR algor;
	ASN1_BIT_STRING public_key;
}X509_PUBKEY;

typedef struct x509_cinf_st
{
	ASN1_INTEGER version;		/* version 1 */
	ASN1_INTEGER serialNumber;
	X509_ALGOR signature;
	X509_NAME_st issuer;
	X509_VALIDITY validity;
	X509_NAME_st subject;
	X509_PUBKEY key;
	ASN1_BIT_STRING issuerUID;	/* version 2 */
	ASN1_BIT_STRING subjectUID;	/* version 2 */
	STACK *extensions;			/* version 3 */
}X509_CINF;

typedef struct x509_st
{
	X509_CINF tbsCertificate;
	X509_ALGOR signatureAlgorithm;
	ASN1_BIT_STRING signatureValue;
	unsigned char certDigest[20];
	unsigned char* subjectname;
}X509_st;

typedef struct AuthorityKeyID
{
	ASN1_OCTET_STRING keyIdentifier;
	X509_NAME_st authorityCertIssuer;
	ASN1_INTEGER authorityCertSerialNumber;
}AUTHORITYKEYID;
/**********************************/
/*       PKCS7 DATA STRUCTURE     */
/**********************************/
#define asn1_info_PKCS7_num 3
#define asn1_info_PKCS7_SignedData_num 8
#define asn1_info_PKCS7_SignerInfo_num 14
#define free_PKCS7_num 7

const ASN1_INFO asn1_info_PKCS7[asn1_info_PKCS7_num]={
 {ASN1_UNIVERSAL,ASN1_SEQUENCE_TAG,0,0,0},
 {ASN1_UNIVERSAL,ASN1_OBJECT_TAG,0,0,1},
 {ASN1_CONTEXT_SPECIFIC, 0, ASN1_EXPLICIT, 1,0}};

const ASN1_INFO asn1_info_PKCS7_SignedData[asn1_info_PKCS7_SignedData_num]={
 {ASN1_UNIVERSAL,ASN1_SEQUENCE_TAG,0,0,0},
 {ASN1_UNIVERSAL, ASN1_INTEGER_TAG, 0,0,1},
 {ASN1_UNIVERSAL, ASN1_SET_TAG, ASN1_PASS_DATA|ASN1_SET_OF,0,1},
 {ASN1_UNIVERSAL, ASN1_SEQUENCE_TAG, ASN1_PASS_DATA,0,1},
 {ASN1_CONTEXT_SPECIFIC, 0, ASN1_IMPLICIT, 1,0},
 {ASN1_UNIVERSAL,ASN1_SET_TAG,ASN1_SET_OF,0,1},
 {ASN1_CONTEXT_SPECIFIC, 1, ASN1_PASS_DATA, 1,1},
 {ASN1_UNIVERSAL,ASN1_SET_TAG,ASN1_SET_OF|ASN1_SIGNERINFO ,0,1}};

const ASN1_INFO asn1_info_PKCS7_SignerInfo[asn1_info_PKCS7_SignerInfo_num]={
 {ASN1_UNIVERSAL,ASN1_SEQUENCE_TAG,0,0,0},
 {ASN1_UNIVERSAL,ASN1_INTEGER_TAG,0,0,1},
 {ASN1_UNIVERSAL,ASN1_SEQUENCE_TAG,0,0,0},
 {ASN1_UNIVERSAL,0,ASN1_NAME,1},
 {ASN1_UNIVERSAL,ASN1_INTEGER_TAG,0,0,1},
 {ASN1_UNIVERSAL,ASN1_SEQUENCE_TAG,0,0,0},
 {ASN1_UNIVERSAL,ASN1_OBJECT_TAG,0,0,1},
 {ASN1_UNIVERSAL,ASN1_ANY,0,1,1},
 {ASN1_CONTEXT_SPECIFIC, 0, ASN1_PASS_DATA|ASN1_IMPLICIT, 1,1},
 {ASN1_UNIVERSAL,ASN1_SEQUENCE_TAG,0,0,0},
 {ASN1_UNIVERSAL,ASN1_OBJECT_TAG,0,0,1},
 {ASN1_UNIVERSAL,ASN1_ANY,0,1,1},
 {ASN1_UNIVERSAL,ASN1_OCTET_STRING_TAG,0,0,1},
 {ASN1_CONTEXT_SPECIFIC, 1, ASN1_PASS_DATA|ASN1_IMPLICIT, 1,1}};


const int free_PKCS7[free_PKCS7_num]={
	FREE_OBJECT_TAG, FREE_STRING_TAG, FREE_PASS_TAG,FREE_PASS_TAG, 
	FREE_X509_STACK_TAG,FREE_PASS_TAG,FREE_SIGNERINFO_STACK_TAG};

typedef struct PKCS7_IssuerAndSerialNumber
{
	X509_NAME_st issuer;
	ASN1_INTEGER serialNumber;
}ISSUERANDSERIALNUMBER;

typedef struct PKCS7_AlgorithmIdentifier
{
	ASN1_OBJECT algorithm;
	ASN1_ANY_ST   parameters;
}ALGORITHMIDENTIFIER;

typedef struct PKCS7_SignerInfo_st
{
	ASN1_INTEGER version;
	ISSUERANDSERIALNUMBER issuerAndSerialNumber;
	X509_ALGOR digestAlgorithm;
	STACK *authenticatedAttributes;
	X509_ALGOR digestEncryptionAlgorithm;
	ASN1_OCTET_STRING encryptedDigest;
	STACK *unauthenticatedAttributes;	
}PKCS7_SIGNERINFO;

typedef struct PKCS7_SignedData_st
{
	ASN1_INTEGER version;
	STACK *digestAlgorithms;
	struct pkcs7_st *contentInfo;
	STACK *certificates;
	STACK *crls;
	STACK *signerInfos;
}PKCS7_SignedData;

typedef struct pkcs7_st
{
	ASN1_OBJECT ContentType;
	union{
		ASN1_OCTET_STRING *data;
		PKCS7_SignedData *SignedData;
	}content;
}PKCS7_st;

#endif
