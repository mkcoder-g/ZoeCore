-- ZoeCore Transmog V2 Global Catalog Vendor
-- Aplicar no WORLD.
--
-- Este SQL cria uma tabela opcional para controlar manualmente quais itens aparecem
-- no catálogo global do transmog.
--
-- Se a config estiver:
--   Transmogrification.Zoe.GlobalCatalog.Source = 1
-- o módulo usa item_template diretamente e esta tabela é opcional.
--
-- Se a config estiver:
--   Transmogrification.Zoe.GlobalCatalog.Source = 2
-- o módulo usa esta tabela custom_transmog_catalog.

CREATE TABLE IF NOT EXISTS `custom_transmog_catalog` (
  `item_entry` INT UNSIGNED NOT NULL,
  `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `category` VARCHAR(64) NOT NULL DEFAULT 'GLOBAL',
  `note` VARCHAR(255) NULL DEFAULT NULL,
  PRIMARY KEY (`item_entry`),
  KEY `idx_enabled_category` (`enabled`, `category`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Popular catálogo manual com armas/armaduras elegíveis do item_template.
-- Use apenas se for trabalhar com Source = 2.
INSERT IGNORE INTO `custom_transmog_catalog` (`item_entry`, `enabled`, `category`, `note`)
SELECT `entry`, 1, 'AUTO_ITEM_TEMPLATE', 'Auto importado do item_template'
FROM `item_template`
WHERE `class` IN (2,4)
  AND `displayid` > 0
  AND `Quality` >= 2
  AND `InventoryType` NOT IN (0,18,19,24,27,28);

-- Exemplos para desativar algum item específico:
-- UPDATE `custom_transmog_catalog` SET `enabled` = 0 WHERE `item_entry` IN (12345, 67890);

-- Exemplos para cadastrar manualmente seus sets Cruel custom:
-- INSERT IGNORE INTO `custom_transmog_catalog` (`item_entry`, `enabled`, `category`, `note`) VALUES
-- (900100, 1, 'CRUEL_PLUS0', 'Cruel +0 Warrior Head'),
-- (900101, 1, 'CRUEL_PLUS1', 'Cruel +1 Warrior Head');
