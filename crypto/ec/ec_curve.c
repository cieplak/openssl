/* crypto/ec/ec_curve.c */
/* ====================================================================
 * Copyright (c) 1998-2002 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

#include "ec_lcl.h"
#include <openssl/err.h>
#include <openssl/obj_mac.h>
#include <openssl/asn1.h>
#include <openssl/asn1t.h>

EC_GROUP *EC_GROUP_nid2group(const int nid)
	{
	switch(nid)
		{
	case NID_X9_62_prime192v1:
		return EC_GROUP_get_group_by_name(EC_GROUP_X9_62_PRIME_192V1);
	case NID_X9_62_prime192v2:
		return EC_GROUP_get_group_by_name(EC_GROUP_X9_62_PRIME_192V2);
	case NID_X9_62_prime192v3:
		return EC_GROUP_get_group_by_name(EC_GROUP_X9_62_PRIME_192V3);
	case NID_X9_62_prime239v1:
		return EC_GROUP_get_group_by_name(EC_GROUP_X9_62_PRIME_239V1);
	case NID_X9_62_prime239v2:
		return EC_GROUP_get_group_by_name(EC_GROUP_X9_62_PRIME_239V2);
	case NID_X9_62_prime239v3:
		return EC_GROUP_get_group_by_name(EC_GROUP_X9_62_PRIME_239V3);
	case NID_X9_62_prime256v1:
		return EC_GROUP_get_group_by_name(EC_GROUP_X9_62_PRIME_256V1);
		}
	ECerr(EC_F_EC_GROUP_NID2GROUP, EC_R_UNKNOWN_NID);
	return NULL;
	}

int EC_GROUP_group2nid(const EC_GROUP *group)
	{
	return EC_GROUP_get_nid(group);

#if 0	
/* TODO: a real compare function for EC_GROUPs */
#define EC_GROUP_cmp(a,b) ((a) != (b))

	if (group == NULL)
		{
		ECerr(EC_F_EC_GROUP_GROUP2NID, EC_R_MISSING_PARAMETERS);
		return 0;
		}
	if (!EC_GROUP_cmp(group, EC_GROUP_GET_X9_62_192V1_GROUP()))
		return NID_X9_62_prime192v1;
	else if (!EC_GROUP_cmp(group, EC_GROUP_get_x9_62_192v2_group()))
		return NID_X9_62_prime192v2;
	else if (!EC_GROUP_cmp(group, EC_GROUP_get_x9_62_192v3_group()))
		return NID_X9_62_prime192v3;
	else if (!EC_GROUP_cmp(group, EC_GROUP_get_x9_62_239v1_group()))
		return NID_X9_62_prime239v1;
	else if (!EC_GROUP_cmp(group, EC_GROUP_get_x9_62_239v2_group()))
		return NID_X9_62_prime239v2;
	else if (!EC_GROUP_cmp(group, EC_GROUP_get_x9_62_239v3_group()))
		return NID_X9_62_prime239v3;
	else if (!EC_GROUP_cmp(group, EC_GROUP_get_x9_62_256v1_group()))
		return NID_X9_62_prime256v1;
	ECerr(EC_F_EC_GROUP_GROUP2NID, EC_R_UNKNOWN_GROUP);
	return 0;
#endif
	}

