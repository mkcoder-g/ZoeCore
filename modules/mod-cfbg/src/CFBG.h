/*
 * Copyright (С) since 2019 Andrei Guluaev (Winfidonarleyan/Kargatum) https://github.com/Winfidonarleyan
 * Copyright (С) since 2019+ AzerothCore <www.azerothcore.org>
 * Licence MIT https://opensource.org/MIT
 */

#ifndef _CFBG_H_
#define _CFBG_H_

#include "SharedDefines.h"
#include "DBCEnums.h"
#include "ObjectGuid.h"
#include <array>
#include <optional>
#include <unordered_map>
#include <vector>

class Player;
class Item;
class Battlefield;
class Battleground;
class BattlegroundQueue;
class Group;

struct GroupQueueInfo;
struct PvPDifficultyEntry;

enum FakeMorphs
{
    FAKE_M_FEL_ORC        = 21267,
    FAKE_F_ORC            = 20316,
    FAKE_M_DWARF          = 20317,
    FAKE_M_NIGHT_ELF      = 20318,
    FAKE_F_DRAENEI        = 20323,
    FAKE_M_BROKEN_DRAENEI = 21105,
    FAKE_M_TROLL          = 20321,
    FAKE_M_HUMAN          = 19723,
    FAKE_F_HUMAN          = 19724,
    FAKE_M_BLOOD_ELF      = 20578,
    FAKE_F_BLOOD_ELF      = 20579,
    FAKE_F_GNOME          = 20320,
    FAKE_M_GNOME          = 20580,
    FAKE_F_TAUREN         = 20584,
    FAKE_M_TAUREN         = 20585
};

constexpr auto FACTION_FROSTWOLF_CLAN = 729;
constexpr auto FACTION_STORMPIKE_GUARD = 730;

// Cfbg settings
constexpr auto SETTING_CFBG_RACE = 0;

struct FakePlayer
{
    // Fake
    uint8   FakeRace;
    uint32  FakeMorph;
    TeamId  FakeTeamID;

    // Real
    uint8   RealRace;
    uint32  RealMorph;
    uint32  RealNativeMorph;
    TeamId  RealTeamID;
};

struct RaceData
{
    uint8 charClass;
    std::vector<uint8> availableRacesA;
    std::vector<uint8> availableRacesH;
};

struct CFBGRaceInfo
{
    uint8 RaceId;
    std::string RaceName;
    uint8 TeamId;
};

struct CrossFactionGroupInfo
{
    explicit CrossFactionGroupInfo(GroupQueueInfo* groupInfo);

    uint32 AveragePlayersLevel{ 0 };
    uint32 AveragePlayersItemLevel{ 0 };
    bool IsHunterJoining{ false };
    uint32 SumAverageItemLevel{ 0 };
    uint32 SumPlayerLevel{ 0 };

    CrossFactionGroupInfo() = delete;
    CrossFactionGroupInfo(CrossFactionGroupInfo&&) = delete;
};

struct CrossFactionQueueInfo
{
    explicit CrossFactionQueueInfo(BattlegroundQueue* bgQueue);

    TeamId GetLowerTeamIdInBG(GroupQueueInfo* groupInfo);

    std::array<uint32, 2> PlayersCount{};
    std::array<uint32, 2> SumAverageItemLevel{};
    std::array<uint32, 2> SumPlayerLevel{};

private:
    CrossFactionQueueInfo() = delete;
    CrossFactionQueueInfo(CrossFactionQueueInfo&&) = delete;
};

