/*
 * Copyright (С) since 2019 Andrei Guluaev (Winfidonarleyan/Kargatum) https://github.com/Winfidonarleyan
 * Copyright (С) since 2019+ AzerothCore <www.azerothcore.org>
 * Licence MIT https://opensource.org/MIT
 */

#include "CFBG.h"
#include "BattlegroundQueue.h"
#include "BattlegroundUtils.h"
#include "Chat.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Containers.h"
#include "Language.h"
#include "Opcodes.h"
#include "ReputationMgr.h"
#include "ScriptMgr.h"
#include "GameTime.h"
#include "Item.h"
#include "Log.h"
#include "Player.h"
#include "WorldSessionMgr.h"

constexpr uint32 MapAlteracValley = 30;

CrossFactionGroupInfo::CrossFactionGroupInfo(GroupQueueInfo* groupInfo)
{
    uint32 sumLevels = 0;
    uint32 sumAverageItemLevels = 0;
    uint32 playersCount = 0;

    for (auto const& playerGuid : groupInfo->Players)
    {
        auto player = ObjectAccessor::FindConnectedPlayer(playerGuid);
        if (!player)
            continue;

        if (player->getClass() == CLASS_HUNTER && !IsHunterJoining)
            IsHunterJoining = true;

        sumLevels += player->GetLevel();
        sumAverageItemLevels += player->GetAverageItemLevel();
        playersCount++;

        SumAverageItemLevel += player->GetAverageItemLevel();
        SumPlayerLevel += player->GetLevel();
    }

    if (!playersCount)
        return;

    AveragePlayersLevel = sumLevels / playersCount;
    AveragePlayersItemLevel = sumAverageItemLevels / playersCount;
}

CrossFactionQueueInfo::CrossFactionQueueInfo(BattlegroundQueue* bgQueue)
{
    auto FillStats = [this, bgQueue](TeamId team)
    {
        for (auto const& groupInfo : bgQueue->m_SelectionPools[team].SelectedGroups)
        {
            for (auto const& playerGuid : groupInfo->Players)
            {
                auto player = ObjectAccessor::FindConnectedPlayer(playerGuid);
                if (!player)
                    continue;

                SumAverageItemLevel[team] += player->GetAverageItemLevel();
                SumPlayerLevel[team] += player->GetLevel();
                PlayersCount[team]++;
            }
        }
    };

    FillStats(TEAM_ALLIANCE);
    FillStats(TEAM_HORDE);
}

TeamId CrossFactionQueueInfo::GetLowerTeamIdInBG(GroupQueueInfo* groupInfo)
{
    // Formation path: no BG instance exists yet, so all aggregates come from the
    // queue tallies. Route through the shared cascade (CFBG::ResolveBalancedTeam).
    auto cfGroupInfo = CrossFactionGroupInfo(groupInfo);

    TeamBalanceContext ctx;
    ctx.countA = PlayersCount.at(TEAM_ALLIANCE);
    ctx.countH = PlayersCount.at(TEAM_HORDE);

    ctx.levelSumA = SumPlayerLevel.at(TEAM_ALLIANCE);
    ctx.levelSumH = SumPlayerLevel.at(TEAM_HORDE);

    // Fold the candidate's level sum into its projected side.
    if (groupInfo->teamId == TEAM_ALLIANCE)
        ctx.levelSumA += cfGroupInfo.SumPlayerLevel;
    else
        ctx.levelSumH += cfGroupInfo.SumPlayerLevel;

    ctx.avgIlvlA = SumAverageItemLevel.at(TEAM_ALLIANCE);
    ctx.avgIlvlH = SumAverageItemLevel.at(TEAM_HORDE);

    ctx.evenCountA = PlayersCount.at(TEAM_ALLIANCE);
    ctx.evenCountH = PlayersCount.at(TEAM_HORDE);

    // Formation skips the hunter override (no live BG to count hunters against).
    ctx.hunterOverride = std::nullopt;
    ctx.fallback = groupInfo->teamId;

    return sCFBG->ResolveBalancedTeam(ctx);
}

CFBG::CFBG()
{
    _raceData =
    {
        RaceData{ CLASS_NONE,           { 0 }, { 0 } },
        RaceData{ CLASS_WARRIOR,        { RACE_HUMAN, RACE_DWARF, RACE_GNOME, RACE_DRAENEI  }, { RACE_ORC, RACE_TAUREN, RACE_TROLL } },
        RaceData{ CLASS_PALADIN,        { RACE_HUMAN, RACE_DWARF, RACE_DRAENEI }, { RACE_BLOODELF } },
        RaceData{ CLASS_HUNTER,         { RACE_DWARF, RACE_DRAENEI }, { RACE_ORC, RACE_TAUREN, RACE_TROLL, RACE_BLOODELF } },
        RaceData{ CLASS_ROGUE,          { RACE_HUMAN, RACE_DWARF, RACE_GNOME }, { RACE_ORC, RACE_TROLL, RACE_BLOODELF } },
        RaceData{ CLASS_PRIEST,         { RACE_HUMAN, RACE_DWARF, RACE_DRAENEI  }, { RACE_TROLL, RACE_BLOODELF } },
        RaceData{ CLASS_DEATH_KNIGHT,   { RACE_HUMAN, RACE_DWARF, RACE_GNOME, RACE_DRAENEI }, { RACE_ORC, RACE_TAUREN, RACE_TROLL, RACE_BLOODELF } },
        RaceData{ CLASS_SHAMAN,         { RACE_DRAENEI }, { RACE_ORC, RACE_TAUREN, RACE_TROLL  } },
        RaceData{ CLASS_MAGE,           { RACE_HUMAN, RACE_GNOME }, { RACE_BLOODELF, RACE_TROLL } },
        RaceData{ CLASS_WARLOCK,        { RACE_HUMAN, RACE_GNOME }, { RACE_ORC, RACE_BLOODELF } },
        RaceData{ CLASS_NONE,           { 0 }, { 0 } },
        RaceData{ CLASS_DRUID,          { RACE_HUMAN }, { RACE_TAUREN } }
    };

    _raceInfo =
    {
        CFBGRaceInfo{ RACE_HUMAN,    "human",    TEAM_HORDE    },
        CFBGRaceInfo{ RACE_NIGHTELF, "nightelf", TEAM_HORDE    },
        CFBGRaceInfo{ RACE_DWARF,    "dwarf",    TEAM_HORDE    },
        CFBGRaceInfo{ RACE_GNOME,    "gnome",    TEAM_HORDE    },
        CFBGRaceInfo{ RACE_DRAENEI,  "draenei",  TEAM_HORDE    },
        CFBGRaceInfo{ RACE_ORC,      "orc",      TEAM_ALLIANCE },
        CFBGRaceInfo{ RACE_BLOODELF, "bloodelf", TEAM_ALLIANCE },
        CFBGRaceInfo{ RACE_TROLL,    "troll",    TEAM_ALLIANCE },
        CFBGRaceInfo{ RACE_TAUREN,   "tauren",   TEAM_ALLIANCE }
    };
}

CFBG* CFBG::instance()
{
    static CFBG instance;
    return &instance;
}

