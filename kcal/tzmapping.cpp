/*
  Copyright (c) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
/**
  @file
  This file is part of the API for handling calendar data and provides
  static methods for mapping Windows timezone names to/from UTC offsets
  or to/from Olson zoneinfo names.

  @brief
  Provides methods to map timezone names in various ways.

  author Allen Winter \<allen@kdab.com\>
*/
#include "tzmapping.h"

#include <KDebug>
#include <KLocale>

using namespace KCal;

QString TZMaps::winZoneStandardToDisplay( const QString &standardName )
{
  static QHash<QString,QString> standardToDisplay;

  if ( standardToDisplay.isEmpty() ) {
    standardToDisplay["Dateline Standard Time"] =
      "International Date Line West"; //UTC-12

    standardToDisplay["Samoa Standard Time"] =
      "Midway Island, Samoa"; //UTC-11

    standardToDisplay["Hawaiian Standard Time"] =
      "Hawaii"; //UTC-10

    standardToDisplay["Alaskan Standard Time"] =
      "Alaska"; //UTC-9

    standardToDisplay["Pacific Standard Time (Mexico)"] =
      "Tijuana, Baja California"; //UTC-8

    standardToDisplay["Pacific Standard Time"] =
      "Pacific Time (US & Canada)"; //UTC-8

    standardToDisplay["Mountain Standard Time"] =
      "Mountain Time (US & Canada)"; //UTC-7

    standardToDisplay["Mountain Standard Time (Mexico)"] =
      "Chihuahua, La Paz, Mazatlan"; //UTC-7

    standardToDisplay["US Mountain Standard Time"] =
      "Arizona"; //UTC-7

    standardToDisplay["Canada Central Standard Time"] =
      "Saskatchewan"; //UTC-6

    standardToDisplay["Central Standard Time (Mexico)"] =
      "Guadalajara, Mexico City, Monterrey"; //UTC-6

    standardToDisplay["Central Standard Time"] =
      "Central Time (US & Canada)"; //UTC-6

    standardToDisplay["Central America Standard Time"] =
      "Central America"; //UTC-6

    standardToDisplay["US Eastern Standard Time"] =
      "Indiana (East)"; //UTC-5

    standardToDisplay["Eastern Standard Time"] =
      "Eastern Time (US & Canada)"; //UTC-5

    standardToDisplay["SA Pacific Standard Time"] =
      "Bogota, Lima, Quito, Rio Branco"; //UTC-5

    standardToDisplay["Venezuela Standard Time"] =
      "Caracas"; //UTC-4

    standardToDisplay["Pacific SA Standard Time"] =
      "Santiago"; //UTC-4

    standardToDisplay["Central Brazilian Standard Time"] =
      "Manaus"; //UTC-4

    standardToDisplay["SA Western Standard Time"] =
      "La Paz"; //UTC-4

    standardToDisplay["Atlantic Standard Time"] =
      "Atlantic Time (Canada)"; //UTC-4

    standardToDisplay["Newfoundland Standard Time"] =
      "Newfoundland"; //UTC-3

    standardToDisplay["Montevideo Standard Time"] =
      "Montevideo"; //UTC-3

    standardToDisplay["Greenland Standard Time"] =
      "Greenland"; //UTC-3

    standardToDisplay["SA Eastern Standard Time"] =
      "Georgetown"; //UTC-3

    standardToDisplay["Argentina Standard Time"] =
      "Buenos Aires"; //UTC-3

    standardToDisplay["E. South America Standard Time"] =
      "Brasilia"; //UTC-3

    standardToDisplay["Mid-Atlantic Standard Time"] =
      "Mid-Atlantic"; //UTC-2

    standardToDisplay["Cape Verde Standard Time"] =
      "Cape Verde Is."; //UTC-1

    standardToDisplay["Azores Standard Time"] =
      "Azores"; //UTC-1

    standardToDisplay["Tonga Standard Time"] =
      "Nuku'alofa"; //UTC+13

    standardToDisplay["Fiji Standard Time"] =
      "Fiji, Kamchatka, Marshall Is."; //UTC+12

    standardToDisplay["New Zealand Standard Time"] =
      "Auckland, Wellington"; //UTC+12

    standardToDisplay["Central Pacific Standard Time"] =
      "Magadan, Solomon Is., New Caledonia"; //UTC+11

    standardToDisplay["Vladivostok Standard Time"] =
      "Vladivostok"; //UTC+10

    standardToDisplay["Tasmania Standard Time"] =
      "Hobart"; //UTC+10

    standardToDisplay["West Pacific Standard Time"] =
      "Guam, Port Moresby"; //UTC+10

    standardToDisplay["AUS Eastern Standard Time"] =
      "Canberra, Melbourne, Sydney"; //UTC+10

    standardToDisplay["E. Australia Standard Time"] =
      "Brisbane"; //UTC+10

    standardToDisplay["AUS Central Standard Time"] =
      "Darwin"; //UTC+9

    standardToDisplay["Cen. Australia Standard Time"] =
      "Adelaide"; //UTC+9

    standardToDisplay["Yakutsk Standard Time"] =
      "Yakutsk"; //UTC+9

    standardToDisplay["Korea Standard Time"] =
      "Seoul"; //UTC+9

    standardToDisplay["Tokyo Standard Time"] =
      "Osaka, Sapporo, Tokyo"; //UTC+9

    standardToDisplay["Taipei Standard Time"] =
      "Taipei"; //UTC+8

    standardToDisplay["W. Australia Standard Time"] =
      "Perth"; //UTC+8

    standardToDisplay["Malay Peninsula Standard Time"] =
      "Kuala Lumpur, Singapore"; //UTC+8

    standardToDisplay["North Asia East Standard Time"] =
      "Irkutsk, Ulaan Bataar"; //UTC+8

    standardToDisplay["China Standard Time"] =
      "Beijing, Chongqing, Hong Kong, Urumqi"; //UTC+8

    standardToDisplay["North Asia Standard Time"] =
      "Krasnoyarsk"; //UTC+7

    standardToDisplay["SE Asia Standard Time"] =
      "Bangkok, Hanoi, Jakarta"; //UTC+7

    standardToDisplay["Myanmar Standard Time"] =
      "Yangon (Rangoon)"; //UTC+6

    standardToDisplay["Central Asia Standard Time"] =
      "Astana, Dhaka"; //UTC+6

    standardToDisplay["N. Central Asia Standard Time"] =
      "Almaty, Novosibirsk"; //UTC+6

    standardToDisplay["Nepal Standard Time"] =
      "Kathmandu"; //UTC+5

    standardToDisplay["Sri Lanka Standard Time"] =
      "Sri Jayawardenepura"; //UTC+5

    standardToDisplay["India Standard Time"] =
      "Chennai, Kolkata, Mumbai, New Delhi"; //UTC+5

    standardToDisplay["West Asia Standard Time"] =
      "Tashkent"; //UTC+5

    standardToDisplay["Pakistan Standard Time"] =
      "Islamabad, Karachi"; //UTC+5

    standardToDisplay["Ekaterinburg Standard Time"] =
      "Ekaterinburg"; //UTC+5

    standardToDisplay["Afghanistan Standard Time"] =
      "Kabul"; //UTC+4

    standardToDisplay["Caucasus Standard Time"] =
      "Yerevan"; //UTC+4

    standardToDisplay["Azerbaijan Standard Time"] =
      "Baku"; //UTC+4

    standardToDisplay["Arabian Standard Time"] =
      "Abu Dhabi, Muscat"; //UTC+4

    standardToDisplay["Iran Standard Time"] =
      "Tehran"; //UTC+3

    standardToDisplay["Georgian Standard Time"] =
      "Tbilisi"; //UTC+3

    standardToDisplay["E. Africa Standard Time"] =
      "Nairobi"; //UTC+3

    standardToDisplay["Russian Standard Time"] =
      "Moscow, St. Petersburg, Volgograd"; //UTC+3

    standardToDisplay["Arab Standard Time"] =
      "Kuwait, Riyadh"; //UTC+3

    standardToDisplay["Arabic Standard Time"] =
      "Baghdad"; //UTC+3

    standardToDisplay["Namibia Standard Time"] =
      "Windhoek"; //UTC+2

    standardToDisplay["E. Europe Standard Time"] =
      "Minsk"; //UTC+2

    standardToDisplay["Jerusalem Standard Time"] =
      "Jerusalem"; //UTC+2

    standardToDisplay["FLE Standard Time"] =
      "Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius"; //UTC+2

    standardToDisplay["South Africa Standard Time"] =
      "Harare, Pretoria"; //UTC+2

    standardToDisplay["Egypt Standard Time"] =
      "Cairo"; //UTC+2

    standardToDisplay["Middle East Standard Time"] =
      "Beirut"; //UTC+2

    standardToDisplay["GTB Standard Time"] =
      "Athens, Bucharest, Istanbul"; //UTC+2

    standardToDisplay["Jordan Standard Time"] =
      "Amman"; //UTC+2

    standardToDisplay["W. Central Africa Standard Time"] =
      "West Central Africa"; //UTC+1

    standardToDisplay["Central European Standard Time"] =
      "Sarajevo, Skopje, Warsaw, Zagreb"; //UTC+1

    standardToDisplay["Romance Standard Time"] =
      "Brussels, Copenhagen, Madrid, Paris"; //UTC+1

    standardToDisplay["Central Europe Standard Time"] =
      "Belgrade, Bratislava, Budapest, Ljubljana, Prague"; //UTC+1

    standardToDisplay["W. Europe Standard Time"] =
      "Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna"; //UTC+1

    standardToDisplay["Greenwich Standard Time"] =
      "Monrovia, Reykjavik"; //UTC

    standardToDisplay["GMT Standard Time"] =
      "Greenwich Mean Time : Dublin, Edinburgh, Lisbon, London"; //UTC

    standardToDisplay["Morocco Standard Time"] =
      "Casablanca"; //UTC
  }

  QString displayStr = standardToDisplay[standardName];
  if ( displayStr.isEmpty() ) {
    kWarning() << "Unknown/invalid standardName specified:" << standardName;
  }
  return displayStr;
}

