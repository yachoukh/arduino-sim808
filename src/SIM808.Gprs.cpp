#include "SIM808.h"

SIM808_COMMAND(SET_BEARER_SETTING, "AT+SAPBR=3,1,\"%S\",\"%s\"");
SIM808_COMMAND(BEARER_CLOSE, "AT+SAPBR=0,1");
SIM808_COMMAND(BEARER_OPEN, "AT+SAPBR=1,1");
SIM808_COMMAND(GPRS_START_TASK, "AT+CSTT=\"%s\"");
SIM808_COMMAND(GET_NETWORK_REGISTRATION, "AT+CGREG?");
SIM808_COMMAND(GPRS_DISABLE_CONTEXT, "AT+CIPSHUT");
SIM808_COMMAND(GET_GPRS_STATE, "AT+CGATT?");
SIM808_COMMAND(GPRS_ATTACH, "AT+CGATT=1");
SIM808_COMMAND(GPRS_DETACH, "AT+CGATT=0");
SIM808_COMMAND(CIICR, "AT+CIICR");

SIM808_COMMAND_PARAMETER(BEARER, CONTYPE);
SIM808_COMMAND_PARAMETER(BEARER, APN);
SIM808_COMMAND_PARAMETER(BEARER, USER);
SIM808_COMMAND_PARAMETER(BEARER, PWD);

SIM808_TOKEN_COMPLEX(SHUT_OK, "SHUT OK");

const char SIM808_COMMAND_STRING_PARAMETER[] PROGMEM = ",\"%s\"";
const char SIM808_COMMAND_GET_NETWORK_REGISTRATION_RESPONSE[] PROGMEM = "+CGREG:";
const char SIM808_COMMAND_GET_GPRS_STATE_RESPONSE[] PROGMEM = "+CGATT:";

bool SIM808::setBearerSetting(const __FlashStringHelper* parameter, const char* value)
{
	SENDARROW;	
	_output.verbose(PSTRPTR(SIM808_COMMAND_SET_BEARER_SETTING), parameter, value);
	return sendAssertResponse(PSTRPTR(SIM808_TOKEN_OK));
}

bool SIM808::getGprsPowerState(bool *state)
{
	uint8_t result;
	SENDARROW;
	_output.verbose(PSTRPTR(SIM808_COMMAND_GET_GPRS_STATE));

	send();
	readLine();
	if (strstr_P(replyBuffer, SIM808_COMMAND_GET_GPRS_STATE_RESPONSE) == 0) return false;

	if (!parseReply(',', 0, &result)) return false;

	*state = result;

	readLine();
	return assertResponse(PSTRPTR(SIM808_TOKEN_OK));
}

bool SIM808::enableGprs(const char *apn)
{
	return enableGprs(apn, NULL, NULL);
}

bool SIM808::enableGprs(const char *apn, const char* user, const char *password)
{
	bool success = sendAssertResponse(PSTRPTR(SIM808_COMMAND_GPRS_DISABLE_CONTEXT), PSTRPTR(SIM808_TOKEN_SHUT_OK), 65000) &&
		sendAssertResponse(PSTRPTR(SIM808_COMMAND_GPRS_ATTACH), PSTRPTR(SIM808_TOKEN_OK), 10000) &&
		setBearerSetting(PSTRPTR(SIM808_COMMAND_PARAMETER_BEARER_CONTYPE), "GPRS") &&
		setBearerSetting(PSTRPTR(SIM808_COMMAND_PARAMETER_BEARER_APN), apn) &&
		(user == NULL || setBearerSetting(PSTRPTR(SIM808_COMMAND_PARAMETER_BEARER_USER), user)) &&
		(password == NULL || setBearerSetting(PSTRPTR(SIM808_COMMAND_PARAMETER_BEARER_PWD), password));

	if (!success) return false;

	SENDARROW;
	_output.verbose(PSTRPTR(SIM808_COMMAND_GPRS_START_TASK), apn);

	if (user) {
		_output.verbose(PSTRPTR(SIM808_COMMAND_STRING_PARAMETER), user);
	}

	if (password) {
		_output.verbose(PSTRPTR(SIM808_COMMAND_STRING_PARAMETER), password);
	}

	if (!sendAssertResponse(PSTRPTR(SIM808_TOKEN_OK))) return false;

	return sendAssertResponse(PSTRPTR(SIM808_COMMAND_BEARER_OPEN), PSTRPTR(SIM808_TOKEN_OK), 65000) &&
		sendAssertResponse(PSTRPTR(SIM808_COMMAND_CIICR), PSTRPTR(SIM808_TOKEN_OK), 65000);

}

bool SIM808::disableGprs()
{
	sendAssertResponse(PSTRPTR(SIM808_COMMAND_GPRS_DISABLE_CONTEXT), PSTRPTR(SIM808_TOKEN_SHUT_OK), 65000);
	sendAssertResponse(PSTRPTR(SIM808_COMMAND_BEARER_CLOSE), PSTRPTR(SIM808_TOKEN_OK), 65000);
	sendAssertResponse(PSTRPTR(SIM808_COMMAND_GPRS_DETACH), PSTRPTR(SIM808_TOKEN_OK), 10000);

	return true;
}

SIM808RegistrationStatus SIM808::getNetworkRegistrationStatus()
{
	uint8_t n;
	uint8_t stat;
	SIM808RegistrationStatus result = { -1, SIM808_NETWORK_REGISTRATION_STATE::ERROR };

	SENDARROW;
	_output.verbose(PSTRPTR(SIM808_COMMAND_GET_NETWORK_REGISTRATION));

	send();
	readLine();
	
	if (strstr_P(replyBuffer, SIM808_COMMAND_GET_NETWORK_REGISTRATION_RESPONSE) == 0) return result;

	if (!parseReply(',', (uint8_t)SIM808_REGISTRATION_STATUS_RESPONSE::N, &n) ||
		!parseReply(',', (uint8_t)SIM808_REGISTRATION_STATUS_RESPONSE::STAT, &stat)) return result;

	readLine();
	if (!assertResponse(PSTRPTR(SIM808_TOKEN_OK))) return result;

	result.n = n;
	result.stat = (SIM808_NETWORK_REGISTRATION_STATE)stat;

	return result;
}