// Precomputed numeric aggregates fed to CFBG::ResolveBalancedTeam, the single
// shared team-decision cascade. Each caller (formation / reinforcement) builds
// one from its own data source (queue tallies or live BG counts).
struct TeamBalanceContext
{
    int32  countA{ 0 };               // head counts, candidate EXCLUDED
    int32  countH{ 0 };
    uint32 levelSumA{ 0 };            // candidate level ALREADY folded into its side
    uint32 levelSumH{ 0 };
    uint32 avgIlvlA{ 0 };            // per-team ilvl metric (bg avg OR queue sum)
    uint32 avgIlvlH{ 0 };
    uint32 evenCountA{ 0 };          // denominators for the EvenTeams avg-level step
    uint32 evenCountH{ 0 };
    std::optional<TeamId> hunterOverride{ std::nullopt }; // set only when the bg-gated hunter step fires
    TeamId fallback{ TEAM_NEUTRAL }; // provisional / candidate team
};

class CFBG
{
public:
    using RandomSkinInfo = std::pair<uint8/*race*/, uint32/*morph*/>;
    using GroupsList = std::vector<GroupQueueInfo*>;
    using SameCountGroupsList = std::vector<std::pair<GroupQueueInfo*, GroupsList>>;

    static CFBG* instance();

    void LoadConfig();

    inline bool IsZoeGearBracketEnable() const { return _ZoeGearBracketEnable; }
    inline bool IsZoeGearBracketLockEquipmentInsideBG() const { return _ZoeGearBracketLockEquipmentInsideBG; }

    void LoadZoeGearBracketItemTiers();
    void SetZoePlayerBracket(Player* player, bool announce = true);
    void ClearZoePlayerBracket(Player* player);
    bool CanEquipZoeGearBracketItem(Player* player, Item* item);
    void AuditZoeGearBracketEquipment(Player* player, bool notify);
    bool IsZoeGearBracketQueueSplitEnabled() const;
    void SetZoeBattlegroundBracket(Battleground* bg, Player* player);
    void ClearZoeBattlegroundBracket(Battleground* bg);

    inline bool IsEnableSystem() const { return _IsEnableSystem; }
    inline bool IsEnableWGSystem() const { return _IsEnableWGSystem; }
    inline bool IsEnableWGTeamLock() const { return _IsEnableWGTeamLock; }
    inline bool IsEnableWGReapplyOnResurrect() const { return _IsEnableWGReapplyOnResurrect; }
    inline bool IsEnableAvgIlvl() const { return _IsEnableAvgIlvl; }
    inline bool IsEnableBalancedTeams() const { return _IsEnableBalancedTeams; }
    inline bool IsEnableBalanceClassLowLevel() const { return _IsEnableBalanceClassLowLevel; }
    inline bool IsEnableEvenTeams() const { return _IsEnableEvenTeams; }
    inline bool IsEnableResetCooldowns() const { return _IsEnableResetCooldowns; }
    inline bool IsEnableBalanceTeamsOnEntry() const { return _IsEnableBalanceTeamsOnEntry; }
    inline uint32 EvenTeamsMaxPlayersThreshold() const { return _EvenTeamsMaxPlayersThreshold; }
    inline uint32 GetMaxPlayersCountInGroup() const { return _MaxPlayersCountInGroup; }
    inline uint8 GetBalanceClassMinLevel() const { return _balanceClassMinLevel; }
    inline uint8 GetBalanceClassMaxLevel() const { return _balanceClassMaxLevel; }
    inline uint8 GetBalanceClassLevelDiff() const { return _balanceClassLevelDiff; }
    inline bool RandomizeRaces() const { return _randomizeRaces; }

    uint32 GetBGTeamAverageItemLevel(Battleground* bg, TeamId team);
    uint32 GetBGTeamSumPlayerLevel(Battleground* bg, TeamId team);

    TeamId GetLowerTeamIdInBG(Battleground* bg, BattlegroundQueue* bgQueue, GroupQueueInfo* groupInfo);
    TeamId ResolveBalancedTeam(TeamBalanceContext const& ctx);

    bool SendRealNameQuery(Player* player);
    bool IsPlayerFake(Player* player);
    FakePlayer const* GetFakePlayer(Player* player) const;