QString TZMaps::winZoneDisplayToStandard( const QString &displayName )
{
  static QHash<QString,QString> displayToStandard;

  if ( displayToStandard.isEmpty() ) {
    displayToStandard["International Date Line West"] =
      "Dateline Standard Time"; //UTC-12

    displayToStandard["Midway Island, Samoa"] =
      "Samoa Standard Time"; //UTC-11

    displayToStandard["Hawaii"] =
      "Hawaiian Standard Time"; //UTC-10

    displayToStandard["Alaska"] =
      "Alaskan Standard Time"; //UTC-9

    displayToStandard["Tijuana, Baja California"] =
      "Pacific Standard Time (Mexico)"; //UTC-8

    displayToStandard["Pacific Time (US & Canada)"] =
      "Pacific Standard Time"; //UTC-8

    displayToStandard["Mountain Time (US & Canada)"] =
      "Mountain Standard Time"; //UTC-7

    displayToStandard["Chihuahua, La Paz, Mazatlan"] =
      "Mountain Standard Time (Mexico)"; //UTC-7

    displayToStandard["Arizona"] =
      "US Mountain Standard Time"; //UTC-7

    displayToStandard["Saskatchewan"] =
      "Canada Central Standard Time"; //UTC-6

    displayToStandard["Guadalajara, Mexico City, Monterrey"] =
      "Central Standard Time (Mexico)"; //UTC-6

    displayToStandard["Central Time (US & Canada)"] =
      "Central Standard Time"; //UTC-6

    displayToStandard["Central America"] =
      "Central America Standard Time"; //UTC-6

    displayToStandard["Indiana (East)"] =
      "US Eastern Standard Time"; //UTC-5

    displayToStandard["Eastern Time (US & Canada)"] =
      "Eastern Standard Time"; //UTC-5

    displayToStandard["Bogota, Lima, Quito, Rio Branco"] =
      "SA Pacific Standard Time"; //UTC-5

    displayToStandard["Caracas"] =
      "Venezuela Standard Time"; //UTC-4

    displayToStandard["Santiago"] =
      "Pacific SA Standard Time"; //UTC-4

    displayToStandard["Manaus"] =
      "Central Brazilian Standard Time"; //UTC-4

    displayToStandard["La Paz"] =
      "SA Western Standard Time"; //UTC-4

    displayToStandard["Atlantic Time (Canada)"] =
      "Atlantic Standard Time"; //UTC-4

    displayToStandard["Newfoundland"] =
      "Newfoundland Standard Time"; //UTC-3

    displayToStandard["Montevideo"] =
      "Montevideo Standard Time"; //UTC-3

    displayToStandard["Greenland"] =
      "Greenland Standard Time"; //UTC-3

    displayToStandard["Georgetown"] =
      "SA Eastern Standard Time"; //UTC-3

    displayToStandard["Buenos Aires"] =
      "Argentina Standard Time"; //UTC-3

    displayToStandard["Brasilia"] =
      "E. South America Standard Time"; //UTC-3

    displayToStandard["Mid-Atlantic"] =
      "Mid-Atlantic Standard Time"; //UTC-2

    displayToStandard["Cape Verde Is."] =
      "Cape Verde Standard Time"; //UTC-1

    displayToStandard["Azores"] =
      "Azores Standard Time"; //UTC-1

    displayToStandard["Nuku'alofa"] =
      "Tonga Standard Time"; //UTC+13

    displayToStandard["Fiji, Kamchatka, Marshall Is."] =
      "Fiji Standard Time"; //UTC+12

    displayToStandard["Auckland, Wellington"] =
      "New Zealand Standard Time"; //UTC+12

    displayToStandard["Magadan, Solomon Is., New Caledonia"] =
      "Central Pacific Standard Time"; //UTC+11

    displayToStandard["Vladivostok"] =
      "Vladivostok Standard Time"; //UTC+10

    displayToStandard["Hobart"] =
      "Tasmania Standard Time"; //UTC+10

    displayToStandard["Guam, Port Moresby"] =
      "West Pacific Standard Time"; //UTC+10

    displayToStandard["Canberra, Melbourne, Sydney"] =
      "AUS Eastern Standard Time"; //UTC+10

    displayToStandard["Brisbane"] =
      "E. Australia Standard Time"; //UTC+10

    displayToStandard["Darwin"] =
      "AUS Central Standard Time"; //UTC+9

    displayToStandard["Adelaide"] =
      "Cen. Australia Standard Time"; //UTC+9

    displayToStandard["Yakutsk"] =
      "Yakutsk Standard Time"; //UTC+9

    displayToStandard["Seoul"] =
      "Korea Standard Time"; //UTC+9

    displayToStandard["Osaka, Sapporo, Tokyo"] =
      "Tokyo Standard Time"; //UTC+9

    displayToStandard["Taipei"] =
      "Taipei Standard Time"; //UTC+8

    displayToStandard["Perth"] =
      "W. Australia Standard Time"; //UTC+8

    displayToStandard["Kuala Lumpur, Singapore"] =
      "Malay Peninsula Standard Time"; //UTC+8

    displayToStandard["Irkutsk, Ulaan Bataar"] =
      "North Asia East Standard Time"; //UTC+8

    displayToStandard["Beijing, Chongqing, Hong Kong, Urumqi"] =
      "China Standard Time"; //UTC+8

    displayToStandard["Krasnoyarsk"] =
      "North Asia Standard Time"; //UTC+7

    displayToStandard["Bangkok, Hanoi, Jakarta"] =
      "SE Asia Standard Time"; //UTC+7

    displayToStandard["Yangon (Rangoon)"] =
      "Myanmar Standard Time"; //UTC+6

    displayToStandard["Astana, Dhaka"] =
      "Central Asia Standard Time"; //UTC+6

    displayToStandard["Almaty, Novosibirsk"] =
      "N. Central Asia Standard Time"; //UTC+6

    displayToStandard["Kathmandu"] =
      "Nepal Standard Time"; //UTC+5

    displayToStandard["Sri Jayawardenepura"] =
      "Sri Lanka Standard Time"; //UTC+5

    displayToStandard["Chennai, Kolkata, Mumbai, New Delhi"] =
      "India Standard Time"; //UTC+5

    displayToStandard["Tashkent"] =
      "West Asia Standard Time"; //UTC+5

    displayToStandard["Islamabad, Karachi"] =
      "Pakistan Standard Time"; //UTC+5

    displayToStandard["Ekaterinburg"] =
      "Ekaterinburg Standard Time"; //UTC+5

    displayToStandard["Kabul"] =
      "Afghanistan Standard Time"; //UTC+4

    displayToStandard["Yerevan"] =
      "Caucasus Standard Time"; //UTC+4

    displayToStandard["Baku"] =
      "Azerbaijan Standard Time"; //UTC+4

    displayToStandard["Abu Dhabi, Muscat"] =
      "Arabian Standard Time"; //UTC+4

    displayToStandard["Tehran"] =
      "Iran Standard Time"; //UTC+3

    displayToStandard["Tbilisi"] =
      "Georgian Standard Time"; //UTC+3

    displayToStandard["Nairobi"] =
      "E. Africa Standard Time"; //UTC+3

    displayToStandard["Moscow, St. Petersburg, Volgograd"] =
      "Russian Standard Time"; //UTC+3

    displayToStandard["Kuwait, Riyadh"] =
      "Arab Standard Time"; //UTC+3

    displayToStandard["Baghdad"] =
      "Arabic Standard Time"; //UTC+3

    displayToStandard["Windhoek"] =
      "Namibia Standard Time"; //UTC+2

    displayToStandard["Minsk"] =
      "E. Europe Standard Time"; //UTC+2

    displayToStandard["Jerusalem"] =
      "Jerusalem Standard Time"; //UTC+2

    displayToStandard["Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius"] =
      "FLE Standard Time"; //UTC+2

    displayToStandard["Harare, Pretoria"] =
      "South Africa Standard Time"; //UTC+2

    displayToStandard["Cairo"] =
      "Egypt Standard Time"; //UTC+2

    displayToStandard["Beirut"] =
      "Middle East Standard Time"; //UTC+2

    displayToStandard["Athens, Bucharest, Istanbul"] =
      "GTB Standard Time"; //UTC+2

    displayToStandard["Amman"] =
      "Jordan Standard Time"; //UTC+2

    displayToStandard["West Central Africa"] =
      "W. Central Africa Standard Time"; //UTC+1

    displayToStandard["Sarajevo, Skopje, Warsaw, Zagreb"] =
      "Central European Standard Time"; //UTC+1

    displayToStandard["Brussels, Copenhagen, Madrid, Paris"] =
      "Romance Standard Time"; //UTC+1

    displayToStandard["Belgrade, Bratislava, Budapest, Ljubljana, Prague"] =
      "Central Europe Standard Time"; //UTC+1

    displayToStandard["Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna"] =
      "W. Europe Standard Time"; //UTC+1

    displayToStandard["Monrovia, Reykjavik"] =
      "Greenwich Standard Time"; //UTC

    displayToStandard["Greenwich Mean Time : Dublin, Edinburgh, Lisbon, London"] =
      "GMT Standard Time"; //UTC

    displayToStandard["Casablanca"] =
      "Morocco Standard Time"; //UTC
  }

  QString standardStr = displayToStandard[displayName];
  if ( standardStr.isEmpty() ) {
    kWarning() << "Unknown/invalid displayName specified:" << displayName;
  }
  return standardStr;
}


