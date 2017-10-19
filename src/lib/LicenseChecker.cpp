#include "LicenseChecker.h"

#include "utility/AppPath.h"
#include "utility/logging/logging.h"
#include "utility/utilityApp.h"
#include "utility/messaging/type/MessageForceEnterLicense.h"

#include "License.h"
#include "PublicKey.h"
#include "settings/ApplicationSettings.h"

void LicenseChecker::createInstance()
{
	if (!s_instance)
	{
		s_instance = std::shared_ptr<LicenseChecker>(new LicenseChecker());
	}
}

std::shared_ptr<LicenseChecker> LicenseChecker::getInstance()
{
	createInstance();

	return s_instance;
}

LicenseChecker::~LicenseChecker()
{
}

std::string LicenseChecker::getCurrentLicenseString() const
{
	License license;
	bool isLoaded = license.loadFromEncodedString(
		ApplicationSettings::getInstance()->getLicenseString(), AppPath::getAppPath());

	if (isLoaded)
	{
		return license.getLicenseString();
	}

	return "";
}

void LicenseChecker::saveCurrentLicenseString(const std::string& licenseString) const
{
	License license;
	bool isLoaded = license.loadFromString(licenseString);
	if (!isLoaded)
	{
		return;
	}

	utility::saveLicense(&license);
}

bool LicenseChecker::isCurrentLicenseValid()
{
	return checkCurrentLicense() == LICENSE_VALID;
}

LicenseChecker::LicenseState LicenseChecker::checkCurrentLicense() const
{
	ApplicationSettings* appSettings = ApplicationSettings::getInstance().get();
	if (appSettings == NULL)
	{
		LOG_ERROR_STREAM(<< "Unable to retrieve app settings");
		return LICENSE_EMPTY;
	}

	std::string licenseCheck = appSettings->getLicenseCheck();
	std::string appPath(AppPath::getAppPath());

	if (appPath.size() <= 0)
	{
		LOG_ERROR_STREAM(<< "Failed to retrieve app path");
		return LICENSE_EMPTY;
	}

	std::string licenseString = appSettings->getLicenseString();
	if (licenseString.size() == 0)
	{
		return LICENSE_EMPTY;
	}

	if (!License::checkLocation(FilePath(appPath).absolute().str(), licenseCheck))
	{
		return LICENSE_MOVED;
	}

	License license;
	bool isLoaded = license.loadFromEncodedString(licenseString, appPath);
	if (!isLoaded)
	{
		return LICENSE_MOVED;
	}

	return checkLicense(license);
}

LicenseChecker::LicenseState LicenseChecker::checkLicenseString(const std::string& licenseString) const
{
	if (licenseString.size() == 0)
	{
		return LICENSE_EMPTY;
	}

	License license;
	bool isLoaded = license.loadFromString(licenseString);
	if (!isLoaded)
	{
		return LICENSE_MALFORMED;
	}

	license.print();

	return checkLicense(license);
}

MessageEnteredLicense::LicenseType LicenseChecker::getCurrentLicenseType() const
{
	License license;
	bool isLoaded = license.loadFromEncodedString(
		ApplicationSettings::getInstance()->getLicenseString(), AppPath::getAppPath());

	if (!isLoaded)
	{
		return MessageEnteredLicense::LICENSE_NONE;
	}

	LicenseState state = checkLicense(license);
	if (state != LICENSE_VALID)
	{
		return MessageEnteredLicense::LICENSE_NONE;
	}
	else if (license.isTestLicense())
	{
		return MessageEnteredLicense::LICENSE_TEST;
	}
	else if (license.isNonCommercialLicenseType())
	{
		return MessageEnteredLicense::LICENSE_NON_COMMERCIAL;
	}

	return MessageEnteredLicense::LICENSE_COMMERCIAL;
}

MessageEnteredLicense::LicenseType LicenseChecker::getLicenseType(const std::string& licenseString) const
{
	if (licenseString.size() == 0)
	{
		return MessageEnteredLicense::LICENSE_NONE;
	}

	License license;
	bool isLoaded = license.loadFromString(licenseString);
	if (!isLoaded)
	{
		return MessageEnteredLicense::LICENSE_NONE;
	}

	LicenseState state = checkLicense(license);
	if (state != LICENSE_VALID)
	{
		return MessageEnteredLicense::LICENSE_NONE;
	}
	else if (license.isTestLicense())
	{
		return MessageEnteredLicense::LICENSE_TEST;
	}
	else if (license.isNonCommercialLicenseType())
	{
		return MessageEnteredLicense::LICENSE_NON_COMMERCIAL;
	}

	return MessageEnteredLicense::LICENSE_COMMERCIAL;
}

LicenseChecker::LicenseChecker()
{
}

LicenseChecker::LicenseState LicenseChecker::checkLicense(License& license) const
{
	if (license.isExpired())
	{
		return LICENSE_EXPIRED;
	}

	if (license.isValid())
	{
		return LICENSE_VALID;
	}

	return LICENSE_INVALID;
}

std::shared_ptr<LicenseChecker> LicenseChecker::s_instance;
