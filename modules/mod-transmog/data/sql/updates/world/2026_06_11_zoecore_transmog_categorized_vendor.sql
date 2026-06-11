-- ZoeCore Transmog V3 Categorized Vendor
-- Aplicar no WORLD somente se quiser usar Source = 2 manual.
--
-- A V3 também funciona sem este SQL usando Source = 1, puxando direto do item_template.

CREATE TABLE IF NOT EXISTS `custom_transmog_catalog` (
  `item_entry` INT UNSIGNED NOT NULL,
  `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `category` VARCHAR(64) NOT NULL DEFAULT 'GLOBAL',
  `note` VARCHAR(255) NULL DEFAULT NULL,
  PRIMARY KEY (`item_entry`),
  KEY `idx_enabled_category` (`enabled`, `category`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Catálogo manual amplo por item_template, com ItemLevel >= 60.
INSERT IGNORE INTO `custom_transmog_catalog` (`item_entry`, `enabled`, `category`, `note`)
SELECT `entry`, 1, 'AUTO_ILVL60_PLUS', 'Auto importado do item_template ItemLevel >= 60'
FROM `item_template`
WHERE `class` IN (2,4)
  AND `displayid` > 0
  AND `InventoryType` NOT IN (0,18,24,27,28)
  AND `ItemLevel` >= 60;

-- Para catálogo quase total, remova o filtro ItemLevel manualmente ou rode:
-- INSERT IGNORE INTO `custom_transmog_catalog` (`item_entry`, `enabled`, `category`, `note`)
-- SELECT `entry`, 1, 'AUTO_ALL_VISUAL', 'Auto importado do item_template sem filtro de itemlevel'
-- FROM `item_template`
-- WHERE `class` IN (2,4)
--   AND `displayid` > 0
--   AND `InventoryType` NOT IN (0,18,24,27,28);