QString TZMaps::winZoneToOlson( const QString &windowsZone )
{
  static QHash<QString,QString> winToOlson;

  if ( winToOlson.isEmpty() ) {
    winToOlson["International Date Line West"] =
      "Etc/GMT+12"; //UTC-12

    winToOlson["Midway Island, Samoa"] =
      "Pacific/Apia"; //UTC-11

    winToOlson["Hawaii"] =
      "Pacific/Honolulu"; //UTC-10

    winToOlson["Alaska"] =
      "America/Anchorage"; //UTC-9

    winToOlson["Pacific Time (US & Canada)"] =
      "America/Los_Angeles"; //UTC-8

    winToOlson["Tijuana, Baja California"] =
      "America/Tijuana"; //UTC-8

    winToOlson["Chihuahua, La Paz, Mazatlan"] =
      "America/Chihuahua"; //UTC-7

    winToOlson["Arizona"] =
      "America/Phoenix"; //UTC-7

    winToOlson["Mountain Time (US & Canada)"] =
      "America/Denver"; //UTC-7

    winToOlson["Chihuahua, La Paz, Mazatlan - Old"] =
      "America/Chihuahua"; //UTC-7

    winToOlson["Central America"] =
      "America/Guatemala"; //UTC-6

    winToOlson["Saskatchewan"] =
      "America/Regina"; //UTC-6

    winToOlson["Guadalajara, Mexico City, Monterrey - Old"] =
      "America/Mexico_City"; //UTC-6

    winToOlson["Central Time (US & Canada)"] =
      "America/Chicago"; //UTC-6

    winToOlson["Guadalajara, Mexico City, Monterrey"] =
      "America/Mexico_City"; //UTC-6

    winToOlson["Bogota, Lima, Quito, Rio Branco"] =
      "America/Bogota"; //UTC-5

    winToOlson["Eastern Time (US & Canada)"] =
      "America/New_York"; //UTC-5

    winToOlson["Indiana (East)"] =
      "Etc/GMT+5"; //UTC-5

    winToOlson["Caracas"] =
      "America/Caracas"; //UTC-4:30

    winToOlson["Manaus"] =
      "America/Manaus"; //UTC-4

    winToOlson["La Paz"] =
      "America/La_Paz"; //UTC-4

    winToOlson["Atlantic Time (Canada)"] =
      "America/Halifax"; //UTC-4

    winToOlson["Santiago"] =
      "America/Santiago"; //UTC-4

    winToOlson["Newfoundland"] =
      "America/St_Johns"; //UTC-3:30

    winToOlson["Brasilia"] =
      "America/Sao_Paulo"; //UTC-3

    winToOlson["Buenos Aires"] =
      "America/Buenos_Aires"; //UTC-3

    winToOlson["Georgetown"] =
      "Etc/GMT+3"; //UTC-3

    winToOlson["Montevideo"] =
      "America/Montevideo"; //UTC-3

    winToOlson["Greenland"] =
      "America/Godthab"; //UTC-3

    winToOlson["Mid-Atlantic"] =
      "Atlantic/South_Georgia"; //UTC-2

    winToOlson["Azores"] =
      "Atlantic/Azores"; //UTC-1

    winToOlson["Cape Verde Is."] =
      "Atlantic/Cape_Verde"; //UTC-1

    winToOlson["Greenwich Mean Time : Dublin, Edinburgh, Lisbon, London"] =
      "Europe/London"; //UTC

    winToOlson["Casablanca, Monrovia, Reykjavik"] =
      "Africa/Casablanca"; //UTC

    winToOlson["Sarajevo, Skopje, Warsaw, Zagreb"] =
      "Europe/Warsaw"; //UTC+1

    winToOlson["Belgrade, Bratislava, Budapest, Ljubljana, Prague"] =
      "Europe/Budapest"; //UTC+1

    winToOlson["Brussels, Copenhagen, Madrid, Paris"] =
      "Europe/Paris"; //UTC+1

    winToOlson["West Central Africa"] =
      "Africa/Lagos"; //UTC+1

    winToOlson["Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna"] =
      "Europe/Berlin"; //UTC+1

    winToOlson["Beirut"] =
      "Asia/Beirut"; //UTC+2

    winToOlson["Athens, Bucharest, Istanbul"] =
      "Europe/Istanbul"; //UTC+2

    winToOlson["Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius"] =
      "Europe/Kiev"; //UTC+2

    winToOlson["Harare, Pretoria"] =
      "Africa/Johannesburg"; //UTC+2

    winToOlson["Minsk"] =
      "Europe/Minsk"; //UTC+2

    winToOlson["Amman"] =
      "Asia/Amman"; //UTC+2

    winToOlson["Windhoek"] =
      "Africa/Windhoek"; //UTC+2

    winToOlson["Jerusalem"] =
      "Asia/Jerusalem"; //UTC+2

    winToOlson["Cairo"] =
      "Africa/Cairo"; //UTC+2

    winToOlson["Kuwait, Riyadh"] =
      "Asia/Riyadh"; //UTC+3

    winToOlson["Moscow, St. Petersburg, Volgograd"] =
      "Europe/Moscow"; //UTC+3

    winToOlson["Nairobi"] =
      "Africa/Nairobi"; //UTC+3

    winToOlson["Baghdad"] =
      "Asia/Baghdad"; //UTC+3

    winToOlson["Tbilisi"] =
      "Etc/GMT-3"; //UTC+3

    winToOlson["Tehran"] =
      "Asia/Tehran"; //UTC+3:30

    winToOlson["Baku"] =
      "Asia/Baku"; //UTC+4

    winToOlson["Caucasus Standard Time"] =
      "Asia/Tbilisi"; //UTC+4

    winToOlson["Yerevan"] =
      "Asia/Yerevan"; //UTC+4

    winToOlson["Abu Dhabi, Muscat"] =
      "Asia/Dubai"; //UTC+4

    winToOlson["Kabul"] =
      "Asia/Kabul"; //UTC+4:30

    winToOlson["Islamabad, Karachi, Tashkent"] =
      "Asia/Karachi"; //UTC+5

    winToOlson["Ekaterinburg"] =
      "Asia/Yekaterinburg"; //UTC+5

    winToOlson["Chennai, Kolkata, Mumbai, New Delhi"] =
      "Asia/Kolkata"; //UTC+5:30

    winToOlson["Sri Jayawardenepura"] =
      "Asia/Colombo"; //UTC+5:30

    winToOlson["Kathmandu"] =
      "Asia/Katmandu"; //UTC+5:45

    winToOlson["Astana, Dhaka"] =
      "Asia/Dhaka"; //UTC+6

    winToOlson["Almaty, Novosibirsk"] =
      "Asia/Novosibirsk"; //UTC+6

    winToOlson["Yangon (Rangoon)"] =
      "Asia/Rangoon"; //UTC+6:30

    winToOlson["Bangkok, Hanoi, Jakarta"] =
      "Asia/Bangkok"; //UTC+7

    winToOlson["Krasnoyarsk"] =
      "Asia/Krasnoyarsk"; //UTC+7

    winToOlson["Beijing, Chongqing, Hong Kong, Urumqi"] =
      "Asia/Shanghai"; //UTC+8

    winToOlson["Kuala Lumpur, Singapore"] =
      "Asia/Singapore"; //UTC+8

    winToOlson["Irkutsk, Ulaan Bataar"] =
      "Asia/Irkutsk"; //UTC+8

    winToOlson["Taipei"] =
      "Asia/Taipei"; //UTC+8

    winToOlson["Perth"] =
      "Australia/Perth"; //UTC+8

    winToOlson["Osaka, Sapporo, Tokyo"] =
      "Asia/Tokyo"; //UTC+9

    winToOlson["Seoul"] =
      "Asia/Seoul"; //UTC+9

    winToOlson["Yakutsk"] =
      "Asia/Yakutsk"; //UTC+9

    winToOlson["Darwin"] =
      "Australia/Darwin"; //UTC+9:30

    winToOlson["Adelaide"] =
      "Australia/Adelaide"; //UTC+9:30

    winToOlson["Canberra, Melbourne, Sydney"] =
      "Australia/Sydney"; //UTC+10

    winToOlson["Vladivostok"] =
      "Asia/Vladivostok"; //UTC+10

    winToOlson["Hobart"] =
      "Australia/Hobart"; //UTC+10

    winToOlson["Brisbane"] =
      "Australia/Brisbane"; //UTC+10

    winToOlson["Guam, Port Moresby"] =
      "Pacific/Port_Moresby"; //UTC+10

    winToOlson["Magadan, Solomon Is., New Caledonia"] =
      "Pacific/Guadalcanal"; //UTC+11

    winToOlson["Fiji, Kamchatka, Marshall Is."] =
      "Pacific/Fiji"; //UTC+12

    winToOlson["Auckland, Wellington"] =
      "Pacific/Auckland"; //UTC+12

    winToOlson["Nuku'alofa"] =
      "Pacific/Tongatapu"; //UTC+13
  }

  QString olsonStr = winToOlson[windowsZone];
  if ( olsonStr.isEmpty() ) {
    kWarning() << "Unknown/invalid windowsZone specified:" << windowsZone;
  }
  return olsonStr;
}

