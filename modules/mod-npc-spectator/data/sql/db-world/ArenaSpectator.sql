-- ZoeCore Espectador Arena V1 - WORLD DB
-- Rode no banco world.
-- NPC Espectador Arena: 190000
-- Uso: .npc add 190000

DELETE FROM `creature_template` WHERE `entry`=190000;
INSERT INTO `creature_template`
(`entry`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`,
 `speed_walk`, `speed_run`, `rank`, `dmgschool`, `baseattacktime`, `rangeattacktime`, `unit_class`,
 `unit_flags`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `AIName`, `MovementType`,
 `HoverHeight`, `RacialLeader`, `movementId`, `RegenHealth`, `flags_extra`, `ScriptName`, `VerifiedBuild`)
VALUES
(190000, 'Espectador Arena', 'ZoeCore', 'Speak', 0, 80, 80, 2, 35, 3,
 1, 1.14286, 0, 0, 2000, 0, 1,
 0, 7, 138936390, 0, 0, 0, '', 0,
 1, 0, 0, 1, 16777216, 'ArenaSpectatorNPC_Creature', 0);

DELETE FROM `creature_template_model` WHERE `CreatureID` = 190000;
INSERT INTO `creature_template_model`
(`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`)
VALUES
(190000, 0, 29348, 1, 1, 0);

SET @NPC_TEXT_SPECTATOR = 'As listas do menu mostram todas as arenas em andamento. Para assistir, escolha uma partida. Tambem e possivel assistir um jogador especifico digitando o nome dele.$B$BComando para sair do espectador:$B.spectate leave$B$BPara assistir pela camera de um jogador use:$B.spectate watch nome/target$B$BVoce pode digitar o nome ou selecionar o alvo.$BSem alvo, a camera volta para voce.$B$BLembrete: enquanto estiver espectando, o chat /say fica bloqueado. Use guild/party/outros canais para comandos.';
DELETE FROM `npc_text` WHERE `id`=190017;
INSERT INTO `npc_text` (`id`, `text0_0`, `text0_1`, `Probability0`)
VALUES
(190017, @NPC_TEXT_SPECTATOR, @NPC_TEXT_SPECTATOR, 1);