    // Per-war WG team lock, GUID-keyed so it survives leaving the war/zone and
    // relog. Cleared when the war ends.
    std::optional<TeamId> GetWGWarAssignment(ObjectGuid guid) const;
    void SetWGWarAssignment(ObjectGuid guid, TeamId team);
    void ClearWGWarAssignments();

    bool ShouldForgetInListPlayers(Player* player);
    bool IsPlayingNative(Player* player);

    void ValidatePlayerForBG(Battleground* bg, Player* player);
    void SetFakeRaceAndMorph(Player* player);
    void SetFakeRaceAndMorphForBF(Player* player, TeamId assignedTeam);
    void SetFactionForRace(Player* player, uint8 Race, TeamId teamId);
    void ClearFakePlayer(Player* player);
    void ReapplyFakePlayer(Player* player);
    void DoForgetPlayersInList(Player* player);
    void FitPlayerInTeam(Player* player, bool action, Battleground* bg);
    void DoForgetPlayersInBG(Player* player, Battleground* bg);
    void SetForgetBGPlayers(Player* player, bool value);
    bool ShouldForgetBGPlayers(Player* player);
    void SetForgetInListPlayers(Player* player, bool value);
    void UpdateForget(Player* player);
    void SendMessageQueue(BattlegroundQueue* bgQueue, Battleground* bg, PvPDifficultyEntry const* bracketEntry, Player* leader);

    bool FillPlayersToCFBG(BattlegroundQueue* bgqueue, Battleground* bg, BattlegroundBracketId bracket_id);
    bool CheckCrossFactionMatch(BattlegroundQueue* bgqueue, BattlegroundBracketId bracket_id, uint32 minPlayers, uint32 maxPlayers);

    bool IsRaceValidForFaction(uint8 teamId, uint8 race);
    TeamId getTeamWithLowerClass(Battleground* bg, Classes c);
    uint8 getBalanceClassMinLevel(const Battleground* bg) const;

    inline auto GetRaceData() { return &_raceData; }
    inline auto GetRaceInfo() { return &_raceInfo; }

private:
    bool isClassJoining(uint8 _class, Player* player, uint32 minLevel);

    // Live, bg-gated hunter-class override for the reinforcement path. Returns a
    // team only when EvenTeams + class-low-level balancing apply to the candidate.
    std::optional<TeamId> ResolveHunterOverride(Battleground* bg, CrossFactionGroupInfo const& cfGroupInfo);

    // Arrival-time head-count correction for solo entrants.
    void BalanceTeamsOnEntry(Battleground* bg, Player* player);

    RandomSkinInfo GetRandomRaceMorph(TeamId team, uint8 playerClass, uint8 gender);

    uint32 GetMorphFromRace(uint8 race, uint8 gender);

    void InviteSameCountGroups(GroupsList& groups, BattlegroundQueue* bgQueue, uint32 maxAli, uint32 maxHorde, Battleground* bg = nullptr);
    TeamId InviteGroupToBG(GroupQueueInfo* gInfo, BattlegroundQueue* bgQueue, uint32 maxAli, uint32 maxHorde, Battleground* bg = nullptr);

    std::unordered_map<Player*, FakePlayer> _fakePlayerStore;
    std::unordered_map<ObjectGuid, TeamId> _wgWarAssignmentStore;
    std::unordered_map<Player*, ObjectGuid> _fakeNamePlayersStore;
    std::unordered_map<Player*, bool> _forgetBGPlayersStore;
    std::unordered_map<Player*, bool> _forgetInListPlayersStore;

