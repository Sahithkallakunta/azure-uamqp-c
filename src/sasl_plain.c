// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <string.h>
#include "sasl_plain.h"
#include "amqpalloc.h"

typedef struct SASL_PLAIN_INSTANCE_TAG
{
	unsigned char* init_bytes;
	size_t init_bytes_length;
} SASL_PLAIN_INSTANCE;

static const SASL_MECHANISM_INTERFACE_DESCRIPTION saslplain_interface =
{
	saslplain_create,
	saslplain_destroy,
	saslplain_get_init_bytes,
	saslplain_get_mechanism_name,
	saslplain_challenge
};

CONCRETE_SASL_MECHANISM_HANDLE saslplain_create(void* config)
{
	SASL_PLAIN_INSTANCE* result;

	if (config == NULL)
	{
		/* Codes_SRS_SASL_PLAIN_01_003: [If the config argument is NULL, then saslplain_create shall fail and return NULL.] */
		result = NULL;
	}
	else
	{
		SASL_PLAIN_CONFIG* sasl_plain_config = (SASL_PLAIN_CONFIG*)config;

		/* Codes_SRS_SASL_PLAIN_01_004: [If either the authcid or passwd member of the config structure is NULL, then saslplain_create shall fail and return NULL.] */
		if ((sasl_plain_config->authcid == NULL) ||
			(sasl_plain_config->passwd == NULL))
		{
			result = NULL;
		}
		else
		{
			/* Codes_SRS_SASL_PLAIN_01_001: [saslplain_create shall return on success a non-NULL handle to a new SASL plain mechanism.] */
			result = amqpalloc_malloc(sizeof(SASL_PLAIN_INSTANCE));
			/* Codes_SRS_SASL_PLAIN_01_002: [If allocating the memory needed for the saslplain instance fails then saslplain_create shall return NULL.] */
			if (result != NULL)
			{
				size_t authzid_length = sasl_plain_config->authzid == NULL ? 0 : strlen(sasl_plain_config->authcid);
				size_t authcid_length = strlen(sasl_plain_config->authcid);
				size_t passwd_length = strlen(sasl_plain_config->passwd);

				/* Ignore UTF8 for now */
				result->init_bytes = (unsigned char*)amqpalloc_malloc(authzid_length + authcid_length + passwd_length + 2);
				if (result->init_bytes == NULL)
				{
					/* Codes_SRS_SASL_PLAIN_01_002: [If allocating the memory needed for the saslplain instance fails then saslplain_create shall return NULL.] */
					amqpalloc_free(result);
					result = NULL;
				}
				else
				{
					result->init_bytes[0] = 0;
					(void)memcpy(result->init_bytes + 1, sasl_plain_config->authcid, authcid_length);
					result->init_bytes[authcid_length + 1] = 0;
					(void)memcpy(result->init_bytes + authcid_length + 2, sasl_plain_config->passwd, passwd_length);
					result->init_bytes_length = authcid_length + passwd_length + 2;
				}
			}
		}
	}

	return result;
}

void saslplain_destroy(CONCRETE_SASL_MECHANISM_HANDLE sasl_mechanism_concrete_handle)
{
	if (sasl_mechanism_concrete_handle != NULL)
	{
		SASL_PLAIN_INSTANCE* sasl_plain_instance = (SASL_PLAIN_INSTANCE*)sasl_mechanism_concrete_handle;
		if (sasl_plain_instance->init_bytes != NULL)
		{
			amqpalloc_free(sasl_plain_instance->init_bytes);
		}

		amqpalloc_free(sasl_plain_instance);
	}
}

int saslplain_get_init_bytes(CONCRETE_SASL_MECHANISM_HANDLE sasl_mechanism_concrete_handle, SASL_MECHANISM_BYTES* init_bytes)
{
	int result;

	if (sasl_mechanism_concrete_handle == NULL)
	{
		result = __LINE__;
	}
	else
	{
		SASL_PLAIN_INSTANCE* sasl_plain_instance = (SASL_PLAIN_INSTANCE*)sasl_mechanism_concrete_handle;

		init_bytes->bytes = sasl_plain_instance->init_bytes;
		init_bytes->length = sasl_plain_instance->init_bytes_length;

		result = 0;
	}

	return result;
}

const char* saslplain_get_mechanism_name(CONCRETE_SASL_MECHANISM_HANDLE sasl_mechanism)
{
	const char* result;

	if (sasl_mechanism == NULL)
	{
		result = NULL;
	}
	else
	{
		result = "PLAIN";
	}

	return result;
}

int saslplain_challenge(CONCRETE_SASL_MECHANISM_HANDLE concrete_sasl_mechanism, const SASL_MECHANISM_BYTES* challenge_bytes, SASL_MECHANISM_BYTES* response_bytes)
{
	return 0;
}

const SASL_MECHANISM_INTERFACE_DESCRIPTION* saslplain_get_interface(void)
{
	return &saslplain_interface;
}
