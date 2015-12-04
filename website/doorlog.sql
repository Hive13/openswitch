DROP TABLE IF EXISTS 'doorEvents';
CREATE TABLE IF NOT EXISTS `doorEvents` (
  `pk_DoorEventID` bigint(20) NOT NULL AUTO_INCREMENT,
  `dtEventDate`  timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `bDoorIsOpen` boolean NOT NULL,
  PRIMARY KEY (`pk_DoorEventID`),
  KEY `dtEventDate` (`dtEventDate`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1;