static EC_GROUP *ec_group_set_prime_group(const char *prime_in,
	    const char *a_in, const char *b_in,
	    const char *x_in, const int y_bit, const char *order_in)
	{
	EC_GROUP *group=NULL;
	EC_POINT *P=NULL;
	BN_CTX	 *ctx=NULL;
	BIGNUM 	 *prime=NULL,*a=NULL,*b=NULL,*x=NULL,*order=NULL;
	int	 ok=0;

	if ((ctx = BN_CTX_new()) == NULL) goto bn_err;
	if ((prime = BN_new()) == NULL || (a = BN_new()) == NULL || (b = BN_new()) == NULL ||
		(x = BN_new()) == NULL || (order = BN_new()) == NULL) goto bn_err;
	
	if (!BN_hex2bn(&prime, prime_in)) goto bn_err;
	if (!BN_hex2bn(&a, a_in)) goto bn_err;
	if (!BN_hex2bn(&b, b_in)) goto bn_err;

	if ((group = EC_GROUP_new_curve_GFp(prime, a, b, ctx)) == NULL) goto err;
	if ((P = EC_POINT_new(group)) == NULL) goto err;
	
	if (!BN_hex2bn(&x, x_in)) goto bn_err;
	if (!EC_POINT_set_compressed_coordinates_GFp(group, P, x, y_bit, ctx)) goto err;
	if (!BN_hex2bn(&order, order_in)) goto bn_err;
	if (!EC_GROUP_set_generator(group, P, order, BN_value_one())) goto err;
	ok=1;
bn_err:
	if (!ok)
		ECerr(EC_F_EC_GROUP_SET_PRIME_GROUP, ERR_R_BN_LIB);
err:
	if (!ok)
		{
		EC_GROUP_free(group);
		group = NULL;
		}
	if (P) 	   EC_POINT_free(P);
	if (ctx)   BN_CTX_free(ctx);
	if (prime) BN_free(prime);
	if (a)     BN_free(a);
	if (b)     BN_free(b);
	if (order) BN_free(order);
	if (x)     BN_free(x);
	return(group);
}