void CFBG::LoadConfig()
{
    _IsEnableSystem = sConfigMgr->GetOption<bool>("CFBG.Enable", false);
    if (!_IsEnableSystem)
        return;

    _IsEnableWGSystem = sConfigMgr->GetOption<bool>("CFBG.Battlefield.Enable", true);
    _IsEnableWGTeamLock = sConfigMgr->GetOption<bool>("CFBG.Battlefield.TeamLock.Enable", true);
    _IsEnableWGReapplyOnResurrect = sConfigMgr->GetOption<bool>("CFBG.Battlefield.ReapplyOnResurrect.Enable", true);
    _IsEnableAvgIlvl = sConfigMgr->GetOption<bool>("CFBG.Include.Avg.Ilvl.Enable", false);
    _IsEnableBalancedTeams = sConfigMgr->GetOption<bool>("CFBG.BalancedTeams", false);
    _IsEnableEvenTeams = sConfigMgr->GetOption<bool>("CFBG.EvenTeams.Enabled", false);
    _IsEnableBalanceClassLowLevel = sConfigMgr->GetOption<bool>("CFBG.BalancedTeams.Class.LowLevel", true);
    _IsEnableResetCooldowns = sConfigMgr->GetOption<bool>("CFBG.ResetCooldowns", false);
    _IsEnableBalanceTeamsOnEntry = sConfigMgr->GetOption<bool>("CFBG.BalanceTeamsOnEntry.Enabled", true);
    _showPlayerName = sConfigMgr->GetOption<bool>("CFBG.Show.PlayerName", false);
    _EvenTeamsMaxPlayersThreshold = sConfigMgr->GetOption<uint32>("CFBG.EvenTeams.MaxPlayersThreshold", 0);
    _MaxPlayersCountInGroup = sConfigMgr->GetOption<uint32>("CFBG.Players.Count.In.Group", 3);
    _balanceClassMinLevel = sConfigMgr->GetOption<uint8>("CFBG.BalancedTeams.Class.MinLevel", 10);
    _balanceClassMaxLevel = sConfigMgr->GetOption<uint8>("CFBG.BalancedTeams.Class.MaxLevel", 19);
    _balanceClassLevelDiff = sConfigMgr->GetOption<uint8>("CFBG.BalancedTeams.Class.LevelDiff", 2);
    _randomizeRaces = sConfigMgr->GetOption<bool>("CFBG.RandomRaceSelection", true);

    _ZoeGearBracketEnable = sConfigMgr->GetOption<bool>("CFBG.ZoeGearBracket.Enable", false);
    _ZoeGearBracketMode = sConfigMgr->GetOption<uint8>("CFBG.ZoeGearBracket.Mode", 1);
    _ZoeGearBracketUseSqlItemTierTable = sConfigMgr->GetOption<bool>("CFBG.ZoeGearBracket.UseSqlItemTierTable", true);
    _ZoeGearBracketNormalMaxTier = sConfigMgr->GetOption<uint8>("CFBG.ZoeGearBracket.Normal.MaxTier", 1);
    _ZoeGearBracketAdvancedMaxTier = sConfigMgr->GetOption<uint8>("CFBG.ZoeGearBracket.Advanced.MaxTier", 3);
    _ZoeGearBracketEliteMaxTier = sConfigMgr->GetOption<uint8>("CFBG.ZoeGearBracket.Elite.MaxTier", 4);
    _ZoeGearBracketAllowWeaponSwap = sConfigMgr->GetOption<bool>("CFBG.ZoeGearBracket.AllowWeaponSwap", true);
    _ZoeGearBracketLockEquipmentInsideBG = sConfigMgr->GetOption<bool>("CFBG.ZoeGearBracket.LockEquipmentInsideBG", true);
    _ZoeGearBracketPeriodicCheckEnable = sConfigMgr->GetOption<bool>("CFBG.ZoeGearBracket.PeriodicCheck.Enable", true);
    _ZoeGearBracketPeriodicCheckMs = sConfigMgr->GetOption<uint32>("CFBG.ZoeGearBracket.PeriodicCheck.Seconds", 5) * IN_MILLISECONDS;
    _ZoeGearBracketLowPopulationFallback = sConfigMgr->GetOption<bool>("CFBG.ZoeGearBracket.LowPopulationFallback", true);
    _ZoeGearBracketFallbackWaitMinutes = sConfigMgr->GetOption<uint32>("CFBG.ZoeGearBracket.FallbackWaitMinutes", 5);
    _ZoeGearBracketDisableEquipmentLockOnFallback = sConfigMgr->GetOption<bool>("CFBG.ZoeGearBracket.DisableEquipmentLockOnFallback", false);
    _ZoeGearBracketAnnounce = sConfigMgr->GetOption<bool>("CFBG.ZoeGearBracket.Announce", true);
    _ZoeGearBracketLogEnable = sConfigMgr->GetOption<bool>("CFBG.ZoeGearBracket.Log.Enable", true);
    _ZoeGearBracketQueueAnnouncerEnable = sConfigMgr->GetOption<bool>("CFBG.ZoeGearBracket.QueueAnnouncer.Enable", true);
    _ZoeGearBracketQueueAnnouncerShowBracket = sConfigMgr->GetOption<bool>("CFBG.ZoeGearBracket.QueueAnnouncer.ShowBracket", true);
    _ZoeGearBracketQueueAnnouncerUseBracketPlayerCount = sConfigMgr->GetOption<bool>("CFBG.ZoeGearBracket.QueueAnnouncer.UseBracketPlayerCount", true);
    _ZoeGearBracketMessagePrefix = sConfigMgr->GetOption<std::string>("CFBG.ZoeGearBracket.MessagePrefix", "[ZoeCore BG]");

    if (_ZoeGearBracketEnable && _ZoeGearBracketUseSqlItemTierTable)
        LoadZoeGearBracketItemTiers();
}


void CFBG::LoadZoeGearBracketItemTiers()
{
    _zoeItemTierStore.clear();

    QueryResult result = WorldDatabase.Query("SELECT `entry`, `tier` FROM `custom_bg_item_tier` WHERE `enabled` = 1");
    if (!result)
    {
        LOG_WARN("module", "CFBG ZoeGearBracket: tabela custom_bg_item_tier vazia ou inexistente. O bloqueio por gear ficara inativo.");
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        uint32 entry = fields[0].Get<uint32>();
        uint8 tier = fields[1].Get<uint8>();

        _zoeItemTierStore[entry] = tier;
    }
    while (result->NextRow());

    LOG_INFO("module", "CFBG ZoeGearBracket: carregados {} itens custom na tabela custom_bg_item_tier.", _zoeItemTierStore.size());
}

uint8 CFBG::GetZoeItemTier(uint32 itemEntry) const
{
    auto itr = _zoeItemTierStore.find(itemEntry);
    if (itr == _zoeItemTierStore.end())
        return 255;

    return itr->second;
}

uint8 CFBG::GetZoeMaxTierForBracket(uint8 bracket) const
{
    switch (bracket)
    {
        case 1:
            return _ZoeGearBracketNormalMaxTier;
        case 2:
            return _ZoeGearBracketAdvancedMaxTier;
        case 3:
            return _ZoeGearBracketEliteMaxTier;
        default:
            return _ZoeGearBracketNormalMaxTier;
    }
}

char const* CFBG::GetZoeBracketName(uint8 bracket) const
{
    switch (bracket)
    {
        case 1:
            return "Normal +0/+1";
        case 2:
            return "Advanced +2/+3";
        case 3:
            return "Elite +4";
        default:
            return "Normal +0/+1";
    }
}

std::string CFBG::GetZoeBracketColoredName(uint8 bracket) const
{
    switch (bracket)
    {
        case 1:
            return "|cff00ff00[Normal]|r";
        case 2:
            return "|cffffcc00[Advanced]|r";
        case 3:
            return "|cffff0000[Elite]|r";
        default:
            return "|cff00ff00[Normal]|r";
    }
}

std::string CFBG::GetZoeBracketAllowedTierText(uint8 bracket) const
{
    switch (bracket)
    {
        case 1:
            return "Cruel +0/+1";
        case 2:
            return "Cruel +0/+1/+2/+3";
        case 3:
            return "Cruel +0/+1/+2/+3/+4";
        default:
            return "Cruel +0/+1";
    }
}

std::string CFBG::GetZoeBracketQueueDescription(uint8 bracket) const
{
    switch (bracket)
    {
        case 1:
            return "Voce entrou na fila NORMAL para jogadores iniciantes/intermediarios. Protecao ativa contra itens +2/+3/+4.";
        case 2:
            return "Voce entrou na fila ADVANCED para jogadores +2/+3. Jogadores +0/+1 ficam protegidos em outra bracket quando houver fila suficiente.";
        case 3:
            return "Voce entrou na fila ELITE para jogadores +4. Esta bracket e separada para manter o PvP justo.";
        default:
            return "Voce entrou na fila NORMAL.";
    }
}

std::string CFBG::BuildZoeQueueAnnouncerMessage(std::string const& bgName, uint32 minLevel, uint32 maxLevel, uint32 queuedPlayers, uint32 requiredPlayers, uint8 bracket) const
{
    std::string message = "|cffff0000[BG Queue]:|r " + bgName +
        " -- [" + std::to_string(minLevel) + "-" + std::to_string(maxLevel) + "]" +
        " [" + std::to_string(queuedPlayers) + "/" + std::to_string(requiredPlayers) + "]";

    if (_ZoeGearBracketEnable && _ZoeGearBracketQueueAnnouncerShowBracket && bracket >= 1 && bracket <= 3)
        message += " - " + GetZoeBracketColoredName(bracket);

    return message;
}