QString TZMaps::winZoneToUtcOffset( const QString &windowsZone )
{
  static QHash<QString,QString> winToUtcOffset;

  if ( winToUtcOffset.isEmpty() ) {
    winToUtcOffset["International Date Line West"] = "UTC-12";

    winToUtcOffset["Midway Island, Samoa"] = "UTC-11";

    winToUtcOffset["Hawaii"] = "UTC-10";

    winToUtcOffset["Alaska"] = "UTC-9";

    winToUtcOffset["Pacific Time (US & Canada)"] = "UTC-8";

    winToUtcOffset["Tijuana, Baja California"] = "UTC-8";

    winToUtcOffset["Chihuahua, La Paz, Mazatlan"] = "UTC-7";

    winToUtcOffset["Arizona"] = "UTC-7";

    winToUtcOffset["Mountain Time (US & Canada)"] = "UTC-7";

    winToUtcOffset["Chihuahua, La Paz, Mazatlan - Old"] = "UTC-7";

    winToUtcOffset["Central America"] = "UTC-6";

    winToUtcOffset["Saskatchewan"] = "UTC-6";

    winToUtcOffset["Guadalajara, Mexico City, Monterrey - Old"] = "UTC-6";

    winToUtcOffset["Central Time (US & Canada)"] = "UTC-6";

    winToUtcOffset["Guadalajara, Mexico City, Monterrey"] = "UTC-6";

    winToUtcOffset["Bogota, Lima, Quito, Rio Branco"] = "UTC-5";

    winToUtcOffset["Eastern Time (US & Canada)"] = "UTC-5";

    winToUtcOffset["Indiana (East)"] = "UTC-5";

    winToUtcOffset["Caracas"] = "UTC-4:30";

    winToUtcOffset["Manaus"] = "UTC-4";

    winToUtcOffset["La Paz"] = "UTC-4";

    winToUtcOffset["Atlantic Time (Canada)"] = "UTC-4";

    winToUtcOffset["Santiago"] = "UTC-4";

    winToUtcOffset["Newfoundland"] = "UTC-3:30";

    winToUtcOffset["Brasilia"] = "UTC-3";

    winToUtcOffset["Buenos Aires"] = "UTC-3";

    winToUtcOffset["Georgetown"] = "UTC-3";

    winToUtcOffset["Montevideo"] = "UTC-3";

    winToUtcOffset["Greenland"] = "UTC-3";

    winToUtcOffset["Mid-Atlantic"] = "UTC-2";

    winToUtcOffset["Azores"] = "UTC-1";

    winToUtcOffset["Cape Verde Is."] = "UTC-1";

    winToUtcOffset["Greenwich Mean Time : Dublin, Edinburgh, Lisbon, London"] = "UTC";

    winToUtcOffset["Casablanca, Monrovia, Reykjavik"] = "UTC";

    winToUtcOffset["Sarajevo, Skopje, Warsaw, Zagreb"] = "UTC+1";

    winToUtcOffset["Belgrade, Bratislava, Budapest, Ljubljana, Prague"] = "UTC+1";

    winToUtcOffset["Brussels, Copenhagen, Madrid, Paris"] = "UTC+1";

    winToUtcOffset["West Central Africa"] = "UTC+1";

    winToUtcOffset["Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna"] = "UTC+1";

    winToUtcOffset["Beirut"] = "UTC+2";

    winToUtcOffset["Athens, Bucharest, Istanbul"] = "UTC+2";

    winToUtcOffset["Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius"] = "UTC+2";

    winToUtcOffset["Harare, Pretoria"] = "UTC+2";

    winToUtcOffset["Minsk"] = "UTC+2";

    winToUtcOffset["Amman"] = "UTC+2";

    winToUtcOffset["Windhoek"] = "UTC+2";

    winToUtcOffset["Jerusalem"] = "UTC+2";

    winToUtcOffset["Cairo"] = "UTC+2";

    winToUtcOffset["Kuwait, Riyadh"] = "UTC+3";

    winToUtcOffset["Moscow, St. Petersburg, Volgograd"] = "UTC+3";

    winToUtcOffset["Nairobi"] = "UTC+3";

    winToUtcOffset["Baghdad"] = "UTC+3";

    winToUtcOffset["Tbilisi"] = "UTC+3";

    winToUtcOffset["Tehran"] = "UTC+3:30";

    winToUtcOffset["Baku"] = "UTC+4";

    winToUtcOffset["Caucasus Standard Time"] = "UTC+4";

    winToUtcOffset["Yerevan"] = "UTC+4";

    winToUtcOffset["Abu Dhabi, Muscat"] = "UTC+4";

    winToUtcOffset["Kabul"] = "UTC+4:30";

    winToUtcOffset["Islamabad, Karachi, Tashkent"] = "UTC+5";

    winToUtcOffset["Ekaterinburg"] = "UTC+5";

    winToUtcOffset["Chennai, Kolkata, Mumbai, New Delhi"] = "UTC+5:30";

    winToUtcOffset["Sri Jayawardenepura"] = "UTC+5:30";

    winToUtcOffset["Kathmandu"] = "UTC+5:45";

    winToUtcOffset["Astana, Dhaka"] = "UTC+6";

    winToUtcOffset["Almaty, Novosibirsk"] = "UTC+6";

    winToUtcOffset["Yangon (Rangoon)"] = "UTC+6:30";

    winToUtcOffset["Bangkok, Hanoi, Jakarta"] = "UTC+7";

    winToUtcOffset["Krasnoyarsk"] = "UTC+7";

    winToUtcOffset["Beijing, Chongqing, Hong Kong, Urumqi"] = "UTC+8";

    winToUtcOffset["Kuala Lumpur, Singapore"] = "UTC+8";

    winToUtcOffset["Irkutsk, Ulaan Bataar"] = "UTC+8";

    winToUtcOffset["Taipei"] = "UTC+8";

    winToUtcOffset["Perth"] = "UTC+8";

    winToUtcOffset["Osaka, Sapporo, Tokyo"] = "UTC+9";

    winToUtcOffset["Seoul"] = "UTC+9";

    winToUtcOffset["Yakutsk"] = "UTC+9";

    winToUtcOffset["Darwin"] = "UTC+9:30";

    winToUtcOffset["Adelaide"] = "UTC+9:30";

    winToUtcOffset["Canberra, Melbourne, Sydney"] = "UTC+10";

    winToUtcOffset["Vladivostok"] = "UTC+10";

    winToUtcOffset["Hobart"] = "UTC+10";

    winToUtcOffset["Brisbane"] = "UTC+10";

    winToUtcOffset["Guam, Port Moresby"] = "UTC+10";

    winToUtcOffset["Magadan, Solomon Is., New Caledonia"] = "UTC+11";

    winToUtcOffset["Fiji, Kamchatka, Marshall Is."] = "UTC+12";

    winToUtcOffset["Auckland, Wellington"] = "UTC+12";

    winToUtcOffset["Nuku'alofa"] = "UTC+13";
  }

  QString utcOffset = winToUtcOffset[windowsZone];
  if ( utcOffset.isEmpty() ) {
    kWarning() << "Unknown/invalid windowsZone specified:" << windowsZone;
  }
  return utcOffset;
}

QString TZMaps::utcOffsetToWinZone( const QString &utcOffset )
{
  static QHash<QString,QString> utcOffsetToWin;

  if ( utcOffsetToWin.isEmpty() ) {
    utcOffsetToWin["UTC-12"] = "International Date Line West";

    utcOffsetToWin["UTC-11"] = "Midway Island, Samoa";

    utcOffsetToWin["UTC-10"] = "Hawaii";

    utcOffsetToWin["UTC-9"] = "Alaska";

    utcOffsetToWin["UTC-8"] = "Pacific Time (US & Canada)";

    utcOffsetToWin["UTC-7"] = "Mountain Time (US & Canada)";

    utcOffsetToWin["UTC-6"] = "Central Time (US & Canada)";

    utcOffsetToWin["UTC-5"] = "Eastern Time (US & Canada)";

    utcOffsetToWin["UTC-4:30"] = "Caracas";

    utcOffsetToWin["UTC-4"] = "Atlantic Time (Canada)";

    utcOffsetToWin["UTC-3:30"] = "Newfoundland";

    utcOffsetToWin["UTC-3"] = "Buenos Aires";

    utcOffsetToWin["UTC-2"] = "Mid-Atlantic";

    utcOffsetToWin["UTC-1"] = "Cape Verde Is.";

    utcOffsetToWin["UTC"] = "Greenwich Mean Time : Dublin, Edinburgh, Lisbon, London";

    utcOffsetToWin["UTC+1"] = "Brussels, Copenhagen, Madrid, Paris";

    utcOffsetToWin["UTC+2"] = "Cairo";

    utcOffsetToWin["UTC+3"] = "Moscow, St. Petersburg, Volgograd";

    utcOffsetToWin["UTC+3:30"] = "Tehran";

    utcOffsetToWin["UTC+4"] = "Caucasus Standard Time";

    utcOffsetToWin["UTC+4:30"] = "Kabul";

    utcOffsetToWin["UTC+5"] = "Islamabad, Karachi, Tashkent";

    utcOffsetToWin["UTC+5:30"] = "Chennai, Kolkata, Mumbai, New Delhi";

    utcOffsetToWin["UTC+5:45"] = "Kathmandu";

    utcOffsetToWin["UTC+6"] = "Astana, Dhaka";

    utcOffsetToWin["UTC+6:30"] = "Yangon (Rangoon)";

    utcOffsetToWin["UTC+7"] = "Bangkok, Hanoi, Jakarta";

    utcOffsetToWin["UTC+8"] = "Beijing, Chongqing, Hong Kong, Urumqi";

    utcOffsetToWin["UTC+9"] = "Osaka, Sapporo, Tokyo";

    utcOffsetToWin["UTC+9:30"] = "Adelaide";

    utcOffsetToWin["UTC+10"] = "Canberra, Melbourne, Sydney";

    utcOffsetToWin["UTC+11"] = "Magadan, Solomon Is., New Caledonia";

    utcOffsetToWin["UTC+12"] = "Auckland, Wellington";

    utcOffsetToWin["UTC+13"] = "Nuku'alofa";
  }

  QString windowsZone = utcOffsetToWin[utcOffset.toUpper()];
  if ( windowsZone.isEmpty() ) {
    kWarning() << "Unknown/invalid UTC offset specified:" << utcOffset;
  }
  return windowsZone;
}

