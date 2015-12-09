DROP TABLE IF EXISTS 'tempEvents';
CREATE TABLE IF NOT EXISTS `tempEvents` (
  `pk_TempEventID` bigint(20) NOT NULL AUTO_INCREMENT,
  `dtEventDate`  timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `dcTemp` DECIMAL(10,2) NOT NULL,
  `vcLocation` varchar(512) CHARACTER SET latin1 NULL, 
  PRIMARY KEY (`pk_TempEventID`),
  KEY `dtEventDate` (`dtEventDate`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1;