void CFBG::LogZoeGearBracket(Player* player, std::string const& action, uint8 bracket, std::string const& details) const
{
    if (!_ZoeGearBracketLogEnable || !player)
        return;

    std::string extra = details.empty() ? std::string() : std::string(" | ") + details;

    LOG_INFO("module", "CFBG ZoeGearBracket: {} | player='{}' | bracket='{}' | maxTier=+{}{}",
        action, player->GetName(), GetZoeBracketName(bracket), GetZoeMaxTierForBracket(bracket), extra);
}

void CFBG::SendZoeGearBracketMessage(Player* player, std::string const& message) const
{
    if (!player || !_ZoeGearBracketAnnounce || !player->GetSession())
        return;

    std::string fullMessage = _ZoeGearBracketMessagePrefix + " " + message;
    ChatHandler(player->GetSession()).SendSysMessage(fullMessage.c_str());
}

bool CFBG::IsZoeGearBracketQueueSplitEnabled() const
{
    return _ZoeGearBracketEnable && _ZoeGearBracketMode >= 3;
}

uint8 CFBG::CalculateZoeGroupBracket(GroupQueueInfo const* groupInfo) const
{
    if (!groupInfo)
        return 1;

    uint8 groupBracket = 1;

    for (ObjectGuid const& playerGuid : groupInfo->Players)
    {
        Player* player = ObjectAccessor::FindConnectedPlayer(playerGuid);
        if (!player)
            continue;

        uint8 playerBracket = CalculateZoePlayerBracket(player);
        if (playerBracket > groupBracket)
            groupBracket = playerBracket;
    }

    return groupBracket;
}

uint32 CFBG::CountZoeQueuedPlayersByBracket(BattlegroundQueue* bgQueue, BattlegroundBracketId bracketId, uint8 bracket) const
{
    if (!bgQueue || bracket < 1 || bracket > 3)
        return 0;

    uint32 count = 0;

    for (GroupQueueInfo const* groupInfo : bgQueue->m_QueuedGroups[bracketId][BG_QUEUE_CFBG])
    {
        if (!groupInfo || groupInfo->IsInvitedToBGInstanceGUID)
            continue;

        if (CalculateZoeGroupBracket(groupInfo) == bracket)
            count += groupInfo->Players.size();
    }

    return count;
}

uint8 CFBG::SelectZoeQueueBracket(GroupsList const& groups, uint32 minPlayersPerTeam, bool& fallback) const
{
    fallback = false;

    if (!IsZoeGearBracketQueueSplitEnabled())
        return 0;

    std::array<uint32, 4> playersByBracket{};
    uint32 totalPlayers = 0;

    for (GroupQueueInfo const* groupInfo : groups)
    {
        if (!groupInfo || groupInfo->IsInvitedToBGInstanceGUID)
            continue;

        uint8 bracket = CalculateZoeGroupBracket(groupInfo);
        if (bracket < 1 || bracket > 3)
            bracket = 1;

        uint32 count = groupInfo->Players.size();
        playersByBracket[bracket] += count;
        totalPlayers += count;
    }

    uint32 requiredPlayers = minPlayersPerTeam * 2;

    // Preferir bracket mais fraca primeiro para proteger novatos.
    // Se Normal tiver jogadores suficientes, inicia Normal separado.
    for (uint8 bracket = 1; bracket <= 3; ++bracket)
    {
        if (playersByBracket[bracket] >= requiredPlayers)
        {
            if (_ZoeGearBracketLogEnable)
                LOG_INFO("module", "CFBG ZoeGearBracket: fila separada pronta para '{}' | players={} | required={}.",
                    GetZoeBracketName(bracket), playersByBracket[bracket], requiredPlayers);

            return bracket;
        }
    }

    // Fallback de baixa população: só mistura se explicitamente ligado.
    if (_ZoeGearBracketLowPopulationFallback && totalPlayers >= requiredPlayers)
    {
        fallback = true;

        if (_ZoeGearBracketLogEnable)
            LOG_WARN("module", "CFBG ZoeGearBracket: fallback de baixa populacao ativado | totalPlayers={} | required={}. Filas podem ser misturadas temporariamente.",
                totalPlayers, requiredPlayers);

        return 0; // 0 = misto/liberado
    }

    return 255; // 255 = nao iniciar ainda
}

CFBG::GroupsList CFBG::FilterZoeGroupsByBracket(GroupsList const& groups, uint8 bracket) const
{
    if (!IsZoeGearBracketQueueSplitEnabled() || bracket == 0)
        return groups;

    GroupsList filtered;
    for (GroupQueueInfo* groupInfo : groups)
    {
        if (!groupInfo)
            continue;

        if (CalculateZoeGroupBracket(groupInfo) == bracket)
            filtered.emplace_back(groupInfo);
    }

    return filtered;
}

uint8 CFBG::GetZoeBattlegroundBracket(Battleground* bg) const
{
    if (!bg)
        return 0;

    auto itr = _zoeBattlegroundBracketStore.find(bg->GetInstanceID());
    if (itr == _zoeBattlegroundBracketStore.end())
        return 0;

    return itr->second;
}

void CFBG::SetZoeBattlegroundBracket(Battleground* bg, Player* player)
{
    if (!IsZoeGearBracketQueueSplitEnabled() || !bg || !player || bg->isArena())
        return;

    uint32 instanceId = bg->GetInstanceID();
    uint8 playerBracket = GetZoePlayerBracket(player);

    auto itr = _zoeBattlegroundBracketStore.find(instanceId);
    if (itr == _zoeBattlegroundBracketStore.end())
    {
        _zoeBattlegroundBracketStore[instanceId] = playerBracket;

        LOG_INFO("module", "CFBG ZoeGearBracket: BG instance {} marcada como '{}' pelo player '{}'.",
            instanceId, GetZoeBracketName(playerBracket), player->GetName());

        SendZoeGearBracketMessage(player, "Esta Battleground foi aberta como " + std::string(GetZoeBracketName(playerBracket)) + ". Limite de equipamento: " + GetZoeBracketAllowedTierText(playerBracket) + ".");
        return;
    }

    // Segurança: se por algum motivo um jogador de bracket maior cair numa BG
    // mais fraca, força o lock dele para a bracket da BG.
    if (playerBracket != itr->second)
    {
        LOG_WARN("module", "CFBG ZoeGearBracket: player '{}' entrou em BG instance {} com bracket '{}' mas a BG esta marcada como '{}'. Aplicando lock da BG.",
            player->GetName(), instanceId, GetZoeBracketName(playerBracket), GetZoeBracketName(itr->second));

        _zoePlayerBracketStore[player->GetGUID()] = itr->second;
        SendZoeGearBracketMessage(player, "Sua categoria foi ajustada para a bracket desta Battleground: " + std::string(GetZoeBracketName(itr->second)) + ". Limite: " + GetZoeBracketAllowedTierText(itr->second) + ".");
    }
}

void CFBG::ClearZoeBattlegroundBracket(Battleground* bg)
{
    if (!bg)
        return;

    _zoeBattlegroundBracketStore.erase(bg->GetInstanceID());
}

uint8 CFBG::CalculateZoePlayerBracket(Player* player) const
{
    if (!player)
        return 1;

    uint8 maxTier = 0;

    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!item)
            continue;

        uint8 tier = GetZoeItemTier(item->GetEntry());
        if (tier == 255)
            continue;

        if (tier > maxTier)
            maxTier = tier;
    }

    if (maxTier <= _ZoeGearBracketNormalMaxTier)
        return 1;

    if (maxTier <= _ZoeGearBracketAdvancedMaxTier)
        return 2;

    return 3;
}

uint8 CFBG::GetZoePlayerBracket(Player* player) const
{
    if (!player)
        return 1;

    auto itr = _zoePlayerBracketStore.find(player->GetGUID());
    if (itr != _zoePlayerBracketStore.end())
        return itr->second;

    return CalculateZoePlayerBracket(player);
}