QString TZMaps::olsonToUtcOffset( const QString &olsonZone )
{
  static QHash<QString,QString> olsonToUtcOffset;

  if ( olsonToUtcOffset.isEmpty() ) {
    olsonToUtcOffset["Africa/Abidjan"] = "UTC";
    olsonToUtcOffset["Africa/Accra"] = "UTC";
    olsonToUtcOffset["Africa/Addis_Ababa"] = "UTC+3";
    olsonToUtcOffset["Africa/Algiers"] = "UTC+1";
    olsonToUtcOffset["Africa/Asmara"] = "UTC+3";
    olsonToUtcOffset["Africa/Bamako"] = "UTC";
    olsonToUtcOffset["Africa/Bangui"] = "UTC+1";
    olsonToUtcOffset["Africa/Banjul"] = "UTC";
    olsonToUtcOffset["Africa/Bissau"] = "UTC";
    olsonToUtcOffset["Africa/Blantyre"] = "UTC+2";
    olsonToUtcOffset["Africa/Brazzaville"] = "UTC+1";
    olsonToUtcOffset["Africa/Bujumbura"] = "UTC+2";
    olsonToUtcOffset["Africa/Cairo"] = "UTC+2";
    olsonToUtcOffset["Africa/Casablanca"] = "UTC+1";
    olsonToUtcOffset["Africa/Ceuta"] = "UTC+1";
    olsonToUtcOffset["Africa/Conakry"] = "UTC";
    olsonToUtcOffset["Africa/Dakar"] = "UTC";
    olsonToUtcOffset["Africa/Dar_es_Salaam"] = "UTC+3";
    olsonToUtcOffset["Africa/Djibouti"] = "UTC+3";
    olsonToUtcOffset["Africa/Douala"] = "UTC+1";
    olsonToUtcOffset["Africa/El_Aaiun"] = "UTC";
    olsonToUtcOffset["Africa/Freetown"] = "UTC";
    olsonToUtcOffset["Africa/Gaborone"] = "UTC+2";
    olsonToUtcOffset["Africa/Harare"] = "UTC+2";
    olsonToUtcOffset["Africa/Johannesburg"] = "UTC+2";
    olsonToUtcOffset["Africa/Kampala"] = "UTC+3";
    olsonToUtcOffset["Africa/Khartoum"] = "UTC+3";
    olsonToUtcOffset["Africa/Kigali"] = "UTC+2";
    olsonToUtcOffset["Africa/Kinshasa"] = "UTC+1";
    olsonToUtcOffset["Africa/Lagos"] = "UTC+1";
    olsonToUtcOffset["Africa/Libreville"] = "UTC+1";
    olsonToUtcOffset["Africa/Lome"] = "UTC";
    olsonToUtcOffset["Africa/Luanda"] = "UTC+1";
    olsonToUtcOffset["Africa/Lubumbashi"] = "UTC+2";
    olsonToUtcOffset["Africa/Lusaka"] = "UTC+2";
    olsonToUtcOffset["Africa/Malabo"] = "UTC+1";
    olsonToUtcOffset["Africa/Maputo"] = "UTC+2";
    olsonToUtcOffset["Africa/Maseru"] = "UTC+2";
    olsonToUtcOffset["Africa/Mbabane"] = "UTC+2";
    olsonToUtcOffset["Africa/Mogadishu"] = "UTC+3";
    olsonToUtcOffset["Africa/Monrovia"] = "UTC";
    olsonToUtcOffset["Africa/Nairobi"] = "UTC+3";
    olsonToUtcOffset["Africa/Ndjamena"] = "UTC+1";
    olsonToUtcOffset["Africa/Niamey"] = "UTC+1";
    olsonToUtcOffset["Africa/Nouakchott"] = "UTC";
    olsonToUtcOffset["Africa/Ouagadougou"] = "UTC";
    olsonToUtcOffset["Africa/Porto-Novo"] = "UTC+1";
    olsonToUtcOffset["Africa/Sao_Tome"] = "UTC";
    olsonToUtcOffset["Africa/Tripoli"] = "UTC+2";
    olsonToUtcOffset["Africa/Tunis"] = "UTC+1";
    olsonToUtcOffset["Africa/Windhoek"] = "UTC+1";
    olsonToUtcOffset["America/Adak"] = "UTC-10";
    olsonToUtcOffset["America/Anchorage"] = "UTC-9";
    olsonToUtcOffset["America/Anguilla"] = "UTC-4";
    olsonToUtcOffset["America/Antigua"] = "UTC-4";
    olsonToUtcOffset["America/Araguaina"] = "UTC-3";
    olsonToUtcOffset["America/Argentina/La_Rioja"] = "UTC-4";
    olsonToUtcOffset["America/Argentina/Rio_Gallegos"] = "UTC-4";
    olsonToUtcOffset["America/Argentina/San_Juan"] = "UTC-4";
    olsonToUtcOffset["America/Argentina/San_Luis"] = "UTC-3";
    olsonToUtcOffset["America/Argentina/Tucuman"] = "UTC-4";
    olsonToUtcOffset["America/Argentina/Ushuaia"] = "UTC-4";
    olsonToUtcOffset["America/Aruba"] = "UTC-4";
    olsonToUtcOffset["America/Asuncion"] = "UTC-4";
    olsonToUtcOffset["America/Bahia"] = "UTC-3";
    olsonToUtcOffset["America/Barbados"] = "UTC-4";
    olsonToUtcOffset["America/Belem"] = "UTC-3";
    olsonToUtcOffset["America/Belize"] = "UTC-6";
    olsonToUtcOffset["America/Blanc-Sablon"] = "UTC-4";
    olsonToUtcOffset["America/Boa_Vista"] = "UTC-4";
    olsonToUtcOffset["America/Bogota"] = "UTC-5";
    olsonToUtcOffset["America/Boise"] = "UTC-7";
    olsonToUtcOffset["America/Argentina/Buenos_Aires"] = "UTC-3";
    olsonToUtcOffset["America/Cambridge_Bay"] = "UTC-6";
    olsonToUtcOffset["America/Campo_Grande"] = "UTC-4";
    olsonToUtcOffset["America/Cancun"] = "UTC-6";
    olsonToUtcOffset["America/Caracas"] = "UTC-4:30";
    olsonToUtcOffset["America/Argentina/Catamarca"] = "UTC-4";
    olsonToUtcOffset["America/Cayenne"] = "UTC-3";
    olsonToUtcOffset["America/Cayman"] = "UTC-5";
    olsonToUtcOffset["America/Chicago"] = "UTC-5";
    olsonToUtcOffset["America/Chihuahua"] = "UTC-6";
    olsonToUtcOffset["America/Atikokan"] = "UTC-5";
    olsonToUtcOffset["America/Argentina/Cordoba"] = "UTC-3";
    olsonToUtcOffset["America/Costa_Rica"] = "UTC-6";
    olsonToUtcOffset["America/Cuiaba"] = "UTC-4";
    olsonToUtcOffset["America/Curacao"] = "UTC-4";
    olsonToUtcOffset["America/Danmarkshavn"] = "UTC";
    olsonToUtcOffset["America/Dawson"] = "UTC-8";
    olsonToUtcOffset["America/Dawson_Creek"] = "UTC-7";
    olsonToUtcOffset["America/Denver"] = "UTC-7";
    olsonToUtcOffset["America/Detroit"] = "UTC-5";
    olsonToUtcOffset["America/Dominica"] = "UTC-4";
    olsonToUtcOffset["America/Edmonton"] = "UTC-7";
    olsonToUtcOffset["America/Eirunepe"] = "UTC-5";
    olsonToUtcOffset["America/El_Salvador"] = "UTC-6";
    olsonToUtcOffset["America/Fortaleza"] = "UTC-3";
    olsonToUtcOffset["America/Glace_Bay"] = "UTC-4";
    olsonToUtcOffset["America/Godthab"] = "UTC-3";
    olsonToUtcOffset["America/Goose_Bay"] = "UTC-4";
    olsonToUtcOffset["America/Grand_Turk"] = "UTC-5";
    olsonToUtcOffset["America/Grenada"] = "UTC-4";
    olsonToUtcOffset["America/Guadeloupe"] = "UTC-4";
    olsonToUtcOffset["America/Guatemala"] = "UTC-6";
    olsonToUtcOffset["America/Guayaquil"] = "UTC-5";
    olsonToUtcOffset["America/Guyana"] = "UTC-4";
    olsonToUtcOffset["America/Halifax"] = "UTC-4";
    olsonToUtcOffset["America/Havana"] = "UTC-5";
    olsonToUtcOffset["America/Hermosillo"] = "UTC-7";
    olsonToUtcOffset["America/Indiana/Knox"] = "UTC-5";
    olsonToUtcOffset["America/Indiana/Marengo"] = "UTC-5";
    olsonToUtcOffset["America/Indiana/Petersburg"] = "UTC-5";
    olsonToUtcOffset["America/Indiana/Tell_City"] = "UTC-5";
    olsonToUtcOffset["America/Indiana/Vevay"] = "UTC-5";
    olsonToUtcOffset["America/Indiana/Vincennes"] = "UTC-5";
    olsonToUtcOffset["America/Indiana/Winamac"] = "UTC-5";
    olsonToUtcOffset["America/Fort_Wayne"] = "UTC-5";
    olsonToUtcOffset["America/Inuvik"] = "UTC-7";
    olsonToUtcOffset["America/Iqaluit"] = "UTC-5";
    olsonToUtcOffset["America/Jamaica"] = "UTC-5";
    olsonToUtcOffset["America/Argentina/Jujuy"] = "UTC-3";
    olsonToUtcOffset["America/Juneau"] = "UTC-9";
    olsonToUtcOffset["America/Kentucky/Monticello"] = "UTC-5";
    olsonToUtcOffset["America/La_Paz"] = "UTC-4";
    olsonToUtcOffset["America/Lima"] = "UTC-5";
    olsonToUtcOffset["America/Los_Angeles"] = "UTC-8";
    olsonToUtcOffset["America/Kentucky/Louisville"] = "UTC-5";
    olsonToUtcOffset["America/Maceio"] = "UTC-3";
    olsonToUtcOffset["America/Managua"] = "UTC-6";
    olsonToUtcOffset["America/Manaus"] = "UTC-4";
    olsonToUtcOffset["America/Marigot"] = "UTC-4";
    olsonToUtcOffset["America/Martinique"] = "UTC-4";
    olsonToUtcOffset["America/Mazatlan"] = "UTC-7";
    olsonToUtcOffset["America/Argentina/Mendoza"] = "UTC-3";
    olsonToUtcOffset["America/Menominee"] = "UTC-5";
    olsonToUtcOffset["America/Merida"] = "UTC-6";
    olsonToUtcOffset["America/Mexico_City"] = "UTC-6";
    olsonToUtcOffset["America/Miquelon"] = "UTC-3";
    olsonToUtcOffset["America/Moncton"] = "UTC-4";
    olsonToUtcOffset["America/Monterrey"] = "UTC-6";
    olsonToUtcOffset["America/Montevideo"] = "UTC-3";
    olsonToUtcOffset["America/Montreal"] = "UTC-5";
    olsonToUtcOffset["America/Montserrat"] = "UTC-4";
    olsonToUtcOffset["America/Nassau"] = "UTC-5";
    olsonToUtcOffset["America/New_York"] = "UTC-5";
    olsonToUtcOffset["America/Nipigon"] = "UTC-5";
    olsonToUtcOffset["America/Nome"] = "UTC-9";
    olsonToUtcOffset["America/Noronha"] = "UTC-2";
    olsonToUtcOffset["America/North_Dakota/Center"] = "UTC-6";
    olsonToUtcOffset["America/North_Dakota/New_Salem"] = "UTC-6";
    olsonToUtcOffset["America/Panama"] = "UTC-5";
    olsonToUtcOffset["America/Pangnirtung"] = "UTC-5";
    olsonToUtcOffset["America/Paramaribo"] = "UTC-3";
    olsonToUtcOffset["America/Phoenix"] = "UTC-7";
    olsonToUtcOffset["America/Port_of_Spain"] = "UTC-4";
    olsonToUtcOffset["America/Port-au-Prince"] = "UTC-5";
    olsonToUtcOffset["America/Porto_Velho"] = "UTC-4";
    olsonToUtcOffset["America/Puerto_Rico"] = "UTC-4";
    olsonToUtcOffset["America/Rainy_River"] = "UTC-6";
    olsonToUtcOffset["America/Rankin_Inlet"] = "UTC-5";
    olsonToUtcOffset["America/Recife"] = "UTC-3";
    olsonToUtcOffset["America/Regina"] = "UTC-6";
    olsonToUtcOffset["America/Resolute"] = "UTC-5";
    olsonToUtcOffset["America/Rio_Branco"] = "UTC-5";
    olsonToUtcOffset["America/Santarem"] = "UTC-5";
    olsonToUtcOffset["America/Santiago"] = "UTC-4";
    olsonToUtcOffset["America/Santo_Domingo"] = "UTC-4";
    olsonToUtcOffset["America/Sao_Paulo"] = "UTC-3";
    olsonToUtcOffset["America/Scoresbysund"] = "UTC-1";
    olsonToUtcOffset["America/Shiprock"] = "UTC-7";
    olsonToUtcOffset["America/St_Barthelemy"] = "UTC-4";
    olsonToUtcOffset["America/St_Johns"] = "UTC-3:30";
    olsonToUtcOffset["America/St_Kitts"] = "UTC-4";
    olsonToUtcOffset["America/St_Lucia"] = "UTC-4";
    olsonToUtcOffset["America/St_Thomas"] = "UTC-4";
    olsonToUtcOffset["America/St_Vincent"] = "UTC-4";
    olsonToUtcOffset["America/Swift_Current"] = "UTC-6";
    olsonToUtcOffset["America/Tegucigalpa"] = "UTC-6";
    olsonToUtcOffset["America/Thule"] = "UTC-4";
    olsonToUtcOffset["America/Thunder_Bay"] = "UTC-5";
    olsonToUtcOffset["America/Tijuana"] = "UTC-8";
    olsonToUtcOffset["America/Toronto"] = "UTC-5";
    olsonToUtcOffset["America/Tortola"] = "UTC-4";
    olsonToUtcOffset["America/Vancouver"] = "UTC-8";
    olsonToUtcOffset["America/Whitehorse"] = "UTC-8";
    olsonToUtcOffset["America/Winnipeg"] = "UTC-6";
    olsonToUtcOffset["America/Yakutat"] = "UTC-9";
    olsonToUtcOffset["America/Yellowknife"] = "UTC-7";
    olsonToUtcOffset["Antarctica/Casey"] = "UTC+8";
    olsonToUtcOffset["Antarctica/Davis"] = "UTC";
    olsonToUtcOffset["Antarctica/DumontDUrville"] = "UTC+10";
    olsonToUtcOffset["Antarctica/Mawson"] = "UTC+6";
    olsonToUtcOffset["Antarctica/McMurdo"] = "UTC+12";
    olsonToUtcOffset["Antarctica/Palmer"] = "UTC-4";
    olsonToUtcOffset["Antarctica/Rothera"] = "UTC-3";
    olsonToUtcOffset["Antarctica/South_Pole"] = "UTC+12";
    olsonToUtcOffset["Antarctica/Syowa"] = "UTC+3";
    olsonToUtcOffset["Antarctica/Vostok"] = "UTC+6";
    olsonToUtcOffset["Arctic/Longyearbyen"] = "UTC+1";
    olsonToUtcOffset["Asia/Aden"] = "UTC+3";
    olsonToUtcOffset["Asia/Almaty"] = "UTC+6";
    olsonToUtcOffset["Asia/Amman"] = "UTC+2";
    olsonToUtcOffset["Asia/Anadyr"] = "UTC+12";
    olsonToUtcOffset["Asia/Aqtau"] = "UTC+5";
    olsonToUtcOffset["Asia/Aqtobe"] = "UTC+5";
    olsonToUtcOffset["Asia/Ashgabat"] = "UTC+5";
    olsonToUtcOffset["Asia/Baghdad"] = "UTC+3";
    olsonToUtcOffset["Asia/Bahrain"] = "UTC+3";
    olsonToUtcOffset["Asia/Baku"] = "UTC+4";
    olsonToUtcOffset["Asia/Bangkok"] = "UTC+7";
    olsonToUtcOffset["Asia/Beirut"] = "UTC+2";
    olsonToUtcOffset["Asia/Bishkek"] = "UTC+6";
    olsonToUtcOffset["Asia/Brunei"] = "UTC+8";
    olsonToUtcOffset["Asia/Kolkata"] = "UTC+5:30";
    olsonToUtcOffset["Asia/Choibalsan"] = "UTC+9";
    olsonToUtcOffset["Asia/Chongqing"] = "UTC+8";
    olsonToUtcOffset["Asia/Colombo"] = "UTC+5:30";
    olsonToUtcOffset["Asia/Damascus"] = "UTC+2";
    olsonToUtcOffset["Asia/Dhaka"] = "UTC+6";
    olsonToUtcOffset["Asia/Dili"] = "UTC+9";
    olsonToUtcOffset["Asia/Dubai"] = "UTC+4";
    olsonToUtcOffset["Asia/Dushanbe"] = "UTC+5";
    olsonToUtcOffset["Asia/Gaza"] = "UTC+2";
    olsonToUtcOffset["Asia/Harbin"] = "UTC+8";
    olsonToUtcOffset["Asia/Hong_Kong"] = "UTC+8";
    olsonToUtcOffset["Asia/Hovd"] = "UTC+7";
    olsonToUtcOffset["Asia/Irkutsk"] = "UTC+7";
    olsonToUtcOffset["Asia/Jakarta"] = "UTC+7";
    olsonToUtcOffset["Asia/Jayapura"] = "UTC+9";
    olsonToUtcOffset["Asia/Jerusalem"] = "UTC+2";
    olsonToUtcOffset["Asia/Kabul"] = "UTC+4:30";
    olsonToUtcOffset["Asia/Kamchatka"] = "UTC+11";
    olsonToUtcOffset["Asia/Karachi"] = "UTC+5";
    olsonToUtcOffset["Asia/Kashgar"] = "UTC+8";
    olsonToUtcOffset["Asia/Katmandu"] = "UTC+5:45";
    olsonToUtcOffset["Asia/Krasnoyarsk"] = "UTC+6";
    olsonToUtcOffset["Asia/Kuala_Lumpur"] = "UTC+8";
    olsonToUtcOffset["Asia/Kuching"] = "UTC+8";
    olsonToUtcOffset["Asia/Kuwait"] = "UTC+3";
    olsonToUtcOffset["Asia/Macau"] = "UTC+8";
    olsonToUtcOffset["Asia/Magadan"] = "UTC+10";
    olsonToUtcOffset["Asia/Makassar"] = "UTC+8";
    olsonToUtcOffset["Asia/Manila"] = "UTC+9";
    olsonToUtcOffset["Asia/Muscat"] = "UTC+4";
    olsonToUtcOffset["Asia/Nicosia"] = "UTC+2";
    olsonToUtcOffset["Asia/Novosibirsk"] = "UTC+6";
    olsonToUtcOffset["Asia/Omsk"] = "UTC+5";
    olsonToUtcOffset["Asia/Oral"] = "UTC+5";
    olsonToUtcOffset["Asia/Phnom_Penh"] = "UTC+7";
    olsonToUtcOffset["Asia/Pontianak"] = "UTC+7";
    olsonToUtcOffset["Asia/Pyongyang"] = "UTC+9";
    olsonToUtcOffset["Asia/Qatar"] = "UTC+3";
    olsonToUtcOffset["Asia/Qyzylorda"] = "UTC+6";
    olsonToUtcOffset["Asia/Rangoon"] = "UTC+6:30";
    olsonToUtcOffset["Asia/Riyadh"] = "UTC+3";
    olsonToUtcOffset["Asia/Saigon"] = "UTC+7";
    olsonToUtcOffset["Asia/Sakhalin"] = "UTC+11";
    olsonToUtcOffset["Asia/Samarkand"] = "UTC+5";
    olsonToUtcOffset["Asia/Seoul"] = "UTC+9";
    olsonToUtcOffset["Asia/Shanghai"] = "UTC+8";
    olsonToUtcOffset["Asia/Singapore"] = "UTC+8";
    olsonToUtcOffset["Asia/Taipei"] = "UTC+8";
    olsonToUtcOffset["Asia/Tashkent"] = "UTC+5";
    olsonToUtcOffset["Asia/Tbilisi"] = "UTC+4";
    olsonToUtcOffset["Asia/Tehran"] = "UTC+3:30";
    olsonToUtcOffset["Asia/Thimphu"] = "UTC+6";
    olsonToUtcOffset["Asia/Tokyo"] = "UTC+9";
    olsonToUtcOffset["Asia/Ulaanbaatar"] = "UTC+8";
    olsonToUtcOffset["Asia/Urumqi"] = "UTC+8";
    olsonToUtcOffset["Asia/Vientiane"] = "UTC+7";
    olsonToUtcOffset["Asia/Vladivostok"] = "UTC+9";
    olsonToUtcOffset["Asia/Yakutsk"] = "UTC+8";
    olsonToUtcOffset["Asia/Yekaterinburg"] = "UTC+5";
    olsonToUtcOffset["Asia/Yerevan"] = "UTC+4";
    olsonToUtcOffset["Atlantic/Azores"] = "UTC";
    olsonToUtcOffset["Atlantic/Bermuda"] = "UTC-4";
    olsonToUtcOffset["Atlantic/Canary"] = "UTC";
    olsonToUtcOffset["Atlantic/Cape_Verde"] = "UTC-1";
    olsonToUtcOffset["Atlantic/Faroe"] = "UTC";
    olsonToUtcOffset["Atlantic/Madeira"] = "UTC";
    olsonToUtcOffset["Atlantic/Reykjavik"] = "UTC";
    olsonToUtcOffset["Atlantic/South_Georgia"] = "UTC-2";
    olsonToUtcOffset["Atlantic/St_Helena"] = "UTC";
    olsonToUtcOffset["Atlantic/Stanley"] = "UTC-3";
    olsonToUtcOffset["Australia/Adelaide"] = "UTC+9:30";
    olsonToUtcOffset["Australia/Brisbane"] = "UTC+10";
    olsonToUtcOffset["Australia/Broken_Hill"] = "UTC+9:30";
    olsonToUtcOffset["Australia/Currie"] = "UTC+10";
    olsonToUtcOffset["Australia/Darwin"] = "UTC+9:30";
    olsonToUtcOffset["Australia/Eucla"] = "UTC+8:45";
    olsonToUtcOffset["Australia/Hobart"] = "UTC+10";
    olsonToUtcOffset["Australia/Lindeman"] = "UTC+10";
    olsonToUtcOffset["Australia/Lord_Howe"] = "UTC+10:30";
    olsonToUtcOffset["Australia/Melbourne"] = "UTC+10";
    olsonToUtcOffset["Australia/Perth"] = "UTC+8";
    olsonToUtcOffset["Australia/Sydney"] = "UTC+10";
    olsonToUtcOffset["Etc/GMT"] = "UTC";
    olsonToUtcOffset["Etc/GMT-1"] = "UTC-1";
    olsonToUtcOffset["Etc/GMT-2"] = "UTC-2";
    olsonToUtcOffset["Etc/GMT-3"] = "UTC-3";
    olsonToUtcOffset["Etc/GMT-4"] = "UTC-4";
    olsonToUtcOffset["Etc/GMT-5"] = "UTC-5";
    olsonToUtcOffset["Etc/GMT-6"] = "UTC-6";
    olsonToUtcOffset["Etc/GMT-7"] = "UTC-7";
    olsonToUtcOffset["Etc/GMT-8"] = "UTC-8";
    olsonToUtcOffset["Etc/GMT-9"] = "UTC-9";
    olsonToUtcOffset["Etc/GMT-10"] = "UTC-10";
    olsonToUtcOffset["Etc/GMT-11"] = "UTC-11";
    olsonToUtcOffset["Etc/GMT-12"] = "UTC-12";
    olsonToUtcOffset["Etc/GMT-13"] = "UTC-13";
    olsonToUtcOffset["Etc/GMT-14"] = "UTC-14";
    olsonToUtcOffset["Etc/GMT+1"] = "UTC+1";
    olsonToUtcOffset["Etc/GMT+2"] = "UTC+2";
    olsonToUtcOffset["Etc/GMT+3"] = "UTC+3";
    olsonToUtcOffset["Etc/GMT+4"] = "UTC+4";
    olsonToUtcOffset["Etc/GMT+5"] = "UTC+5";
    olsonToUtcOffset["Etc/GMT+6"] = "UTC+6";
    olsonToUtcOffset["Etc/GMT+7"] = "UTC+7";
    olsonToUtcOffset["Etc/GMT+8"] = "UTC+8";
    olsonToUtcOffset["Etc/GMT+9"] = "UTC+9";
    olsonToUtcOffset["Etc/GMT+10"] = "UTC+10";
    olsonToUtcOffset["Etc/GMT+11"] = "UTC+11";
    olsonToUtcOffset["Etc/GMT+12"] = "UTC+12";
    olsonToUtcOffset["Europe/Amsterdam"] = "UTC+1";
    olsonToUtcOffset["Europe/Andorra"] = "UTC+1";
    olsonToUtcOffset["Europe/Athens"] = "UTC+2";
    olsonToUtcOffset["Europe/Belgrade"] = "UTC+1";
    olsonToUtcOffset["Europe/Berlin"] = "UTC+1";
    olsonToUtcOffset["Europe/Bratislava"] = "UTC+1";
    olsonToUtcOffset["Europe/Brussels"] = "UTC+1";
    olsonToUtcOffset["Europe/Bucharest"] = "UTC+2";
    olsonToUtcOffset["Europe/Budapest"] = "UTC+1";
    olsonToUtcOffset["Europe/Chisinau"] = "UTC+2";
    olsonToUtcOffset["Europe/Copenhagen"] = "UTC+1";
    olsonToUtcOffset["Europe/Dublin"] = "UTC+1";
    olsonToUtcOffset["Europe/Gibraltar"] = "UTC+1";
    olsonToUtcOffset["Europe/Guernsey"] = "UTC";
    olsonToUtcOffset["Europe/Helsinki"] = "UTC+2";
    olsonToUtcOffset["Europe/Isle_of_Man"] = "UTC";
    olsonToUtcOffset["Europe/Istanbul"] = "UTC+2";
    olsonToUtcOffset["Europe/Jersey"] = "UTC";
    olsonToUtcOffset["Europe/Kaliningrad"] = "UTC+2";
    olsonToUtcOffset["Europe/Kiev"] = "UTC+2";
    olsonToUtcOffset["Europe/Lisbon"] = "UTC+1";
    olsonToUtcOffset["Europe/Ljubljana"] = "UTC+1";
    olsonToUtcOffset["Europe/London"] = "UTC";
    olsonToUtcOffset["Europe/Luxembourg"] = "UTC+1";
    olsonToUtcOffset["Europe/Madrid"] = "UTC+1";
    olsonToUtcOffset["Europe/Malta"] = "UTC+1";
    olsonToUtcOffset["Europe/Mariehamn"] = "UTC+2";
    olsonToUtcOffset["Europe/Minsk"] = "UTC+2";
    olsonToUtcOffset["Europe/Monaco"] = "UTC+1";
    olsonToUtcOffset["Europe/Moscow"] = "UTC+3";
    olsonToUtcOffset["Europe/Oslo"] = "UTC+1";
    olsonToUtcOffset["Europe/Paris"] = "UTC+1";
    olsonToUtcOffset["Europe/Podgorica"] = "UTC+1";
    olsonToUtcOffset["Europe/Prague"] = "UTC+1";
    olsonToUtcOffset["Europe/Riga"] = "UTC+2";
    olsonToUtcOffset["Europe/Rome"] = "UTC+1";
    olsonToUtcOffset["Europe/Samara"] = "UTC+4";
    olsonToUtcOffset["Europe/San_Marino"] = "UTC+1";
    olsonToUtcOffset["Europe/Sarajevo"] = "UTC+1";
    olsonToUtcOffset["Europe/Simferopol"] = "UTC+2";
    olsonToUtcOffset["Europe/Skopje"] = "UTC+1";
    olsonToUtcOffset["Europe/Sofia"] = "UTC+2";
    olsonToUtcOffset["Europe/Stockholm"] = "UTC+1";
    olsonToUtcOffset["Europe/Tallinn"] = "UTC+2";
    olsonToUtcOffset["Europe/Tirane"] = "UTC+1";
    olsonToUtcOffset["Europe/Uzhgorod"] = "UTC+2";
    olsonToUtcOffset["Europe/Vaduz"] = "UTC+1";
    olsonToUtcOffset["Europe/Vatican"] = "UTC+1";
    olsonToUtcOffset["Europe/Vienna"] = "UTC+1";
    olsonToUtcOffset["Europe/Vilnius"] = "UTC+2";
    olsonToUtcOffset["Europe/Volgograd"] = "UTC+4";
    olsonToUtcOffset["Europe/Warsaw"] = "UTC+1";
    olsonToUtcOffset["Europe/Zagreb"] = "UTC+1";
    olsonToUtcOffset["Europe/Zaporozhye"] = "UTC+2";
    olsonToUtcOffset["Europe/Zurich"] = "UTC+1";
    olsonToUtcOffset["Indian/Antananarivo"] = "UTC+3";
    olsonToUtcOffset["Indian/Chagos"] = "UTC+6";
    olsonToUtcOffset["Indian/Christmas"] = "UTC+7";
    olsonToUtcOffset["Indian/Cocos"] = "UTC+6:30";
    olsonToUtcOffset["Indian/Comoro"] = "UTC+3";
    olsonToUtcOffset["Indian/Kerguelen"] = "UTC+5";
    olsonToUtcOffset["Indian/Mahe"] = "UTC+4";
    olsonToUtcOffset["Indian/Maldives"] = "UTC+5";
    olsonToUtcOffset["Indian/Mauritius"] = "UTC+4";
    olsonToUtcOffset["Indian/Mayotte"] = "UTC+3";
    olsonToUtcOffset["Indian/Reunion"] = "UTC+4";
    olsonToUtcOffset["Pacific/Apia"] = "UTC-11";
    olsonToUtcOffset["Pacific/Auckland"] = "UTC+12";
    olsonToUtcOffset["Pacific/Chatham"] = "UTC+12:45";
    olsonToUtcOffset["Pacific/Easter"] = "UTC-6";
    olsonToUtcOffset["Pacific/Efate"] = "UTC+11";
    olsonToUtcOffset["Pacific/Enderbury"] = "UTC+13";
    olsonToUtcOffset["Pacific/Fakaofo"] = "UTC-10";
    olsonToUtcOffset["Pacific/Fiji"] = "UTC+12";
    olsonToUtcOffset["Pacific/Funafuti"] = "UTC+12";
    olsonToUtcOffset["Pacific/Galapagos"] = "UTC-6";
    olsonToUtcOffset["Pacific/Gambier"] = "UTC-9";
    olsonToUtcOffset["Pacific/Guadalcanal"] = "UTC+11";
    olsonToUtcOffset["Pacific/Guam"] = "UTC+10";
    olsonToUtcOffset["Pacific/Honolulu"] = "UTC-10";
    olsonToUtcOffset["Pacific/Johnston"] = "UTC-10";
    olsonToUtcOffset["Pacific/Kiritimati"] = "UTC+14";
    olsonToUtcOffset["Pacific/Kosrae"] = "UTC+11";
    olsonToUtcOffset["Pacific/Kwajalein"] = "UTC+12";
    olsonToUtcOffset["Pacific/Majuro"] = "UTC+12";
    olsonToUtcOffset["Pacific/Marquesas"] = "UTC-9:30";
    olsonToUtcOffset["Pacific/Midway"] = "UTC-11";
    olsonToUtcOffset["Pacific/Nauru"] = "UTC+12";
    olsonToUtcOffset["Pacific/Niue"] = "UTC-11";
    olsonToUtcOffset["Pacific/Norfolk"] = "UTC+11:30";
    olsonToUtcOffset["Pacific/Noumea"] = "UTC+11";
    olsonToUtcOffset["Pacific/Pago_Pago"] = "UTC-11";
    olsonToUtcOffset["Pacific/Palau"] = "UTC+9";
    olsonToUtcOffset["Pacific/Pitcairn"] = "UTC-8";
    olsonToUtcOffset["Pacific/Ponape"] = "UTC+11";
    olsonToUtcOffset["Pacific/Port_Moresby"] = "UTC+10";
    olsonToUtcOffset["Pacific/Rarotonga"] = "UTC-10";
    olsonToUtcOffset["Pacific/Saipan"] = "UTC+10";
    olsonToUtcOffset["Pacific/Tahiti"] = "UTC-10";
    olsonToUtcOffset["Pacific/Tarawa"] = "UTC+12";
    olsonToUtcOffset["Pacific/Tongatapu"] = "UTC+13";
    olsonToUtcOffset["Pacific/Truk"] = "UTC+10";
    olsonToUtcOffset["Pacific/Wake"] = "UTC+12";
    olsonToUtcOffset["Pacific/Wallis"] = "UTC+12";
  }

  QString utcOffset = olsonToUtcOffset[olsonZone];
  if ( utcOffset.isEmpty() ) {
    kWarning() << "Unknown/invalid olsonZone specified:" << olsonZone;
  }
  return utcOffset;
}