EC_GROUP *EC_GROUP_get_group_by_name(int name)
	{
	EC_GROUP *ret = NULL;
	switch (name)
		{
	case EC_GROUP_NO_CURVE:
		return NULL;

	case EC_GROUP_NIST_PRIME_224:
		return ec_group_set_prime_group(
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000001",
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFE",
			"B4050A850C04B3ABF54132565044B0B7D7BFD8BA270B39432355FFB4",
			"B70E0CBD6BB4BF7F321390B94A03C1D356C21122343280D6115C1D21",0,
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFF16A2E0B8F03E13DD29455C5C2A3D");

	case EC_GROUP_NIST_PRIME_384:
		return ec_group_set_prime_group(
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFF0000000000000000FFFFFFFF",
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFF0000000000000000FFFFFFFC",
			"B3312FA7E23EE7E4988E056BE3F82D19181D9C6EFE8141120314088F5013875AC656398D8A2ED19D2A85C8EDD3EC2AEF",
			"AA87CA22BE8B05378EB1C71EF320AD746E1D3B628BA79B9859F741E082542A385502F25DBF55296C3A545E3872760AB7",1,
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC7634D81F4372DDF581A0DB248B0A77AECEC196ACCC52973");

	case EC_GROUP_NIST_PRIME_521:
		return ec_group_set_prime_group(
			"1FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
			"1FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC",
			"051953EB9618E1C9A1F929A21A0B68540EEA2DA725B99B"
			"315F3B8B489918EF109E156193951EC7E937B1652C0BD3BB1BF073573DF883D2C34F1EF451FD46B503F00",
			"C6858E06B70404E9CD9E3ECB662395B4429C648139053F"
			"B521F828AF606B4D3DBAA14B5E77EFE75928FE1DC127A2FFA8DE3348B3C1856A429BF97E7E31C2E5BD66",0,
			"1FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
			"FFFFFFFFFFFFFFFFFFFFA51868783BF2F966B7FCC0148F709A5D03BB5C9B8899C47AEBB6FB71E91386409");

	case EC_GROUP_NIST_PRIME_192:
	case EC_GROUP_X9_62_PRIME_192V1:
		ret = ec_group_set_prime_group(
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFF",
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFC",
			"64210519E59C80E70FA7E9AB72243049FEB8DEECC146B9B1",
			"188DA80EB03090F67CBF20EB43A18800F4FF0AFD82FF1012",1,
			"FFFFFFFFFFFFFFFFFFFFFFFF99DEF836146BC9B1B4D22831");
		EC_GROUP_set_nid(ret, NID_X9_62_prime192v1);
		return ret;

	case EC_GROUP_X9_62_PRIME_192V2:
		ret = ec_group_set_prime_group(
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFF",
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFC",
			"CC22D6DFB95C6B25E49C0D6364A4E5980C393AA21668D953",
			"EEA2BAE7E1497842F2DE7769CFE9C989C072AD696F48034A",1,
			"FFFFFFFFFFFFFFFFFFFFFFFE5FB1A724DC80418648D8DD31");
		EC_GROUP_set_nid(ret, NID_X9_62_prime192v2);
		return ret;

	case EC_GROUP_X9_62_PRIME_192V3:
		ret = ec_group_set_prime_group(
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFF",
			"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFC",
			"22123DC2395A05CAA7423DAECCC94760A7D462256BD56916",
			"7D29778100C65A1DA1783716588DCE2B8B4AEE8E228F1896",0,
			"FFFFFFFFFFFFFFFFFFFFFFFF7A62D031C83F4294F640EC13");
		EC_GROUP_set_nid(ret, NID_X9_62_prime192v3);
		return ret;

	case EC_GROUP_X9_62_PRIME_239V1:
		ret = ec_group_set_prime_group(
			"7FFFFFFFFFFFFFFFFFFFFFFF7FFFFFFFFFFF8000000000007FFFFFFFFFFF",
			"7FFFFFFFFFFFFFFFFFFFFFFF7FFFFFFFFFFF8000000000007FFFFFFFFFFC",
			"6B016C3BDCF18941D0D654921475CA71A9DB2FB27D1D37796185C2942C0A",
			"0FFA963CDCA8816CCC33B8642BEDF905C3D358573D3F27FBBD3B3CB9AAAF",0,
			"7FFFFFFFFFFFFFFFFFFFFFFF7FFFFF9E5E9A9F5D9071FBD1522688909D0B");
		EC_GROUP_set_nid(ret, NID_X9_62_prime239v1);
		return ret;

	case EC_GROUP_X9_62_PRIME_239V2:
		ret = ec_group_set_prime_group(
			"7FFFFFFFFFFFFFFFFFFFFFFF7FFFFFFFFFFF8000000000007FFFFFFFFFFF",
			"7FFFFFFFFFFFFFFFFFFFFFFF7FFFFFFFFFFF8000000000007FFFFFFFFFFC",
			"617FAB6832576CBBFED50D99F0249C3FEE58B94BA0038C7AE84C8C832F2C",
			"38AF09D98727705120C921BB5E9E26296A3CDCF2F35757A0EAFD87B830E7",0,
			"7FFFFFFFFFFFFFFFFFFFFFFF800000CFA7E8594377D414C03821BC582063");
		EC_GROUP_set_nid(ret, NID_X9_62_prime239v2);
		return ret;

	case EC_GROUP_X9_62_PRIME_239V3:
		ret = ec_group_set_prime_group(
			"7FFFFFFFFFFFFFFFFFFFFFFF7FFFFFFFFFFF8000000000007FFFFFFFFFFF",
			"7FFFFFFFFFFFFFFFFFFFFFFF7FFFFFFFFFFF8000000000007FFFFFFFFFFC",
			"255705FA2A306654B1F4CB03D6A750A30C250102D4988717D9BA15AB6D3E",
			"6768AE8E18BB92CFCF005C949AA2C6D94853D0E660BBF854B1C9505FE95A",1,
			"7FFFFFFFFFFFFFFFFFFFFFFF7FFFFF975DEB41B3A6057C3C432146526551");
		EC_GROUP_set_nid(ret, NID_X9_62_prime239v3);
		return ret;

	case EC_GROUP_NIST_PRIME_256:
	case EC_GROUP_X9_62_PRIME_256V1:
		ret = ec_group_set_prime_group(
			"FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF",
			"FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFC",
			"5AC635D8AA3A93E7B3EBBD55769886BC651D06B0CC53B0F63BCE3C3E27D2604B",
			"6B17D1F2E12C4247F8BCE6E563A440F277037D812DEB33A0F4A13945D898C296",1,
			"FFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551");
		EC_GROUP_set_nid(ret, NID_X9_62_prime256v1);
		return ret;
	}

	ECerr(EC_F_EC_GROUP_GET_GROUP_BY_NAME, EC_R_UNKNOWN_GROUP);
	return NULL;
}