void CFBG::SetZoePlayerBracket(Player* player, bool announce)
{
    if (!_ZoeGearBracketEnable || !player)
        return;

    uint8 originalBracket = CalculateZoePlayerBracket(player);
    uint8 bracket = originalBracket;
    bool forcedByBgBracket = false;

    // Se a BG ja foi marcada como Normal/Advanced/Elite, todos dentro dela
    // obedecem a mesma bracket. Isso evita +4 cair em BG Normal e manter lock Elite.
    if (IsZoeGearBracketQueueSplitEnabled())
    {
        if (Battleground* bg = player->GetBattleground())
        {
            uint8 bgBracket = GetZoeBattlegroundBracket(bg);
            if (bgBracket >= 1 && bgBracket <= 3)
            {
                forcedByBgBracket = (bgBracket != bracket);
                bracket = bgBracket;
            }
        }
    }

    _zoePlayerBracketStore[player->GetGUID()] = bracket;

    LogZoeGearBracket(player, forcedByBgBracket ? "lock aplicado pela bracket da BG" : "classificado para fila/BG", bracket,
        "original=" + std::string(GetZoeBracketName(originalBracket)) + " | allowed=" + GetZoeBracketAllowedTierText(bracket));

    if (announce)
    {
        SendZoeGearBracketMessage(player, "Classificacao de fila: " + std::string(GetZoeBracketName(bracket)) + ". " + GetZoeBracketQueueDescription(bracket));
        SendZoeGearBracketMessage(player, "Protecao ativa: dentro desta BG somente " + GetZoeBracketAllowedTierText(bracket) + " podem ser equipados.");

        if (forcedByBgBracket)
            SendZoeGearBracketMessage(player, "A categoria desta BG ja estava definida. Seu limite temporario foi ajustado para " + std::string(GetZoeBracketName(bracket)) + ".");
    }
}

void CFBG::ClearZoePlayerBracket(Player* player)
{
    if (!player)
        return;

    _zoePlayerBracketStore.erase(player->GetGUID());
}

bool CFBG::CanEquipZoeGearBracketItem(Player* player, Item* item)
{
    if (!_ZoeGearBracketEnable || !_ZoeGearBracketLockEquipmentInsideBG || !player || !item)
        return true;

    Battleground* bg = player->GetBattleground();
    if (!bg || bg->isArena())
        return true;

    uint8 itemTier = GetZoeItemTier(item->GetEntry());
    if (itemTier == 255)
        return true; // Item nao mapeado como Cruel; nao bloqueia.

    uint8 bracket = GetZoePlayerBracket(player);
    uint8 maxAllowedTier = GetZoeMaxTierForBracket(bracket);

    if (itemTier <= maxAllowedTier)
        return true;

    SendZoeGearBracketMessage(player, "Troca bloqueada: este item Cruel +" + std::to_string(itemTier) + " pertence a uma bracket superior. Sua BG atual permite somente " + GetZoeBracketAllowedTierText(bracket) + ".");
    LogZoeGearBracket(player, "equip bloqueado", bracket, "itemEntry=" + std::to_string(item->GetEntry()) + " | itemTier=+" + std::to_string(itemTier));

    return false;
}

void CFBG::AuditZoeGearBracketEquipment(Player* player, bool notify)
{
    if (!_ZoeGearBracketEnable || !_ZoeGearBracketPeriodicCheckEnable || !player)
        return;

    Battleground* bg = player->GetBattleground();
    if (!bg || bg->isArena())
        return;

    uint8 bracket = GetZoePlayerBracket(player);
    uint8 maxAllowedTier = GetZoeMaxTierForBracket(bracket);

    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!item)
            continue;

        uint8 itemTier = GetZoeItemTier(item->GetEntry());
        if (itemTier == 255)
            continue;

        if (itemTier > maxAllowedTier)
        {
            if (notify)
                SendZoeGearBracketMessage(player, "Auditoria anti-bypass: item Cruel +" + std::to_string(itemTier) + " invalido para esta BG. Limite atual: " + GetZoeBracketAllowedTierText(bracket) + ".");

            LogZoeGearBracket(player, "auditoria detectou item invalido", bracket, "itemEntry=" + std::to_string(item->GetEntry()) + " | itemTier=+" + std::to_string(itemTier));
            return;
        }
    }
}


uint32 CFBG::GetBGTeamAverageItemLevel(Battleground* bg, TeamId team)
{
    if (!bg)
    {
        return 0;
    }

    uint32 sum = 0;
    uint32 count = 0;

    for (auto const& [playerGuid, player] : bg->GetPlayers())
    {
        if (player && player->GetTeamId() == team)
        {
            sum += player->GetAverageItemLevel();
            count++;
        }
    }

    if (!count || !sum)
    {
        return 0;
    }

    return sum / count;
}

uint32 CFBG::GetBGTeamSumPlayerLevel(Battleground* bg, TeamId team)
{
    if (!bg)
    {
        return 0;
    }

    uint32 sum = 0;

    for (auto const& [playerGuid, player] : bg->GetPlayers())
    {
        if (player && player->GetTeamId() == team)
        {
            sum += player->GetLevel();
        }
    }

    return sum;
}

TeamId CFBG::GetLowerTeamIdInBG(Battleground* bg, BattlegroundQueue* bgQueue, GroupQueueInfo* groupInfo)
{
    // Reinforcement path: a BG instance exists. Aggregate live in-BG counts with
    // the pending-queue tallies, then route through the shared cascade.
    auto queueInfo = CrossFactionQueueInfo{ bgQueue };
    auto cfGroupInfo = CrossFactionGroupInfo(groupInfo);

    TeamBalanceContext ctx;
    ctx.countA = bg->GetPlayersCountByTeam(TEAM_ALLIANCE) + queueInfo.PlayersCount.at(TEAM_ALLIANCE);
    ctx.countH = bg->GetPlayersCountByTeam(TEAM_HORDE) + queueInfo.PlayersCount.at(TEAM_HORDE);

    ctx.levelSumA = GetBGTeamSumPlayerLevel(bg, TEAM_ALLIANCE) + queueInfo.SumPlayerLevel.at(TEAM_ALLIANCE);
    ctx.levelSumH = GetBGTeamSumPlayerLevel(bg, TEAM_HORDE) + queueInfo.SumPlayerLevel.at(TEAM_HORDE);

    // Fold the candidate's level sum into its projected side.
    if (groupInfo->teamId == TEAM_ALLIANCE)
        ctx.levelSumA += cfGroupInfo.SumPlayerLevel;
    else
        ctx.levelSumH += cfGroupInfo.SumPlayerLevel;

    ctx.avgIlvlA = GetBGTeamAverageItemLevel(bg, TEAM_ALLIANCE);
    ctx.avgIlvlH = GetBGTeamAverageItemLevel(bg, TEAM_HORDE);

    ctx.evenCountA = bg->GetPlayersCountByTeam(TEAM_ALLIANCE) + queueInfo.PlayersCount.at(TEAM_ALLIANCE);
    ctx.evenCountH = bg->GetPlayersCountByTeam(TEAM_HORDE) + queueInfo.PlayersCount.at(TEAM_HORDE);

    ctx.hunterOverride = ResolveHunterOverride(bg, cfGroupInfo);
    ctx.fallback = groupInfo->teamId;

    return ResolveBalancedTeam(ctx);
}

std::optional<TeamId> CFBG::ResolveHunterOverride(Battleground* bg, CrossFactionGroupInfo const& cfGroupInfo)
{
    if (!IsEnableEvenTeams())
        return std::nullopt;

    uint32 playerLevel = cfGroupInfo.AveragePlayersLevel;

    // if CFBG.BalancedTeams.Class.LowLevel is enabled, balance the quantity of
    // hunters per team when a hunter is joining within the configured level band.
    if (IsEnableBalanceClassLowLevel() &&
        (playerLevel >= _balanceClassMinLevel && playerLevel <= _balanceClassMaxLevel) &&
        (playerLevel >= getBalanceClassMinLevel(bg)) &&
        cfGroupInfo.IsHunterJoining)
    {
        return getTeamWithLowerClass(bg, CLASS_HUNTER);
    }

    return std::nullopt;
}