QString TZMaps::utcOffsetToOlson( const QString &utcOffset )
{
  static QHash<QString,QString> utcOffsetToOlson;

  if ( utcOffsetToOlson.isEmpty() ) {
    utcOffsetToOlson["UTC-14"] = "Etc/GMT-14";

    utcOffsetToOlson["UTC-13"] = "Etc/GMT-13";

    utcOffsetToOlson["UTC-12"] = "Etc/GMT-12";

    utcOffsetToOlson["UTC-11"] = "Pacific/Midway";

    utcOffsetToOlson["UTC-10"] = "Pacific/Honolulu";

    utcOffsetToOlson["UTC-9:30"] = "Pacific/Marquesas";

    utcOffsetToOlson["UTC-9"] = "America/Anchorage";

    utcOffsetToOlson["UTC-8"] = "America/Los_Angeles";

    utcOffsetToOlson["UTC-7"] = "America/Denver";

    utcOffsetToOlson["UTC-6"] = "America/Chicago";

    utcOffsetToOlson["UTC-5"] = "America/New_York";

    utcOffsetToOlson["UTC-4:30"] = "America/Caracas";

    utcOffsetToOlson["UTC-4"] = "America/Puerto_Rico";

    utcOffsetToOlson["UTC-3:30"] = "America/St_Johns";

    utcOffsetToOlson["UTC-3"] = "America/Sao_Paulo";

    utcOffsetToOlson["UTC-2"] = "America/Noronha";

    utcOffsetToOlson["UTC-1"] = "Atlantic/Azores";

    utcOffsetToOlson["UTC"] = "Europe/London";

    utcOffsetToOlson["UTC+1"] = "Europe/Berlin";

    utcOffsetToOlson["UTC+2"] = "Europe/Helsinki";

    utcOffsetToOlson["UTC+3"] = "Europe/Moscow";

    utcOffsetToOlson["UTC+3:30"] = "Asia/Tehran";

    utcOffsetToOlson["UTC+4"] = "Asia/Dubai";

    utcOffsetToOlson["UTC+4:30"] = "Asia/Kabul";

    utcOffsetToOlson["UTC+5"] = "Asia/Tashkent";

    utcOffsetToOlson["UTC+5:30"] = "Asia/Kolkata";

    utcOffsetToOlson["UTC+5:45"] = "Asia/Katmandu";

    utcOffsetToOlson["UTC+6"] = "Asia/Karachi";

    utcOffsetToOlson["UTC+6:30"] = "Asia/Rangoon";

    utcOffsetToOlson["UTC+7"] = "Asia/Bangkok";

    utcOffsetToOlson["UTC+8"] = "Asia/Hong_Kong";

    utcOffsetToOlson["UTC+8:45"] = "Australia/Eucla";

    utcOffsetToOlson["UTC+9"] = "Asia/Tokyo";

    utcOffsetToOlson["UTC+9:30"] = "Australia/Adelaide";

    utcOffsetToOlson["UTC+10"] = "Australia/Sydney";

    utcOffsetToOlson["UTC+10:30"] = "Australia/Lord_Howe";

    utcOffsetToOlson["UTC+11"] = "Asia/Magadan";

    utcOffsetToOlson["UTC+11:30"] = "Pacific/Norfolk";

    utcOffsetToOlson["UTC+12"] = "Pacific/Fiji";

    utcOffsetToOlson["UTC+12:45"] = "Pacific/Chatham";

    utcOffsetToOlson["UTC+13"] = "Pacific/Enderbury";

    utcOffsetToOlson["UTC+14"] = "Pacific/Kiritimati";
  }

  QString olsonStr = utcOffsetToOlson[utcOffset.toUpper()];
  if ( olsonStr.isEmpty() ) {
    kWarning() << "Unknown/invalid UTC offset specified:" << utcOffset;
  }
  return olsonStr;
}