    uint8 GetZoeItemTier(uint32 itemEntry) const;
    uint8 GetZoePlayerBracket(Player* player) const;
    uint8 CalculateZoePlayerBracket(Player* player) const;
    uint8 GetZoeMaxTierForBracket(uint8 bracket) const;
    char const* GetZoeBracketName(uint8 bracket) const;
    std::string GetZoeBracketColoredName(uint8 bracket) const;
    std::string GetZoeBracketAllowedTierText(uint8 bracket) const;
    std::string GetZoeBracketQueueDescription(uint8 bracket) const;
    std::string BuildZoeQueueAnnouncerMessage(std::string const& bgName, uint32 minLevel, uint32 maxLevel, uint32 queuedPlayers, uint32 requiredPlayers, uint8 bracket) const;
    void SendZoeGearBracketMessage(Player* player, std::string const& message) const;
    void LogZoeGearBracket(Player* player, std::string const& action, uint8 bracket, std::string const& details) const;
    uint8 CalculateZoeGroupBracket(GroupQueueInfo const* groupInfo) const;
    uint32 CountZoeQueuedPlayersByBracket(BattlegroundQueue* bgQueue, BattlegroundBracketId bracketId, uint8 bracket) const;
    uint8 SelectZoeQueueBracket(GroupsList const& groups, uint32 minPlayersPerTeam, bool& fallback) const;
    GroupsList FilterZoeGroupsByBracket(GroupsList const& groups, uint8 bracket) const;
    uint8 GetZoeBattlegroundBracket(Battleground* bg) const;

    std::unordered_map<uint32, uint8> _zoeItemTierStore;
    std::unordered_map<ObjectGuid, uint8> _zoePlayerBracketStore;
    std::unordered_map<uint32, uint8> _zoeBattlegroundBracketStore;

    std::array<RaceData, 12> _raceData{};
    std::array<CFBGRaceInfo, 9> _raceInfo{};

    // For config
    bool _IsEnableSystem;
    bool _IsEnableWGSystem;
    bool _IsEnableWGTeamLock;
    bool _IsEnableWGReapplyOnResurrect;
    bool _IsEnableAvgIlvl;
    bool _IsEnableBalancedTeams;
    bool _IsEnableBalanceClassLowLevel;
    bool _IsEnableEvenTeams;
    bool _IsEnableResetCooldowns;
    bool _IsEnableBalanceTeamsOnEntry;
    bool _showPlayerName;
    bool _randomizeRaces;
    uint32 _EvenTeamsMaxPlayersThreshold;
    uint32 _MaxPlayersCountInGroup;
    uint8 _balanceClassMinLevel;
    uint8 _balanceClassMaxLevel;
    uint8 _balanceClassLevelDiff;

    bool _ZoeGearBracketEnable{ false };
    bool _ZoeGearBracketUseSqlItemTierTable{ true };
    bool _ZoeGearBracketLockEquipmentInsideBG{ true };
    bool _ZoeGearBracketAllowWeaponSwap{ true };
    bool _ZoeGearBracketPeriodicCheckEnable{ true };
    bool _ZoeGearBracketLowPopulationFallback{ true };
    bool _ZoeGearBracketDisableEquipmentLockOnFallback{ false };
    bool _ZoeGearBracketAnnounce{ true };
    bool _ZoeGearBracketLogEnable{ true };
    bool _ZoeGearBracketQueueAnnouncerEnable{ true };
    bool _ZoeGearBracketQueueAnnouncerShowBracket{ true };
    bool _ZoeGearBracketQueueAnnouncerUseBracketPlayerCount{ true };
    uint8 _ZoeGearBracketMode{ 1 };
    uint8 _ZoeGearBracketNormalMaxTier{ 1 };
    uint8 _ZoeGearBracketAdvancedMaxTier{ 3 };
    uint8 _ZoeGearBracketEliteMaxTier{ 4 };
    uint32 _ZoeGearBracketPeriodicCheckMs{ 5000 };
    uint32 _ZoeGearBracketFallbackWaitMinutes{ 5 };
    std::string _ZoeGearBracketMessagePrefix{ "[ZoeCore BG]" };

    CFBG();
    ~CFBG() = default;

    CFBG(CFBG const&) = delete;
    CFBG(CFBG&&) = delete;
    CFBG& operator=(CFBG const&) = delete;
    CFBG& operator=(CFBG&&) = delete;
};

#define sCFBG CFBG::instance()

#endif // _KARGATUM_CFBG_H_