TeamId CFBG::ResolveBalancedTeam(TeamBalanceContext const& ctx)
{
    // 1. Head-count: the smaller side always wins.
    if (ctx.countA != ctx.countH)
        return ctx.countA < ctx.countH ? TEAM_ALLIANCE : TEAM_HORDE;

    // 2. Level sum (only if CFBG.BalancedTeams).
    if (IsEnableBalancedTeams())
    {
        TeamId team = ctx.fallback;

        // First select team - where the sum of the levels is less
        if (ctx.levelSumA != ctx.levelSumH)
            team = ctx.levelSumA < ctx.levelSumH ? TEAM_ALLIANCE : TEAM_HORDE;

        // EvenTeams refinement (only if CFBG.EvenTeams.Enabled).
        if (IsEnableEvenTeams())
        {
            if (ctx.hunterOverride)
            {
                team = *ctx.hunterOverride;
            }
            // Zero-denominator guard: formation / low-occupancy can present an
            // empty side; float division by 0 would yield inf/NaN and garbage.
            else if (ctx.evenCountA > 0 && ctx.evenCountH > 0)
            {
                // We need to have a diff of 0.5f
                // Range of calculation: [minBgLevel, maxBgLevel], i.e: [10,20)
                float avgLvlAlliance = ctx.levelSumA / (float)ctx.evenCountA;
                float avgLvlHorde = ctx.levelSumH / (float)ctx.evenCountH;

                if (std::abs(avgLvlAlliance - avgLvlHorde) >= 0.5f)
                    team = avgLvlAlliance < avgLvlHorde ? TEAM_ALLIANCE : TEAM_HORDE;
                else // it's balanced, so we should only check the ilvl
                    team = ctx.avgIlvlA < ctx.avgIlvlH ? TEAM_ALLIANCE : TEAM_HORDE;
            }
        }
        else if (ctx.levelSumA == ctx.levelSumH)
        {
            team = ctx.avgIlvlA < ctx.avgIlvlH ? TEAM_ALLIANCE : TEAM_HORDE;
        }

        return team;
    }

    // 3. Item level (only if CFBG.Include.Avg.Ilvl.Enable).
    if (IsEnableAvgIlvl() && ctx.avgIlvlA != ctx.avgIlvlH)
        return ctx.avgIlvlA < ctx.avgIlvlH ? TEAM_ALLIANCE : TEAM_HORDE;

    // 4. Fallback: the provisional / candidate team.
    return ctx.fallback;
}

uint8 CFBG::getBalanceClassMinLevel(const Battleground* bg) const
{
    return static_cast<uint8>(bg->GetMaxLevel()) - _balanceClassLevelDiff;
}

TeamId CFBG::getTeamWithLowerClass(Battleground *bg, Classes c)
{
    uint16 hordeClassQty = 0;
    uint16 allianceClassQty = 0;

    for (auto const& [playerGuid, player] : bg->GetPlayers())
    {
        if (player && player->getClass() == c)
        {
            if (player->GetTeamId() == TEAM_ALLIANCE)
            {
                allianceClassQty++;
            }
            else
            {
                hordeClassQty++;
            }
        }
    }

    return hordeClassQty > allianceClassQty ? TEAM_ALLIANCE : TEAM_HORDE;
}

void CFBG::ValidatePlayerForBG(Battleground* bg, Player* player)
{
    if (!_IsEnableSystem || !bg || bg->isArena() || !player)
        return;

    // A WG fake survives the teleport into a BG: OnPlayerUpdateZone's
    // InBattleground() guard blocks the deferred clear. Drop it so
    // BalanceTeamsOnEntry and SetFakeRaceAndMorph run against BG state.
    if (IsPlayerFake(player))
        ClearFakePlayer(player);

    BalanceTeamsOnEntry(bg, player);

    TeamId teamId{ player->GetBgTeamId() };

    if (player->GetTeamId(true) == teamId)
        return;

    BGData& bgdata = player->GetBGData();

    if (bgdata.bgTeamId != teamId)
        bgdata.bgTeamId = teamId;

    SetFakeRaceAndMorph(player);

    if (bg->GetMapId() == MapAlteracValley)
    {
        if (teamId == TEAM_HORDE)
        {
            player->GetReputationMgr().ApplyForceReaction(FACTION_FROSTWOLF_CLAN, REP_FRIENDLY, true);
            player->GetReputationMgr().ApplyForceReaction(FACTION_STORMPIKE_GUARD, REP_HOSTILE, true);
        }
        else
        {
            player->GetReputationMgr().ApplyForceReaction(FACTION_FROSTWOLF_CLAN, REP_HOSTILE, true);
            player->GetReputationMgr().ApplyForceReaction(FACTION_STORMPIKE_GUARD, REP_FRIENDLY, true);
        }

        player->GetReputationMgr().SendForceReactions();
    }
}

void CFBG::BalanceTeamsOnEntry(Battleground* bg, Player* player)
{
    // The invite-time team was chosen using level/ilvl, but declined invites can
    // leave the teams uneven once players actually arrive. Here we only correct
    // that head-count distortion for solo entrants.
    if (!IsEnableSystem() || !IsEnableBalanceTeamsOnEntry())
        return;

    if (bg->isArena() || bg->isRated())
        return;

    // Solo entrants only: never split a party across teams.
    if (player->GetGroup())
        return;

    // Genuine first entry only: skip relog re-adds (already in the BG), otherwise
    // the invited-count books would be adjusted a second time.
    if (bg->GetPlayers().find(player->GetGUID()) != bg->GetPlayers().end())
        return;

    // Never flip a player who is already faked. The faction sync that backs
    // GetTeamId() -- SetFakeRaceAndMorph -> SetFactionForRace -> setTeamId() -- is
    // skipped for already-faked players (its IsPlayerFake guard). Flipping bgTeamId
    // now would therefore move the player to the new side for grouping/graveyards/
    // win purposes while GetTeamId() (used by flag capture and scoring) stays on the
    // OLD side -- e.g. an Alliance player on Horde's side capturing Alliance flags.
    // Fresh entrants are not faked yet (the morph runs right after this), so they
    // are still rebalanced normally.
    if (IsPlayerFake(player))
        return;

    TeamId provisional = player->GetBgTeamId();

    // Live head counts correctly EXCLUDE the entering player (counted later in
    // Battleground::AddPlayer).
    int32 countA = bg->GetPlayersCountByTeam(TEAM_ALLIANCE);
    int32 countH = bg->GetPlayersCountByTeam(TEAM_HORDE);

    // Sides already balanced: keep the provisional team, no morph / count churn.
    if (countA == countH)
        return;

    TeamId corrected = (countA < countH) ? TEAM_ALLIANCE : TEAM_HORDE;

    if (corrected == provisional)
        return;

    // Move the reserved slot to the corrected side. DecreaseInvitedCount is safe:
    // accept does not decrement, so the player is still counted as invited on the
    // provisional team; the method is underflow-guarded. The matching decrement
    // happens in RemovePlayerAtLeave on the (corrected) current team -> zero-sum.
    bg->DecreaseInvitedCount(provisional);
    bg->IncreaseInvitedCount(corrected);
    player->GetBGData().bgTeamId = corrected;

    // The player was already teleported to the provisional base before AddPlayer;
    // move them to the corrected base so they don't spawn at the enemy's.
    Position const* startPos = bg->GetTeamStartPosition(corrected);
    player->TeleportTo(bg->GetMapId(), startPos->GetPositionX(), startPos->GetPositionY(),
        startPos->GetPositionZ(), startPos->GetOrientation());
}

uint32 CFBG::GetMorphFromRace(uint8 race, uint8 gender)
{
    switch (race)
    {
        case RACE_BLOODELF:
            return gender == GENDER_MALE ? FAKE_M_BLOOD_ELF : FAKE_F_BLOOD_ELF;
        case RACE_ORC:
            return gender == GENDER_MALE ? FAKE_M_FEL_ORC : FAKE_F_ORC;
        case RACE_TROLL:
            return gender == GENDER_MALE ? FAKE_M_TROLL : FAKE_F_BLOOD_ELF;
        case RACE_TAUREN:
            return gender == GENDER_MALE ? FAKE_M_TAUREN : FAKE_F_TAUREN;
        case RACE_DRAENEI:
            return gender == GENDER_MALE ? FAKE_M_BROKEN_DRAENEI : FAKE_F_DRAENEI;
        case RACE_DWARF:
            return gender == GENDER_MALE ? FAKE_M_DWARF : FAKE_F_HUMAN;
        case RACE_GNOME:
            return gender == GENDER_MALE ? FAKE_M_GNOME : FAKE_F_GNOME;
        case RACE_NIGHTELF: // female is missing and male causes client crashes...
        case RACE_HUMAN:
            return gender == GENDER_MALE ? FAKE_M_HUMAN : FAKE_F_HUMAN;
        default:
            // Default: Blood elf.
            return gender == GENDER_MALE ? FAKE_M_BLOOD_ELF : FAKE_F_BLOOD_ELF;
    }
}