QString TZMaps::olsonToWinZone( const QString &olsonZone )
{
  QString winzone;
  QString offsetStr = olsonToUtcOffset( olsonZone );
  if ( offsetStr.isEmpty() ) {
    kWarning() << "Unknown/invalid olsonZone specified:" << olsonZone;
  } else {
    winzone = utcOffsetToWinZone( offsetStr );
  }
  return winzone;
}

QList<QByteArray> TZMaps::utcOffsetToAbbreviation( const QString &utcOffset )
{
  static QHash<QString,QByteArray> utcOffsetToAbbrev;

  if ( utcOffsetToAbbrev.isEmpty() ) {
    utcOffsetToAbbrev["UTC-14"] = "";

    utcOffsetToAbbrev["UTC-13"] = "";

    utcOffsetToAbbrev["UTC-12"] = "BIT";

    utcOffsetToAbbrev["UTC-11"] = "SST";

    utcOffsetToAbbrev["UTC-10"] = "CKT,HAST,TAHT";

    utcOffsetToAbbrev["UTC-9:30"] = "MIT";

    utcOffsetToAbbrev["UTC-9"] = "AKST,GIT";

    utcOffsetToAbbrev["UTC-8"] = "CIST,PST";

    utcOffsetToAbbrev["UTC-7"] = "MST,PDT,THA";

    utcOffsetToAbbrev["UTC-6"] = "CST,EAST,GALT,MDT";

    utcOffsetToAbbrev["UTC-5"] = "CDT,COT,ECT,EST";

    utcOffsetToAbbrev["UTC-4:30"] = "VST";

    utcOffsetToAbbrev["UTC-4"] = "AST,BOT,CLT,COST,ECT,EDT,FKST,GYT";

    utcOffsetToAbbrev["UTC-3:30"] = "NT";

    utcOffsetToAbbrev["UTC-3"] = "ART,BRT,CLST,GFT,UYT";

    utcOffsetToAbbrev["UTC-2"] = "GST,UYST";

    utcOffsetToAbbrev["UTC-1"] = "AZOST,CVT";

    utcOffsetToAbbrev["UTC"] = "GMT,WET";

    utcOffsetToAbbrev["UTC+1"] = "CET,WAT,WEST";

    utcOffsetToAbbrev["UTC+2"] = "CAT,CEST,EET,IST,SAST";

    utcOffsetToAbbrev["UTC+3"] = "AST,EAT,EEST,MSK";

    utcOffsetToAbbrev["UTC+3:30"] = "IRST";

    utcOffsetToAbbrev["UTC+4"] = "AMT,AST,AZT,GET,MUT,RET,SAMT,SCT";

    utcOffsetToAbbrev["UTC+4:30"] = "AFT";

    utcOffsetToAbbrev["UTC+5"] = "AMST,HMT,PKT,YEKT";

    utcOffsetToAbbrev["UTC+5:30"] = "IST,SLT";

    utcOffsetToAbbrev["UTC+5:45"] = "NPT";

    utcOffsetToAbbrev["UTC+6"] = "BIOT,BST,BTT,OMST";

    utcOffsetToAbbrev["UTC+6:30"] = "CCT,MST";

    utcOffsetToAbbrev["UTC+7"] = "CXT,KRAT";

    utcOffsetToAbbrev["UTC+8"] = "ACT,AWST,BDT,CST,HKT,IRKT,MST,PST,SST";

    utcOffsetToAbbrev["UTC+8:45"] = "";

    utcOffsetToAbbrev["UTC+9"] = "JST,KST";

    utcOffsetToAbbrev["UTC+9:30"] = "ACST";

    utcOffsetToAbbrev["UTC+10"] = "AEST,ChST,VLAT";

    utcOffsetToAbbrev["UTC+10:30"] = "LHST";

    utcOffsetToAbbrev["UTC+11"] = "MAGT,SBT";

    utcOffsetToAbbrev["UTC+11:30"] = "NFT";

    utcOffsetToAbbrev["UTC+12"] = "FJT,GILT,PETT";

    utcOffsetToAbbrev["UTC+12:45"] = "CHAST";

    utcOffsetToAbbrev["UTC+13"] = "PHOT";

    utcOffsetToAbbrev["UTC+14"] = "LINT";
  }

  QByteArray abbrev = utcOffsetToAbbrev[utcOffset.toUpper()];
  if ( abbrev.isEmpty() ) {
    kWarning() << "Unknown/invalid UTC offset specified:" << utcOffset;
    return QList<QByteArray>();
  } else {
    return abbrev.split( ',' );
  }
}

