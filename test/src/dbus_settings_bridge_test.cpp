#include <gtest/gtest.h>
#include "dbus_settings.h"
#include "dbus_settings_bridge.h"
#include "dbus_settings_bridge_test.h"
#include "settings.h"
#include "test_helper.h"

void checkValue(const QVariant &expected, const QVariant &actual)
{
	EXPECT_EQ(expected.isValid(), actual.isValid());
	EXPECT_EQ(expected, actual);
}

TEST(DBusSettingsBridgeTest2, addObjects)
{
	DBusSettings settingsClient;
	checkValue(QVariant(), settingsClient.getValue("/Settings/Fronius/AutoDetect"));
	DBusSettingsBridge::addDBusObjects();
	checkValue(QVariant(false), settingsClient.getValue("/Settings/Fronius/AutoDetect"));
	checkValue(QVariant(""), settingsClient.getValue("/Settings/Fronius/IPAddresses"));
	checkValue(QVariant(""), settingsClient.getValue("/Settings/Fronius/KnownIPAddresses"));
	checkValue(QVariant(""), settingsClient.getValue("/Settings/Fronius/ScanProgress"));
}

TEST_F(DBusSettingsBridgeTest, changeAutoDetect)
{
	// Check default value from DBus
	EXPECT_FALSE(mSettings->autoDetect());
	mSettingsClient->resetChangedPaths();
	mSettings->setAutoDetect(true);
	qWait(100);
	// Check new value
	QVariant autoDetect = mSettingsClient->getValue("/Settings/Fronius/AutoDetect");
	EXPECT_TRUE(autoDetect.isValid());
	EXPECT_TRUE(autoDetect.toBool());
	// Check DBus signal
	const QList<QString> &cp = mSettingsClient->changedPaths();
	ASSERT_EQ(1, cp.size());
	EXPECT_EQ(QString("/Settings/Fronius/AutoDetect"), cp.first());
}

TEST_F(DBusSettingsBridgeTest, changeAutoDetectRemote)
{
	EXPECT_FALSE(mSettings->autoDetect());
	// Set new value on DBus
	mSettingsClient->setValue("/Settings/Fronius/AutoDetect", true);
	// Allow value form DBus to trickle to our settings object
	qWait(100);
	EXPECT_TRUE(mSettings->autoDetect());
}

TEST_F(DBusSettingsBridgeTest, changeIPAddresses)
{
	changeIPAddresses("ipAddresses", "/Settings/Fronius/IPAddresses");
}

TEST_F(DBusSettingsBridgeTest, changeIPAddressesRemote)
{
	changeIPAddressesRemote("ipAddresses", "/Settings/Fronius/IPAddresses");
}

TEST_F(DBusSettingsBridgeTest, changeKnownIPAddresses)
{
	changeIPAddresses("knownIpAddresses", "/Settings/Fronius/KnownIPAddresses");
}

TEST_F(DBusSettingsBridgeTest, changeKnownIPAddressesRemote)
{
	changeIPAddressesRemote("knownIpAddresses", "/Settings/Fronius/KnownIPAddresses");
}

void DBusSettingsBridgeTest::SetUp()
{
	mSettingsClient.reset(new DBusSettings());
	DBusSettingsBridge::addDBusObjects();
	mSettings.reset(new Settings());
	mBridge.reset(new DBusSettingsBridge(mSettings.data(), 0));
	// Wait for DBusSettingsBridge to fill settings object with default values
	// specified by DBusSettingsBridge::addDBusObjects
	qWait(300);
}

void DBusSettingsBridgeTest::TearDown()
{
	mSettings.reset();
	mBridge.reset();
	mSettingsClient.reset();
}

void DBusSettingsBridgeTest::changeIPAddresses(const char *property, const char *path)
{
	// Check default value from DBus
	EXPECT_TRUE(mSettings->property(property).value<QList<QHostAddress> >().isEmpty());
	mSettingsClient->resetChangedPaths();
	QList<QHostAddress> newValues;
	newValues.append(QHostAddress("192.168.1.3"));
	newValues.append(QHostAddress("192.168.1.7"));
	mSettings->setProperty(property, QVariant::fromValue(newValues));
	qWait(100);
	// Check new value
	QVariant qv = mSettingsClient->getValue(path);
	EXPECT_EQ(QString("192.168.1.3,192.168.1.7"), qv.toString());
	// Check DBus signal
	const QList<QString> &cp = mSettingsClient->changedPaths();
	ASSERT_EQ(1, cp.size());
	EXPECT_EQ(QString(path), cp.first());
}

void DBusSettingsBridgeTest::changeIPAddressesRemote(const char *property, const char *path)
{
	EXPECT_TRUE(mSettings->property(property).value<QList<QHostAddress> >().isEmpty());
	// Set new value on DBus
	mSettingsClient->setValue(path, "192.168.1.3,192.168.1.7");
	// Allow value form DBus to trickle to our settings object
	qWait(100);
	QList<QHostAddress> addr = mSettings->property(property).
			value<QList<QHostAddress> >();
	QList<QHostAddress> expected;
	expected.append(QHostAddress("192.168.1.3"));
	expected.append(QHostAddress("192.168.1.7"));
	// Check DBus signal
	EXPECT_EQ(expected, addr);
}