CFBG::RandomSkinInfo CFBG::GetRandomRaceMorph(TeamId team, uint8 playerClass, uint8 gender)
{
    uint8 playerRace = Acore::Containers::SelectRandomContainerElement(team == TEAM_ALLIANCE ? _raceData[playerClass].availableRacesH : _raceData[playerClass].availableRacesA);
    uint32 playerMorph = GetMorphFromRace(playerRace, gender);

    return { playerRace, playerMorph };
}

void CFBG::SetFakeRaceAndMorph(Player* player)
{
    if (!player->InBattleground() || player->GetTeamId(true) == player->GetBgTeamId() || IsPlayerFake(player))
        return;

    // generate random race and morph
    RandomSkinInfo skinInfo{ GetRandomRaceMorph(player->GetTeamId(true), player->getClass(), player->getGender()) };

    uint8 selectedRace = player->GetPlayerSetting("mod-cfbg", SETTING_CFBG_RACE).value;

    if (!RandomizeRaces() && selectedRace && IsRaceValidForFaction(player->GetTeamId(true), selectedRace))
    {
        skinInfo.first = selectedRace;
        skinInfo.second = GetMorphFromRace(skinInfo.first, player->getGender());
    }

    FakePlayer fakePlayerInfo
    {
        skinInfo.first,
        skinInfo.second,
        player->TeamIdForRace(skinInfo.first),
        player->getRace(true),
        player->GetDisplayId(),
        player->GetNativeDisplayId(),
        player->GetTeamId(true)
    };

    player->setRace(fakePlayerInfo.FakeRace);
    SetFactionForRace(player, fakePlayerInfo.FakeRace, fakePlayerInfo.FakeTeamID);
    player->SetDisplayId(fakePlayerInfo.FakeMorph);
    player->SetNativeDisplayId(fakePlayerInfo.FakeMorph);

    _fakePlayerStore.emplace(player, std::move(fakePlayerInfo));
}

void CFBG::SetFakeRaceAndMorphForBF(Player* player, TeamId assignedTeam)
{
    if (!player || IsPlayerFake(player))
        return;

    TeamId realTeam = player->GetTeamId(true);
    if (realTeam == assignedTeam)
        return;

    // Generate a race/morph from the assigned team's faction (opposite of real faction)
    RandomSkinInfo skinInfo{ GetRandomRaceMorph(realTeam, player->getClass(), player->getGender()) };

    uint8 selectedRace = player->GetPlayerSetting("mod-cfbg", SETTING_CFBG_RACE).value;

    if (!RandomizeRaces() && selectedRace && IsRaceValidForFaction(realTeam, selectedRace))
    {
        skinInfo.first = selectedRace;
        skinInfo.second = GetMorphFromRace(skinInfo.first, player->getGender());
    }

    FakePlayer fakePlayerInfo
    {
        skinInfo.first,
        skinInfo.second,
        assignedTeam,
        player->getRace(true),
        player->GetDisplayId(),
        player->GetNativeDisplayId(),
        realTeam
    };

    player->setRace(fakePlayerInfo.FakeRace);
    SetFactionForRace(player, fakePlayerInfo.FakeRace, assignedTeam);
    player->SetDisplayId(fakePlayerInfo.FakeMorph);
    player->SetNativeDisplayId(fakePlayerInfo.FakeMorph);

    _fakePlayerStore.emplace(player, std::move(fakePlayerInfo));
}

void CFBG::SetFactionForRace(Player* player, uint8 Race, TeamId teamId)
{
    if (!player)
        return;

    player->setTeamId(teamId);

    ChrRacesEntry const* DBCRace = sChrRacesStore.LookupEntry(Race);
    player->SetFaction(DBCRace ? DBCRace->FactionID : 0);

    for (Unit* controlled : player->m_Controlled)
    {
        if (controlled)
            controlled->SetFaction(player->GetFaction());
    }
}

void CFBG::ClearFakePlayer(Player* player)
{
    if (!IsPlayerFake(player))
        return;

    player->setRace(_fakePlayerStore[player].RealRace);
    player->SetDisplayId(_fakePlayerStore[player].RealMorph);
    player->SetNativeDisplayId(_fakePlayerStore[player].RealNativeMorph);
    SetFactionForRace(player, _fakePlayerStore[player].RealRace, _fakePlayerStore[player].RealTeamID);

    // Clear forced faction reactions. Rank doesn't matter here, not used when they are removed.
    player->GetReputationMgr().ApplyForceReaction(FACTION_FROSTWOLF_CLAN, REP_FRIENDLY, false);
    player->GetReputationMgr().ApplyForceReaction(FACTION_STORMPIKE_GUARD, REP_FRIENDLY, false);

    _fakePlayerStore.erase(player);
}

void CFBG::ReapplyFakePlayer(Player* player)
{
    FakePlayer const* info = GetFakePlayer(player);
    if (!info)
        return;

    // Re-push the stored fake values after a resurrect so the assigned faction
    // and morph survive the ghost->alive transition.
    player->setRace(info->FakeRace);
    SetFactionForRace(player, info->FakeRace, info->FakeTeamID);
    player->SetDisplayId(info->FakeMorph);
    player->SetNativeDisplayId(info->FakeMorph);
}

bool CFBG::IsPlayerFake(Player* player)
{
    return _fakePlayerStore.contains(player);
}

FakePlayer const* CFBG::GetFakePlayer(Player* player) const
{
    return Acore::Containers::MapGetValuePtr(_fakePlayerStore, player);
}

std::optional<TeamId> CFBG::GetWGWarAssignment(ObjectGuid guid) const
{
    auto const& itr = _wgWarAssignmentStore.find(guid);
    if (itr == _wgWarAssignmentStore.end())
        return std::nullopt;

    return itr->second;
}

void CFBG::SetWGWarAssignment(ObjectGuid guid, TeamId team)
{
    _wgWarAssignmentStore[guid] = team;
}

void CFBG::ClearWGWarAssignments()
{
    _wgWarAssignmentStore.clear();
}

void CFBG::DoForgetPlayersInList(Player* player)
{
    // m_FakePlayers is filled from a vector within the battleground
    // they were in previously so all players that have been in that BG will be invalidated.
    for (auto const& itr : _fakeNamePlayersStore)
    {
        WorldPacket data(SMSG_INVALIDATE_PLAYER, 8);
        data << itr.second;
        player->GetSession()->SendPacket(&data);

        if (Player* _player = ObjectAccessor::FindPlayer(itr.second))
            player->GetSession()->SendNameQueryOpcode(_player->GetGUID());
    }

    _fakeNamePlayersStore.erase(player);
}

void CFBG::FitPlayerInTeam(Player* player, bool action, Battleground* bg)
{
    if (!_IsEnableSystem)
        return;

    if (!bg)
        bg = player->GetBattleground();

    if ((!bg || bg->isArena()) && action)
        return;

    if (action)
        SetForgetBGPlayers(player, true);
    else
        SetForgetInListPlayers(player, true);
}

void CFBG::SetForgetBGPlayers(Player* player, bool value)
{
    _forgetBGPlayersStore[player] = value;
}

bool CFBG::ShouldForgetBGPlayers(Player* player)
{
    return _forgetBGPlayersStore[player];
}

void CFBG::SetForgetInListPlayers(Player* player, bool value)
{
    _forgetInListPlayersStore[player] = value;
}

bool CFBG::ShouldForgetInListPlayers(Player* player)
{
    return _forgetInListPlayersStore.find(player) != _forgetInListPlayersStore.end() && _forgetInListPlayersStore[player];
}

void CFBG::DoForgetPlayersInBG(Player* player, Battleground* bg)
{
    for (auto const& itr : bg->GetPlayers())
    {
        // Here we invalidate players in the bg to the added player
        WorldPacket data1(SMSG_INVALIDATE_PLAYER, 8);
        data1 << itr.first;
        player->GetSession()->SendPacket(&data1);

        if (Player* _player = ObjectAccessor::FindPlayer(itr.first))
        {
            player->GetSession()->SendNameQueryOpcode(_player->GetGUID()); // Send namequery answer instantly if player is available

            // Here we invalidate the player added to players in the bg
            WorldPacket data2(SMSG_INVALIDATE_PLAYER, 8);
            data2 << player->GetGUID();
            _player->GetSession()->SendPacket(&data2);
            _player->GetSession()->SendNameQueryOpcode(player->GetGUID());
        }
    }
}

bool CFBG::SendRealNameQuery(Player* player)
{
    if (IsPlayingNative(player))
        return false;

    WorldPacket data(SMSG_NAME_QUERY_RESPONSE, (8 + 1 + 1 + 1 + 1 + 1 + 10));
    data << player->GetGUID().WriteAsPacked();                  // player guid
    data << uint8(0);                                           // added in 3.1; if > 1, then end of packet
    data << player->GetName();                                  // played name
    data << uint8(0);                                           // realm name for cross realm BG usage
    data << uint8(player->getRace(true));
    data << uint8(player->getGender());
    data << uint8(player->getClass());
    data << uint8(0);                                           // is not declined
    player->GetSession()->SendPacket(&data);

    return true;
}

bool CFBG::IsPlayingNative(Player* player)
{
    return player->GetTeamId(true) == player->GetBGData().bgTeamId;
}

bool CFBG::CheckCrossFactionMatch(BattlegroundQueue* queue, BattlegroundBracketId bracket_id, uint32 minPlayers, uint32 maxPlayers)
{
    if (!IsEnableSystem())
        return false;

    queue->m_SelectionPools[TEAM_ALLIANCE].Init();
    queue->m_SelectionPools[TEAM_HORDE].Init();

    GroupsList groups{ queue->m_QueuedGroups[bracket_id][BG_QUEUE_CFBG].begin(), queue->m_QueuedGroups[bracket_id][BG_QUEUE_CFBG].end() };

    if (IsZoeGearBracketQueueSplitEnabled())
    {
        bool zoeFallback = false;
        uint8 zoeBracket = SelectZoeQueueBracket(groups, minPlayers, zoeFallback);

        if (zoeBracket == 255)
        {
            queue->m_SelectionPools[TEAM_ALLIANCE].Init();
            queue->m_SelectionPools[TEAM_HORDE].Init();
            return true;
        }

        if (zoeBracket != 0)
            groups = FilterZoeGroupsByBracket(groups, zoeBracket);
    }

    if (IsEnableEvenTeams())
    {
        // Sort for check same count groups
        std::sort(groups.begin(), groups.end(), [](GroupQueueInfo const* a, GroupQueueInfo const* b) { return a->Players.size() > b->Players.size(); });

        InviteSameCountGroups(groups, queue, maxPlayers, maxPlayers);
    }
    else
    {
        // Default sort
        std::sort(groups.begin(), groups.end(), [](GroupQueueInfo const* a, GroupQueueInfo const* b) { return a->JoinTime > b->JoinTime; });

        for (auto const& gInfo : groups)
        {
            if (gInfo->IsInvitedToBGInstanceGUID)
                continue;

            auto queueInfo = CrossFactionQueueInfo{ queue };
            auto targetTeam = queueInfo.GetLowerTeamIdInBG(gInfo);
            gInfo->teamId = targetTeam;

            if (!queue->m_SelectionPools[targetTeam].AddGroup(gInfo, maxPlayers))
                break;
        }
    }

    // Return when we're ready to start a BG, if we're in startup process
    if (queue->m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount() >= minPlayers &&
        queue->m_SelectionPools[TEAM_HORDE].GetPlayerCount() >= minPlayers)
        return true;

    // Return false when we didn't manage to fill the BattleGround in Filling "mode".
    // reset selectionpool for further attempts
    queue->m_SelectionPools[TEAM_ALLIANCE].Init();
    queue->m_SelectionPools[TEAM_HORDE].Init();
    return true;
}

bool CFBG::FillPlayersToCFBG(BattlegroundQueue* bgqueue, Battleground* bg, BattlegroundBracketId bracket_id)
{
    if (!IsEnableSystem() || bg->isArena() || bg->isRated())
        return false;

    uint32 maxAli{ bg->GetFreeSlotsForTeam(TEAM_ALLIANCE) };
    uint32 maxHorde{ bg->GetFreeSlotsForTeam(TEAM_HORDE) };

    GroupsList groups{ bgqueue->m_QueuedGroups[bracket_id][BG_QUEUE_CFBG].begin(), bgqueue->m_QueuedGroups[bracket_id][BG_QUEUE_CFBG].end() };

    if (IsZoeGearBracketQueueSplitEnabled())
    {
        uint8 bgBracket = GetZoeBattlegroundBracket(bg);

        if (bgBracket >= 1 && bgBracket <= 3)
        {
            groups = FilterZoeGroupsByBracket(groups, bgBracket);
        }
        else
        {
            // BG ainda vazia/sem bracket conhecida: seleciona uma bracket com
            // jogadores suficientes. Sem fallback, nao mistura.
            bool zoeFallback = false;
            uint8 zoeBracket = SelectZoeQueueBracket(groups, 1, zoeFallback);

            if (zoeBracket == 255)
                return true;

            if (zoeBracket != 0)
                groups = FilterZoeGroupsByBracket(groups, zoeBracket);
        }
    }

    auto DefaultInvitePlayersToBG = [this, bg, bgqueue, &groups, maxAli, maxHorde]()
    {
        GroupsList toDeleteGroups;

        for (auto const& gInfo : groups)
        {
            if (gInfo->IsInvitedToBGInstanceGUID)
                continue;

            TeamId targetTeam = GetLowerTeamIdInBG(bg, bgqueue, gInfo);
            gInfo->teamId = targetTeam;

            if (bgqueue->m_SelectionPools[targetTeam].AddGroup(gInfo, targetTeam == TEAM_ALLIANCE ? maxAli : maxHorde))
                toDeleteGroups.emplace_back(gInfo);
        }

        for (auto const& itr : toDeleteGroups)
            std::erase(groups, itr);
    };

    if (IsEnableEvenTeams())
    {
        // Projected team sizes: players already in the BG plus players invited in
        // earlier queue updates that have not entered yet.
        uint32 playersInBGAli{ bg->GetPlayersCountByTeam(TEAM_ALLIANCE) };
        uint32 playersInBGHorde{ bg->GetPlayersCountByTeam(TEAM_HORDE) };

        for (auto const& gInfo : bgqueue->m_QueuedGroups[bracket_id][BG_QUEUE_CFBG])
            if (gInfo->IsInvitedToBGInstanceGUID)
                (gInfo->teamId == TEAM_ALLIANCE ? playersInBGAli : playersInBGHorde) += gInfo->Players.size();

        uint32 evenTeamsCount{ EvenTeamsMaxPlayersThreshold() };
        uint32 playersInBG{ playersInBGAli + playersInBGHorde };

        // A threshold of 0 enforces even teams for all match sizes. A non-zero
        // threshold stops enforcing once both teams reach it (total >= threshold * 2).
        if (evenTeamsCount && playersInBG >= evenTeamsCount * 2)
        {
            DefaultInvitePlayersToBG();
            return true;
        }

        // 1. Close any existing imbalance by topping up the smaller team only.
        if (playersInBGAli != playersInBGHorde)
        {
            TeamId targetTeam{ playersInBGAli < playersInBGHorde ? TEAM_ALLIANCE : TEAM_HORDE };
            uint32 gap{ targetTeam == TEAM_ALLIANCE ? playersInBGHorde - playersInBGAli : playersInBGAli - playersInBGHorde };
            uint32 targetCount{ std::min<uint32>(gap, targetTeam == TEAM_ALLIANCE ? maxAli : maxHorde) };

            std::sort(groups.begin(), groups.end(), [](GroupQueueInfo const* a, GroupQueueInfo const* b) { return a->Players.size() < b->Players.size(); });

            GroupsList toDeleteGroups;

            for (auto const& gInfo : groups)
            {
                if (gInfo->IsInvitedToBGInstanceGUID)
                    continue;

                if (bgqueue->m_SelectionPools[targetTeam].GetPlayerCount() >= targetCount)
                    break;

                gInfo->teamId = targetTeam;
                auto before{ bgqueue->m_SelectionPools[targetTeam].GetPlayerCount() };
                bgqueue->m_SelectionPools[targetTeam].AddGroup(gInfo, targetCount);

                if (bgqueue->m_SelectionPools[targetTeam].GetPlayerCount() > before)
                    toDeleteGroups.emplace_back(gInfo);
            }

            for (auto const& gInfo : toDeleteGroups)
                std::erase(groups, gInfo);
        }

        // 2. Invite the rest only in balanced same-size pairs. Any group that would
        //    break N vs N stays in the queue for the next update.
        std::sort(groups.begin(), groups.end(), [](GroupQueueInfo const* a, GroupQueueInfo const* b) { return a->Players.size() > b->Players.size(); });
        InviteSameCountGroups(groups, bgqueue, maxAli, maxHorde, bg);

        return true;
    }

    DefaultInvitePlayersToBG();
    return true;
}

bool CFBG::isClassJoining(uint8 _class, Player* player, uint32 minLevel)
{
    if (!player)
    {
        return false;
    }

    return player->getClass() == _class && (player->GetLevel() >= minLevel);
}

void CFBG::UpdateForget(Player* player)
{
    Battleground* bg = player->GetBattleground();
    if (bg)
    {
        if (ShouldForgetBGPlayers(player) && bg)
        {
            DoForgetPlayersInBG(player, bg);
            SetForgetBGPlayers(player, false);
        }
    }
    else if (ShouldForgetInListPlayers(player))
    {
        DoForgetPlayersInList(player);
        SetForgetInListPlayers(player, false);
    }
}

void CFBG::SendMessageQueue(BattlegroundQueue* bgQueue, Battleground* bg, PvPDifficultyEntry const* bracketEntry, Player* leader)
{
    if (!bgQueue || !bg || !bracketEntry)
        return;

    BattlegroundBracketId bracketId = bracketEntry->GetBracketId();

    auto bgName = bg->GetName();
    uint32 q_min_level = std::min(bracketEntry->minLevel, (uint32)80);
    uint32 q_max_level = std::min(bracketEntry->maxLevel, (uint32)80);
    uint32 MinPlayers = GetMinPlayersPerTeam(bg, bracketEntry) * 2;
    uint32 qTotal = bgQueue->GetPlayersCountInGroupsQueue(bracketId, (BattlegroundQueueGroupTypes)BG_QUEUE_CFBG);

    uint8 announceBracket = 0;
    bool useZoeQueueAnnouncer = _ZoeGearBracketEnable && _ZoeGearBracketQueueAnnouncerEnable;

    if (useZoeQueueAnnouncer && leader)
    {
        announceBracket = CalculateZoePlayerBracket(leader);

        if (IsZoeGearBracketQueueSplitEnabled() && _ZoeGearBracketQueueAnnouncerUseBracketPlayerCount)
            qTotal = CountZoeQueuedPlayersByBracket(bgQueue, bracketId, announceBracket);
    }

    std::string announceMessage = useZoeQueueAnnouncer
        ? BuildZoeQueueAnnouncerMessage(bgName, q_min_level, q_max_level, qTotal, MinPlayers, announceBracket)
        : "CFBG " + bgName + " (Levels: " + std::to_string(q_min_level) + " - " + std::to_string(q_max_level) + "). Registered: " + std::to_string(qTotal) + "/" + std::to_string(MinPlayers);

    // ZoeCore announcer envia a mensagem por conta propria e NAO agenda o announcer padrao do core.
    // Antes a gente chamava SetQueueAnnouncementTimer(...) aqui, e isso fazia o core mandar uma segunda
    // mensagem default: "[BG Queue Announcer]: ...".
    // Resultado esperado agora: apenas uma mensagem, a customizada do ZoeCore.
    if (useZoeQueueAnnouncer)
    {
        if (sWorld->getBoolConfig(CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_PLAYERONLY))
        {
            if (leader && leader->GetSession())
                ChatHandler(leader->GetSession()).SendSysMessage(announceMessage.c_str());
        }
        else
        {
            sWorldSessionMgr->SendServerMessage(SERVER_MSG_STRING, announceMessage.c_str());
        }

        return;
    }

    // Fallback original do CFBG/core quando o announcer custom ZoeCore estiver desligado.
    if (sWorld->getBoolConfig(CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_PLAYERONLY))
    {
        if (leader && leader->GetSession())
            ChatHandler(leader->GetSession()).SendSysMessage(announceMessage.c_str());

        return;
    }

    if (sWorld->getBoolConfig(CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_TIMED))
    {
        if (bgQueue->GetQueueAnnouncementTimer(bracketEntry->bracketId) < 0)
        {
            sWorldSessionMgr->SendServerMessage(SERVER_MSG_STRING, announceMessage.c_str());
            bgQueue->SetQueueAnnouncementTimer(bracketEntry->bracketId, sWorld->getIntConfig(CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_TIMER));
        }
    }
    else
    {
        if (bgQueue->GetQueueAnnouncementTimer(bracketId) < 0)
        {
            sWorldSessionMgr->SendServerMessage(SERVER_MSG_STRING, announceMessage.c_str());
            bgQueue->SetQueueAnnouncementTimer(bracketId, BG_QUEUE_ANNOUNCER_IMMEDIATE_DEBOUNCE, true);
        }
    }
}

bool CFBG::IsRaceValidForFaction(uint8 teamId, uint8 race)
{
    for (auto const& raceVariable : _raceInfo)
    {
        if (race == raceVariable.RaceId && teamId == raceVariable.TeamId)
        {
            return true;
        }
    }

    return false;
}

void CFBG::InviteSameCountGroups(GroupsList& groups, BattlegroundQueue* bgQueue, uint32 maxAli, uint32 maxHorde, Battleground* bg /*= nullptr*/)
{
    if (groups.size() < 2 || !bgQueue)
        return;

    GroupsList groupList;
    GroupsList addedGroups;
    SameCountGroupsList container;

    for (auto const& targetGroup : groups)
    {
        if (targetGroup->IsInvitedToBGInstanceGUID)
            continue;

        if (std::find(addedGroups.begin(), addedGroups.end(), targetGroup) != addedGroups.end())
            continue;

        groupList.clear();
        auto groupSizeNeed{ targetGroup->Players.size() };

        for (auto const& itrGroup : groups)
        {
            if (itrGroup == targetGroup)
                continue;

            if (itrGroup->IsInvitedToBGInstanceGUID)
                continue;

            if (std::find(addedGroups.begin(), addedGroups.end(), itrGroup) != addedGroups.end())
                continue;

            auto groupSizeItr{ itrGroup->Players.size() };
            if (groupSizeItr <= groupSizeNeed)
            {
                groupList.emplace_back(itrGroup);
                groupSizeNeed -= groupSizeItr;
            }

            if (!groupSizeNeed)
            {
                container.emplace_back(targetGroup, groupList);
                addedGroups.emplace_back(targetGroup);

                for (auto const& itr : groupList)
                    addedGroups.emplace_back(itr);

                groupList.clear();
                break;
            }
        }
    }

    if (container.empty())
        return;

    auto DeleteGroup = [bgQueue](GroupQueueInfo* gInfo)
    {
        std::erase(bgQueue->m_SelectionPools[TEAM_ALLIANCE].SelectedGroups, gInfo);
        std::erase(bgQueue->m_SelectionPools[TEAM_HORDE].SelectedGroups, gInfo);
    };

    for (auto& [groupTarget, groupListForTarger] : container)
    {
        auto teamTarget{ InviteGroupToBG(groupTarget, bgQueue, maxAli, maxHorde, bg) };
        if (teamTarget == TEAM_NEUTRAL)
            continue;

        bool IsAllInvited{ true };

        for (auto const& groupItr : groupListForTarger)
        {
            auto teamItr{ InviteGroupToBG(groupItr, bgQueue, maxAli, maxHorde, bg) };
            if (teamItr == TEAM_NEUTRAL)
            {
                IsAllInvited = false;
                break;
            }
        }

        if (!IsAllInvited)
        {
            for (auto const& groupItr : groupListForTarger)
                DeleteGroup(groupItr);

            DeleteGroup(groupTarget);
            continue;
        }

        for (auto const& groupItr : groupListForTarger)
            std::erase(groups, groupItr);

        std::erase(groups, groupTarget);
    }
}

TeamId CFBG::InviteGroupToBG(GroupQueueInfo* gInfo, BattlegroundQueue* bgQueue, uint32 maxAli, uint32 maxHorde, Battleground* bg /*= nullptr*/)
{
    if (bg)
    {
        auto targetTeam = GetLowerTeamIdInBG(bg, bgQueue, gInfo);
        gInfo->teamId = targetTeam;
    }
    else
    {
        auto queueInfo = CrossFactionQueueInfo{ bgQueue };
        auto targetTeam = queueInfo.GetLowerTeamIdInBG(gInfo);
        gInfo->teamId = targetTeam;
    }

    if (bgQueue->m_SelectionPools[gInfo->teamId].AddGroup(gInfo, gInfo->teamId == TEAM_ALLIANCE ? maxAli : maxHorde))
        return gInfo->teamId;

    return TEAM_NEUTRAL;
}
