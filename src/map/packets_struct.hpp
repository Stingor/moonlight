// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// Copyright (c) Hercules Dev Team - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef MAP_PACKETS_STRUCT_HPP
#define MAP_PACKETS_STRUCT_HPP

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>

/**
 *
 **/
enum packet_headers {
	banking_withdraw_ackType = 0x9aa,
	banking_deposit_ackType = 0x9a8,
	banking_checkType = 0x9a6,
	cart_additem_ackType = 0x12c,
	sc_notickType = 0x196,
#if PACKETVER < 4
	idle_unitType = 0x78,
#elif PACKETVER < 7
	idle_unitType = 0x1d8,
#elif PACKETVER < 20080102
	idle_unitType = 0x22a,
#elif PACKETVER < 20091103
	idle_unitType = 0x2ee,
#elif PACKETVER < 20101124
	idle_unitType = 0x7f9,
#elif PACKETVER < 20120221
	idle_unitType = 0x857,
#elif PACKETVER < 20131223
	idle_unitType = 0x915,
#elif PACKETVER < 20150513
	idle_unitType = 0x9dd,
#else
	idle_unitType = 0x9ff,
#endif
#if PACKETVER >= 20120618
	status_changeType = 0x983,
#elif PACKETVER >= 20090121
	status_changeType = 0x43f,
#else
	status_changeType = sc_notickType,/* 0x196 */
#endif
	status_change2Type = 0x43f,
	status_change_endType = 0x196,
#if PACKETVER < 20091103
	spawn_unit2Type = 0x7c,
	idle_unit2Type = 0x78,
#endif
#if PACKETVER < 20071113
	damageType = 0x8a,
#elif PACKETVER < 20131223
	damageType = 0x2e1,
#else
	damageType = 0x8c8,
#endif
#if PACKETVER < 4
	spawn_unitType = 0x79,
#elif PACKETVER < 7
	spawn_unitType = 0x1d9,
#elif PACKETVER < 20080102
	spawn_unitType = 0x22b,
#elif PACKETVER < 20091103
	spawn_unitType = 0x2ed,
#elif PACKETVER < 20101124
	spawn_unitType = 0x7f8,
#elif PACKETVER < 20120221
	spawn_unitType = 0x858,
#elif PACKETVER < 20131223
	spawn_unitType = 0x90f,
#elif PACKETVER < 20150513
	spawn_unitType = 0x9dc,
#else
	spawn_unitType = 0x9fe,
#endif
#if PACKETVER < 20080102
	authokType = 0x73,
#elif PACKETVER < 20141022
	authokType = 0x2eb,
// Some clients smaller than 20160330 cant be tested [4144]
#elif PACKETVER < 20160330
	authokType = 0xa18,
#else
	authokType = 0x2eb,
#endif
	script_clearType = 0x8d6,
	package_item_announceType = 0x7fd,
	item_drop_announceType = 0x7fd,
#if PACKETVER < 4
	unit_walkingType = 0x7b,
#elif PACKETVER < 7
	unit_walkingType = 0x1da,
#elif PACKETVER < 20080102
	unit_walkingType = 0x22c,
#elif PACKETVER < 20091103
	unit_walkingType = 0x2ec,
#elif PACKETVER < 20101124
	unit_walkingType = 0x7f7,
#elif PACKETVER < 20120221
	unit_walkingType = 0x856,
#elif PACKETVER < 20131223
	unit_walkingType = 0x914,
#elif PACKETVER < 20150513
	unit_walkingType = 0x9db,
#else
	unit_walkingType = 0x9fd,
#endif
	bgqueue_ackType = 0x8d8,
	bgqueue_notice_deleteType = 0x8db,
	bgqueue_registerType = 0x8d7,
	bgqueue_updateinfoType = 0x8d9,
	bgqueue_checkstateType = 0x90a,
	bgqueue_revokereqType = 0x8da,
	bgqueue_battlebeginackType = 0x8e0,
	bgqueue_notify_entryType = 0x8d9,
	bgqueue_battlebeginsType = 0x8df,
	notify_bounditemType = 0x2d3,
#if PACKETVER < 20110718
	skill_entryType = 0x11f,
#elif PACKETVER < 20121212
	skill_entryType = 0x8c7,
#elif PACKETVER < 20130731
	skill_entryType = 0x99f,
#else
	skill_entryType = 0x9ca,
#endif
	graffiti_entryType = 0x1c9,
#if defined(PACKETVER_ZERO) || PACKETVER >= 20180418
	dropflooritemType = 0xadd,
#elif PACKETVER > 20130000 /* not sure date */
	dropflooritemType = 0x84b,
#else
	dropflooritemType = 0x9e,
#endif
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	inventorylistnormalType = 0xb09,
#elif PACKETVER >= 20120925
	inventorylistnormalType = 0x991,
#elif PACKETVER >= 20080102
	inventorylistnormalType = 0x2e8,
#elif PACKETVER >= 20071002
	inventorylistnormalType = 0x1ee,
#else
	inventorylistnormalType = 0xa3,
#endif
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
	inventorylistequipType = 0xb39,
#elif PACKETVER_MAIN_NUM >= 20181002 || PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919
	inventorylistequipType = 0xb0a,
#elif PACKETVER >= 20150226
	inventorylistequipType = 0xa0d,
#elif PACKETVER >= 20120925
	inventorylistequipType = 0x992,
#elif PACKETVER >= 20080102
	inventorylistequipType = 0x2d0,
#elif PACKETVER >= 20071002
	inventorylistequipType = 0x295,
#else
	inventorylistequipType = 0xa4,
#endif
#if PACKETVER_RE_NUM >= 20180829 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	storageListNormalType = 0xb09,
#elif PACKETVER >= 20120925
	storageListNormalType = 0x995,
#elif PACKETVER >= 20080102
	storageListNormalType = 0x2ea,
#elif PACKETVER >= 20071002
	storageListNormalType = 0x295,
#else
	storageListNormalType = 0xa5,
#endif
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
	storageListEquipType = 0xb39,
#elif PACKETVER_MAIN_NUM >= 20181002 || PACKETVER_RE_NUM >= 20180829 || PACKETVER_ZERO_NUM >= 20180919
	storageListEquipType = 0xb0a,
#elif PACKETVER >= 20150226
	storageListEquipType = 0xa10,
#elif PACKETVER >= 20120925
	storageListEquipType = 0x996,
#elif PACKETVER >= 20080102
	storageListEquipType = 0x2d1,
#elif PACKETVER >= 20071002
	storageListEquipType = 0x296,
#else
	storageListEquipType = 0xa6,
#endif
#if PACKETVER_RE_NUM >= 20180829 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	cartlistnormalType = 0xb09,
#elif PACKETVER >= 20120925
	cartlistnormalType = 0x993,
#elif PACKETVER >= 20080102
	cartlistnormalType = 0x2e9,
#elif PACKETVER >= 20071002
	cartlistnormalType = 0x1ef,
#else
	cartlistnormalType = 0x123,
#endif
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
	cartlistequipType = 0xb39,
#elif PACKETVER_MAIN_NUM >= 20181002 || PACKETVER_RE_NUM >= 20180829 || PACKETVER_ZERO_NUM >= 20180919
	cartlistequipType = 0xb0a,
#elif PACKETVER >= 20150226
	cartlistequipType = 0xa0f,
#elif PACKETVER >= 20120925
	cartlistequipType = 0x994,
#elif PACKETVER >= 20080102
	cartlistequipType = 0x2d2,
#elif PACKETVER >= 20071002
	cartlistequipType = 0x297,
#else
	cartlistequipType = 0x122,
#endif
	openvendingType = 0x136,
#if PACKETVER >= 20120925
	equipitemType = 0x998,
#else
	equipitemType = 0xa9,
#endif
#if PACKETVER >= 20120925
	unequipitemackType = 0x99a,
#else
	unequipitemackType = 0xac,
#endif
	notifybindonequip = 0x2d3,
	monsterhpType = 0x977,
	maptypeproperty2Type = 0x99b,
#if PACKETVER >= 20131223  // version probably can be 20131030 [4144]
	wisendType = 0x9df,
#else
	wisendType = 0x98,
#endif
	partyleaderchangedType = 0x7fc,
	rouletteinfoackType = 0xa1c,
	roulettgenerateackType = 0xa20,
	roulettercvitemackType = 0xa22,
#if PACKETVER >= 20141016
	achievementListType = 0xa23,
	achievementUpdateType = 0xa24,
	achievementRewardAckType = 0xa26,
#endif // PACKETVER >= 20141016
#if PACKETVER_ZERO_NUM >= 20181010 || PACKETVER >= 20181017
	questListType = 0xaff, ///< ZC_ALL_QUEST_LIST4
#elif PACKETVER >= 20150513  // [4144] 0x09f8 handling in client from 2014-10-29aRagexe and 2014-03-26cRagexeRE
	questListType = 0x9f8, ///< ZC_ALL_QUEST_LIST3
#elif PACKETVER >= 20141022
	questListType = 0x97a, ///< ZC_ALL_QUEST_LIST2
#else // PACKETVER < 20141022
	questListType = 0x2b1, ///< ZC_ALL_QUEST_LIST
#endif // PACKETVER >= 20141022
	/* Rodex */
	rodexicon = 0x09E7,
	rodexwriteresult = 0x09ED,
	rodexnextpage = 0x09F0,
	rodexgetzeny = 0x09F2,
	rodexgetitem = 0x09F4,
	rodexdelete = 0x09F6,
	rodexremoveitem = 0x0A07,
	rodexopenwrite = 0x0A12,
#if PACKETVER < 20160601
	rodexmailList = 0x09F0,
#elif PACKETVER < 20170419
	rodexmailList = 0x0A7D,
#else // PACKETVER >= 20170419
	rodexmailList = 0x0Ac2,
#endif
#if PACKETVER >= 20151223
	skillscale = 0xA41,
#endif
#if PACKETVER >= 20130821
	progressbarunit = 0x09D1,
#endif
#if PACKETVER >= 20171207
	partymemberinfo = 0x0ae4,
	partyinfo = 0x0ae5,
#elif PACKETVER_MAIN_NUM >= 20170524 || PACKETVER_RE_NUM >= 20170502 || defined(PACKETVER_ZERO)
	partymemberinfo = 0x0a43,
	partyinfo = 0x0a44,
#else
	partymemberinfo = 0x01e9,
	partyinfo = 0x00fb,
#endif
#if PACKETVER >= 20120716
	clanOnlineCount = 0x0988, ///< ZC_NOTIFY_CLAN_CONNECTINFO
	clanLeave = 0x0989, ///< ZC_ACK_CLAN_LEAVE
	clanMessage = 0x098E, ///< ZC_NOTIFY_CLAN_CHAT
#endif
#if PACKETVER_ZERO_NUM >= 20181010 || PACKETVER >= 20181017
	questAddType = 0xb0c,
#elif PACKETVER >= 20150513 // [4144] 0x09f9 handled in client from 2014-10-29aRagexe and 2014-03-26cRagexeRE
	questAddType = 0x9f9,
#else
	questAddType = 0x2b3,
#endif // PACKETVER < 20150513
#if PACKETVER_ZERO_NUM >= 20181010 || PACKETVER >= 20181017
	questUpdateType = 0xafe,
#elif PACKETVER >= 20150513
	questUpdateType = 0x9fa,
#else
	questUpdateType = 0x2b5,
#endif // PACKETVER < 20150513
	questUpdateType2 = 0x8fe,
#if PACKETVER >= 20180627
	authError = 0xb02,
#elif PACKETVER >= 20101123
	authError = 0x83e,
#else
	authError = 0x6a,
#endif
#if PACKETVER >= 3
	useItemAckType = 0x1c8,
#else
	useItemAckType = 0xa8,
#endif
#if PACKETVER >= 4
	sendLookType = 0x1d7,
#else
	sendLookType = 0xc3,
#endif
#if PACKETVER >= 20141016
	buyingStoreUpdateItemType = 0x9e6,
#else
	buyingStoreUpdateItemType = 0x81b,
#endif
	reqName = 0x95,
#if PACKETVER_MAIN_NUM >= 20170502 || PACKETVER_RE_NUM >= 20170419 || defined(PACKETVER_ZERO)
	skilWarpPointType = 0xabe,
#else
	skilWarpPointType = 0x11c,
#endif
#if PACKETVER_MAIN_NUM >= 20161019 || PACKETVER_RE_NUM >= 20160921 || defined(PACKETVER_ZERO)
	guildExpulsion = 0xa82,
#elif PACKETVER >= 20100803
	guildExpulsion = 0x839,
#else
	guildExpulsion = 0x15c,
#endif
#if PACKETVER_MAIN_NUM >= 20161019 || PACKETVER_RE_NUM >= 20160921 || defined(PACKETVER_ZERO)
	guildLeave = 0xa83,
#else
	guildLeave = 0x15a,
#endif
};

#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
DEFINE_PACKET_ID(ZC_PAR_4JOB_CHANGE, 0x0b25);
#endif  // PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024

#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(push, 1)
#endif // not NetBSD < 6 / Solaris

struct PACKET_ZC_PAR_CHANGE {
	int16 PacketType;
	uint16 varID;
	int32 count;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PAR_CHANGE, 0x00b0);

struct PACKET_ZC_LONGPAR_CHANGE {
	int16 PacketType;
	uint16 varID;
	int32 amount;
} __attribute__((packed));
DEFINE_PACKET_ID(ZC_LONGPAR_CHANGE, 0x00b1);

struct PACKET_ZC_STATUS_CHANGE {
	int16 PacketType;
	uint16 statusID;
	uint8 value;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_STATUS_CHANGE, 0x00be);

struct PACKET_ZC_NOTIFY_CARTITEM_COUNTINFO {
	int16 PacketType;
	int16 curCount;
	int16 maxCount;
	int32 curWeight;
	int32 maxWeight;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_CARTITEM_COUNTINFO, 0x0121);

struct PACKET_ZC_ATTACK_RANGE {
	int16 PacketType;
	int16 currentAttRange;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ATTACK_RANGE, 0x013a);

struct PACKET_ZC_COUPLESTATUS {
	int16 PacketType;
	uint32 statusType;
	int32 defaultStatus;
	int32 plusStatus;
} __attribute__((packed));
DEFINE_PACKET_ID(ZC_COUPLESTATUS, 0x0141);

#if PACKETVER_MAIN_NUM >= 20170906 || PACKETVER_RE_NUM >= 20170830 || defined(PACKETVER_ZERO)
struct PACKET_ZC_LONGLONGPAR_CHANGE {
	int16 PacketType;
	uint16 varID;
	int64 amount;
} __attribute__((packed));
DEFINE_PACKET_ID(ZC_LONGLONGPAR_CHANGE, 0x0acb);
#endif  // PACKETVER_MAIN_NUM >= 20170906 || PACKETVER_RE_NUM >= 20170830 || defined(PACKETVER_ZERO)

/**
 * structs for data
 */
struct EQUIPSLOTINFO {
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 card[4];
#else
	uint16 card[4];
#endif
} __attribute__((packed));

struct NORMALITEM_INFO {
	int16 index;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 ITID;
#else
	uint16 ITID;
#endif
	uint8 type;
#if PACKETVER < 20120925
	uint8 IsIdentified;
#endif
	int16 count;
#if PACKETVER >= 20120925
	uint32 WearState;
#else
	uint16 WearState;
#endif
#if PACKETVER >= 5
	struct EQUIPSLOTINFO slot;
#endif
#if PACKETVER >= 20080102
	int32 HireExpireDate;
#endif
#if PACKETVER >= 20120925
	struct {
		uint8 IsIdentified : 1;
		uint8 PlaceETCTab : 1;
		uint8 SpareBits : 6;
	} Flag;
#endif
} __attribute__((packed));

struct ItemOptions {
	int16 index;
	int16 value;
	uint8 param;
} __attribute__((packed));

// TODO split struct inside blocks of #if/#elif
struct EQUIPITEM_INFO {
	int16 index;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 ITID;
#else
	uint16 ITID;
#endif
	uint8 type;
#if PACKETVER < 20120925
	uint8 IsIdentified;
#endif
#if PACKETVER >= 20120925
	uint32 location;
	uint32 WearState;
#else
	uint16 location;
	uint16 WearState;
#endif
#if PACKETVER < 20120925
	uint8 IsDamaged;
#endif
#if !(PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024)
	uint8 RefiningLevel;
#endif  // !(PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024)
	struct EQUIPSLOTINFO slot;
#if PACKETVER >= 20071002
	int32 HireExpireDate;
#endif
#if PACKETVER >= 20080102
	uint16 bindOnEquipType;
#endif
#if PACKETVER >= 20100629
	uint16 wItemSpriteNumber;
#endif
#if PACKETVER >= 20150226
	uint8 option_count;
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
#endif
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
	uint8 RefiningLevel;
	uint8 grade;
#endif  // PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
#if PACKETVER >= 20120925
	struct {
		uint8 IsIdentified : 1;
		uint8 IsDamaged : 1;
		uint8 PlaceETCTab : 1;
		uint8 SpareBits : 5;
	} Flag;
#endif
} __attribute__((packed));

struct packet_authok {
	int16 PacketType;
	uint32 startTime;
	uint8 PosDir[3];
	uint8 xSize;
	uint8 ySize;
#if PACKETVER >= 20080102
	int16 font;
#endif
// Some clients smaller than 20160330 cant be tested [4144]
#if PACKETVER >= 20141022 && PACKETVER < 20160330
	uint8 sex;
#endif
} __attribute__((packed));

struct packet_monster_hp {
	int16 PacketType;
	uint32 GID;
	int32 HP;
	int32 MaxHP;
} __attribute__((packed));

struct packet_sc_notick {
	int16 PacketType;
	int16 index;
	uint32 AID;
	uint8 state;
} __attribute__((packed));

// TODO put struct under #ifdef/#elif
// [4144] dates unconfirmed
struct PACKET_ZC_ITEM_PICKUP_ACK {
	int16 PacketType;
	uint16 Index;
	uint16 count;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 nameid;
#else
	uint16 nameid;
#endif
	uint8 IsIdentified;
	uint8 IsDamaged;
#if !(PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024)
	uint8 refiningLevel;
#endif  // !(PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024)
	struct EQUIPSLOTINFO slot;
#if PACKETVER >= 20120925
	uint32 location;
#else
	uint16 location;
#endif
	uint8 type;
	uint8 result;
#if PACKETVER >= 20061218
	int32 HireExpireDate;
#endif
#if PACKETVER >= 20071002
	uint16 bindOnEquipType;
#endif
#if PACKETVER >= 20150226
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
#endif
#if PACKETVER >= 20160921
	uint8 favorite;
	uint16 look;
#endif
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
	uint8 refiningLevel;
	uint8 grade;
#endif  // PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
DEFINE_PACKET_HEADER(ZC_ITEM_PICKUP_ACK, 0x0b41);
#elif PACKETVER >= 20160921
DEFINE_PACKET_HEADER(ZC_ITEM_PICKUP_ACK, 0x0a37);
#elif PACKETVER >= 20150226
DEFINE_PACKET_HEADER(ZC_ITEM_PICKUP_ACK, 0x0a0c);
#elif PACKETVER >= 20120925
DEFINE_PACKET_HEADER(ZC_ITEM_PICKUP_ACK, 0x0990);
#elif PACKETVER >= 20071002
DEFINE_PACKET_HEADER(ZC_ITEM_PICKUP_ACK, 0x02d4);
#elif PACKETVER >= 20061218
DEFINE_PACKET_HEADER(ZC_ITEM_PICKUP_ACK, 0x029a);
#else
DEFINE_PACKET_HEADER(ZC_ITEM_PICKUP_ACK, 0x00a0);
#endif

struct packet_dropflooritem {
	int16 PacketType;
	uint32 ITAID;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 ITID;
#else
	uint16 ITID;
#endif
#if PACKETVER >= 20130000 /* not sure date */
	uint16 type;
#endif
	uint8 IsIdentified;
	int16 xPos;
	int16 yPos;
	uint8 subX;
	uint8 subY;
	int16 count;
#if defined(PACKETVER_ZERO) || PACKETVER >= 20180418
	int8 showdropeffect;
	int16 dropeffectmode;
#endif
} __attribute__((packed));
struct packet_idle_unit2 {
#if PACKETVER < 20091103
	int16 PacketType;
#if PACKETVER >= 20071106
	uint8 objecttype;
#endif
	uint32 GID;
	int16 speed;
	int16 bodyState;
	int16 healthState;
	int16 effectState;
	int16 job;
	uint16 head;
	uint16 weapon;
	uint16 accessory;
	uint16 shield;
	uint16 accessory2;
	uint16 accessory3;
	int16 headpalette;
	int16 bodypalette;
	int16 headDir;
	uint32 GUID;
	int16 GEmblemVer;
	int16 honor;
	int16 virtue;
	uint8 isPKModeON;
	uint8 sex;
	uint8 PosDir[3];
	uint8 xSize;
	uint8 ySize;
	uint8 state;
	int16 clevel;
#else // ! PACKETVER < 20091103
	UNAVAILABLE_STRUCT;
#endif // PACKETVER < 20091103
} __attribute__((packed));

struct packet_spawn_unit2 {
#if PACKETVER < 20091103
	int16 PacketType;
#if PACKETVER >= 20071106
	uint8 objecttype;
#endif
	uint32 GID;
	int16 speed;
	int16 bodyState;
	int16 healthState;
	int16 effectState;
	uint16 head;
	uint16 weapon;
	uint16 accessory;
	int16 job;
	uint16 shield;
	uint16 accessory2;
	uint16 accessory3;
	int16 headpalette;
	int16 bodypalette;
	int16 headDir;
	uint8 isPKModeON;
	uint8 sex;
	uint8 PosDir[3];
	uint8 xSize;
	uint8 ySize;
#else // ! PACKETVER < 20091103
	UNAVAILABLE_STRUCT;
#endif // PACKETVER < 20091103
} __attribute__((packed));

struct packet_spawn_unit {
	int16 PacketType;
#if PACKETVER >= 20091103
	int16 PacketLength;
	uint8 objecttype;
#endif
#if PACKETVER >= 20131223
	uint32 AID;
#endif
	uint32 GID;
	int16 speed;
	int16 bodyState;
	int16 healthState;
#if PACKETVER < 20080102
	int16 effectState;
#else
	int32 effectState;
#endif
	int16 job;
	uint16 head;
#if PACKETVER < 7
	uint16 weapon;
#else
	uint32 weapon;
#endif
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 shield;
#endif
	uint16 accessory;
#if PACKETVER < 7
	uint16 shield;
#endif
	uint16 accessory2;
	uint16 accessory3;
	int16 headpalette;
	int16 bodypalette;
	int16 headDir;
#if PACKETVER >= 20101124
	uint16 robe;
#endif
	uint32 GUID;
	int16 GEmblemVer;
	int16 honor;
#if PACKETVER > 7
	int32 virtue;
#else
	int16 virtue;
#endif
	uint8 isPKModeON;
	uint8 sex;
	uint8 PosDir[3];
	uint8 xSize;
	uint8 ySize;
	int16 clevel;
#if PACKETVER >= 20080102
	int16 font;
#endif
#if PACKETVER >= 20120221
	int32 maxHP;
	int32 HP;
	uint8 isBoss;
#endif
#if PACKETVER >= 20150513
	int16 body;
#endif
/* Might be earlier, this is when the named item bug began */
#if PACKETVER >= 20131223
	char name[NAME_LENGTH];
#endif
} __attribute__((packed));

struct packet_unit_walking {
	int16 PacketType;
#if PACKETVER >= 20091103
	int16 PacketLength;
#endif
#if PACKETVER >= 20071106
	uint8 objecttype;
#endif
#if PACKETVER >= 20131223
	uint32 AID;
#endif
	uint32 GID;
	int16 speed;
	int16 bodyState;
	int16 healthState;
#if PACKETVER < 7
	int16 effectState;
#else
	int32 effectState;
#endif
	int16 job;
	uint16 head;
#if PACKETVER < 7
	uint16 weapon;
#else
	uint32 weapon;
#endif
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 shield;
#endif
	uint16 accessory;
	uint32 moveStartTime;
#if PACKETVER < 7
	uint16 shield;
#endif
	uint16 accessory2;
	uint16 accessory3;
	int16 headpalette;
	int16 bodypalette;
	int16 headDir;
#if PACKETVER >= 20101124
	uint16 robe;
#endif
	uint32 GUID;
	int16 GEmblemVer;
	int16 honor;
#if PACKETVER > 7
	int32 virtue;
#else
	int16 virtue;
#endif
	uint8 isPKModeON;
	uint8 sex;
	uint8 MoveData[6];
	uint8 xSize;
	uint8 ySize;
	int16 clevel;
#if PACKETVER >= 20080102
	int16 font;
#endif
#if PACKETVER >= 20120221
	int32 maxHP;
	int32 HP;
	uint8 isBoss;
#endif
#if PACKETVER >= 20150513
	uint16 body;
#endif
/* Might be earlier, this is when the named item bug began */
#if PACKETVER >= 20131223
	char name[NAME_LENGTH];
#endif
} __attribute__((packed));

struct packet_idle_unit {
	int16 PacketType;
#if PACKETVER >= 20091103
	int16 PacketLength;
	uint8 objecttype;
#endif
#if PACKETVER >= 20131223
	uint32 AID;
#endif
	uint32 GID;
	int16 speed;
	int16 bodyState;
	int16 healthState;
#if PACKETVER < 20080102
	int16 effectState;
#else
	int32 effectState;
#endif
	int16 job;
	uint16 head;
#if PACKETVER < 7
	uint16 weapon;
#else
	uint32 weapon;
#endif
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 shield;
#endif
	uint16 accessory;
#if PACKETVER < 7
	uint16 shield;
#endif
	uint16 accessory2;
	uint16 accessory3;
	int16 headpalette;
	int16 bodypalette;
	int16 headDir;
#if PACKETVER >= 20101124
	uint16 robe;
#endif
	uint32 GUID;
	int16 GEmblemVer;
	int16 honor;
#if PACKETVER > 7
	int32 virtue;
#else
	int16 virtue;
#endif
	uint8 isPKModeON;
	uint8 sex;
	uint8 PosDir[3];
	uint8 xSize;
	uint8 ySize;
	uint8 state;
	int16 clevel;
#if PACKETVER >= 20080102
	int16 font;
#endif
#if PACKETVER >= 20120221
	int32 maxHP;
	int32 HP;
	uint8 isBoss;
#endif
#if PACKETVER >= 20150513
	uint16 body;
#endif
/* Might be earlier, this is when the named item bug began */
#if PACKETVER >= 20131223
	char name[NAME_LENGTH];
#endif
} __attribute__((packed));

struct packet_status_change {
	int16 PacketType;
	int16 index;
	uint32 AID;
	uint8 state;
#if PACKETVER >= 20120618
	uint32 Total;
#endif
#if PACKETVER >= 20090121
	uint32 Left;
	int32 val1;
	int32 val2;
	int32 val3;
#endif
} __attribute__((packed));

struct packet_status_change_end {
	int16 PacketType;
	int16 index;
	uint32 AID;
	uint8 state;
} __attribute__((packed));

struct packet_status_change2 {
	int16 PacketType;
	int16 index;
	uint32 AID;
	uint8 state;
	uint32 Left;
	int32 val1;
	int32 val2;
	int32 val3;
} __attribute__((packed));

struct packet_maptypeproperty2 {
	int16 PacketType;
	int16 type;
	struct {
		uint32 party             : 1;  // Show attack cursor on non-party members (PvP)
		uint32 guild             : 1;  // Show attack cursor on non-guild members (GvG)
		uint32 siege             : 1;  // Show emblem over characters' heads when in GvG (WoE castle)
		uint32 mineffect         : 1;  // Automatically enable /mineffect
		uint32 nolockon          : 1;  // TODO: What does this do? (shows attack cursor on non-party members)
		uint32 countpk           : 1;  /// Show the PvP counter
		uint32 nopartyformation  : 1;  /// Prevent party creation/modification
		uint32 bg                : 1;  // TODO: What does this do? Probably related to Battlegrounds, but I'm not sure on the effect
		uint32 nocostume         : 1;  /// Does not show costume sprite.
		uint32 usecart           : 1;  /// Allow opening cart inventory
		uint32 summonstarmiracle : 1;  // TODO: What does this do? Related to Taekwon Masters, but I have no idea.
		uint32 SpareBits         : 21; /// Currently ignored, reserved for future updates
	} flag;
} __attribute__((packed));

struct packet_bgqueue_ack {
	int16 PacketType;
	uint8 type;
	char bg_name[NAME_LENGTH];
} __attribute__((packed));

struct packet_bgqueue_notice_delete {
	int16 PacketType;
	uint8 type;
	char bg_name[NAME_LENGTH];
} __attribute__((packed));

struct packet_bgqueue_register {
	int16 PacketType;
	int16 type;
	char bg_name[NAME_LENGTH];
} __attribute__((packed));

struct packet_bgqueue_update_info {
	int16 PacketType;
	char bg_name[NAME_LENGTH];
	int32 position;
} __attribute__((packed));

struct packet_bgqueue_checkstate {
	int16 PacketType;
	char bg_name[NAME_LENGTH];
} __attribute__((packed));

struct packet_bgqueue_revoke_req {
	int16 PacketType;
	char bg_name[NAME_LENGTH];
} __attribute__((packed));

struct packet_bgqueue_battlebegin_ack {
	int16 PacketType;
	uint8 result;
	char bg_name[NAME_LENGTH];
	char game_name[NAME_LENGTH];
} __attribute__((packed));

struct packet_bgqueue_notify_entry {
	int16 PacketType;
	char name[NAME_LENGTH];
	int32 position;
} __attribute__((packed));

struct packet_bgqueue_battlebegins {
	int16 PacketType;
	char bg_name[NAME_LENGTH];
	char game_name[NAME_LENGTH];
} __attribute__((packed));

struct packet_script_clear {
	int16 PacketType;
	uint32 NpcID;
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20220518 || PACKETVER_ZERO_NUM >= 20220518
struct PACKET_ZC_BROADCASTING_SPECIAL_ITEM_OBTAIN_item {
	int16 PacketType;
	int16 PacketLength;
	uint8 type;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 ItemID;
#else
	uint16 ItemID;
#endif
	int8 len;
	char Name[NAME_LENGTH];
	int8 boxItemID_len;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 BoxItemID;
#else
	uint16 BoxItemID;
#endif
	int8 refineLevel_len;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 refineLevel;
#else
	uint16 refineLevel;
#endif
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BROADCASTING_SPECIAL_ITEM_OBTAIN_item, 0x0bba)
#elif PACKETVER >= 20091201
/* made possible thanks to Yommy!! */
struct PACKET_ZC_BROADCASTING_SPECIAL_ITEM_OBTAIN_item {
	int16 PacketType;
	int16 PacketLength;
	uint8 type;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 ItemID;
#else
	uint16 ItemID;
#endif
	int8 len;
	char Name[NAME_LENGTH];
	int8 boxItemID_len;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 BoxItemID;
#else
	uint16 BoxItemID;
#endif
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BROADCASTING_SPECIAL_ITEM_OBTAIN_item, 0x07fd)
#endif

/* made possible thanks to Yommy!! */
struct packet_item_drop_announce {
	int16 PacketType;
	int16 PacketLength;
	uint8 type;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 ItemID;
#else
	uint16 ItemID;
#endif
	int8 len;
	char Name[NAME_LENGTH];
	char monsterNameLen;
	char monsterName[NAME_LENGTH];
} __attribute__((packed));

struct packet_cart_additem_ack {
	int16 PacketType;
	int8 result;
} __attribute__((packed));

struct packet_banking_check {
	int16 PacketType;
	int64 Money;
	int16 Reason;
} __attribute__((packed));

struct packet_banking_deposit_req {
	int16 PacketType;
	uint32 AID;
	int32 Money;
} __attribute__((packed));

struct packet_banking_withdraw_req {
	int16 PacketType;
	uint32 AID;
	int32 Money;
} __attribute__((packed));

struct packet_banking_deposit_ack {
	int16 PacketType;
	int16 Reason;
	int64 Money;
	int32 Balance;
} __attribute__((packed));

struct packet_banking_withdraw_ack {
	int16 PacketType;
	int16 Reason;
	int64 Money;
	int32 Balance;
} __attribute__((packed));

/* Roulette System [Yommy/Hercules] */
struct packet_roulette_open_ack {
	int16 PacketType;
	int8 Result;
	int32 Serial;
	int8 Step;
	int8 Idx;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 AdditionItemID;
#else
	uint16 AdditionItemID;
#endif
	int32 GoldPoint;
	int32 SilverPoint;
	int32 BronzePoint;
} __attribute__((packed));

struct packet_roulette_info_ack {
	int16 PacketType;
	int16 PacketLength;
	uint32 RouletteSerial;
	struct {
		uint16 Row;
		uint16 Position;
#if PACKETVER >= 20180511
		uint32 ItemId;
		uint16 Count;
		uint16 unused;
#else
		uint16 ItemId;
		uint16 Count;
#endif
	} ItemInfo[42];
} __attribute__((packed));

struct packet_roulette_close_ack {
	int16 PacketType;
	uint8 Result;
} __attribute__((packed));

struct packet_roulette_generate_ack {
	int16 PacketType;
	uint8 Result;
	uint16 Step;
	uint16 Idx;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 AdditionItemID;
#else
	uint16 AdditionItemID;
#endif
	int32 RemainGold;
	int32 RemainSilver;
	int32 RemainBronze;
} __attribute__((packed));

struct packet_roulette_itemrecv_req {
	int16 PacketType;
	uint8 Condition;
} __attribute__((packed));

struct packet_roulette_itemrecv_ack {
	int16 PacketType;
	uint8 Result;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 AdditionItemID;
#else
	uint16 AdditionItemID;
#endif
} __attribute__((packed));

struct packet_itemlist_normal {
	int16 PacketType;
	int16 PacketLength;
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	uint8 invType;
#endif
	struct NORMALITEM_INFO list[MAX_ITEMLIST];
} __attribute__((packed));

struct packet_itemlist_equip {
	int16 PacketType;
	int16 PacketLength;
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	uint8 invType;
#endif
	struct EQUIPITEM_INFO list[MAX_ITEMLIST];
} __attribute__((packed));

struct ZC_STORE_ITEMLIST_NORMAL {
	int16 PacketType;
	int16 PacketLength;
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	uint8 invType;
#endif
#if PACKETVER >= 20120925 && PACKETVER_RE_NUM < 20180829 && PACKETVER_ZERO_NUM < 20180919 && PACKETVER_MAIN_NUM < 20181002
	char name[NAME_LENGTH];
#endif
	struct NORMALITEM_INFO list[MAX_ITEMLIST];
} __attribute__((packed));

#if PACKETVER_RE_NUM >= 20180829 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
struct PACKET_ZC_INVENTORY_START {
	int16 packetType;
#if PACKETVER_RE_NUM >= 20180919 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	int16 packetLength;
#endif
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	uint8 invType;
#endif
#if PACKETVER_RE_NUM >= 20180919 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	char name[];
#else
	char name[NAME_LENGTH];
#endif
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_INVENTORY_START, 0x0b08);
#endif  // PACKETVER_RE_NUM >= 20180829 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002

#if PACKETVER_RE_NUM >= 20180829 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
struct PACKET_ZC_INVENTORY_END {
	int16 packetType;
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	uint8 invType;
#endif
	char flag;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_INVENTORY_END, 0x0b0b);
#endif  // PACKETVER_RE_NUM >= 20180829 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002

struct ZC_STORE_ITEMLIST_EQUIP {
	int16 PacketType;
	int16 PacketLength;
#if PACKETVER_RE_NUM >= 20180912 || PACKETVER_ZERO_NUM >= 20180919 || PACKETVER_MAIN_NUM >= 20181002
	uint8 invType;
#endif
#if PACKETVER >= 20120925 && PACKETVER_RE_NUM < 20180829 && PACKETVER_ZERO_NUM < 20180919 && PACKETVER_MAIN_NUM < 20181002
	char name[NAME_LENGTH];
#endif
	struct EQUIPITEM_INFO list[MAX_ITEMLIST];
} __attribute__((packed));

struct packet_equip_item {
	int16 PacketType;
	uint16 index;
#if PACKETVER >= 20120925
	uint32 wearLocation;
#else
	uint16 wearLocation;
#endif
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20121205 || PACKETVER_RE_NUM >= 20121107 || defined(PACKETVER_ZERO)
struct PACKET_ZC_REQ_WEAR_EQUIP_ACK {
	int16 PacketType;
	uint16 index;
	uint32 wearLocation;
	uint16 wItemSpriteNumber;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_REQ_WEAR_EQUIP_ACK, 0x0999)
#elif PACKETVER_MAIN_NUM >= 20101123 || PACKETVER_RE_NUM >= 20100629
struct PACKET_ZC_REQ_WEAR_EQUIP_ACK {
	int16 PacketType;
	uint16 index;
	uint16 wearLocation;
	uint16 wItemSpriteNumber;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_REQ_WEAR_EQUIP_ACK, 0x00aa)
#else  // PACKETVER_MAIN_NUM >= 20121205 || PACKETVER_RE_NUM >= 20121107 || defined(PACKETVER_ZERO)
struct PACKET_ZC_REQ_WEAR_EQUIP_ACK {
	int16 PacketType;
	uint16 index;
	uint16 wearLocation;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_REQ_WEAR_EQUIP_ACK, 0x00aa)
#endif  // PACKETVER_MAIN_NUM >= 20121205 || PACKETVER_RE_NUM >= 20121107 || defined(PACKETVER_ZERO)

struct packet_unequipitem_ack {
	int16 PacketType;
	uint16 index;
#if PACKETVER >= 20120925
	uint32 wearLocation;
#else
	uint16 wearLocation;
#endif
	uint8 result;
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_EQUIPWIN_MICROSCOPE {
	int16 PacketType;
	int16 PacketLength;
	char characterName[NAME_LENGTH];
	int16 job;
	int16 head;
	int16 accessory;
	int16 accessory2;
	int16 accessory3;
	int16 robe;
	int16 headpalette;
	int16 bodypalette;
	int16 body2;
	uint8 sex;
	struct EQUIPITEM_INFO list[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_EQUIPWIN_MICROSCOPE, 0x0b37);
#elif PACKETVER_MAIN_NUM >= 20180801 || PACKETVER_RE_NUM >= 20180801 || PACKETVER_ZERO_NUM >= 20180808
struct PACKET_ZC_EQUIPWIN_MICROSCOPE {
	int16 PacketType;
	int16 PacketLength;
	char characterName[NAME_LENGTH];
	int16 job;
	int16 head;
	int16 accessory;
	int16 accessory2;
	int16 accessory3;
	int16 robe;
	int16 headpalette;
	int16 bodypalette;
	int16 body2;
	uint8 sex;
	struct EQUIPITEM_INFO list[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_EQUIPWIN_MICROSCOPE, 0x0b03);
#elif PACKETVER >= 20140820
struct PACKET_ZC_EQUIPWIN_MICROSCOPE {
	int16 PacketType;
	int16 PacketLength;
	char characterName[NAME_LENGTH];
	int16 job;
	int16 head;
	int16 accessory;
	int16 accessory2;
	int16 accessory3;
	int16 robe;
	int16 headpalette;
	int16 bodypalette;
	uint8 sex;
	struct EQUIPITEM_INFO list[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_EQUIPWIN_MICROSCOPE, 0x0a2d);
#elif PACKETVER_MAIN_NUM >= 20121205 || PACKETVER_RE_NUM >= 20121107
struct PACKET_ZC_EQUIPWIN_MICROSCOPE {
	int16 PacketType;
	int16 PacketLength;
	char characterName[NAME_LENGTH];
	int16 job;
	int16 head;
	int16 accessory;
	int16 accessory2;
	int16 accessory3;
	int16 robe;
	int16 headpalette;
	int16 bodypalette;
	uint8 sex;
	struct EQUIPITEM_INFO list[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_EQUIPWIN_MICROSCOPE, 0x0997);
#elif PACKETVER_MAIN_NUM >= 20111207 || PACKETVER_RE_NUM >= 20111122
struct PACKET_ZC_EQUIPWIN_MICROSCOPE {
	int16 PacketType;
	int16 PacketLength;
	char characterName[NAME_LENGTH];
	int16 job;
	int16 head;
	int16 accessory;
	int16 accessory2;
	int16 accessory3;
	int16 robe;
	int16 headpalette;
	int16 bodypalette;
	uint8 sex;
	struct EQUIPITEM_INFO list[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_EQUIPWIN_MICROSCOPE, 0x0906);
#elif PACKETVER >= 20101123
struct PACKET_ZC_EQUIPWIN_MICROSCOPE {
	int16 PacketType;
	int16 PacketLength;
	char characterName[NAME_LENGTH];
	int16 job;
	int16 head;
	int16 accessory;
	int16 accessory2;
	int16 accessory3;
	int16 robe;
	int16 headpalette;
	int16 bodypalette;
	uint8 sex;
	struct EQUIPITEM_INFO list[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_EQUIPWIN_MICROSCOPE, 0x0859);
#elif PACKETVER_AD_NUM >= 20071211 || PACKETVER_SAK_NUM >= 20071127 || PACKETVER_MAIN_NUM >= 20071211 || defined(PACKETVER_RE)
struct PACKET_ZC_EQUIPWIN_MICROSCOPE {
	int16 PacketType;
	int16 PacketLength;
	char characterName[NAME_LENGTH];
	int16 job;
	int16 head;
	int16 accessory;
	int16 accessory2;
	int16 accessory3;
	int16 headpalette;
	int16 bodypalette;
	uint8 sex;
	struct EQUIPITEM_INFO list[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_EQUIPWIN_MICROSCOPE, 0x02d7);
#endif

struct packet_notify_bounditem {
	int16 PacketType;
	uint16 index;
} __attribute__((packed));

struct packet_skill_entry {
	int16 PacketType;
#if PACKETVER >= 20110718
	int16 PacketLength;
#endif
	uint32 AID;
	uint32 creatorAID;
	int16 xPos;
	int16 yPos;
#if PACKETVER >= 20121212
	int32 job;
#else
	uint8 job;
#endif
#if PACKETVER >= 20110718
	int8 RadiusRange;
#endif
	uint8 isVisible;
#if PACKETVER >= 20130731
	uint8 level;
#endif
} __attribute__((packed));

struct packet_graffiti_entry {
	int16 PacketType;
	uint32 AID;
	uint32 creatorAID;
	int16 xPos;
	int16 yPos;
	uint8 job;
	uint8 isVisible;
	uint8 isContens;
	char msg[80];
} __attribute__((packed));

struct packet_damage {
	int16 PacketType;
	uint32 GID;
	uint32 targetGID;
	uint32 startTime;
	int32 attackMT;
	int32 attackedMT;
#if PACKETVER < 20071113
	int16 damage;
#else
	int32 damage;
#endif
#if PACKETVER >= 20131223
	uint8 is_sp_damaged;
#endif
	int16 count;
	uint8 action;
#if PACKETVER < 20071113
	int16 leftDamage;
#else
	int32 leftDamage;
#endif
} __attribute__((packed));

struct packet_gm_monster_item {
	int16 PacketType;
#if PACKETVER >= 20131218
	char str[100];
#else
	char str[24];
#endif
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20130911 || PACKETVER_RE_NUM >= 20130911 || defined(PACKETVER_ZERO)
struct PACKET_CZ_NPC_MARKET_PURCHASE_sub {
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 ITID;
#else
	uint16 ITID;
#endif
	int32 qty;
} __attribute__((packed));

struct PACKET_CZ_NPC_MARKET_PURCHASE {
	int16 PacketType;
	int16 PacketLength;
	struct PACKET_CZ_NPC_MARKET_PURCHASE_sub list[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_NPC_MARKET_PURCHASE, 0x09d6)
#endif

#if PACKETVER_MAIN_NUM >= 20210203 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_NPC_MARKET_OPEN_sub {
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 nameid;
#else
	uint16 nameid;
#endif
	uint8 type;
	uint32 price;
	uint32 qty;
	uint16 weight;
	uint32 location;
} __attribute__((packed));
struct PACKET_ZC_NPC_MARKET_OPEN {
	int16 packetType;
	int16 packetLength;
	struct PACKET_ZC_NPC_MARKET_OPEN_sub list[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NPC_MARKET_OPEN, 0x0b7a);
#elif PACKETVER_MAIN_NUM >= 20131120 || PACKETVER_RE_NUM >= 20131106 || defined(PACKETVER_ZERO)
/* inner struct figured by Ind after some annoying hour of debugging (data Thanks to Yommy) */
struct PACKET_ZC_NPC_MARKET_OPEN_sub {
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 nameid;
#else
	uint16 nameid;
#endif
	uint8 type;
	uint32 price;
	uint32 qty;
	uint16 weight;
} __attribute__((packed));
struct PACKET_ZC_NPC_MARKET_OPEN {
	int16 packetType;
	int16 packetLength;
	struct PACKET_ZC_NPC_MARKET_OPEN_sub list[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NPC_MARKET_OPEN, 0x09d5);
#endif

struct packet_wis_end {
	int16 PacketType;
	int8 result;
#if PACKETVER >= 20131223
	uint32 AID;
#endif
} __attribute__((packed));


struct packet_party_leader_changed {
	int16 PacketType;
	uint32 prev_leader_aid;
	uint32 new_leader_aid;
} __attribute__((packed));

#ifdef HOTKEY_SAVING
struct hotkey_data {
	int8 isSkill; // 0: Item, 1:Skill
	uint32 id;    // Item/Skill ID
	int16 count;  // Item Quantity/Skill Level
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20190522 || PACKETVER_RE_NUM >= 20190508 || PACKETVER_ZERO_NUM >= 20190605
#define MAX_HOTKEYS_PACKET 38
struct PACKET_ZC_SHORTCUT_KEY_LIST {
	int16 packetType;
	int8 rotate;
	int16 tab;
	struct hotkey_data hotkey[MAX_HOTKEYS_PACKET];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SHORTCUT_KEY_LIST, 0x0b20);
#elif PACKETVER_MAIN_NUM >= 20141022 || PACKETVER_RE_NUM >= 20141015 || defined(PACKETVER_ZERO)
#define MAX_HOTKEYS_PACKET 38
struct PACKET_ZC_SHORTCUT_KEY_LIST {
	int16 packetType;
	int8 rotate;
	struct hotkey_data hotkey[MAX_HOTKEYS_PACKET];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SHORTCUT_KEY_LIST, 0x0a00);
#elif PACKETVER_MAIN_NUM >= 20090617 || PACKETVER_RE_NUM >= 20090617 || PACKETVER_SAK_NUM >= 20090617
#define MAX_HOTKEYS_PACKET 38
struct PACKET_ZC_SHORTCUT_KEY_LIST {
	int16 packetType;
	struct hotkey_data hotkey[MAX_HOTKEYS_PACKET];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SHORTCUT_KEY_LIST, 0x07d9);
#elif PACKETVER_MAIN_NUM >= 20090603 || PACKETVER_RE_NUM >= 20090603 || PACKETVER_SAK_NUM >= 20090603
#define MAX_HOTKEYS_PACKET 36
struct PACKET_ZC_SHORTCUT_KEY_LIST {
	int16 packetType;
	struct hotkey_data hotkey[MAX_HOTKEYS_PACKET];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SHORTCUT_KEY_LIST, 0x07d9);
#elif PACKETVER_MAIN_NUM >= 20070711 || PACKETVER_RE_NUM >= 20080827 || PACKETVER_AD_NUM >= 20070711 || PACKETVER_SAK_NUM >= 20070628
#define MAX_HOTKEYS_PACKET 27
struct PACKET_ZC_SHORTCUT_KEY_LIST {
	int16 packetType;
	struct hotkey_data hotkey[MAX_HOTKEYS_PACKET];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SHORTCUT_KEY_LIST, 0x02b9);
#endif

#if PACKETVER_MAIN_NUM >= 20070618 || defined(PACKETVER_RE) || defined(PACKETVER_ZERO) || PACKETVER_AD_NUM >= 20070618 || PACKETVER_SAK_NUM >= 20070618
struct PACKET_CZ_SHORTCUT_KEY_CHANGE1 {
	int16 packetType;
	uint16 index;
	struct hotkey_data hotkey;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_SHORTCUT_KEY_CHANGE1, 0x02ba);
#endif

#if PACKETVER_MAIN_NUM >= 20190522 || PACKETVER_RE_NUM >= 20190508 || PACKETVER_ZERO_NUM >= 20190605
struct PACKET_CZ_SHORTCUT_KEY_CHANGE2 {
	int16 packetType;
	uint16 tab;
	uint16 index;
	struct hotkey_data hotkey;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_SHORTCUT_KEY_CHANGE2, 0x0b21);
#endif

#if PACKETVER_MAIN_NUM >= 20140129 || PACKETVER_RE_NUM >= 20140129 || defined(PACKETVER_ZERO)
struct PACKET_CZ_SHORTCUTKEYBAR_ROTATE1 {
	int16 packetType;
	uint8 rowshift;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_SHORTCUTKEYBAR_ROTATE1, 0x0a01);
#endif

#if PACKETVER_MAIN_NUM >= 20190522 || PACKETVER_RE_NUM >= 20190508 || PACKETVER_ZERO_NUM >= 20190605
struct PACKET_CZ_SHORTCUTKEYBAR_ROTATE2 {
	int16 packetType;
	uint16 tab;
	uint8 rowshift;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_SHORTCUTKEYBAR_ROTATE2, 0x0b22);
#endif

#endif // HOTKEY_SAVING

/**
 * MISSION_HUNT_INFO (PACKETVER >= 20141022)
 * MISSION_HUNT_INFO_EX (PACKETVER >= 20150513)
 */
struct packet_mission_info_sub {
#if PACKETVER_ZERO_NUM >= 20181010 || PACKETVER >= 20181017
	uint32 huntIdent;
	uint32 huntIdent2;
	uint32 mobType;
#elif PACKETVER >= 20150513
	uint32 huntIdent;
	uint32 mobType;
#endif
	uint32 mob_id;
#if PACKETVER >= 20150513
	int16 levelMin;
	int16 levelMax;
#endif
	int16 huntCount;
	int16 maxCount;
	char mobName[NAME_LENGTH];
} __attribute__((packed));

/**
 * PACKET_ZC_ALL_QUEST_LIST2_INFO (PACKETVER >= 20141022)
 * PACKET_ZC_ALL_QUEST_LIST3_INFO (PACKETVER >= 20150513)
 */
struct packet_quest_list_info {
	int32 questID;
	int8 active;
#if PACKETVER >= 20141022
	int32 quest_svrTime;
	int32 quest_endTime;
	int16 hunting_count;
	struct packet_mission_info_sub objectives[]; // Note: This will be < MAX_QUEST_OBJECTIVES
#endif // PACKETVER >= 20141022
} __attribute__((packed));

/**
 * Header for:
 * PACKET_ZC_ALL_QUEST_LIST (PACKETVER < 20141022)
 * PACKET_ZC_ALL_QUEST_LIST2 (PACKETVER >= 20141022)
 * PACKET_ZC_ALL_QUEST_LIST3 (PACKETVER >= 20150513)
 *
 * @remark
 *     Contains (is followed by) a variable-length array of packet_quest_list_info
 */
struct packet_quest_list_header {
	uint16 PacketType;
	uint16 PacketLength;
	int32 questCount;
	//struct packet_quest_list_info list[]; // Variable-length
} __attribute__((packed));

struct packet_chat_message {
	uint16 packet_id;
	int16 packet_len;
	char message[];
} __attribute__((packed));

struct packet_whisper_message {
	uint16 packet_id;
	int16 packet_len;
	char name[NAME_LENGTH];
	char message[];
} __attribute__((packed));

/* RoDEX */
struct PACKET_CZ_ADD_ITEM_TO_MAIL {
	int16 PacketType;
	int16 index;
	int16 count;
} __attribute__((packed));

// [4144] this packet exists from
// PACKETVER_MAIN_NUM >= 20141112 || PACKETVER_RE_NUM >= 20140924 || defined(PACKETVER_ZERO)
// but used only packet versions with known struct
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_ACK_ADD_ITEM_RODEX {
	int16 PacketType;
	int8 result;
	int16 index;
	int16 count;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	int8 type;
	int8 IsIdentified;
	int8 IsDamaged;
	struct EQUIPSLOTINFO slot;
	struct ItemOptions optionData[MAX_ITEM_OPTIONS];
	int16 weight;
	uint8 favorite;
	uint32 location;
	int8 refiningLevel;
	int8 grade;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_ADD_ITEM_RODEX, 0x0b3f);
#elif PACKETVER >= 20141119
struct PACKET_ZC_ACK_ADD_ITEM_RODEX {
	int16 PacketType;
	int8 result;
	int16 index;
	int16 count;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	int8 type;
	int8 IsIdentified;
	int8 IsDamaged;
	int8 refiningLevel;
	struct EQUIPSLOTINFO slot;
	struct ItemOptions optionData[MAX_ITEM_OPTIONS];
	int16 weight;
	uint8 favorite;
	uint32 location;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_ADD_ITEM_RODEX, 0x0a05);
#endif  // PACKETVER >= 20141119


struct PACKET_CZ_REQ_OPEN_WRITE_MAIL {
	int16 PacketType;
	char receiveName[NAME_LENGTH];
} __attribute__((packed));

struct PACKET_ZC_ACK_OPEN_WRITE_MAIL {
	int16 PacketType;
	char receiveName[NAME_LENGTH];
	int8 result;
} __attribute__((packed));

struct PACKET_CZ_REQ_REMOVE_ITEM_MAIL {
	int16 PacketType;
	int16 index;
	uint16 cnt;
} __attribute__((packed));

struct PACKET_ZC_ACK_REMOVE_ITEM_MAIL {
	int16 PacketType;
	int8 result;
	int16 index;
	uint16 cnt;
	int16 weight;
} __attribute__((packed));

struct PACKET_CZ_SEND_MAIL {
	int16 PacketType;
	int16 PacketLength;
	char receiveName[24];
	char senderName[24];
	int64 zeny;
	int16 Titlelength;
	int16 TextcontentsLength;
#if PACKETVER > 20160600
	int32 receiver_char_id;
#endif // PACKETVER > 20160600
	char string[];
} __attribute__((packed));

struct PACKET_ZC_WRITE_MAIL_RESULT {
	int16 PacketType;
	int8 result;
} __attribute__((packed));

#if PACKETVER >= 20140423
struct PACKET_CZ_CHECKNAME1 {
	int16 PacketType;
	char Name[24];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_CHECKNAME1, 0x0a13)
#endif  // PACKETVER >= 20140423

#if PACKETVER_MAIN_NUM >= 20201104 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20201118
struct PACKET_CZ_CHECKNAME2 {
	int16 PacketType;
	char Name[24];
	char own_char;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_CHECKNAME2, 0x0b97)
#endif  // PACKETVER_MAIN_NUM >= 20201104 || PACKETVER_ZERO_NUM >= 20201118

#if PACKETVER >= 20160302
struct PACKET_ZC_CHECKNAME {
	int16 PacketType;
	int32 CharId;
	int16 Class;
	int16 BaseLevel;
	char Name[24];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_CHECKNAME, 0x0a51)
#elif PACKETVER >= 20141119
struct PACKET_ZC_CHECKNAME {
	int16 PacketType;
	int32 CharId;
	int16 Class;
	int16 BaseLevel;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_CHECKNAME, 0x0a14)
#endif

struct PACKET_ZC_NOTIFY_UNREADMAIL {
	int16 PacketType;
	char result;
} __attribute__((packed));

struct maillistinfo {
#if PACKETVER >= 20170419
	uint8 openType;
#endif
	int64 MailID;
	int8 Isread;
	uint8 type;
	char SenderName[24];
#if PACKETVER < 20170419
	int32 regDateTime;
#endif
	int32 expireDateTime;
	int16 Titlelength;
	char title[];
} __attribute__((packed));

struct PACKET_ZC_MAIL_LIST {
	int16 PacketType;
	int16 PacketLength;
#if PACKETVER < 20170419
	int8 opentype;
	int8 cnt;
#endif
	int8 IsEnd;
} __attribute__((packed));

struct PACKET_CZ_REQ_NEXT_MAIL_LIST {
	int16 PacketType;
	int8 opentype;
	int64 Lower_MailID;
} __attribute__((packed));

struct PACKET_CZ_REQ_OPEN_MAIL {
	int16 PacketType;
#if PACKETVER >= 20170419
	int64 char_Upper_MailID;
	int64 return_Upper_MailID;
	int64 account_Upper_MailID;
#else
	int8 opentype;
	int64 Upper_MailID;
#endif
} __attribute__((packed));

struct PACKET_CZ_REQ_READ_MAIL {
	int16 PacketType;
	int8 opentype;
	int64 MailID;
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_ACK_READ_RODEX_SUB {
	int16 count;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 ITID;
#else
	uint16 ITID;
#endif
	int8 IsIdentified;
	int8 IsDamaged;
	struct EQUIPSLOTINFO slot;
	uint32 location;
	uint8 type;
	uint16 viewSprite;
	uint16 bindOnEquip;
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
	int8 refiningLevel;
	int8 grade;
} __attribute__((packed));

struct PACKET_ZC_ACK_READ_RODEX {
	int16 PacketType;
	int16 PacketLength;
	int8 opentype;
	int64 MailID;
	int16 TextcontentsLength;
	int64 zeny;
	int8 ItemCnt;
	char Textcontent[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_READ_RODEX, 0x0b63);
// [4144] date unconfirmed
#elif PACKETVER >= 20140115
struct PACKET_ZC_ACK_READ_RODEX_SUB {
	int16 count;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 ITID;
#else
	uint16 ITID;
#endif
	int8 IsIdentified;
	int8 IsDamaged;
	int8 refiningLevel;
	struct EQUIPSLOTINFO slot;
	uint32 location;
	uint8 type;
	uint16 viewSprite;
	uint16 bindOnEquip;
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
} __attribute__((packed));

struct PACKET_ZC_ACK_READ_RODEX {
	int16 PacketType;
	int16 PacketLength;
	int8 opentype;
	int64 MailID;
	int16 TextcontentsLength;
	int64 zeny;
	int8 ItemCnt;
	char Textcontent[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_READ_RODEX, 0x09eb);
#endif  // PACKETVER >= 20140115

struct PACKET_CZ_REQ_DELETE_MAIL {
	int16 PacketType;
	int8 opentype;
	int64 MailID;
} __attribute__((packed));

struct PACKET_ZC_ACK_DELETE_MAIL {
	int16 PacketType;
	int8 opentype;
	int64 MailID;
} __attribute__((packed));

struct PACKET_CZ_REQ_REFRESH_MAIL_LIST {
	int16 PacketType;
#if PACKETVER >= 20170419
	int64 Upper_MailID;
	int8 unknown[16];
#else
	int8 opentype;
	int64 Upper_MailID;
#endif
} __attribute__((packed));

struct PACKET_CZ_REQ_ZENY_FROM_MAIL {
	int16 PacketType;
	int64 MailID;
	int8 opentype;
} __attribute__((packed));

struct PACKET_ZC_ACK_ZENY_FROM_MAIL {
	int16 PacketType;
	int64 MailID;
	int8 opentype;
	int8 result;
} __attribute__((packed));

struct PACKET_CZ_REQ_ITEM_FROM_MAIL {
	int16 PacketType;
	int64 MailID;
	int8 opentype;
} __attribute__((packed));

struct PACKET_ZC_ACK_ITEM_FROM_MAIL {
	int16 PacketType;
	int64 MailID;
	int8 opentype;
	int8 result;
} __attribute__((packed));

struct PACKET_ZC_SKILL_SCALE {
	int16 PacketType;
	uint32 AID;
	int16 skill_id;
	int16 skill_lv;
	int16 x;
	int16 y;
	uint32 casttime;
} __attribute__((packed));

struct ZC_PROGRESS_ACTOR {
	int16 PacketType;
	int32 GID;
	int32 color;
	uint32 time;
} __attribute__((packed));

struct PACKET_ZC_ADD_MEMBER_TO_GROUP {
	int16 packetType;
	uint32 AID;
#if PACKETVER >= 20171207
	uint32 GID;
#endif
	uint32 leader;
#if PACKETVER_MAIN_NUM >= 20170524 || PACKETVER_RE_NUM >= 20170502 || defined(PACKETVER_ZERO)
	int16 class_;
	int16 baseLevel;
#endif
	int16 x;
	int16 y;
	uint8 offline;
	char partyName[NAME_LENGTH];
	char playerName[NAME_LENGTH];
	char mapName[MAP_NAME_LENGTH_EXT];
	int8 sharePickup;
	int8 shareLoot;
} __attribute__((packed));

struct PACKET_ZC_GROUP_LIST_SUB {
	uint32 AID;
#if PACKETVER >= 20171207
	uint32 GID;
#endif
	char playerName[NAME_LENGTH];
	char mapName[MAP_NAME_LENGTH_EXT];
	uint8 leader;
	uint8 offline;
#if PACKETVER_MAIN_NUM >= 20170524 || PACKETVER_RE_NUM >= 20170502 || defined(PACKETVER_ZERO)
	int16 class_;
	int16 baseLevel;
#endif
} __attribute__((packed));

struct PACKET_ZC_GROUP_LIST {
	int16 packetType;
	int16 packetLen;
	char partyName[NAME_LENGTH];
	struct PACKET_ZC_GROUP_LIST_SUB members[];
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20130626 || PACKETVER_RE_NUM >= 20130605 || defined(PACKETVER_ZERO)
struct PACKET_ZC_CLANINFO {
	int16 PacketType;
	int16 PacketLength;
	uint32 ClanID;
	char ClanName[NAME_LENGTH];
	char MasterName[NAME_LENGTH];
	char Map[MAP_NAME_LENGTH_EXT];
	uint8 AllyCount;
	uint8 AntagonistCount;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_CLANINFO, 0x098a);
#endif

struct PACKET_ZC_NOTIFY_CLAN_CONNECTINFO {
	int16 PacketType;
	int16 NumConnect;
	int16 NumTotal;
} __attribute__((packed));

struct PACKET_ZC_ACK_CLAN_LEAVE {
	int16 PacketType;
} __attribute__((packed));

struct PACKET_ZC_NOTIFY_CLAN_CHAT {
	int16 PacketType;
	int16 PacketLength;
	char MemberName[NAME_LENGTH];
	char Message[];
} __attribute__((packed));

/**
 * PACKET_ZC_MISSION_HUNT (PACKETVER < 20150513)
 * PACKET_ZC_MISSION_HUNT_EX (PACKETVER >= 20150513)
 */
struct packet_quest_hunt_sub {
#if PACKETVER_ZERO_NUM >= 20181010 || PACKETVER >= 20181017
	uint32 huntIdent;
	uint32 huntIdent2;
	uint32 mobType;
#elif PACKETVER >= 20150513
	uint32 huntIdent;
	uint32 mobType;
#endif
	uint32 mob_id;
#if PACKETVER >= 20150513
	int16 levelMin;
	int16 levelMax;
#endif
	int16 huntCount;
	char mobName[NAME_LENGTH];
} __attribute__((packed));

/**
 * Header for:
 * PACKET_ZC_ADD_QUEST (PACKETVER < 20150513)
 * PACKET_ZC_ADD_QUEST_EX (PACKETVER >= 20150513)
 */
struct packet_quest_add_header {
	uint16 PacketType;
	uint32 questID;
	uint8 active;
	int32 quest_svrTime;
	int32 quest_endTime;
	int16 count;
	struct packet_quest_hunt_sub objectives[];
} __attribute__((packed));

/**
 * PACKET_MOB_HUNTING (PACKETVER < 20150513)
 * PACKET_MOB_HUNTING_EX (PACKETVER >= 20150513)
 */
struct packet_quest_update_hunt {
	uint32 questID;
#if PACKETVER_ZERO_NUM >= 20181010 || PACKETVER >= 20181017
	uint32 huntIdent;
	uint32 huntIdent2;
#elif PACKETVER >= 20150513
	uint32 huntIdent;
#else
	uint32 mob_id;
#endif // PACKETVER < 20150513
	int16 maxCount;
	int16 count;
} __attribute__((packed));

/**
 * Header for:
 * PACKET_ZC_UPDATE_MISSION_HUNT (PACKETVER < 20150513)
 * PACKET_ZC_UPDATE_MISSION_HUNT_EX (PACKETVER >= 20150513)
 */
struct packet_quest_update_header {
	uint16 PacketType;
	uint16 PacketLength;
	int16 count;
	struct packet_quest_update_hunt objectives[];
} __attribute__((packed));

/**
 * Header for:
 * PACKET_MOB_HUNTING (PACKETVER >= 20150513)
 */
struct packet_quest_hunt_info_sub {
	uint32 questID;
	uint32 mob_id;
	int16 maxCount;
	int16 count;
} __attribute__((packed));

/**
 * Header for:
 * ZC_HUNTING_QUEST_INFO (PACKETVER >= 20150513)
 */
struct packet_quest_hunt_info {
	uint16 PacketType;
	uint16 PacketLength;
	struct packet_quest_hunt_info_sub info[];
} __attribute__((packed));

struct PACKET_ZC_FORMATSTRING_MSG {
	uint16 PacketType;
	uint16 PacketLength;
	uint16 MessageId;
	char MessageString[];
} __attribute__((packed));

struct PACKET_ZC_FORMATSTRING_MSG_COLOR {
	uint16 PacketType;
	uint16 PacketLength;
	uint16 messageId;
#if PACKETVER >= 20160406
	uint32 color;
#endif
	char messageString[];
} __attribute__((packed));

struct PACKET_ZC_MSG_COLOR {
	uint16 PacketType;
	uint16 MessageId;
	uint32 MessageColor;
} __attribute__((packed));

struct PACKET_CZ_OPEN_UI {
	int16 PacketType;
	int8 UIType;
} __attribute__((packed));

#if PACKETVER >= 20171122
struct PACKET_ZC_UI_OPEN {
	int16 PacketType;
	int8 UIType;
	int32 data;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_UI_OPEN, 0x0ae2);
#elif PACKETVER >= 20151202
struct PACKET_ZC_UI_OPEN {
	int16 PacketType;
	int8 UIType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_UI_OPEN, 0x0a38);
#endif

#if PACKETVER_MAIN_NUM >= 20210203 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_UI_OPEN2 {
	int16 PacketType;
	int8 UIType;
	int64 data;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_UI_OPEN2, 0x0b9a);
#endif  // PACKETVER_MAIN_NUM >= 20210203 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024

struct PACKET_ZC_UI_ACTION {
	int16 PacketType;
	int32 UIType;
	int32 data;
} __attribute__((packed));

struct PACKET_CZ_PRIVATE_AIRSHIP_REQUEST {
	int16 PacketType;
	char mapName[MAP_NAME_LENGTH_EXT];
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 ItemID;
#else
	uint16 ItemID;
#endif
} __attribute__((packed));

struct PACKET_ZC_PRIVATE_AIRSHIP_RESPONSE {
	int16 PacketType;
	uint32 flag;
} __attribute__((packed));

struct PACKET_CZ_REQ_STYLE_CHANGE {
	int16 PacketType;
	int16 HeadPalette;
	int16 HeadStyle;
	int16 BodyPalette;
	int16 TopAccessory;
	int16 MidAccessory;
	int16 BottomAccessory;
} __attribute__((packed));

struct PACKET_CZ_REQ_STYLE_CHANGE2 {
	int16 PacketType;
	int16 HeadPalette;
	int16 HeadStyle;
	int16 BodyPalette;
	int16 TopAccessory;
	int16 MidAccessory;
	int16 BottomAccessory;
	int16 BodyStyle;
} __attribute__((packed));

struct PACKET_ZC_STYLE_CHANGE_RES {
	int16 PacketType;
	int8 flag;
} __attribute__((packed));

struct pet_evolution_items {
	int16 index;
	int16 amount;
} __attribute__((packed));

struct PACKET_CZ_PET_EVOLUTION {
	int16 PacketType;
	uint16 PacketLength;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 EvolvedPetEggID;
#else
	uint16 EvolvedPetEggID;
#endif
	// struct pet_evolution_items items[];
} __attribute__((packed));

struct packet_ZC_REFUSE_LOGIN {
	int16 PacketType;
#if PACKETVER >= 20101123
	uint32 error_code;
#else
	uint8 error_code;
#endif
	char block_date[20];
} __attribute__((packed));

struct PACKET_ZC_NOTIFY_CHAT {
	int16 PacketType;
	int16 PacketLength;
	uint32 GID;
	char Message[];
} __attribute__((packed));

struct PACKET_ZC_NOTIFY_PLAYERCHAT {
	int16 PacketType;
	int16 PacketLength;
	char Message[];
} __attribute__((packed));

struct PACKET_ZC_ITEM_ENTRY {
	int16 packetType;
	uint32 AID;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 identify;
	uint16 x;
	uint16 y;
	uint16 amount;
	uint8 subX;
	uint8 subY;
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_ADD_ITEM_TO_STORE {
	int16 packetType;
	int16 index;
	int32 amount;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 itemType;
	uint8 identified;
	uint8 damaged;
	struct EQUIPSLOTINFO slot;
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
	uint8 refine;
	uint8 grade;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ADD_ITEM_TO_STORE, 0x0b44)
#elif PACKETVER_MAIN_NUM >= 20140813 || PACKETVER_RE_NUM >= 20140402 || defined(PACKETVER_ZERO)
struct PACKET_ZC_ADD_ITEM_TO_STORE {
	int16 packetType;
	int16 index;
	int32 amount;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 itemType;
	uint8 identified;
	uint8 damaged;
	uint8 refine;
	struct EQUIPSLOTINFO slot;
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ADD_ITEM_TO_STORE, 0x0a0a)
// [4144] this version unconfirmed
#elif PACKETVER >= 5
struct PACKET_ZC_ADD_ITEM_TO_STORE {
	int16 packetType;
	int16 index;
	int32 amount;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 itemType;
	uint8 identified;
	uint8 damaged;
	uint8 refine;
	struct EQUIPSLOTINFO slot;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ADD_ITEM_TO_STORE, 0x01c4)
#else
struct PACKET_ZC_ADD_ITEM_TO_STORE {
	int16 packetType;
	int16 index;
	int32 amount;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 identified;
	uint8 damaged;
	uint8 refine;
	struct EQUIPSLOTINFO slot;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ADD_ITEM_TO_STORE, 0x00f4)
#endif

struct PACKET_ZC_MVP_GETTING_ITEM {
	int16 packetType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_ACK_TOUSESKILL {
	int16 packetType;
	uint16 skillId;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	int32 btype;
	uint32 itemId;
#else
	int16 btype;
	uint16 itemId;
#endif
	uint8 flag;
	uint8 cause;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_TOUSESKILL, 0x0110)

#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_ADD_ITEM_TO_CART {
	int16 packetType;
	int16 index;
	int32 amount;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 itemType;
	uint8 identified;
	uint8 damaged;
	struct EQUIPSLOTINFO slot;
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
	uint8 refine;
	uint8 grade;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ADD_ITEM_TO_CART, 0x0b45);
#elif PACKETVER_MAIN_NUM >= 20140813 || PACKETVER_RE_NUM >= 20140402 || defined(PACKETVER_ZERO)
struct PACKET_ZC_ADD_ITEM_TO_CART {
	int16 packetType;
	int16 index;
	int32 amount;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 itemType;
	uint8 identified;
	uint8 damaged;
	uint8 refine;
	struct EQUIPSLOTINFO slot;
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ADD_ITEM_TO_CART, 0x0a0b);
// [4144] this version unconfirmed
#elif PACKETVER >= 5
struct PACKET_ZC_ADD_ITEM_TO_CART {
	int16 packetType;
	int16 index;
	int32 amount;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 itemType;
	uint8 identified;
	uint8 damaged;
	uint8 refine;
	struct EQUIPSLOTINFO slot;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ADD_ITEM_TO_CART, 0x01c5);
#else
struct PACKET_ZC_ADD_ITEM_TO_CART {
	int16 packetType;
	int16 index;
	int32 amount;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 identified;
	uint8 damaged;
	uint8 refine;
	struct EQUIPSLOTINFO slot;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ADD_ITEM_TO_CART, 0x0124);
#endif

struct PACKET_CZ_REQMAKINGITEM {
	int16 packetType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
	uint32 material[3];
#else
	uint16 itemId;
	uint16 material[3];
#endif
} __attribute__((packed));

struct PACKET_ZC_ACK_REQMAKINGITEM {
	int16 packetType;
	int16 result;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_FEED_PET {
	int16 packetType;
	uint8 result;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_FEED_MER {
	int16 packetType;
	uint8 result;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_USE_ITEM_ACK {
	int16 packetType;
	int16 index;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
	uint32 AID;
#elif PACKETVER >= 3
	uint16 itemId;
	uint32 AID;
#endif
	int16 amount;
	uint8 result;
} __attribute__((packed));

struct PACKET_ZC_SPRITE_CHANGE {
	int16 packetType;
	uint32 AID;
	uint8 type;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 val;
	uint32 val2;
#elif PACKETVER >= 4
	uint16 val;
	uint16 val2;
#else
	uint8 val;
#endif
} __attribute__((packed));

// TODO put struct under #ifdef/#elif
// [4144] dates unconfirmed
struct PACKET_ZC_ADD_EXCHANGE_ITEM {
	int16 packetType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
	uint8 itemType;
	int32 amount;
#elif PACKETVER >= 20100223
	uint16 itemId;
	uint8 itemType;
	int32 amount;
#else
	int32 amount;
	uint16 itemId;
#endif
	uint8 identified;
	uint8 damaged;
#if !(PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024)
	uint8 refine;
#endif  // !(PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024)
	struct EQUIPSLOTINFO slot;
#if PACKETVER >= 20150226
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
#endif
#if PACKETVER_MAIN_NUM >= 20161102 || PACKETVER_RE_NUM >= 20161026 || defined(PACKETVER_ZERO)
	uint32 location;
	uint16 look;
#endif  // PACKETVER_MAIN_NUM >= 20161102 || PACKETVER_RE_NUM >= 20161026 || defined(PACKETVER_ZERO)
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
	uint8 refine;
	uint8 grade;
#endif  // PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
DEFINE_PACKET_HEADER(ZC_ADD_EXCHANGE_ITEM, 0x0b42);
#elif PACKETVER_MAIN_NUM >= 20161102 || PACKETVER_RE_NUM >= 20161026 || defined(PACKETVER_ZERO)
DEFINE_PACKET_HEADER(ZC_ADD_EXCHANGE_ITEM, 0x0a96);
#elif PACKETVER >= 20150226
DEFINE_PACKET_HEADER(ZC_ADD_EXCHANGE_ITEM, 0x0a09);
#elif PACKETVER >= 20100223
DEFINE_PACKET_HEADER(ZC_ADD_EXCHANGE_ITEM, 0x080f);
#else
DEFINE_PACKET_HEADER(ZC_ADD_EXCHANGE_ITEM, 0x00e9);
#endif

struct PACKET_ZC_CASH_TIME_COUNTER {
	int16 packetType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint32 seconds;
} __attribute__((packed));

struct PACKET_ZC_CASH_ITEM_DELETE {
	int16 packetType;
	uint16 index;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_ITEM_PICKUP_PARTY {
	int16 packetType;
	uint32 AID;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 identified;
	uint8 damaged;
	struct EQUIPSLOTINFO slot;
	uint16 location;
	uint8 itemType;
	uint8 refine;
	uint8 grade;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ITEM_PICKUP_PARTY, 0x0b67);
#elif PACKETVER >= 20070731
struct PACKET_ZC_ITEM_PICKUP_PARTY {
	int16 packetType;
	uint32 AID;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 identified;
	uint8 damaged;
	uint8 refine;
	struct EQUIPSLOTINFO slot;
	uint16 location;
	uint8 itemType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ITEM_PICKUP_PARTY, 0x02b8);
#endif

struct PACKET_ZC_UPDATE_ITEM_FROM_BUYING_STORE {
	int16 packetType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint16 amount;
#if PACKETVER >= 20141016
	uint32 zeny;
	uint32 zenyLimit;
	uint32 charId;
	uint32 updateTime;
#else
	uint32 zenyLimit;
#endif
} __attribute__((packed));

struct PACKET_ZC_ACK_WEAPONREFINE {
	int16 packetType;
	int32 result;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20210303 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
// PACKET_ZC_PROPERTY_HOMUN4
struct PACKET_ZC_PROPERTY_HOMUN {
	int16 packetType;
	char name[NAME_LENGTH];
	// Bit field, bit 0 : rename_flag (1 = already renamed), bit 1 : homunc vaporized (1 = true), bit 2 : homunc dead (1 = true)
	uint8 flags;
	uint16 level;
	uint16 hunger;
	uint16 intimacy;
	uint16 atk2;
	uint16 matk;
	uint16 hit;
	uint16 crit;
	uint16 def;
	uint16 mdef;
	uint16 flee;
	uint16 amotion;
	uint32 hp;
	uint32 maxHp;
	uint32 sp;
	uint32 maxSp;
	int64 exp;
	int64 expNext;
	uint16 skillPoints;
	uint16 range;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PROPERTY_HOMUN, 0x0ba4);
#elif PACKETVER_MAIN_NUM >= 20200819 || PACKETVER_RE_NUM >= 20200723
// PACKET_ZC_PROPERTY_HOMUN3
struct PACKET_ZC_PROPERTY_HOMUN {
	int16 packetType;
	char name[NAME_LENGTH];
	// Bit field, bit 0 : rename_flag (1 = already renamed), bit 1 : homunc vaporized (1 = true), bit 2 : homunc dead (1 = true)
	uint8 flags;
	uint16 level;
	uint16 hunger;
	uint16 intimacy;
	uint16 atk2;
	uint16 matk;
	uint16 hit;
	uint16 crit;
	uint16 def;
	uint16 mdef;
	uint16 flee;
	uint16 amotion;
	uint32 hp;
	uint32 maxHp;
	uint32 sp;
	uint32 maxSp;
	uint32 exp;
	uint32 expNext;
	uint16 skillPoints;
	uint16 range;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PROPERTY_HOMUN, 0x0b76);
#elif PACKETVER_MAIN_NUM >= 20190619 || PACKETVER_RE_NUM >= 20190605 || PACKETVER_ZERO_NUM >= 20190626
// PACKET_ZC_PROPERTY_HOMUN3
struct PACKET_ZC_PROPERTY_HOMUN {
	int16 packetType;
	char name[NAME_LENGTH];
	// Bit field, bit 0 : rename_flag (1 = already renamed), bit 1 : homunc vaporized (1 = true), bit 2 : homunc dead (1 = true)
	uint8 flags;
	uint16 level;
	uint16 hunger;
	uint16 intimacy;
	uint16 atk2;
	uint16 matk;
	uint16 hit;
	uint16 crit;
	uint16 def;
	uint16 mdef;
	uint16 flee;
	uint16 amotion;
	uint32 hp;
	uint32 maxHp;
	uint16 sp;
	uint16 maxSp;
	uint32 exp;
	uint32 expNext;
	uint16 skillPoints;
	uint16 range;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PROPERTY_HOMUN, 0x0b2f);
#elif PACKETVER_MAIN_NUM >= 20131230 || PACKETVER_RE_NUM >= 20131230 || defined(PACKETVER_ZERO)
// PACKET_ZC_PROPERTY_HOMUN2
struct PACKET_ZC_PROPERTY_HOMUN {
	int16 packetType;
	char name[NAME_LENGTH];
	// Bit field, bit 0 : rename_flag (1 = already renamed), bit 1 : homunc vaporized (1 = true), bit 2 : homunc dead (1 = true)
	uint8 flags;
	uint16 level;
	uint16 hunger;
	uint16 intimacy;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint16 atk2;
	uint16 matk;
	uint16 hit;
	uint16 crit;
	uint16 def;
	uint16 mdef;
	uint16 flee;
	uint16 amotion;
	uint32 hp;
	uint32 maxHp;
	uint16 sp;
	uint16 maxSp;
	uint32 exp;
	uint32 expNext;
	uint16 skillPoints;
	uint16 range;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PROPERTY_HOMUN, 0x09f7);
#elif PACKETVER_MAIN_NUM >= 20101005 || PACKETVER_RE_NUM >= 20080827 || PACKETVER_SAK_NUM >= 20080618 || defined(PACKETVER_ZERO)
// PACKET_ZC_PROPERTY_HOMUN1
struct PACKET_ZC_PROPERTY_HOMUN {
	int16 packetType;
	char name[NAME_LENGTH];
	// Bit field, bit 0 : rename_flag (1 = already renamed), bit 1 : homunc vaporized (1 = true), bit 2 : homunc dead (1 = true)
	uint8 flags;
	uint16 level;
	uint16 hunger;
	uint16 intimacy;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint16 atk2;
	uint16 matk;
	uint16 hit;
	uint16 crit;
	uint16 def;
	uint16 mdef;
	uint16 flee;
	uint16 amotion;
	uint16 hp;
	uint16 maxHp;
	uint16 sp;
	uint16 maxSp;
	uint32 exp;
	uint32 expNext;
	uint16 skillPoints;
	uint16 range;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PROPERTY_HOMUN, 0x022e);
#endif

struct PACKET_ZC_FAILED_TRADE_BUYING_STORE_TO_SELLER {
	int16 packetType;
	uint16 result;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

#if PACKETVER >= 20191224
struct REPAIRITEM_INFO2 {
	int16 index;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	struct EQUIPSLOTINFO slot;  // unused?
	uint8 refine;  // unused?
	uint8 grade;  // unused?
} __attribute__((packed));
#elif PACKETVER >= 20191106
struct REPAIRITEM_INFO2 {
	int16 index;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 refine;  // unused?
	struct EQUIPSLOTINFO slot;  // unused?
} __attribute__((packed));
#endif  // PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024

struct REPAIRITEM_INFO1 {
	int16 index;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 refine;  // unused?
	struct EQUIPSLOTINFO slot;  // unused?
} __attribute__((packed));

#if PACKETVER >= 20191224
struct PACKET_CZ_REQ_ITEMREPAIR2 {
	int16 packetType;
	struct REPAIRITEM_INFO2 item;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_ITEMREPAIR2, 0x0b66);
#endif  // PACKETVER >= 20191224

struct PACKET_CZ_REQ_ITEMREPAIR1 {
	int16 packetType;
	struct REPAIRITEM_INFO1 item;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_ITEMREPAIR1, 0x01fd);

struct PACKET_CZ_REQ_MAKINGITEM {
	int16 packetType;
	int16 type;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_CZ_SSILIST_ITEM_CLICK {
	int16 packetType;
	uint32 AID;
	uint32 storeId;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_ACK_SCHEDULER_CASHITEM_sub {
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint32 price;
#ifdef ENABLE_CASHSHOP_PREVIEW_PATCH
	uint16 viewSprite;
	uint32 location;
#endif  // ENABLE_CASHSHOP_PREVIEW_PATCH
} __attribute__((packed));

struct PACKET_ZC_ACK_SCHEDULER_CASHITEM {
	int16 packetType;
	int16 packetLength;
	int16 count;
	int16 tabNum;
	struct PACKET_ZC_ACK_SCHEDULER_CASHITEM_sub items[];
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_PC_PURCHASE_MYITEMLIST_sub {
	uint32 price;
	int16 index;
	int16 amount;
	uint8 itemType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 identified;
	uint8 damaged;
	struct EQUIPSLOTINFO slot;
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
	uint8 refine;
	uint8 grade;
} __attribute__((packed));
struct PACKET_ZC_PC_PURCHASE_MYITEMLIST {
	int16 packetType;
	int16 packetLength;
	uint32 AID;
	struct PACKET_ZC_PC_PURCHASE_MYITEMLIST_sub items[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PC_PURCHASE_MYITEMLIST, 0x0b40);
#else  // PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_PC_PURCHASE_MYITEMLIST_sub {
	uint32 price;
	int16 index;
	int16 amount;
	uint8 itemType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 identified;
	uint8 damaged;
	uint8 refine;
	struct EQUIPSLOTINFO slot;
#if PACKETVER >= 20150226
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
#endif
} __attribute__((packed));
struct PACKET_ZC_PC_PURCHASE_MYITEMLIST {
	int16 packetType;
	int16 packetLength;
	uint32 AID;
	struct PACKET_ZC_PC_PURCHASE_MYITEMLIST_sub items[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PC_PURCHASE_MYITEMLIST, 0x0136);
#endif  // PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024

#if PACKETVER_MAIN_NUM >= 20210203 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_PC_PURCHASE_ITEMLIST_sub {
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint32 price;
	uint32 discountPrice;
	uint8 itemType;
	uint16 viewSprite;
	uint32 location;
} __attribute__((packed));
struct PACKET_ZC_PC_PURCHASE_ITEMLIST {
	int16 packetType;
	int16 packetLength;
	struct PACKET_ZC_PC_PURCHASE_ITEMLIST_sub items[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PC_PURCHASE_ITEMLIST, 0x0b77)
#else  // PACKETVER_MAIN_NUM >= 20210203
struct PACKET_ZC_PC_PURCHASE_ITEMLIST_sub {
	uint32 price;
	uint32 discountPrice;
	uint8 itemType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));
struct PACKET_ZC_PC_PURCHASE_ITEMLIST {
	int16 packetType;
	int16 packetLength;
	struct PACKET_ZC_PC_PURCHASE_ITEMLIST_sub items[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PC_PURCHASE_ITEMLIST, 0x00c6)
#endif  // PACKETVER_MAIN_NUM >= 20210203

struct PACKET_CZ_PC_PURCHASE_ITEMLIST_sub {
	uint16 amount;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_CZ_PC_PURCHASE_ITEMLIST {
	int16 packetType;
	int16 packetLength;
	struct PACKET_CZ_PC_PURCHASE_ITEMLIST_sub items[];
} __attribute__((packed));

struct PACKET_CZ_REQ_OPEN_BUYING_STORE_sub {
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint16 amount;
	uint32 price;
} __attribute__((packed));

struct PACKET_CZ_REQ_OPEN_BUYING_STORE {
	int16 packetType;
	int16 packetLength;
	uint32 zenyLimit;
	uint8 result;
	char storeName[MESSAGE_SIZE];
	struct PACKET_CZ_REQ_OPEN_BUYING_STORE_sub items[];
} __attribute__((packed));

struct PACKET_ZC_MYITEMLIST_BUYING_STORE_sub {
	uint32 price;
	uint16 amount;
	uint8 itemType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_MYITEMLIST_BUYING_STORE {
	int16 packetType;
	int16 packetLength;
	uint32 AID;
	uint32 zenyLimit;
	struct PACKET_ZC_MYITEMLIST_BUYING_STORE_sub items[];
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_PC_PURCHASE_ITEMLIST_FROMMC_sub {
	uint32 price;
	uint16 amount;
	int16 index;
	uint8 itemType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 identified;
	uint8 damaged;
	struct EQUIPSLOTINFO slot;
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
	uint32 location;
	uint16 viewSprite;
	uint8 refine;
	uint8 grade;
} __attribute__((packed));

struct PACKET_ZC_PC_PURCHASE_ITEMLIST_FROMMC {
	int16 packetType;
	int16 packetLength;
	uint32 AID;
	uint32 venderId;
	struct PACKET_ZC_PC_PURCHASE_ITEMLIST_FROMMC_sub items[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PC_PURCHASE_ITEMLIST_FROMMC, 0x0b3d)
#elif PACKETVER >= 20100105
struct PACKET_ZC_PC_PURCHASE_ITEMLIST_FROMMC_sub {
	uint32 price;
	uint16 amount;
	int16 index;
	uint8 itemType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 identified;
	uint8 damaged;
#if !(PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024)
	uint8 refine;
#endif
	struct EQUIPSLOTINFO slot;
#if PACKETVER >= 20150226
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
#endif
// [4144] date 20160921 not confirmed. Can be bigger or smaller
#if PACKETVER >= 20160921
	uint32 location;
	uint16 viewSprite;
#endif
#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
	uint8 refine;
	uint8 grade;
#endif
} __attribute__((packed));

struct PACKET_ZC_PC_PURCHASE_ITEMLIST_FROMMC {
	int16 packetType;
	int16 packetLength;
	uint32 AID;
// [4144] unconfirmed field
	uint32 venderId;
	struct PACKET_ZC_PC_PURCHASE_ITEMLIST_FROMMC_sub items[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PC_PURCHASE_ITEMLIST_FROMMC, 0x0800)
#else
struct PACKET_ZC_PC_PURCHASE_ITEMLIST_FROMMC_sub {
	uint32 price;
	uint16 amount;
	int16 index;
	uint8 itemType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 identified;
	uint8 damaged;
	uint8 refine;
	struct EQUIPSLOTINFO slot;
} __attribute__((packed));

struct PACKET_ZC_PC_PURCHASE_ITEMLIST_FROMMC {
	int16 packetType;
	int16 packetLength;
	uint32 AID;
	struct PACKET_ZC_PC_PURCHASE_ITEMLIST_FROMMC_sub items[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PC_PURCHASE_ITEMLIST_FROMMC, 0x0133)
#endif

struct PACKET_ZC_ACK_ITEMLIST_BUYING_STORE_sub {
	uint32 price;
	uint16 amount;
	uint8 itemType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_ACK_ITEMLIST_BUYING_STORE {
	int16 packetType;
	int16 packetLength;
	uint32 AID;
	uint32 storeId;
	uint32 zenyLimit;
	struct PACKET_ZC_ACK_ITEMLIST_BUYING_STORE_sub items[];
} __attribute__((packed));

struct PACKET_CZ_REQ_TRADE_BUYING_STORE_sub {
	int16 index;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint16 amount;
} __attribute__((packed));

struct PACKET_CZ_REQ_TRADE_BUYING_STORE {
	int16 packetType;
	int16 packetLength;
	uint32 AID;
	uint32 storeId;
	struct PACKET_CZ_REQ_TRADE_BUYING_STORE_sub items[];
} __attribute__((packed));

struct PACKET_ZC_MAKABLEITEMLIST_sub {
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
	uint32 material[3];
#else
	uint16 itemId;
	uint16 material[3];
#endif
} __attribute__((packed));

struct PACKET_ZC_MAKABLEITEMLIST {
	int16 packetType;
	int16 packetLength;
	struct PACKET_ZC_MAKABLEITEMLIST_sub items[];
} __attribute__((packed));

struct PACKET_ZC_MAKINGARROW_LIST_sub {
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_MAKINGARROW_LIST {
	int16 packetType;
	int16 packetLength;
	struct PACKET_ZC_MAKINGARROW_LIST_sub items[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_MAKINGARROW_LIST, 0x01ad);

struct PACKET_ZC_SKILL_SELECT_REQUEST {
	int16 packetType;
	int16 packetLength;
	int32 flag; //< 0 = old code compatibility; 1 = Auto Shadow Spell; same value is received in CZ_SKILL_SELECT_RESPONSE
	int16 skillIds[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SKILL_SELECT_REQUEST, 0x0442);

struct PACKET_CZ_SKILL_SELECT_RESPONSE {
	int16 packetType;
	int32 flag; //< currently unused, matches ZC_SKILL_SELECT_REQUEST.flag
	int16 selectedSkillId;
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
#define REPAIRITEM_INFO REPAIRITEM_INFO2
struct PACKET_ZC_REPAIRITEMLIST {
	int16 packetType;
	int16 packetLength;
	struct REPAIRITEM_INFO items[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_REPAIRITEMLIST, 0x0b65);
#else  // PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
#define REPAIRITEM_INFO REPAIRITEM_INFO1
struct PACKET_ZC_REPAIRITEMLIST {
	int16 packetType;
	int16 packetLength;
	struct REPAIRITEM_INFO items[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_REPAIRITEMLIST, 0x01fc);
#endif  // PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024

struct PACKET_ZC_NOTIFY_WEAPONITEMLIST_sub {
	int16 index;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 refine;  // unused?
	struct EQUIPSLOTINFO slot;  // unused?
} __attribute__((packed));

struct PACKET_ZC_NOTIFY_WEAPONITEMLIST {
	int16 packetType;
	int16 packetLength;
	struct PACKET_ZC_NOTIFY_WEAPONITEMLIST_sub items[];
} __attribute__((packed));

struct PACKET_ZC_MAKINGITEM_LIST_sub {
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_ZC_MAKINGITEM_LIST {
	int16 packetType;
	int16 packetLength;
	uint16 makeItem;
	struct PACKET_ZC_MAKINGITEM_LIST_sub items[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_MAKINGITEM_LIST, 0x025a);

struct PACKET_ZC_PC_CASH_POINT_ITEMLIST_sub {
	uint32 price;
	uint32 discountPrice;
	uint8 itemType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
#ifdef ENABLE_OLD_CASHSHOP_PREVIEW_PATCH
	uint16 viewSprite;
	uint32 location;
	uint8 unused[6];
#endif  // ENABLE_OLD_CASHSHOP_PREVIEW_PATCH
} __attribute__((packed));

struct PACKET_ZC_PC_CASH_POINT_ITEMLIST {
	int16 packetType;
	int16 packetLength;
	uint32 cashPoints;
#if PACKETVER >= 20070711
	uint32 kafraPoints;
#endif
	struct PACKET_ZC_PC_CASH_POINT_ITEMLIST_sub items[];
} __attribute__((packed));

struct PACKET_CZ_PC_BUY_CASH_POINT_ITEM_sub {
	uint16 amount;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_CZ_PC_BUY_CASH_POINT_ITEM {
	int16 packetType;
#if PACKETVER >= 20101116
	int16 packetLength;
	uint32 kafraPoints;
	uint16 count;
	struct PACKET_CZ_PC_BUY_CASH_POINT_ITEM_sub items[];
#else
	uint16 itemId;
	uint16 amount;
#if PACKETVER >= 20070711
	uint32 kafraPoints;
#endif
#endif
} __attribute__((packed));

struct PACKET_CZ_SEARCH_STORE_INFO_item {
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));

struct PACKET_CZ_SEARCH_STORE_INFO {
	int16 packetType;
	int16 packetLength;
	uint8 searchType;
	uint32 maxPrice;
	uint32 minPrice;
	uint8 itemsCount;
	uint8 cardsCount;
	struct PACKET_CZ_SEARCH_STORE_INFO_item items[];  // items[itemCount]
/*
	struct PACKET_CZ_SEARCH_STORE_INFO_item cards[cardCount];
*/
} __attribute__((packed));

struct PACKET_ZC_SEARCH_STORE_INFO_FAILED {
	int16 packetType;
	uint8 reason;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SEARCH_STORE_INFO_FAILED, 0x837);

struct PACKET_ZC_OPEN_SEARCH_STORE_INFO {
	int16 packetType;
	uint16 effect;
#if PACKETVER_MAIN_NUM >= 20100701 || PACKETVER_RE_NUM >= 20100701 || defined(PACKETVER_ZERO)
	uint8 remainingUses;
#endif
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_OPEN_SEARCH_STORE_INFO, 0x83a);

struct PACKET_ZC_SSILIST_ITEM_CLICK_ACK {
	int16 packetType;
	int16 x;
	int16 y;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SSILIST_ITEM_CLICK_ACK, 0x83d);

#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_SEARCH_STORE_INFO_ACK_sub {
	uint32 storeId;
	uint32 AID;
	char shopName[MESSAGE_SIZE];
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 itemType;
	uint32 price;
	uint16 amount;
	struct EQUIPSLOTINFO slot;
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
	uint8 refine;
	uint8 grade;
} __attribute__((packed));

struct PACKET_ZC_SEARCH_STORE_INFO_ACK {
	int16 packetType;
	int16 packetLength;
	uint8 firstPage;
	uint8 nextPage;
	uint8 usesCount;
	struct PACKET_ZC_SEARCH_STORE_INFO_ACK_sub items[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SEARCH_STORE_INFO_ACK, 0x0b64);
#elif PACKETVER_MAIN_NUM >= 20100817 || PACKETVER_RE_NUM >= 20100706 || defined(PACKETVER_ZERO)
struct PACKET_ZC_SEARCH_STORE_INFO_ACK_sub {
	uint32 storeId;
	uint32 AID;
	char shopName[MESSAGE_SIZE];
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint8 itemType;
	uint32 price;
	uint16 amount;
	uint8 refine;
	struct EQUIPSLOTINFO slot;
#if PACKETVER >= 20150226
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
#endif
} __attribute__((packed));

struct PACKET_ZC_SEARCH_STORE_INFO_ACK {
	int16 packetType;
	int16 packetLength;
	uint8 firstPage;
	uint8 nextPage;
	uint8 usesCount;
	struct PACKET_ZC_SEARCH_STORE_INFO_ACK_sub items[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SEARCH_STORE_INFO_ACK, 0x0836);
#endif  // PACKETVER_MAIN_NUM >= 20100817 || PACKETVER_RE_NUM >= 20100706 || defined(PACKETVER_ZERO)


/* Achievement System */
struct ach_list_info {
	uint32 ach_id;
	uint8 completed;
	uint32 objective[MAX_ACHIEVEMENT_OBJECTIVES];
	uint32 completed_at;
	uint8 reward;
} __attribute__((packed));

struct packet_achievement_list {
	uint16 packet_id;
	uint16 packet_len;
	uint32 total_achievements;
	uint32 total_points;
	uint16 rank;
	uint32 current_rank_points;
	uint32 next_rank_points;
	struct ach_list_info ach[MAX_ACHIEVEMENT_DB];
} __attribute__((packed));

struct packet_achievement_update {
	uint16 packet_id;
	uint32 total_points;
	uint16 rank;
	uint32 current_rank_points;
	uint32 next_rank_points;
	struct ach_list_info ach;
} __attribute__((packed));

struct packet_achievement_reward_ack {
	uint16 packet_id;
	uint8 failed;
	uint32 ach_id;
} __attribute__((packed));

// Name Packet ZC_ACK_REQNAME
struct packet_reqname_ack {
	uint16 packet_id;
	int32 gid;
	char name[NAME_LENGTH];
} __attribute__((packed));

// ZC_ACK_REQNAMEALL / ZC_ACK_REQNAMEALL2
#if PACKETVER_MAIN_NUM >= 20150225 || PACKETVER_RE_NUM >= 20141126 || defined(PACKETVER_ZERO)
struct PACKET_ZC_ACK_REQNAMEALL {
	uint16 packet_id;
	int32 gid;
	char name[NAME_LENGTH];
	char party_name[NAME_LENGTH];
	char guild_name[NAME_LENGTH];
	char position_name[NAME_LENGTH];
	int32 title_id;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_REQNAMEALL, 0x0a30);
#else
struct PACKET_ZC_ACK_REQNAMEALL {
	uint16 packet_id;
	int32 gid;
	char name[NAME_LENGTH];
	char party_name[NAME_LENGTH];
	char guild_name[NAME_LENGTH];
	char position_name[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_REQNAMEALL, 0x0195);
#endif

#if PACKETVER_MAIN_NUM >= 20180207 || PACKETVER_RE_NUM >= 20171129 || PACKETVER_ZERO_NUM >= 20171130
struct PACKET_ZC_ACK_REQNAMEALL_NPC {
	uint16 packet_id;
	int32 gid;
	int32 groupId;
	char name[NAME_LENGTH];
	char title[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_REQNAMEALL_NPC, 0x0adf);
#else
struct PACKET_ZC_ACK_REQNAMEALL_NPC {
	uint16 packet_id;
	int32 gid;
	char name[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_REQNAMEALL_NPC, 0x0095);
#endif

struct PACKET_ZC_OVERWEIGHT_PERCENT {
	int16 packetType;
	uint32 percent;
} __attribute__((packed));

struct PACKET_ZC_WARPLIST_sub {
	char map[MAP_NAME_LENGTH_EXT];
} __attribute__((packed));

struct PACKET_ZC_WARPLIST {
	int16 packetType;
#if PACKETVER_MAIN_NUM >= 20170502 || PACKETVER_RE_NUM >= 20170419 || defined(PACKETVER_ZERO)
	int16 packetLength;
	uint16 skillId;
	struct PACKET_ZC_WARPLIST_sub maps[];
#else
	uint16 skillId;
	struct PACKET_ZC_WARPLIST_sub maps[4];
#endif
} __attribute__((packed));

struct PACKET_ZC_GROUP_ISALIVE {
	int16 packetType;
	uint32 AID;
	uint8 isDead;
} __attribute__((packed));

struct PACKET_ZC_GUILD_POSITION {
	int16 packetType;
	int16 packetLength;
	uint32 AID;
	char position[];
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20161214 || PACKETVER_RE_NUM >= 20161130 || defined(PACKETVER_ZERO)
struct PACKET_ZC_MOVE_ITEM_FAILED {
	int16 packetType;
	int16 itemIndex;
	int16 itemCount;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_MOVE_ITEM_FAILED, 0x0aa7);
#endif  // PACKETVER_MAIN_NUM >= 20161214 || PACKETVER_RE_NUM >= 20161130 || defined(PACKETVER_ZERO)

#if PACKETVER_MAIN_NUM >= 20161019 || PACKETVER_RE_NUM >= 20160921 || defined(PACKETVER_ZERO)
#define PACKET_ZC_ACK_BAN_GUILD PACKET_ZC_ACK_BAN_GUILD3
#elif PACKETVER >= 20100803
#define PACKET_ZC_ACK_BAN_GUILD PACKET_ZC_ACK_BAN_GUILD2
#else
#define PACKET_ZC_ACK_BAN_GUILD PACKET_ZC_ACK_BAN_GUILD1
#endif

struct PACKET_ZC_ACK_BAN_GUILD1 {
	int16 packetType;
	char name[NAME_LENGTH];
	char reason[40];
	char account_name[NAME_LENGTH];
} __attribute__((packed));

struct PACKET_ZC_ACK_BAN_GUILD2 {
	int16 packetType;
	char name[NAME_LENGTH];
	char reason[40];
} __attribute__((packed));

struct PACKET_ZC_ACK_BAN_GUILD3 {
	int16 packetType;
	char reason[40];
	uint32 GID;
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20161019 || PACKETVER_RE_NUM >= 20160921 || defined(PACKETVER_ZERO)
#define PACKET_ZC_ACK_LEAVE_GUILD PACKET_ZC_ACK_LEAVE_GUILD2
#else
#define PACKET_ZC_ACK_LEAVE_GUILD PACKET_ZC_ACK_LEAVE_GUILD1
#endif

struct PACKET_ZC_ACK_LEAVE_GUILD1 {
	int16 packetType;
	char name[NAME_LENGTH];
	char reason[40];
} __attribute__((packed));

struct PACKET_ZC_ACK_LEAVE_GUILD2 {
	int16 packetType;
	uint32 GID;
	char reason[40];
} __attribute__((packed));

struct PACKET_CZ_MEMORIALDUNGEON_COMMAND {
	int16 packetType;
	int32 command;
} __attribute__((packed));

struct PACKET_ZC_REMOVE_EFFECT {
	int16 packetType;
	uint32 aid;
	uint32 effectId;
} __attribute__((packed));

#if PACKETVER >= 20160525
struct PACKET_ZC_VIEW_CAMERAINFO {
	int16 packetType;
	int8 action;
	float range;
	float rotation;
	float latitude;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_VIEW_CAMERAINFO, 0x0a78);
#endif

#if PACKETVER >= 20160525
struct PACKET_CZ_VIEW_CAMERAINFO {
	int16 packetType;
	int8 action;
	float range;
	float rotation;
	float latitude;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_VIEW_CAMERAINFO, 0x0a77);
#endif

#if PACKETVER_MAIN_NUM >= 20181128 || PACKETVER_RE_NUM >= 20181031
// PACKET_ZC_AUTOSPELLLIST2
struct PACKET_ZC_AUTOSPELLLIST {
	int16 packetType;
	int16 packetLength;
	int skills[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_AUTOSPELLLIST, 0x0afb);
#elif PACKETVER_MAIN_NUM >= 20090406 || defined(PACKETVER_RE) || defined(PACKETVER_ZERO) || PACKETVER_SAK_NUM >= 20080618
// PACKET_ZC_AUTOSPELLLIST1
struct PACKET_ZC_AUTOSPELLLIST {
	int16 packetType;
	int skills[7];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_AUTOSPELLLIST, 0x01cd);
#endif

#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_CHANGE_ITEM_OPTION {
	int16 packetType;
	int16 index;
	int8 isDamaged;
	struct EQUIPSLOTINFO slot;
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
	uint8 refiningLevel;
	uint8 grade;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_CHANGE_ITEM_OPTION, 0x0b43);
#elif PACKETVER_MAIN_NUM >= 20181017 || PACKETVER_RE_NUM >= 20181017 || PACKETVER_ZERO_NUM >= 20181024
struct PACKET_ZC_CHANGE_ITEM_OPTION {
	int16 packetType;
	int16 index;
	int8 isDamaged;
	int16 refiningLevel;
	struct EQUIPSLOTINFO slot;
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_CHANGE_ITEM_OPTION, 0x0b13);
#elif PACKETVER_MAIN_NUM >= 20170726 || PACKETVER_RE_NUM >= 20170621 || defined(PACKETVER_ZERO)
struct PACKET_ZC_CHANGE_ITEM_OPTION {
	int16 packetType;
	int16 index;
	int16 refiningLevel;
	struct EQUIPSLOTINFO slot;
	struct ItemOptions option_data[MAX_ITEM_OPTIONS];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_CHANGE_ITEM_OPTION, 0x0ab9);
#endif  // PACKETVER_MAIN_NUM >= 20181017 || PACKETVER_RE_NUM >= 20181017 || PACKETVER_ZERO_NUM >= 20181024

#if PACKETVER_MAIN_NUM >= 20160831 || PACKETVER_RE_NUM >= 20151118 || defined(PACKETVER_ZERO)
struct PACKET_ZC_UPDATE_CARDSLOT {
	int16 packetType;
	int16 wearState;
	int16 cardSlot;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	int32 itemId;
#else
	int16 itemId;
#endif
	int8 equipFlag;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_UPDATE_CARDSLOT, 0x0a3f);
#endif  // PACKETVER_MAIN_NUM >= 20160831 || PACKETVER_RE_NUM >= 20151118 || defined(PACKETVER_ZERO)

#if PACKETVER_MAIN_NUM >= 20170830 || PACKETVER_RE_NUM >= 20170830 || defined(PACKETVER_ZERO)
struct PACKET_ZC_DEBUGMSG {
	int16 packetType;
	int16 packetLength;
	int32 color;
	char message[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_DEBUGMSG, 0x0adb);
#endif

#if PACKETVER_MAIN_NUM >= 20181002 || PACKETVER_RE_NUM >= 20181002 || PACKETVER_ZERO_NUM >= 20181010
struct PACKET_CZ_USE_SKILL_START {
	int16 packetType;
	int16 skillId;
	int16 skillLv;
	uint32 targetId;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_USE_SKILL_START, 0x0b10);

struct PACKET_CZ_USE_SKILL_END {
	int16 packetType;
	int16 skillId;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_USE_SKILL_END, 0x0b11);
#endif

#if PACKETVER_MAIN_NUM >= 20181219 || PACKETVER_RE_NUM >= 20181219 || PACKETVER_ZERO_NUM >= 20181212
struct PACKET_ZC_EXTEND_BODYITEM_SIZE {
	int16 packetType;
	int16 expansionSize;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_EXTEND_BODYITEM_SIZE, 0x0b18);
#endif

#if PACKETVER_MAIN_NUM >= 20181219 || PACKETVER_RE_NUM >= 20181219 || PACKETVER_ZERO_NUM >= 20181212
struct PACKET_ZC_ACK_OPEN_MSGBOX_EXTEND_BODYITEM_SIZE {
	int16 packetType;
	uint8 result;
	uint32 itemId;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_OPEN_MSGBOX_EXTEND_BODYITEM_SIZE, 0x0b15);
#endif

#if PACKETVER_MAIN_NUM >= 20181219 || PACKETVER_RE_NUM >= 20181219 || PACKETVER_ZERO_NUM >= 20181212
struct PACKET_ZC_ACK_EXTEND_BODYITEM_SIZE {
	int16 packetType;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_EXTEND_BODYITEM_SIZE, 0x0b17);
#endif

#if PACKETVER_MAIN_NUM >= 20181031 || PACKETVER_RE_NUM >= 20181031 || PACKETVER_ZERO_NUM >= 20181114
struct PACKET_CZ_REQ_OPEN_MSGBOX_EXTEND_BODYITEM_SIZE {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_OPEN_MSGBOX_EXTEND_BODYITEM_SIZE, 0x0b14);
#endif

#if PACKETVER_MAIN_NUM >= 20181031 || PACKETVER_RE_NUM >= 20181031 || PACKETVER_ZERO_NUM >= 20181114
struct PACKET_CZ_REQ_EXTEND_BODYITEM_SIZE {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_EXTEND_BODYITEM_SIZE, 0x0b16);
#endif

#if PACKETVER_MAIN_NUM >= 20181031 || PACKETVER_RE_NUM >= 20181031 || PACKETVER_ZERO_NUM >= 20181114
struct PACKET_CZ_CLOSE_MSGBOX_EXTEND_BODYITEM_SIZE {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_CLOSE_MSGBOX_EXTEND_BODYITEM_SIZE, 0x0b19);
#endif

struct PACKET_CZ_REQ_REMAINTIME {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_REMAINTIME, 0x01c0);

struct PACKET_CZ_PARTY_CONFIG {
	int16 packetType;
	uint8 refuseInvite;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_PARTY_CONFIG, 0x02c8);

#if PACKETVER_MAIN_NUM >= 20210203 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_NPC_BARTER_MARKET_ITEMINFO_sub {
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 nameid;
#else
	uint16 nameid;
#endif
	uint8 type;
	uint32 amount;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 currencyNameid;
#else
	uint16 currencyNameid;
#endif
	uint32 currencyAmount;
	uint32 weight;
	uint32 index;
	uint16 viewSprite;
	uint32 location;
} __attribute__((packed));
struct PACKET_ZC_NPC_BARTER_MARKET_ITEMINFO {
	int16 packetType;
	int16 packetLength;
	struct PACKET_ZC_NPC_BARTER_MARKET_ITEMINFO_sub list[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NPC_BARTER_MARKET_ITEMINFO, 0x0b78);
#elif PACKETVER_MAIN_NUM >= 20190116 || PACKETVER_RE_NUM >= 20190116 || PACKETVER_ZERO_NUM >= 20181226
struct PACKET_ZC_NPC_BARTER_MARKET_ITEMINFO_sub {
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 nameid;
#else
	uint16 nameid;
#endif
	uint8 type;
	uint32 amount;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 currencyNameid;
#else
	uint16 currencyNameid;
#endif
	uint32 currencyAmount;
	uint32 weight;
	uint32 index;
} __attribute__((packed));
struct PACKET_ZC_NPC_BARTER_MARKET_ITEMINFO {
	int16 packetType;
	int16 packetLength;
	struct PACKET_ZC_NPC_BARTER_MARKET_ITEMINFO_sub list[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NPC_BARTER_MARKET_ITEMINFO, 0x0b0e);
#endif

#if PACKETVER_MAIN_NUM >= 20190116 || PACKETVER_RE_NUM >= 20190116 || PACKETVER_ZERO_NUM >= 20181226
struct PACKET_CZ_NPC_BARTER_MARKET_CLOSE {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_NPC_BARTER_MARKET_CLOSE, 0x0b12);
#endif

#if PACKETVER_MAIN_NUM >= 20190116 || PACKETVER_RE_NUM >= 20190116 || PACKETVER_ZERO_NUM >= 20181226
struct PACKET_CZ_NPC_BARTER_MARKET_PURCHASE_sub {
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint32 amount;
	uint16 invIndex;
	uint32 shopIndex;
} __attribute__((packed));

struct PACKET_CZ_NPC_BARTER_MARKET_PURCHASE {
	int16 packetType;
	int16 packetLength;
	struct PACKET_CZ_NPC_BARTER_MARKET_PURCHASE_sub list[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_NPC_BARTER_MARKET_PURCHASE, 0x0b0f);
#endif

#if PACKETVER_MAIN_NUM >= 20181212 || PACKETVER_RE_NUM >= 20181212 ||  PACKETVER_ZERO_NUM >= 20190130
struct PACKET_ZC_USESKILL_ACK {
	int16 packetType;
	uint32 srcId;
	uint32 dstId;
	uint16 x;
	uint16 y;
	uint16 skillId;
	uint32 element;
	uint32 delayTime;
	uint8 disposable;
	uint32 attackMT;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_USESKILL_ACK, 0x0b1a);
#elif PACKETVER_MAIN_NUM >= 20091124 || PACKETVER_RE_NUM >= 20091124 || defined(PACKETVER_ZERO)
struct PACKET_ZC_USESKILL_ACK {
	int16 packetType;
	uint32 srcId;
	uint32 dstId;
	uint16 x;
	uint16 y;
	uint16 skillId;
	uint32 element;
	uint32 delayTime;
	uint8 disposable;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_USESKILL_ACK, 0x07fb);
#else
struct PACKET_ZC_USESKILL_ACK {
	int16 packetType;
	uint32 srcId;
	uint32 dstId;
	uint16 x;
	uint16 y;
	uint16 skillId;
	uint32 element;
	uint32 delayTime;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_USESKILL_ACK, 0x013e);
#endif

#if PACKETVER_MAIN_NUM >= 20090406 || PACKETVER_RE_NUM >= 20090408 || PACKETVER_SAK_NUM >= 20090408 || defined(PACKETVER_ZERO)
struct PACKET_CZ_CLIENT_VERSION {
	int16 packetType;
	uint32 clientVersion;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_CLIENT_VERSION, 0x044a);
#endif

#if PACKETVER_MAIN_NUM >= 20190227 || PACKETVER_RE_NUM >= 20190220 || PACKETVER_ZERO_NUM >= 20190220
struct PACKET_CZ_PING_LIVE {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_PING_LIVE, 0x0b1c);
#endif

#if PACKETVER_MAIN_NUM >= 20190227 || PACKETVER_RE_NUM >= 20190220 || PACKETVER_ZERO_NUM >= 20190220
struct PACKET_ZC_PING_LIVE {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PING_LIVE, 0x0b1d);
#endif

#if PACKETVER >= 20160622
struct PACKET_CZ_CMD_RESETCOOLTIME {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_CMD_RESETCOOLTIME, 0x0a88);
#endif

#if PACKETVER >= 20151104
struct PACKET_CZ_CLOSE_UI_STYLINGSHOP {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_CLOSE_UI_STYLINGSHOP, 0x0a48);
#endif

#if PACKETVER_MAIN_NUM >= 20190403 || PACKETVER_RE_NUM >= 20190320 || PACKETVER_ZERO_NUM >= 20190410
struct PACKET_ZC_NOTIFY_ACTORINIT {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_ACTORINIT, 0x0b1b);
#endif

#if PACKETVER_MAIN_NUM >= 20070911 || defined(PACKETVER_RE) || PACKETVER_AD_NUM >= 20070911 || PACKETVER_SAK_NUM >= 20070904 || defined(PACKETVER_ZERO)
struct PACKET_ZC_PARTY_CONFIG {
	int16 packetType;
	uint8 denyPartyInvites;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PARTY_CONFIG, 0x02c9);
#endif

struct PACKET_ZC_ROLE_CHANGE {
	int16 packetType;
	int32 flag;
	char name[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ROLE_CHANGE, 0x00e1);

#if PACKETVER >= 20200902
struct PACKET_ZC_BAN_LIST_sub {
	int char_id;
	char message[40];
	char char_name[NAME_LENGTH];
} __attribute__((packed));

struct PACKET_ZC_BAN_LIST {
	int16 packetType;
	uint16 packetLen;
	struct PACKET_ZC_BAN_LIST_sub chars[];
} __attribute__((packed));

DEFINE_PACKET_HEADER(ZC_BAN_LIST, 0x0b7c);
#elif PACKETVER_MAIN_NUM >= 20161019 || PACKETVER_RE_NUM >= 20160921 || defined(PACKETVER_ZERO)
struct PACKET_ZC_BAN_LIST_sub {
	int char_id;
	char message[40];
} __attribute__((packed));

struct PACKET_ZC_BAN_LIST {
	int16 packetType;
	uint16 packetLen;
	struct PACKET_ZC_BAN_LIST_sub chars[];
} __attribute__((packed));

DEFINE_PACKET_HEADER(ZC_BAN_LIST, 0x0a87);
// version unconfirmed
#elif PACKETVER >= 20100803
struct PACKET_ZC_BAN_LIST_sub {
	char char_name[NAME_LENGTH];
	char message[40];
} __attribute__((packed));

struct PACKET_ZC_BAN_LIST {
	int16 packetType;
	uint16 packetLen;
	struct PACKET_ZC_BAN_LIST_sub chars[];
} __attribute__((packed));

DEFINE_PACKET_HEADER(ZC_BAN_LIST, 0x0163);
#else
struct PACKET_ZC_BAN_LIST_sub {
	char char_name[NAME_LENGTH];
	char account_name[NAME_LENGTH];
	char message[40];
} __attribute__((packed));

struct PACKET_ZC_BAN_LIST {
	int16 packetType;
	uint16 packetLen;
	struct PACKET_ZC_BAN_LIST_sub chars[];
} __attribute__((packed));

DEFINE_PACKET_HEADER(ZC_BAN_LIST, 0x0163);
#endif

#if PACKETVER_MAIN_NUM >= 20141008 || PACKETVER_RE_NUM >= 20140903 || defined(PACKETVER_ZERO)
struct PACKET_ZC_ACK_CLOSE_ROULETTE {
	int16 packetType;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_CLOSE_ROULETTE, 0x0a1e);
#endif

#if PACKETVER_MAIN_NUM >= 20120314 || PACKETVER_RE_NUM >= 20120221 || defined(PACKETVER_ZERO)
struct PACKET_ZC_ACK_MERGE_ITEM {
	int16 packetType;
	int16 index;
	int16 amount;
	uint8 reason;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_MERGE_ITEM, 0x096f);
#endif

#if PACKETVER_MAIN_NUM >= 20120314 || PACKETVER_RE_NUM >= 20120221 || defined(PACKETVER_ZERO)
struct PACKET_ZC_MERGE_ITEM_OPEN_sub {
	int16 index;
} __attribute__((packed));

struct PACKET_ZC_MERGE_ITEM_OPEN {
	int16 packetType;
	uint16 packetLen;
	struct PACKET_ZC_MERGE_ITEM_OPEN_sub items[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_MERGE_ITEM_OPEN, 0x096d);
#endif

#if PACKETVER_MAIN_NUM >= 20101123 || PACKETVER_RE_NUM >= 20120328 || defined(PACKETVER_ZERO)
struct PACKET_ZC_SE_PC_BUY_CASHITEM_RESULT {
	int16 packetType;
	uint32 itemId;  // unused
	uint16 result;
	uint32 cashPoints;
	uint32 kafraPoints;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SE_PC_BUY_CASHITEM_RESULT, 0x0849);
#endif

#if PACKETVER_MAIN_NUM >= 20161130 || PACKETVER_RE_NUM >= 20161109 || defined(PACKETVER_ZERO)
struct PACKET_ZC_OPEN_REFINING_UI {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_OPEN_REFINING_UI, 0x0aa0);
#endif

#if PACKETVER_MAIN_NUM >= 20161005 || PACKETVER_RE_NUM >= 20161005 || defined(PACKETVER_ZERO)
struct PACKET_CZ_REFINING_SELECT_ITEM {
	int16 packetType;
	int16 index;
};
DEFINE_PACKET_HEADER(CZ_REFINING_SELECT_ITEM, 0x0aa1);
#endif

#if PACKETVER_MAIN_NUM >= 20161130 || PACKETVER_RE_NUM >= 20161109 || defined(PACKETVER_ZERO)
struct PACKET_ZC_REFINING_MATERIAL_LIST_SUB {
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	int8 chance;
	int32 zeny;
} __attribute__((packed));

struct PACKET_ZC_REFINING_MATERIAL_LIST {
	int16 packetType;
	int16 packetLength;
	int16 itemIndex;
	int8 blacksmithBlessing;
	struct PACKET_ZC_REFINING_MATERIAL_LIST_SUB req[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_REFINING_MATERIAL_LIST, 0x0aa2);
#endif

#if PACKETVER_MAIN_NUM >= 20161005 || PACKETVER_RE_NUM >= 20161005 || defined(PACKETVER_ZERO)
struct PACKET_CZ_REQ_REFINING {
	int16 packetType;
	int16 index;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	int8 blacksmithBlessing;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_REFINING, 0x0aa3);

struct PACKET_CZ_CLOSE_REFINING_UI {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_CLOSE_REFINING_UI, 0x0aa4);
#endif

#if PACKETVER_MAIN_NUM >= 20170906 || PACKETVER_RE_NUM >= 20170830 || defined(PACKETVER_ZERO)
struct PACKET_ZC_BROADCAST_ITEMREFINING_RESULT {
	int16 packetType;
	char name[NAME_LENGTH];
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	int8 refine_level;
	int8 status;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BROADCAST_ITEMREFINING_RESULT, 0x0ada);
#endif

struct PACKET_ZC_STATUS_CHANGE_ACK {
	int16 packetType;
	uint16 sp;
	uint8 ok;
	uint8 value;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_STATUS_CHANGE_ACK, 0x00bc);

#if PACKETVER_MAIN_NUM >= 20150507 || PACKETVER_RE_NUM >= 20150429 || defined(PACKETVER_ZERO)
struct PACKET_ZC_EQUIPMENT_EFFECT {
	int16 packetType;
	int16 packetLength;
	uint32 aid;
	int8 status;
	uint16 effects[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_EQUIPMENT_EFFECT, 0x0a3b);
#endif

// [4144] this struct updated not in all packets in client
#if PACKETVER_RE_NUM >= 20190807 || PACKETVER_ZERO_NUM >= 20190918
struct SKILLDATA {
	uint16 id;
	int inf;
	uint16 level;
	uint16 sp;
	uint16 range2;
	uint8 upFlag;
	uint16 level2;
} __attribute__((packed));
#else
struct SKILLDATA {
	uint16 id;
	int inf;
	uint16 level;
	uint16 sp;
	uint16 range2;
	char name[NAME_LENGTH];
	uint8 upFlag;
} __attribute__((packed));
#endif

struct PACKET_ZC_ADD_SKILL {
	int16 packetType;
	struct SKILLDATA skill;
} __attribute__((packed));
#if PACKETVER_RE_NUM >= 20190807 || PACKETVER_ZERO_NUM >= 20190918
DEFINE_PACKET_HEADER(ZC_ADD_SKILL, 0x0b31);
#else
DEFINE_PACKET_HEADER(ZC_ADD_SKILL, 0x0111);
#endif

struct PACKET_ZC_SKILLINFO_LIST {
	int16 packetType;
	int16 packetLength;
	struct SKILLDATA skills[];
} __attribute__((packed));
#if PACKETVER_RE_NUM >= 20190807 || PACKETVER_ZERO_NUM >= 20190918
DEFINE_PACKET_HEADER(ZC_SKILLINFO_LIST, 0x0b32);
#else
DEFINE_PACKET_HEADER(ZC_SKILLINFO_LIST, 0x010f);
#endif

#if PACKETVER_RE_NUM >= 20190807 || PACKETVER_ZERO_NUM >= 20190918
struct PACKET_ZC_SKILLINFO_UPDATE2 {
	int16 packetType;
	uint16 id;
	int32 inf;
	uint16 level;
	uint16 sp;
	uint16 range2;
	uint8 upFlag;
	uint16 level2;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SKILLINFO_UPDATE2, 0x0b33);
#else
struct PACKET_ZC_SKILLINFO_UPDATE2 {
	int16 packetType;
	uint16 id;
	int32 inf;
	uint16 level;
	uint16 sp;
	uint16 range2;
	uint8 upFlag;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SKILLINFO_UPDATE2, 0x07e1);
#endif

struct PACKET_ZC_NPC_MARKET_PURCHASE_RESULT_sub {
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 ITID;
#else
	uint16 ITID;
#endif
	uint16 qty;
	uint32 price;
} __attribute__((packed));

#if PACKETVER_MAIN_NUM >= 20190807 || PACKETVER_RE_NUM >= 20190807 || PACKETVER_ZERO_NUM >= 20190814
struct PACKET_ZC_NPC_MARKET_PURCHASE_RESULT {
	int16 PacketType;
	int16 PacketLength;
	uint16 result;
	struct PACKET_ZC_NPC_MARKET_PURCHASE_RESULT_sub list[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NPC_MARKET_PURCHASE_RESULT, 0x0b4e);
#elif PACKETVER_MAIN_NUM >= 20131120 || PACKETVER_RE_NUM >= 20130911 || defined(PACKETVER_ZERO)
struct PACKET_ZC_NPC_MARKET_PURCHASE_RESULT {
	int16 PacketType;
	int16 PacketLength;
	uint8 result;
	struct PACKET_ZC_NPC_MARKET_PURCHASE_RESULT_sub list[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NPC_MARKET_PURCHASE_RESULT, 0x09d7);
#endif

struct PACKET_ZC_TALKBOX_CHATCONTENTS {
	int16 PacketType;
	uint32 aid;
	char message[TALKBOX_MESSAGE_SIZE];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_TALKBOX_CHATCONTENTS, 0x0191);

#if PACKETVER_MAIN_NUM >= 20190731 || PACKETVER_RE_NUM >= 20190717 || PACKETVER_ZERO_NUM >= 20190814
struct PACKET_ZC_GUILD_AGIT_INFO {
	int16 packetType;
	int16 packetLength;
	int8 castle_list[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_GUILD_AGIT_INFO, 0x0b27);
#endif

#if PACKETVER_MAIN_NUM >= 20190522 || PACKETVER_RE_NUM >= 20190522 || PACKETVER_ZERO_NUM >= 20190515
struct PACKET_CZ_REQ_MOVE_GUILD_AGIT {
	int16 packetType;
	int8 castle_id;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_MOVE_GUILD_AGIT, 0x0b28);
#endif

#if PACKETVER_MAIN_NUM >= 20190731 || PACKETVER_RE_NUM >= 20190717 || PACKETVER_ZERO_NUM >= 20190814
struct PACKET_ZC_REQ_ACK_MOVE_GUILD_AGIT {
	int16 packetType;
	int16 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_REQ_ACK_MOVE_GUILD_AGIT, 0x0b2e);
#endif

#if PACKETVER_MAIN_NUM >= 20190731 || PACKETVER_RE_NUM >= 20190717 || PACKETVER_ZERO_NUM >= 20190814
struct PACKET_ZC_REQ_ACK_AGIT_INVESTMENT {
	int16 packetType;
	int8 castle_id;
	int32 economy;
	int32 defense;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_REQ_ACK_AGIT_INVESTMENT, 0x0b2d);
#endif

#if PACKETVER_MAIN_NUM >= 20190522 || PACKETVER_RE_NUM >= 20190522 || PACKETVER_ZERO_NUM >= 20190515
struct PACKET_CZ_REQ_AGIT_INVESTMENT {
	int16 packetType;
	int8 castle_id;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_AGIT_INVESTMENT, 0x0b2c);
#endif

#if PACKETVER_MAIN_NUM >= 20160601 || PACKETVER_RE_NUM >= 20160525 || defined(PACKETVER_ZERO)
struct PACKET_ZC_RANDOM_COMBINE_ITEM_UI_OPEN {
	int16 packetType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	int32 itemId;
#else
	int16 itemId;
#endif
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_RANDOM_COMBINE_ITEM_UI_OPEN, 0x0a4e);
#endif // PACKETVER_MAIN_NUM >= 20160601 || PACKETVER_RE_NUM >= 20160525 || defined(PACKETVER_ZERO)

#if PACKETVER_MAIN_NUM >= 20160504 || PACKETVER_RE_NUM >= 20160504 || defined(PACKETVER_ZERO)
struct PACKET_CZ_RANDOM_COMBINE_ITEM_UI_CLOSE {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_RANDOM_COMBINE_ITEM_UI_CLOSE, 0x0a70);
#endif // PACKETVER_MAIN_NUM >= 20160504 || PACKETVER_RE_NUM >= 20160504 || defined(PACKETVER_ZERO)

#if PACKETVER >= 20160302
struct PACKET_CZ_REQ_RANDOM_COMBINE_ITEM_sub {
	int16 index;
	int16 count;
} __attribute__((packed));

struct PACKET_CZ_REQ_RANDOM_COMBINE_ITEM {
	int16 packetType;
	int16 packetLength;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	int32 itemId;
#else
	int16 itemId;
#endif
	struct PACKET_CZ_REQ_RANDOM_COMBINE_ITEM_sub items[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_RANDOM_COMBINE_ITEM, 0x0a4f);
#endif // PACKETVER >= 20160302

#if PACKETVER_MAIN_NUM >= 20160601 || PACKETVER_RE_NUM >= 20160525 || defined(PACKETVER_ZERO)
struct PACKET_ZC_ACK_RANDOM_COMBINE_ITEM {
	int16 packetType;
	int16 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_RANDOM_COMBINE_ITEM, 0x0a50);
#endif // PACKETVER_MAIN_NUM >= 20160601 || PACKETVER_RE_NUM >= 20160525 || defined(PACKETVER_ZERO)

#if PACKETVER_MAIN_NUM >= 20190703 || PACKETVER_RE_NUM >= 20190703 || PACKETVER_ZERO_NUM >= 20190709
struct PACKET_CZ_UNINSTALLATION {
	int16 PacketType;
	uint8 InstallationKind;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_UNINSTALLATION, 0x0b35);
#endif

// in 3 clients from same version
#if PACKETVER >= 20191127
struct PACKET_ZC_NOTIFY_EFFECT3 {
	int16 packetType;
	uint32 aid;
	uint32 effectId;
	uint64 num;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_EFFECT3, 0x0b69);
#elif PACKETVER_MAIN_NUM >= 20060911 || PACKETVER_AD_NUM >= 20060911 || PACKETVER_SAK_NUM >= 20060911 || defined(PACKETVER_RE) || defined(PACKETVER_ZERO)
struct PACKET_ZC_NOTIFY_EFFECT3 {
	int16 packetType;
	uint32 aid;
	uint32 effectId;
	uint32 num;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_EFFECT3, 0x0284);
#endif

#if PACKETVER >= 20100824
struct PACKET_CZ_SE_CASHSHOP_OPEN1 {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_SE_CASHSHOP_OPEN1, 0x0844);
#endif

#if PACKETVER >= 20191224
struct PACKET_CZ_SE_CASHSHOP_OPEN2 {
	int16 packetType;
	uint32 tab;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_SE_CASHSHOP_OPEN2, 0x0b6d);
#endif

#if PACKETVER >= 20190724
struct PACKET_CZ_GET_ACCOUNT_LIMTIED_SALE_LIST {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_GET_ACCOUNT_LIMTIED_SALE_LIST, 0x0b4c);
#endif

#if PACKETVER_MAIN_NUM >= 20200129 || PACKETVER_RE_NUM >= 20200205 || PACKETVER_ZERO_NUM >= 20191224
struct PACKET_ZC_SE_CASHSHOP_OPEN {
	int16 packetType;
	uint32 cashPoints;
	uint32 kafraPoints;
	uint32 tab;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SE_CASHSHOP_OPEN, 0x0b6e);
#elif PACKETVER >= 20140730
struct PACKET_ZC_SE_CASHSHOP_OPEN {
	int16 packetType;
	uint32 cashPoints;
	uint32 kafraPoints;
	uint32 tab;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SE_CASHSHOP_OPEN, 0x0a2b);
#elif PACKETVER_MAIN_NUM >= 20101123 || PACKETVER_RE_NUM >= 20120328 || defined(PACKETVER_ZERO)
struct PACKET_ZC_SE_CASHSHOP_OPEN {
	int16 packetType;
	uint32 cashPoints;
	uint32 kafraPoints;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SE_CASHSHOP_OPEN, 0x0845);
#endif

#if PACKETVER_MAIN_NUM >= 20190904 || PACKETVER_RE_NUM >= 20190904 || PACKETVER_ZERO_NUM >= 20190828
struct PACKET_CZ_NPC_EXPANDED_BARTER_MARKET_CLOSE {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_NPC_EXPANDED_BARTER_MARKET_CLOSE, 0x0b58);
#endif

#if PACKETVER_MAIN_NUM >= 20190904 || PACKETVER_RE_NUM >= 20190904 || PACKETVER_ZERO_NUM >= 20190828
struct PACKET_CZ_NPC_EXPANDED_BARTER_MARKET_PURCHASE_sub {
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint32 shopIndex;
	uint32 amount;
} __attribute__((packed));

struct PACKET_CZ_NPC_EXPANDED_BARTER_MARKET_PURCHASE {
	int16 packetType;
	int16 packetLength;
	struct PACKET_CZ_NPC_EXPANDED_BARTER_MARKET_PURCHASE_sub list[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_NPC_EXPANDED_BARTER_MARKET_PURCHASE, 0x0b57);
#endif

#if PACKETVER >= 7
struct PACKET_ZC_STATE_CHANGE {
	int16 packetType;
	uint32 AID;
	int16 bodyState;
	int16 healthState;
	int32 effectState;
	int8 isPKModeON;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_STATE_CHANGE, 0x0229);
#else
struct PACKET_ZC_STATE_CHANGE {
	int16 PacketType;
	uint32 AID;
	int16 bodyState;
	int16 healthState;
	int16 effectState;
	int8 isPKModeON;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_STATE_CHANGE, 0x0119);
#endif

struct PACKET_ZC_AUTORUN_SKILL {
	int16 packetType;
	uint16 skill_id;
	uint32 skill_type;
	uint16 skill_lv;
	uint16 skill_sp;
	uint16 skill_range;
	char skill_name[NAME_LENGTH];
	char up_flag;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_AUTORUN_SKILL, 0x0147);

#if PACKETVER_MAIN_NUM >= 20170726 || PACKETVER_RE_NUM >= 20170621 || defined(PACKETVER_ZERO)
struct PACKET_ZC_RANDOM_UPGRADE_ITEM_UI_OPEN {
	int16 packetType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_RANDOM_UPGRADE_ITEM_UI_OPEN, 0x0ab4);

struct PACKET_ZC_ACK_RANDOM_UPGRADE_ITEM {
	int16 packetType;
	uint16 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_RANDOM_UPGRADE_ITEM, 0x0ab7);
#endif  // PACKETVER_MAIN_NUM >= 20170726 || PACKETVER_RE_NUM >= 20170621 || defined(PACKETVER_ZERO)

#if PACKETVER_MAIN_NUM >= 20170111 || PACKETVER_RE_NUM >= 20170111 || defined(PACKETVER_ZERO)
struct PACKET_CZ_RANDOM_UPGRADE_ITEM_UI_CLOSE {
	int16 packetType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_RANDOM_UPGRADE_ITEM_UI_CLOSE, 0x0ab5);

struct PACKET_CZ_REQ_RANDOM_UPGRADE_ITEM {
	int16 packetType;
#if PACKETVER_MAIN_NUM >= 20181121 || PACKETVER_RE_NUM >= 20180704 || PACKETVER_ZERO_NUM >= 20181114
	uint32 itemId;
#else
	uint16 itemId;
#endif
	uint16 index;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_RANDOM_UPGRADE_ITEM, 0x0ab6);
#endif  // PACKETVER_MAIN_NUM >= 20170111 || PACKETVER_RE_NUM >= 20170111 || defined(PACKETVER_ZERO)

#if PACKETVER_MAIN_NUM >= 20120503 || PACKETVER_RE_NUM >= 20120502 || defined(PACKETVER_ZERO)
struct PACKET_ZC_PERSONAL_INFOMATION_SUB {
	int8 type;
	int32 exp;
	int32 death;
	int32 drop;
} __attribute__((packed));
struct PACKET_ZC_PERSONAL_INFOMATION {
	int16 packetType;
	int16 length;
	int32 total_exp;
	int32 total_death;
	int32 total_drop;
	struct PACKET_ZC_PERSONAL_INFOMATION_SUB details[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PERSONAL_INFOMATION, 0x097b);
#elif PACKETVER_MAIN_NUM >= 20110627 || PACKETVER_RE_NUM >= 20110628
struct PACKET_ZC_PERSONAL_INFOMATION_SUB {
	int8 type;
	int16 exp;
	int16 death;
	int16 drop;
} __attribute__((packed));
struct PACKET_ZC_PERSONAL_INFOMATION {
	int16 packetType;
	int16 length;
	int16 total_exp;
	int16 total_death;
	int16 total_drop;
	struct PACKET_ZC_PERSONAL_INFOMATION_SUB details[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PERSONAL_INFOMATION, 0x08cb);
#endif // PACKETVER_MAIN_NUM >= 20110627 || PACKETVER_RE_NUM >= 20110628

struct PACKET_CZ_REQUEST_ACTNPC {
	int16 packetType;
	uint32 GID;
	uint32 targetGID;
	int8 action;
} __attribute__((packed));

#if PACKETVER < 3
struct PACKET_ZC_NOTIFY_SKILL {
	int16 PacketType;
	uint16 SKID;
	uint32 AID;
	uint32 targetID;
	uint32 startTime;
	int32 attackMT;
	int32 attackedMT;
	int16 damage;
	int16 level;
	int16 count;
	int8 action;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_SKILL, 0x0114);
#else
struct PACKET_ZC_NOTIFY_SKILL {
	int16 PacketType;
	uint16 SKID;
	uint32 AID;
	uint32 targetID;
	uint32 startTime;
	int32 attackMT;
	int32 attackedMT;
	int32 damage;
	int16 level;
	int16 count;
	int8 action;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_SKILL, 0x01de);
#endif

#if PACKETVER_MAIN_NUM >= 20130731 || PACKETVER_RE_NUM >= 20130724 || defined(PACKETVER_ZERO)
struct PACKET_ZC_USE_SKILL {
	int16 PacketType;
	uint16 SKID;
	int32 level;
	uint32 targetAID;
	uint32 srcAID;
	int8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_USE_SKILL, 0x09cb);
#else
struct PACKET_ZC_USE_SKILL {
	int16 PacketType;
	uint16 SKID;
	int16 level;
	uint32 targetAID;
	uint32 srcAID;
	int8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_USE_SKILL, 0x011a);
#endif

struct PACKET_ZC_NOTIFY_GROUNDSKILL {
	int16 PacketType;
	uint16 SKID;
	uint32 AID;
	int16 level;
	int16 xPos;
	int16 yPos;
	uint32 startTime;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_GROUNDSKILL, 0x0117);

#if PACKETVER_MAIN_NUM >= 20081112 || PACKETVER_RE_NUM >= 20081111 || defined(PACKETVER_ZERO)
struct PACKET_ZC_SKILL_POSTDELAY {
	int16 PacketType;
	uint16 SKID;
	uint32 DelayTM;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SKILL_POSTDELAY, 0x043d);
#endif

struct PACKET_ZC_NOTIFY_SKILL_POSITION {
	int16 PacketType;
	uint16 SKID;
	uint32 AID;
	uint32 targetID;
	uint32 startTime;
	int32 attackMT;
	int32 attackedMT;
	int16 xPos;
	int16 yPos;
	int16 damage;
	int16 level;
	int16 count;
	int8 action;
};

DEFINE_PACKET_HEADER(ZC_NOTIFY_SKILL_POSITION, 0x0115);

#if PACKETVER_MAIN_NUM >= 20130731 || PACKETVER_RE_NUM >= 20130707 || defined(PACKETVER_ZERO)
struct PACKET_ZC_C_MARKERINFO {
	int16 PacketType;
	uint32 AID;
	int16 xPos;
	int16 yPos;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_C_MARKERINFO, 0x09c1);
#endif

#if PACKETVER >= 20200902
struct GUILD_MEMBER_INFO {
	uint32 AID;
	uint32 GID;
	int16 head;
	int16 headPalette;
	int16 sex;
	int16 job;
	int16 level;
	int32 contributionExp;
	int32 currentState;
	int32 positionID;
	uint32 lastLoginTime;
	char char_name[NAME_LENGTH];
} __attribute__((packed));
struct PACKET_ZC_MEMBERMGR_INFO {
	int16 PacketType;
	int16 packetLength;
	struct GUILD_MEMBER_INFO guildMemberInfo[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_MEMBERMGR_INFO, 0x0b7d);
#elif PACKETVER_MAIN_NUM >= 20161214 || PACKETVER_RE_NUM >= 20161130 || defined(PACKETVER_ZERO)
struct GUILD_MEMBER_INFO {
	uint32 AID;
	uint32 GID;
	int16 head;
	int16 headPalette;
	int16 sex;
	int16 job;
	int16 level;
	int32 contributionExp;
	int32 currentState;
	int32 positionID;
	uint32 lastLoginTime;
} __attribute__((packed));
struct PACKET_ZC_MEMBERMGR_INFO {
	int16 PacketType;
	int16 packetLength;
	struct GUILD_MEMBER_INFO guildMemberInfo[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_MEMBERMGR_INFO, 0x0aa5);
#else  // false: PACKETVER_MAIN_NUM >= 20161214 || PACKETVER_RE_NUM >= 20161130 || defined(PACKETVER_ZERO)
struct GUILD_MEMBER_INFO {
	uint32 AID;
	uint32 GID;
	int16 head;
	int16 headPalette;
	int16 sex;
	int16 job;
	int16 level;
	int32 contributionExp;
	int32 currentState;
	int32 positionID;
	char intro[50];
	char char_name[NAME_LENGTH];
} __attribute__((packed));
struct PACKET_ZC_MEMBERMGR_INFO {
	int16 PacketType;
	int16 packetLength;
	struct GUILD_MEMBER_INFO guildMemberInfo[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_MEMBERMGR_INFO, 0x0154);
#endif

#if PACKETVER >= 20200902
struct PACKET_ZC_GUILD_INFO {
	int16 PacketType;
	int GDID;
	int level;
	int userNum;
	int maxUserNum;
	int userAverageLevel;
	int exp;
	int maxExp;
	int point;
	int honor;
	int virtue;
	int emblemVersion;
	char guildname[NAME_LENGTH];
	char manageLand[MAP_NAME_LENGTH_EXT];
	int zeny;
	int masterGID;
	char masterName[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_GUILD_INFO, 0x0b7b);
#elif PACKETVER_MAIN_NUM >= 20161019 || PACKETVER_RE_NUM >= 20160921 || defined(PACKETVER_ZERO)
struct PACKET_ZC_GUILD_INFO {
	int16 PacketType;
	int GDID;
	int level;
	int userNum;
	int maxUserNum;
	int userAverageLevel;
	int exp;
	int maxExp;
	int point;
	int honor;
	int virtue;
	int emblemVersion;
	char guildname[NAME_LENGTH];
	char manageLand[MAP_NAME_LENGTH_EXT];
	int zeny;
	int masterGID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_GUILD_INFO, 0x0a84);
#else
struct PACKET_ZC_GUILD_INFO {
	int16 PacketType;
	int GDID;
	int level;
	int userNum;
	int maxUserNum;
	int userAverageLevel;
	int exp;
	int maxExp;
	int point;
	int honor;
	int virtue;
	int emblemVersion;
	char guildname[NAME_LENGTH];
	char masterName[NAME_LENGTH];
	char manageLand[MAP_NAME_LENGTH_EXT];
	int zeny;
} __attribute__((packed));
//0x150; [4144] this is packet for older versions?
DEFINE_PACKET_HEADER(ZC_GUILD_INFO, 0x01b6);
#endif

struct PACKET_ZC_POSITION_ID_NAME_INFO {
	int16 PacketType;
	int16 PacketLength;
	struct {
		int positionID;
		char posName[NAME_LENGTH];
	} posInfo[MAX_GUILDPOSITION];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_POSITION_ID_NAME_INFO, 0x0166);

struct PACKET_ZC_POSITION_INFO_sub {
	int positionID;
	int right;
	int ranking;
	int payRate;
} __attribute__((packed));

struct PACKET_ZC_POSITION_INFO {
	int16 PacketType;
	int16 PacketLength;
	struct PACKET_ZC_POSITION_INFO_sub posInfo[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_POSITION_INFO, 0x0160);

struct GUILD_SKILLDATA {
	uint16 id;
	int inf;
	uint16 level;
	uint16 sp;
	uint16 range2;
	char name[NAME_LENGTH];
	uint8 upFlag;
} __attribute__((packed));

struct PACKET_ZC_GUILD_SKILLINFO {
	int16 PacketType;
	int16 PacketLength;
	int16 skillPoint;
	struct GUILD_SKILLDATA skillInfo[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_GUILD_SKILLINFO, 0x0162);

struct RELATED_GUILD_INFO {
	int relation;
	int GDID;
	char guildname[NAME_LENGTH];
} __attribute__((packed));

struct PACKET_ZC_MYGUILD_BASIC_INFO {
	int16 PacketType;
	int16 PacketLength;
	struct RELATED_GUILD_INFO rgInfo[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_MYGUILD_BASIC_INFO, 0x014c);

#if PACKETVER >= 20160316
struct PACKET_CZ_REQ_UPLOAD_MACRO_DETECTOR {
	int16 PacketType;
	char answer[16];
	uint16 imageSize;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_UPLOAD_MACRO_DETECTOR, 0x0a52);
#endif

#if PACKETVER >= 20160330
struct PACKET_ZC_ACK_UPLOAD_MACRO_DETECTOR {
	int16 PacketType;
	char captchaKey[4];
	int captchaFlag;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_UPLOAD_MACRO_DETECTOR, 0x0a53);
#endif

#if PACKETVER >= 20160316
struct PACKET_CZ_UPLOAD_MACRO_DETECTOR_CAPTCHA {
	int16 PacketType;
	int16 PacketLength;
	char captchaKey[4];
	char imageData[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_UPLOAD_MACRO_DETECTOR_CAPTCHA, 0x0a54);
#endif

#if PACKETVER >= 20160330
struct PACKET_ZC_COMPLETE_UPLOAD_MACRO_DETECTOR_CAPTCHA {
	int16 PacketType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_COMPLETE_UPLOAD_MACRO_DETECTOR_CAPTCHA, 0x0a55);
#endif

#if PACKETVER >= 20160316
struct PACKET_CZ_REQ_APPLY_MACRO_DETECTOR {
	int16 PacketType;
	uint32 AID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_APPLY_MACRO_DETECTOR, 0x0a56);
#endif

#if PACKETVER >= 20160330
struct PACKET_ZC_ACK_APPLY_MACRO_DETECTOR {
	int16 PacketType;
	int status;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_APPLY_MACRO_DETECTOR, 0x0a57);
#endif

#if PACKETVER >= 20160330
struct PACKET_ZC_APPLY_MACRO_DETECTOR {
	int16 PacketType;
	uint16 imageSize;
	char captchaKey[4];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_APPLY_MACRO_DETECTOR, 0x0a58);
#endif

#if PACKETVER >= 20160330
struct PACKET_ZC_APPLY_MACRO_DETECTOR_CAPTCHA {
	int16 PacketType;
	int16 PacketLength;
	char captchaKey[4];
	char imageData[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_APPLY_MACRO_DETECTOR_CAPTCHA, 0x0a59);
#endif

#if PACKETVER >= 20160316
struct PACKET_CZ_COMPLETE_APPLY_MACRO_DETECTOR_CAPTCHA {
	int16 PacketType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_COMPLETE_APPLY_MACRO_DETECTOR_CAPTCHA, 0x0a5a);
#endif

#if PACKETVER >= 20160330
struct PACKET_ZC_REQ_ANSWER_MACRO_DETECTOR {
	int16 PacketType;
	uint8 retryCount;
	int timeout;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_REQ_ANSWER_MACRO_DETECTOR, 0x0a5b);
#endif

#if PACKETVER >= 20160316
struct PACKET_CZ_ACK_ANSWER_MACRO_DETECTOR {
	int16 PacketType;
	char answer[16];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_ACK_ANSWER_MACRO_DETECTOR, 0x0a5c);
#endif

#if PACKETVER >= 20160330
struct PACKET_ZC_CLOSE_MACRO_DETECTOR {
	int16 PacketType;
	int status;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_CLOSE_MACRO_DETECTOR, 0x0a5d);
#endif

#if PACKETVER >= 20160323
struct PACKET_CZ_REQ_PREVIEW_MACRO_DETECTOR {
	int16 PacketType;
	int captchaID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_PREVIEW_MACRO_DETECTOR, 0x0a69);
#endif

#if PACKETVER >= 20160330
struct PACKET_ZC_ACK_PREVIEW_MACRO_DETECTOR {
	int16 PacketType;
	int captchaFlag;
	uint16 imageSize;
	char captchaKey[4];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_PREVIEW_MACRO_DETECTOR, 0x0a6a);
#endif

#if PACKETVER >= 20160330
struct PACKET_ZC_PREVIEW_MACRO_DETECTOR_CAPTCHA {
	int16 PacketType;
	int16 PacketLength;
	char captchaKey[4];
	char imageData[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PREVIEW_MACRO_DETECTOR_CAPTCHA, 0x0a6b);
#endif

#if PACKETVER >= 20160330
struct PACKET_CZ_REQ_PLAYER_AID_IN_RANGE {
	int16 PacketType;
	int16 xPos;
	int16 yPos;
	int8 RadiusRange;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_PLAYER_AID_IN_RANGE, 0x0a6c);
#endif

#if PACKETVER >= 20160330
struct PACKET_ZC_ACK_PLAYER_AID_IN_RANGE {
	int16 PacketType;
	int16 PacketLength;
	uint32 AID[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_PLAYER_AID_IN_RANGE, 0x0a6d);
#endif

struct PACKET_ZC_ACK_MAKE_GROUP {
	int16 PacketType;
	int8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_MAKE_GROUP, 0x00fa);

struct PACKET_ZC_PARTY_JOIN_REQ {
	int16 PacketType;
	int GRID;
	char groupName[NAME_LENGTH];
} __attribute__((packed));
#if PACKETVER < 20070821
DEFINE_PACKET_HEADER(ZC_PARTY_JOIN_REQ, 0x00fe);
#else
DEFINE_PACKET_HEADER(ZC_PARTY_JOIN_REQ, 0x02c6);
#endif

struct PACKET_ZC_PARTY_JOIN_REQ_ACK {
	int16 PacketType;
	char characterName[NAME_LENGTH];
	int result;
} __attribute__((packed));
#if PACKETVER < 20070821
DEFINE_PACKET_HEADER(ZC_PARTY_JOIN_REQ_ACK, 0x00fd);
#else
DEFINE_PACKET_HEADER(ZC_PARTY_JOIN_REQ_ACK, 0x02c5);
#endif

struct PACKET_ZC_NOTIFY_CHAT_PARTY {
	int16 PacketType;
	int16 PacketLength;
	int AID;
	char chatMsg[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_CHAT_PARTY, 0x0109);

struct PACKET_ZC_NOTIFY_POSITION_TO_GROUPM {
	int16 PacketType;
	int AID;
	int16 xPos;
	int16 yPos;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_POSITION_TO_GROUPM, 0x0107);

#if PACKETVER_ZERO_NUM >= 20210504
struct PACKET_ZC_NOTIFY_HP_TO_GROUPM {
	int16 PacketType;
	uint32 AID;
	int hp;
	int maxhp;
	int sp;
	int maxsp;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_HP_TO_GROUPM, 0x0bab);
#elif PACKETVER >= 20100119
struct PACKET_ZC_NOTIFY_HP_TO_GROUPM {
	int16 PacketType;
	uint32 AID;
	int hp;
	int maxhp;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_HP_TO_GROUPM, 0x080e);
#else  // PACKETVER >= 20100119
struct PACKET_ZC_NOTIFY_HP_TO_GROUPM {
	int16 PacketType;
	uint32 AID;
	int16 hp;
	int16 maxhp;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_HP_TO_GROUPM, 0x0106);
#endif  // PACKETVER >= 20100119

#if PACKETVER_MAIN_NUM >= 20170502 || PACKETVER_RE_NUM >= 20170419 || defined(PACKETVER_ZERO)
struct PACKET_ZC_NOTIFY_MEMBERINFO_TO_GROUPM {
	int16 PacketType;
	int AID;
	int16 job;
	int16 level;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_NOTIFY_MEMBERINFO_TO_GROUPM, 0x0abd);
#endif

struct PACKET_ZC_DELETE_MEMBER_FROM_GROUP {
	int16 PacketType;
	int AID;
	char characterName[NAME_LENGTH];
	int8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_DELETE_MEMBER_FROM_GROUP, 0x0105);

#if PACKETVER_MAIN_NUM >= 20230906
struct PACKET_CZ_REQ_TAKEOFF_EQUIP_ALL {
	int16 PacketType;
	uint32 location;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_TAKEOFF_EQUIP_ALL, 0x0bf5);
#elif PACKETVER_MAIN_NUM >= 20210818 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20210818
struct PACKET_CZ_REQ_TAKEOFF_EQUIP_ALL {
	int16 PacketType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_TAKEOFF_EQUIP_ALL, 0x0bad);
#endif  // PACKETVER_MAIN_NUM >= 20210818 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20210818

#if PACKETVER_MAIN_NUM >= 20210818 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_ACK_TAKEOFF_EQUIP_ALL {
	int16 PacketType;
	uint8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ACK_TAKEOFF_EQUIP_ALL, 0x0bae);
#endif  // PACKETVER_MAIN_NUM >= 20210818 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024

#if PACKETVER_ZERO_NUM >= 20210504
struct PACKET_ZC_BATTLEFIELD_NOTIFY_HP {
	int16 PacketType;
	uint32 AID;
	int hp;
	int maxhp;
	int sp;
	int maxsp;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BATTLEFIELD_NOTIFY_HP, 0x0baa);
#elif PACKETVER >= 20140312
struct PACKET_ZC_BATTLEFIELD_NOTIFY_HP {
	int16 PacketType;
	uint32 AID;
	int hp;
	int maxhp;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BATTLEFIELD_NOTIFY_HP, 0x0a0e);
#elif PACKETVER >= 20071009
struct PACKET_ZC_BATTLEFIELD_NOTIFY_HP {
	int16 PacketType;
	uint32 AID;
	char name[NAME_LENGTH];
	int16 hp;
	int16 maxhp;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BATTLEFIELD_NOTIFY_HP, 0x02e0);
#endif  // PACKETVER >= 20071009

#if PACKETVER_ZERO_NUM >= 20210721
struct PACKET_ZC_QUEST_DIALOG {
	int16 PacketType;
	int16 PacketLength;
	uint32 NpcID;
	char message[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_QUEST_DIALOG, 0x0ba6);
#endif  // PACKETVER_ZERO_NUM >= 20210721

#if PACKETVER_ZERO_NUM >= 20210721
struct PACKET_ZC_MONOLOG_DIALOG {
	int16 PacketType;
	int16 PacketLength;
	uint32 NpcID;
	char message[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_MONOLOG_DIALOG, 0x0ba9);
#endif  // PACKETVER_ZERO_NUM >= 20210721

#if PACKETVER_ZERO_NUM >= 20210721
struct PACKET_ZC_QUEST_DIALOG_MENU_LIST {
	int16 PacketType;
	int16 PacketLength;
	uint32 NpcID;
	char message[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_QUEST_DIALOG_MENU_LIST, 0x0ba7);
#endif  // PACKETVER_ZERO_NUM >= 20210721

#if PACKETVER_MAIN_NUM >= 20210317 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20210317
struct PACKET_CZ_CHOOSE_MENU_ZERO {
	int16 PacketType;
	uint32 NpcID;
	uint8 menuIndex;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_CHOOSE_MENU_ZERO, 0x0ba8);
#endif  // PACKETVER_MAIN_NUM >= 20210317 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20210317

#if PACKETVER_MAIN_NUM >= 20210203 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_DIALOG_TEXT_ALIGN {
	int16 PacketType;
	uint8 align;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_DIALOG_TEXT_ALIGN, 0x0ba1);
#endif  // PACKETVER_MAIN_NUM >= 20210203 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024

#if PACKETVER_MAIN_NUM >= 20191016 || PACKETVER_RE_NUM >= 20191016 || PACKETVER_ZERO_NUM >= 20191008
struct PACKET_CZ_GRADE_ENCHANT_SELECT_EQUIPMENT {
	int16 PacketType;
	int16 index;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_GRADE_ENCHANT_SELECT_EQUIPMENT, 0x0b59);
#endif

#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
struct GRADE_ENCHANT_BLESSING {
	int32 id;
	int32 amount;
	int32 max_blessing;
	int32 bonus;
} __attribute__((packed));

struct GRADE_ENCHANT_MATERIAL {
	int32 nameid;
	int32 amount;
	int32 price;
	int32 downgrade;
	int8 breakable;
} __attribute__((packed));

struct PACKET_ZC_GRADE_ENCHANT_MATERIAL_LIST {
	int16 PacketType;
	int16 PacketLength;
	int16 index;
	int32 success_chance;
	struct GRADE_ENCHANT_BLESSING blessing_info;
	int32 protect_itemid; // used only for PACKETVER_RE_NUM >= 20200723 && PACKETVER_RE_NUM <= 20200819
	int32 protect_amount; // used only for PACKETVER_RE_NUM >= 20200723 && PACKETVER_RE_NUM <= 20200819
	struct GRADE_ENCHANT_MATERIAL material_info[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_GRADE_ENCHANT_MATERIAL_LIST, 0x0b5a);
#endif

#if PACKETVER_MAIN_NUM >= 20191016 || PACKETVER_RE_NUM >= 20191016 || PACKETVER_ZERO_NUM >= 20191008
struct PACKET_CZ_GRADE_ENCHANT_REQUEST {
	int16 PacketType;
	int16 index;
	int material_index;
	int8 blessing_flag;
	int blessing_amount;
	int8 protect_flag; // used only for PACKETVER_RE_NUM >= 20200723 && PACKETVER_RE_NUM <= 20200819
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_GRADE_ENCHANT_REQUEST, 0x0b5b);
#endif

#if PACKETVER_MAIN_NUM >= 20191016 || PACKETVER_RE_NUM >= 20191016 || PACKETVER_ZERO_NUM >= 20191008
struct PACKET_CZ_GRADE_ENCHANT_CLOSE_UI {
	int16 PacketType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_GRADE_ENCHANT_CLOSE_UI, 0x0b5c);
#endif

#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_GRADE_ENCHANT_ACK {
	int16 PacketType;
	int16 index;
	int16 grade;
	int result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_GRADE_ENCHANT_ACK, 0x0b5d);
#endif

#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_GRADE_ENCHANT_BROADCAST_RESULT {
	int16 packetType;
	char name[NAME_LENGTH];
	uint32 itemId;
	int16 grade;
	int8 status;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_GRADE_ENCHANT_BROADCAST_RESULT, 0x0b5e);
#endif

struct PACKET_ZC_SHOW_IMAGE {
	int16 packetType;
	char image[64];
	uint8 type;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SHOW_IMAGE, 0x01b3)

#if PACKETVER_MAIN_NUM >= 20131204 || PACKETVER_RE_NUM >= 20131120 || defined(PACKETVER_ZERO)
struct PACKET_ZC_WHISPER {
	int16 PacketType;
	int16 PacketLength;
	uint32 senderGID;
	char sender[NAME_LENGTH];
	uint8 isAdmin;
	char message[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_WHISPER, 0x09de)
// [4144] unconfirmed version
#elif PACKETVER >= 20091104
struct PACKET_ZC_WHISPER {
	int16 PacketType;
	int16 PacketLength;
	char sender[NAME_LENGTH];
	int32 isAdmin;
	char message[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_WHISPER, 0x0097)
#else  // PACKETVER_MAIN_NUM >= 20131204 || PACKETVER_RE_NUM >= 20131120 || defined(PACKETVER_ZERO)
struct PACKET_ZC_WHISPER {
	int16 PacketType;
	int16 PacketLength;
	char sender[NAME_LENGTH];
	char message[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_WHISPER, 0x0097)
#endif  // PACKETVER_MAIN_NUM >= 20131204 || PACKETVER_RE_NUM >= 20131120 || defined(PACKETVER_ZERO)

#if PACKETVER_MAIN_NUM >= 20220216 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_UPDATE_GDID {
	int16 PacketType;
	uint32 guildId;
	int emblemVersion;
	uint32 mode;
	uint8 isMaster;
	int32 interSid;
	char guildName[NAME_LENGTH];
	uint32 masterGID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_UPDATE_GDID, 0x02f7)
#else  // PACKETVER_MAIN_NUM >= 20220216 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_UPDATE_GDID {
	int16 PacketType;
	uint32 guildId;
	int emblemVersion;
	uint32 mode;
	uint8 isMaster;
	int32 interSid;
	char guildName[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_UPDATE_GDID, 0x016c)
#endif  // PACKETVER_MAIN_NUM >= 20220216 || PACKETVER_ZERO_NUM >= 20221024

#if PACKETVER_MAIN_NUM >= 20220216 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_CZ_APPROXIMATE_ACTOR {
	int16 PacketType;
	uint32 masterGID;
	uint16 unused1;
	uint8 unused2;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_APPROXIMATE_ACTOR, 0x0bb0)
#endif  // PACKETVER_MAIN_NUM >= 20220216 || PACKETVER_ZERO_NUM >= 20221024

struct PACKET_CZ_CONTACTNPC {
	int16 PacketType;
	uint32 AID;
	uint8 type;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_CONTACTNPC, 0x0090)

struct PACKET_ZC_ATTACK_FAILURE_FOR_DISTANCE {
	int16 PacketType;
	uint32 targetAID;
	int16 targetXPos;
	int16 targetYPos;
	int16 xPos;
	int16 yPos;
	int16 currentAttRange;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ATTACK_FAILURE_FOR_DISTANCE, 0x0139)

struct PACKET_ZC_START_CAPTURE {
	int16 PacketType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_START_CAPTURE, 0x019e)

struct PACKET_ZC_TRYCAPTURE_MONSTER {
	int16 PacketType;
	int8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_TRYCAPTURE_MONSTER, 0x01a0)

struct PACKET_ZC_PROPERTY_PET {
	int16 PacketType;
	char szName[NAME_LENGTH];
	int8 bModified;
	int16 nLevel;
	int16 nFullness;
	int16 nRelationship;
	int16 ITID;
#if PACKETVER >= 20081126
	int16 job;
#endif
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PROPERTY_PET, 0x01a2)

struct PACKET_ZC_CHANGESTATE_PET {
	int16 PacketType;
	int8 type;
	int GID;
	int data;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_CHANGESTATE_PET, 0x01a4)

struct PACKET_ZC_SPIRITS {
	int16 PacketType;
	uint32 AID;
	int16 num;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SPIRITS, 0x01d0)

struct PACKET_ZC_SPIRITS2 {
	int16 PacketType;
	uint32 AID;
	int16 num;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SPIRITS2, 0x01e1)

#if PACKETVER_MAIN_NUM >= 20200414 || PACKETVER_RE_NUM >= 20200723 || PACKETVER_ZERO_NUM >= 20200506
struct PACKET_ZC_SOULENERGY {
	int16 PacketType;
	uint32 AID;
	uint16 num;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SOULENERGY, 0x0b73)
#endif

struct PACKET_ZC_SAY_DIALOG {
	int16 PacketType;
	int16 PacketLength;
	uint32 NpcID;
	char message[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SAY_DIALOG, 0x00b4)

#if PACKETVER_MAIN_NUM >= 20220504
struct PACKET_ZC_SAY_DIALOG2 {
	int16 PacketType;
	int16 PacketLength;
	uint32 NpcID;
	uint8 type;
	char message[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SAY_DIALOG2, 0x0972)
#else  // PACKETVER_MAIN_NUM >= 20220504
struct PACKET_ZC_SAY_DIALOG2 {
	int16 PacketType;
	int16 PacketLength;
	uint32 NpcID;
	char message[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SAY_DIALOG2, 0x00b4)
#endif  // PACKETVER_MAIN_NUM >= 20220504

struct PACKET_ZC_WAIT_DIALOG {
	int16 PacketType;
	uint32 NpcID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_WAIT_DIALOG, 0x00b5)

#if PACKETVER_MAIN_NUM >= 20220504
struct PACKET_ZC_WAIT_DIALOG2 {
	int16 PacketType;
	uint32 NpcID;
	uint8 type;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_WAIT_DIALOG2, 0x0973)
#else  // PACKETVER_MAIN_NUM >= 20220504
struct PACKET_ZC_WAIT_DIALOG2 {
	int16 PacketType;
	uint32 NpcID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_WAIT_DIALOG2, 0x00b5)
#endif  // PACKETVER_MAIN_NUM >= 20220504

#if PACKETVER_MAIN_NUM >= 20220504
struct PACKET_ZC_DIALOG_WINDOW_SIZE {
	int16 PacketType;
	int height;
	int width;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_DIALOG_WINDOW_SIZE, 0x0ba2)
#endif  // PACKETVER_MAIN_NUM >= 20220504

#if PACKETVER_MAIN_NUM >= 20220504
struct PACKET_ZC_DIALOG_WINDOW_POS {
	int16 PacketType;
	int x;
	int y;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_DIALOG_WINDOW_POS, 0x0ba3)
#endif  // PACKETVER_MAIN_NUM >= 20220504

#if PACKETVER_MAIN_NUM >= 20220504
struct PACKET_ZC_DIALOG_WINDOW_POS2 {
	int16 PacketType;
	int x;
	int y;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_DIALOG_WINDOW_POS2, 0x0bb5)
#endif  // PACKETVER_MAIN_NUM >= 20220504

#if PACKETVER_MAIN_NUM >= 20220504
struct PACKET_ZC_PLAY_NPC_BGM {
	int16 PacketType;
	int16 PacketLength;
	uint8 playType;
	char bgm[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PLAY_NPC_BGM, 0x0b8c)
#elif PACKETVER >= 20091201
struct PACKET_ZC_PLAY_NPC_BGM {
	int16 PacketType;
	char bgm[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_PLAY_NPC_BGM, 0x07fe)
#endif  // PACKETVER >= 20091201

struct PACKET_CZ_MOVE_ITEM_FROM_BODY_TO_CART {
	int16 PacketType;
	int16 index;
	int count;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_MOVE_ITEM_FROM_BODY_TO_CART, 0x0126)

struct PACKET_ZC_SOUND {
	int16 PacketType;
	char name[NAME_LENGTH];
	uint8 act;
	uint32 term;
	uint32 AID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SOUND, 0x01d3)

#if PACKETVER >= 20100420
struct PACKET_ZC_BUYING_STORE_ENTRY {
	int16 packetType;
	uint32 makerAID;
	char storeName[MESSAGE_SIZE];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BUYING_STORE_ENTRY, 0x0814);
#endif

struct PACKET_ZC_STORE_ENTRY {
	int16 packetType;
	uint32 makerAID;
	char storeName[MESSAGE_SIZE];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_STORE_ENTRY, 0x0131);

struct CZ_PURCHASE_ITEM_FROMMC {
	int16 count;
	int16 index;
} __attribute__((packed));

struct PACKET_CZ_PC_PURCHASE_ITEMLIST_FROMMC {
	int16 packetType;
	int16 packetLength;
	uint32 AID;
	struct CZ_PURCHASE_ITEM_FROMMC list[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_PC_PURCHASE_ITEMLIST_FROMMC, 0x0134);

struct PACKET_CZ_PC_PURCHASE_ITEMLIST_FROMMC2 {
	int16 packetType;
	int16 packetLength;
	uint32 AID;
	uint32 UniqueID;
	struct CZ_PURCHASE_ITEM_FROMMC list[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_PC_PURCHASE_ITEMLIST_FROMMC2, 0x0801);

#if PACKETVER >= 20100309
struct PACKET_ZC_DISAPPEAR_BUYING_STORE_ENTRY {
	int16 packetType;
	uint32 makerAID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_DISAPPEAR_BUYING_STORE_ENTRY, 0x0816);
#endif

#if PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_OPEN_REFORM_UI {
	int16 PacketType;
	int32 ITID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_OPEN_REFORM_UI, 0x0b8f);
#endif  // PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024

#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_CZ_CLOSE_REFORM_UI {
	int16 PacketType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_CLOSE_REFORM_UI, 0x0b90);
#endif  // PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024

#if PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_CZ_ITEM_REFORM {
	int16 PacketType;
	int32 ITID;
	int16 index;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_ITEM_REFORM, 0x0b91);
#endif  // PACKETVER_MAIN_NUM >= 20200916 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024

#if PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_ITEM_REFORM_ACK {
	int16 PacketType;
	int16 index;
	int8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ITEM_REFORM_ACK, 0x0b92);
#endif  // PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024

#if PACKETVER_MAIN_NUM >= 20220216 || PACKETVER_ZERO_NUM >= 20220316
struct PACKET_CZ_USE_PACKAGEITEM {
	int16 PacketType;
	uint16 index;
	uint32 AID;
	uint32 itemID;
	uint32 BoxIndex;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_USE_PACKAGEITEM, 0x0baf)
#endif  // PACKETVER_MAIN_NUM >= 20220216 || PACKETVER_ZERO_NUM >= 20220316

#if PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_CZ_REQUEST_RANDOM_ENCHANT {
	int16 PacketType;
	int64 enchant_group;
	int16 index;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQUEST_RANDOM_ENCHANT, 0x0b9b);
#endif // PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024

#if PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_CZ_REQUEST_PERFECT_ENCHANT {
	int16 PacketType;
	int64 enchant_group;
	int16 index;
	uint32 ITID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQUEST_PERFECT_ENCHANT, 0x0b9c);
#endif // PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024

#if PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_CZ_REQUEST_UPGRADE_ENCHANT {
	int16 PacketType;
	int64 enchant_group;
	int16 index;
	int16 slot;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQUEST_UPGRADE_ENCHANT, 0x0b9d);
#endif // PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024

#if PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_CZ_REQUEST_RESET_ENCHANT {
	int16 PacketType;
	int64 enchant_group;
	int16 index;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQUEST_RESET_ENCHANT, 0x0b9e);
#endif // PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024

#if PACKETVER_MAIN_NUM >= 20210203 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_ZC_RESPONSE_ENCHANT {
	int16 PacketType;
	int32 msgId;
	uint32 ITID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_RESPONSE_ENCHANT, 0x0b9f);
#endif // PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024

#if PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024
struct PACKET_CZ_CLOSE_UI_ENCHANT {
	int16 PacketType;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_CLOSE_UI_ENCHANT, 0x0ba0);
#endif // PACKETVER_MAIN_NUM >= 20201118 || PACKETVER_RE_NUM >= 20211103 || PACKETVER_ZERO_NUM >= 20221024

#if PACKETVER_MAIN_NUM >= 20221005
struct PACKET_ZC_SPECIALPOPUP {
	int16 PacketType;
	int32 ppId;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_SPECIALPOPUP, 0x0bbe);
#endif  // PACKETVER_MAIN_NUM >= 20221005

#if PACKETVER >= 20140611
struct PACKET_ZC_GOLDPCCAFE_POINT {
	// Note: 2014-04-30 has 1 byte less, but those packets are only functional after 2014-06-11Ragexe
	uint16 PacketType;
	int8 isActive; //< 1 = yes, 0 = no
	int8 mode;
	int32 point;
	int32 playedTime;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_GOLDPCCAFE_POINT , 0x0a15);
#endif // PACKETVER >= 20140611

#if PACKETVER >= 20140430
struct PACKET_CZ_DYNAMICNPC_CREATE_REQUEST {
	uint16 PacketType;
	char name[NAME_LENGTH];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_DYNAMICNPC_CREATE_REQUEST, 0x0a16);
#endif // PACKETVER >= 20140430

#if PACKETVER_MAIN_NUM >= 20140430 || PACKETVER_RE_NUM >= 20140430 || defined(PACKETVER_ZERO)
struct PACKET_ZC_DYNAMICNPC_CREATE_RESULT {
	uint16 PacketType;
	uint32 result; // enum dynamicnpc_create_result
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_DYNAMICNPC_CREATE_RESULT , 0x0a17);
#endif // PACKETVER_MAIN_NUM >= 20140430 || PACKETVER_RE_NUM >= 20140430 || defined(PACKETVER_ZERO)

struct PACKET_CZ_REQ_GUILD_EMBLEM_IMG1 {
	int16 packetType;
	int32 guild_id;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_GUILD_EMBLEM_IMG1, 0x0151);

#if PACKETVER >= 20190724
struct PACKET_CZ_REQ_GUILD_EMBLEM_IMG3 {
	int16 packetType;
	int32 guild_id;
	int32 emblem_id;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_GUILD_EMBLEM_IMG3, 0x0b46);
#endif  // PACKETVER >= 20190724

#if PACKETVER_MAIN_NUM >= 20190619 || PACKETVER_RE_NUM >= 20190605 || PACKETVER_ZERO_NUM >= 20190626
struct PACKET_CZ_REQ_GUILD_EMBLEM_IMG2 {
	int16 packetType;
	int32 guild_id;
	int32 emblem_id;
	int32 unused;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_GUILD_EMBLEM_IMG2, 0x0b1e);
#elif PACKETVER_MAIN_NUM >= 20190227 || PACKETVER_RE_NUM >= 20190227 || PACKETVER_ZERO_NUM >= 20190313
struct PACKET_CZ_REQ_GUILD_EMBLEM_IMG2 {
	int16 packetType;
	int32 guild_id;
	int32 emblem_id;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_REQ_GUILD_EMBLEM_IMG2, 0x0b1e);
#endif  // PACKETVER_MAIN_NUM >= 20190619 || PACKETVER_RE_NUM >= 20190605 || PACKETVER_ZERO_NUM >= 20190626

#if PACKETVER_MAIN_NUM >= 20190807 || PACKETVER_RE_NUM >= 20190731 || PACKETVER_ZERO_NUM >= 20190814
struct PACKET_ZC_CHANGE_GUILD {
	int16 packetType;
	int32 guild_id;
	uint32 emblem_id;
	uint32 AID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_CHANGE_GUILD, 0x0b47);
// 20190619 main exists in first versions, then removed
// 20190605 re first versions with other packet size
#elif PACKETVER_MAIN_NUM >= 20190703 || PACKETVER_RE_NUM >= 20190605 || PACKETVER_ZERO_NUM >= 20190709
struct PACKET_ZC_CHANGE_GUILD {
	int16 packetType;
	int32 guild_id;
	uint32 emblem_id;
	uint32 AID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_CHANGE_GUILD, 0x0b1f);
#else  // PACKETVER_MAIN_NUM >= 20190807 || PACKETVER_RE_NUM >= 20190731 || PACKETVER_ZERO_NUM >= 20190814
struct PACKET_ZC_CHANGE_GUILD {
	int16 packetType;
	uint32 AID;
	int32 guild_id;
	uint16 emblem_id;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_CHANGE_GUILD, 0x01b4);
#endif  // PACKETVER_MAIN_NUM >= 20190807 || PACKETVER_RE_NUM >= 20190731 || PACKETVER_ZERO_NUM >= 20190814

#if PACKETVER_MAIN_NUM >= 20190821 || PACKETVER_RE_NUM >= 20190807 || PACKETVER_ZERO_NUM >= 20190710
enum ZC_GUILD_EMBLEM_TYPE {
	ZC_GUILD_EMBLEM_TYPE_CLEAR = 0,
	ZC_GUILD_EMBLEM_TYPE_ADD = 1,
	ZC_GUILD_EMBLEM_TYPE_COMPLETE = 2,
};

struct PACKET_ZC_GUILD_EMBLEM_IMG {
	int16 packetType;
	int16 packetLength;
	uint16 result;
	int32 guild_id;
	uint32 emblem_id;
	char emblem_data[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_GUILD_EMBLEM_IMG, 0x0b36);
#else  // PACKETVER_MAIN_NUM >= 20190821 || PACKETVER_RE_NUM >= 20190807 || PACKETVER_ZERO_NUM >= 20190710
struct PACKET_ZC_GUILD_EMBLEM_IMG {
	int16 packetType;
	int16 packetLength;
	int32 guild_id;
	uint32 emblem_id;
	char emblem_data[];
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_GUILD_EMBLEM_IMG, 0x0152);
#endif  // PACKETVER_MAIN_NUM >= 20190821 || PACKETVER_RE_NUM >= 20190807 || PACKETVER_ZERO_NUM >= 20190710

#if PACKETVER_MAIN_NUM >= 20171213 || PACKETVER_RE_NUM >= 20171213 || PACKETVER_ZERO_NUM >= 20171214
struct PACKET_CZ_ADVENTURER_AGENCY_JOIN_REQ {
	int16 packetType;
	int GID;
	int AID;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_ADVENTURER_AGENCY_JOIN_REQ, 0x0ae6);
#endif  // PACKETVER_MAIN_NUM >= 20171213 || PACKETVER_RE_NUM >= 20171213 || PACKETVER_ZERO_NUM >= 20171214

#if PACKETVER_MAIN_NUM >= 20191218 || PACKETVER_RE_NUM >= 20191211 || PACKETVER_ZERO_NUM >= 20191224
struct PACKET_ZC_ADVENTURER_AGENCY_JOIN_RESULT {
	int16 packetType;
	char player_name[NAME_LENGTH];
	char party_name[NAME_LENGTH];
	int AID;
	int result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ADVENTURER_AGENCY_JOIN_RESULT, 0x0afa);
#endif  // PACKETVER_MAIN_NUM >= 20191218 || PACKETVER_RE_NUM >= 20191211 || PACKETVER_ZERO_NUM >= 20191224

#if PACKETVER_MAIN_NUM >= 20191218 || PACKETVER_RE_NUM >= 20191211 || PACKETVER_ZERO_NUM >= 20191224
struct PACKET_ZC_ADVENTURER_AGENCY_JOIN_REQ {
	int16 packetType;
	int GRID;
	int AID;
	char groupName[NAME_LENGTH];
	int16 level;
	int16 job;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_ADVENTURER_AGENCY_JOIN_REQ, 0x0ae7);
#endif  // PACKETVER_MAIN_NUM >= 20191218 || PACKETVER_RE_NUM >= 20191211 || PACKETVER_ZERO_NUM >= 20191224

#if PACKETVER_MAIN_NUM >= 20191218 || PACKETVER_RE_NUM >= 20191211 || PACKETVER_ZERO_NUM >= 20191224
struct PACKET_CZ_ADVENTURER_AGENCY_JOIN_RESULT {
	int16 packetType;
	int GRID;
	int AID;
	int8 result;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_ADVENTURER_AGENCY_JOIN_RESULT, 0x0af8);
#endif  // PACKETVER_MAIN_NUM >= 20191218 || PACKETVER_RE_NUM >= 20191211 || PACKETVER_ZERO_NUM >= 20191224

// [Stingor] Bourgeon DLL <-> server custom packets.
// ZONE 0x0F00..0x0FFF : au-dessus de l'opcode max du client (0x0C35) => vus par le
// client comme flag=-1 (variable, inconnu), immunisés contre les montées de version
// du client ET contre les collisions de longueur fixe. Côté serveur, MAX_PACKET_DB a
// été remonté à 0xFFF (clif.hpp) pour permettre l'enregistrement des CZ.
// !! Doit rester synchronisé avec Bourgeon/src/plugins/bourgeon_opcodes.h (client).
// Carte complète : Bourgeon/docs/opcode_map.md
//
// ZC (server -> client): sends the player's char_id + all settings on login.
// Variable-length.
// Layout: [packetType:2][packetLength:2][char_id:4][count:2][{id:2, value:2} * count]
struct PACKET_ZC_BOURGEON_SETTINGS {
	int16 packetType;
	int16 packetLength;
	uint32 char_id;
	int16 count;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_SETTINGS, 0x0f05);  // ex-0x0bfe

// CZ (client -> server): reports a single setting change. Fixed 10 bytes.
// Layout: [packetType:2][packetLength:2][id:2][value:4]
struct PACKET_CZ_BOURGEON_SETTING {
	int16 packetType;
	int16 packetLength;
	int16 id;
	uint32 value;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BOURGEON_SETTING, 0x0f04);  // ex-0x0bfd

// CZ (client -> server): the Bourgeon DLL reports a SHA-256 of its own ddraw.dll,
// the Windows MachineGuid (for multi-account detection) and the patch level read
// from rpatchur's cache file. Fixed 76 bytes.
// Layout: [packetType:2][packetLength:2][sha256:32][machine_guid:36][patch_index:4]
//
// The hash and the patch level are versioned independently: an approved DLL says
// nothing about whether the player's GRF/loose content is current, since rpatchur
// ships them as separate patches. Hence the extra field.
struct PACKET_CZ_BOURGEON_INTEGRITY {
	int16 packetType;
	int16 packetLength;
	uint8 hash[32];
	char  machine_guid[36];  // registry MachineGuid, NOT null-terminated
	int32 patch_index;       // rpatchur last_patch_index; -1 = never patched
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BOURGEON_INTEGRITY, 0x0f02);  // ex-0x0bfb

// ZC (server -> client): tells the Bourgeon overlay the client is outdated
// before the server kicks it. The overlay shows a popup then the kick fires
// ~5 seconds later. Fixed 4 bytes. Layout: [packetType:2][packetLength:2]
struct PACKET_ZC_BOURGEON_KICK_NOTICE {
	int16 packetType;
	int16 packetLength;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_KICK_NOTICE, 0x0f03);  // ex-0x0bfa

// ZC (server -> client): relays a Discord chat message to the Bourgeon overlay.
// Variable-length. Layout: [packetType:2][packetLength:2][msg:variable, null-terminated]
// The overlay shows the message if the player's relay checkbox is ON, ignores it if OFF.
struct PACKET_ZC_BOURGEON_DISCORD_MSG {
	int16 packetType;
	int16 packetLength;
	char msg[1];  // variable-length, null-terminated; pre-formatted as "[Discord][name] text"
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_DISCORD_MSG, 0x0f08);  // ex-0x0c1f

// CZ (client -> server): preset management command. Variable-length.
// Layout: [type:2][len:2][cmd:1][no:1][name:variable, may be empty, not null-terminated]
// cmd: 1=LIST, 2=SAVE(no,name), 3=LOAD(no), 4=DELETE(no), 5=SET_AUTOLOAD(no, 0=disable all)
struct PACKET_CZ_BOURGEON_PRESET_CMD {
	int16 packetType;
	int16 packetLength;
	uint8 cmd;
	uint8 no;
	char  name[1];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BOURGEON_PRESET_CMD, 0x0f06);  // ex-0x0c20

// ZC (server -> client): sends the preset list. Variable-length.
// Layout: [type:2][len:2][active_no:1][count:1]
// Followed by count entries: [no:1][autoload:1][namelen:1][name:namelen bytes, not null-terminated]
struct PACKET_ZC_BOURGEON_PRESET_LIST {
	int16 packetType;
	int16 packetLength;
	uint8 active_no;
	uint8 count;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_PRESET_LIST, 0x0f07);  // ex-0x0c21

// ZC (server -> client): private damage notification for Bourgeon DPS meter.
// Sent SELF-only when a skill unit (Storm Gust, Meteor Storm, LoV…) deals
// damage, so the client can attribute the hit to the original caster's AID
// without changing the visual ZC_NOTIFY_SKILL packet.
// Layout: [type:2][len:2][src_aid:4][damage:4]  — 12 bytes total.
// Depuis le passage à 0x0F09 (au-dessus de 0x0C35), l'opcode est flag=-1 côté
// client = VARIABLE (longueur lue du flux). L'ancienne contrainte "doit rester
// 12 octets" (0x0C22 était fixe-12 dans le client, ce qui gelait le jeu si on
// ajoutait un champ) NE S'APPLIQUE PLUS : ce paquet peut désormais être étendu
// (ex. ajouter skill_id) sans désync, tant que packetLength est correct.
struct PACKET_ZC_BOURGEON_SKILL_DMG {
	int16  packetType;
	int16  packetLength;
	uint32 src_aid;
	int32  damage;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_SKILL_DMG, 0x0f09);  // ex-0x0c22

// CZ (client -> server): cheat detection report. Fixed 100 bytes.
// Layout: [packetType:2][packetLength:2][tool_name:32][detail:64]
// Sent once per new detection; only accepted from has_bourgeon sessions.
struct PACKET_CZ_BOURGEON_CHEAT_REPORT {
	int16 packetType;
	int16 packetLength;
	char tool_name[32];
	char detail[64];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BOURGEON_CHEAT_REPORT, 0x0f0a);  // ex-0x0c23

// CZ (client -> server): request technical data for an item or a skill. Fixed 10.
// Layout: [packetType:2][packetLength:2][id:4][is_skill:1][scope:1]
//   scope (item only): 0 = normal drops, 1 = MVP rewards (two separate on-demand
//   buttons client-side, so the expensive mvpitem scan runs only when asked).
struct PACKET_CZ_BOURGEON_REQ_TECHDATA {
	int16  packetType;
	int16  packetLength;
	uint32 id;
	uint8  is_skill;
	uint8  scope;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BOURGEON_REQ_TECHDATA, 0x0f0b);

// ZC (server -> client): technical-data response for the enriched description.
// Variable-length. Header: [packetType:2][packetLength:2][id:4][is_skill:1][scope:1] then:
//   is_skill=0 (item drops) : [count:1][truncated:1][treasure_excluded:1] then `count` entries:
//     [mob_id:4][rate:4 (1/100 %, VIP+level-adjusted like @whodrops)]
//     [boss:1 (0 normal / 1 mini-boss / 2 MVP)]
//     [src:1 (0 = normal drop / 1 = MVP reward)][namelen:1][name:namelen]
//   is_skill=1 (skill cast)  : [max_lv:1] then `max_lv` entries:
//     [cast_var:4][cast_fixed:4][cooldown:4][after_delay:4]  (all ms)
// scope echoes the request so the client caches drops/MVP separately.
// The variable payload is appended manually with WFIFO; packetLength is set last.
struct PACKET_ZC_BOURGEON_TECHDATA {
	int16  packetType;
	int16  packetLength;
	uint32 id;
	uint8  is_skill;
	uint8  scope;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_TECHDATA, 0x0f0c);

// CZ (client -> server): estimate a skill's damage. Fixed 14.
// Layout: [packetType:2][packetLength:2][skill_id:4][skill_lv:2][target_mob_id:4]
//   skill_lv 0 = utiliser le niveau appris par le joueur (pc_checkskill).
//   target_mob_id 0 = dummy neutre 0-def (dégâts bruts) ; sinon = vrai monstre
//   (ses vrais def/mdef/élément/race/taille -> dégâts réels contre lui).
struct PACKET_CZ_BOURGEON_REQ_DAMAGE {
	int16  packetType;
	int16  packetLength;
	uint32 skill_id;
	uint16 skill_lv;
	uint32 target_mob_id;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BOURGEON_REQ_DAMAGE, 0x0f0d);

// ZC (server -> client): estimated skill damage. VARIABLE length : les champs
// fixes ci-dessous, PUIS [namelen:1][name] (nom du monstre, vide pour dummy/soi).
// target_mob_id renvoyé en écho (0 = neutre, 0xFFFFFFFF = soi-même, sinon mob).
// status: 0 = ok, 1 = sort non offensif, 2 = erreur. atk_type = BF_WEAPON/MAGIC/MISC.
struct PACKET_ZC_BOURGEON_DAMAGE {
	int16  packetType;
	int16  packetLength;
	uint32 skill_id;
	uint16 skill_lv;
	uint32 target_mob_id;
	uint8  status;
	uint8  atk_type;
	uint16 hits;      // div_ (nombre de coups)
	int64  dmg_min;
	int64  dmg_max;
	int64  dmg_avg;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_DAMAGE, 0x0f0e);

// ZC (server -> client): prix de vente NPC des items du storage. VARIABLE.
// Layout: [packetType:2][packetLength:2][count:2] puis count * [id:4][sell:4].
// Envoyé juste après clif_storagelist (perso/guilde/premium) ; le viewer Bourgeon
// calcule la valeur totale (sell * quantité) + affiche une colonne prix.
// Dédupliqué par nameid côté serveur (le prix de vente est par-id).
struct PACKET_ZC_BOURGEON_STORAGE_PRICES {
	int16 packetType;
	int16 packetLength;
	int16 count;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_STORAGE_PRICES, 0x0f0f);

// CZ (client -> server): sertissage rapide — demande la liste des cartes de
// l'inventaire compatibles avec un équipement donné (menu contextuel du viewer).
// index_equip = index d'inventaire CLIENT de l'équipement (server_index() côté serveur).
// Fixe 6.
struct PACKET_CZ_BOURGEON_REQ_COMPAT_CARDS {
	int16  packetType;
	int16  packetLength;
	uint16 index_equip;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BOURGEON_REQ_COMPAT_CARDS, 0x0f18);

// ZC (server -> client): réponse au sertissage rapide. VARIABLE.
// Layout: [packetType:2][packetLength:2][index_equip:2][count:2] puis
//   count * [index_card:2] (index d'inventaire CLIENT de chaque carte compatible).
// index_equip renvoyé en écho pour que le client valide qu'il s'agit bien de la
// requête en cours. La compatibilité est calculée par pc_can_insert_card (prédicat
// EXACT du sertissage) -> aucun faux positif : chaque carte listée sera acceptée.
struct PACKET_ZC_BOURGEON_COMPAT_CARDS {
	int16  packetType;
	int16  packetLength;
	uint16 index_equip;
	int16  count;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_COMPAT_CARDS, 0x0f19);

// CZ (client -> server): "je viens de sauter" (barre espace). AUCUN payload — le
// serveur connaît l'émetteur par sa session. Fixe 4.
// Le saut est purement ESTHÉTIQUE : le client décale le sprite en hauteur, la
// position logique du personnage ne bouge pas (ni case, ni portée, ni collision).
// Le serveur ne fait donc que RELAYER, sous cooldown anti-flood (cf.
// clif_parse_bourgeon_jump) — même traitement que les emotes, pour la même raison.
struct PACKET_CZ_BOURGEON_JUMP {
	int16 packetType;
	int16 packetLength;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BOURGEON_JUMP, 0x0f1a);

// ZC (server -> client) AREA SANS SELF: le joueur `gid` vient de sauter, joue
// l'animation sur son sprite. Fixe 8. Envoyé UNIQUEMENT aux sessions
// has_bourgeon : un client vanilla qui reçoit un opcode > 0x0C35 vide son buffer
// de réception (RecvBuffer_ResetAll_OnUnknownOpcode) et perdrait les paquets
// suivants du même flush — desync réel, pas cosmétique.
// Sans self : le sauteur s'anime déjà localement au moment de l'appui.
struct PACKET_ZC_BOURGEON_JUMP {
	int16  packetType;
	int16  packetLength;
	uint32 gid;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_JUMP, 0x0f1b);

// ZC (server -> client) SELF: maîtrise culinaire (`COOK_MASTERY`, plage [0,1999]).
//
// Pourquoi un paquet pour UNE valeur : c'est le seul terme de la formule de réussite
// de la cuisine que le client ne peut pas connaître. Tous les autres lui sont
// accessibles (niveau de base, DEX, LUK, itemlv de la recette via le YAML de recettes,
// niveau du kit via l'objet consommé). Sans elle, l'estimation affichable est une
// fourchette de ~22 points de pourcentage — inutilisable :
//   make_per += 100 * (rnd()%(30 + 5*(cm/400) - lo) + lo),  lo = 6 + cm/80
// soit [600,2900] à maîtrise nulle et [3000,4900] à maîtrise pleine (skill.cpp,
// branche `default:` de skill_produce_mix, « Assume Cooking Dish »).
//
// Et elle vaut d'être montrée pour elle-même : la maîtrise monte à chaque plat réussi
// et REDESCEND à chaque échec (skill.cpp, pc_setparam SP_COOKMASTERY), sans que le jeu
// ne l'affiche NULLE PART. Un joueur ne peut aujourd'hui ni la connaître ni la suivre.
//
// Poussée au login vérifié (clif_bourgeon_grant_verified) et à chaque changement de
// valeur (pc_setparam). Fixe 6.
struct PACKET_ZC_BOURGEON_COOK_MASTERY {
	int16 packetType;
	int16 packetLength;
	int16 mastery;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_COOK_MASTERY, 0x0f1c);

// CZ (client -> server): ouvrir un storage, ou BASCULER vers un autre depuis les
// onglets du viewer Bourgeon. Fixe 5.
// Layout: [packetType:2][packetLength:2][stor_id:1]
//   stor_id : 0 = storage principal, 1..N = alternatif (conf/inter_server.yml).
//
// Ce que ce paquet apporte sur les @storagealt existants (src/custom/atcommand.inc) :
// ceux-ci FERMENT quand un storage est déjà ouvert (toggle) — basculer coûte donc
// deux commandes. Le handler ici enchaîne fermeture + ouverture, ce qu'un onglet
// doit faire en un clic. Les DROITS restent exactement ceux des commandes
// (pc_can_use_command sur "storage" / "storagealtN") : un onglet ne peut rien
// ouvrir que le joueur ne puisse déjà taper.
struct PACKET_CZ_BOURGEON_OPEN_STORAGE {
	int16 packetType;
	int16 packetLength;
	uint8 stor_id;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BOURGEON_OPEN_STORAGE, 0x0f1d);

// ZC (server -> client): storages accessibles à CE personnage + celui qui est ouvert.
// VARIABLE : [packetType:2][packetLength:2][cur_id:1][count:1]
//            puis count fois [stor_id:1][name:NAME_LENGTH]
//   cur_id : id du storage ouvert, 0xFF si aucun (liste poussée au login).
//   name   : nom du storage (inter_server.yml), NUL-paddé sur NAME_LENGTH.
// La liste est FILTRÉE par les droits du joueur — le client dessine ce qu'il
// reçoit, il ne connaît aucun nom ni aucun id à l'avance.
struct PACKET_ZC_BOURGEON_STORAGE_LIST {
	int16 packetType;
	int16 packetLength;
	uint8 cur_id;
	uint8 count;
	// suivi de count * [stor_id:1][name:NAME_LENGTH]
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_STORAGE_LIST, 0x0f1e);

// CZ (client -> server): fiche détaillée d'un monstre. Fixe 9.
// Layout: [packetType:2][packetLength:2][mob_id:4][by_view:1]
//   by_view = 1 quand l'id vient de ZC_MONSTER_INFO (skill Sense) : le client y
//   reçoit la classe de VUE (md->vd->look[LOOK_BASE]), pas l'id de mob_db — un
//   monstre déguisé (ViewClass) porte alors l'id d'un AUTRE monstre. Le serveur
//   fait la correspondance inverse, le client n'a pas mob_db.
//   by_view = 0 quand l'id vient déjà d'une source base-de-données (lien depuis
//   la table des drops d'un item, par exemple).
//   Cf. Bourgeon/docs/monster_info_re.md §3.1 et §7.2.
struct PACKET_CZ_BOURGEON_REQ_MOBINFO {
	int16  packetType;
	int16  packetLength;
	uint32 mob_id;
	uint8  by_view;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BOURGEON_REQ_MOBINFO, 0x0f1f);

// ZC (server -> client): fiche détaillée d'un monstre. VARIABLE.
//
// C'est le pendant « monstre » de ZC_BOURGEON_TECHDATA (fiche d'item) : tout ce
// que ZC_MONSTER_INFO (0x018C, skill Sense) NE transporte PAS — EXP, stats de
// base, ATK/MATK, modes, vitesse, drops, cartes de spawn, skills.
//
// Header fixe: [packetType:2][packetLength:2][mob_id:4][status:1]
//   status : 0 = ok, 1 = monstre inconnu (rien ne suit).
// Puis, si status == 0 :
//   [sprite_class:4]   classe de VUE -> c'est CELLE-CI qui charge le .spr/.act
//   [level:2][hp:4][sp:4]
//   [base_exp:4][job_exp:4][mvp_exp:4]
//   [atk_min:2][atk_max:2][matk_min:2][matk_max:2]
//   [def:2][def2:2][mdef:2][mdef2:2]
//   [str:2][agi:2][vit:2][int_:2][dex:2][luk:2]
//   [attack_range:2]
//   [size:1][race:1][element:1][element_lv:1][boss:1][class:1]
//
// ⚠ HIT / FLEE / CRIT ne sont VOLONTAIREMENT pas envoyés : mob_db ne les porte
// pas. Ce sont des dérivés que status_calc_misc() ne calcule qu'au SPAWN, sur
// un block_list ; l'entrée de mob_db les laisse à zéro. Les envoyer serait
// afficher des zéros crédibles — pire qu'une absence.
// Le CLIENT les recalcule à partir des champs ci-dessus, ce qui ne coûte rien :
// en pré-renewal (status.cpp, branche #else) HIT = niveau + DEX et
// FLEE = niveau + AGI, et le critique d'un monstre vaut TOUJOURS zéro puisque
// battle_config.enable_critical (= 17 = BL_PC|BL_MER) exclut BL_MOB — comme
// enable_perfect_flee (= 1 = BL_PC) exclut son esquive parfaite.
//
// ⚠ Retirés du paquet (et pas seulement de l'affichage) : les portées de VUE et
// de POURSUITE, le délai d'attaque et les deux durées d'animation. Ce sont des
// détails de moteur qui n'aidaient personne à décider s'il faut attaquer ou
// fuir. Le bloc fixe passe de 98 à 88 octets.
//   [mode:4]                            bitfield MD_* (agressif, assist, loot…)
//   [speed:2]                           temps de marche d'UNE case, en ms
//                                       (le client l'affiche en cases/seconde)
//   [resist:2 * 10]                     % encaissé par élément d'ATTAQUE 0..9,
//                                       SIGNÉ (le client peut afficher < 0, ce
//                                       que 0x018C ne permet pas : il borne à 0)
//   [namelen:1][name:namelen]           jname (nom affiché)
//   [drop_count:1] puis drop_count fois :
//       [nameid:4][rate:4][kind:1][namelen:1][name:namelen]
//       kind : 0 = drop normal, 1 = récompense MVP ; rate en 1/100 %
//   [spawn_count:1] puis spawn_count fois :
//       [qty:2][maplen:1][map:maplen]   nom d'index de carte (ex. « prt_fild08 »)
//   [skill_count:1] puis skill_count fois :
//       [skill_id:2][skill_lv:2][namelen:1][name:namelen]
//       name = skill_db `desc` (« Emotion »), à défaut l'AegisName
//       (« NPC_EMOTION »). 🔴 Il PART du serveur parce que le client ne sait
//       nommer que les compétences de JOUEUR : son wrapper Lua rend
//       « Unknown-Skill » sur toutes les `NPC_*`, soit l'essentiel de
//       l'arsenal d'un monstre. Le client préfère son propre nom quand il en
//       a un (localisé) et retombe sur celui-ci sinon.
//       Dédupliqué sur (id, niveau) : mob_skill_db porte une LIGNE par état
//       d'IA et par condition, pas une par compétence.
//   [aegislen:1][aegis:aegislen]        AegisName, l'identité UNIQUE du monstre
//   [summoned:1][namesake_count:1][namesake_ref:4]
//   [est_base_exp:4][est_job_exp:4]     ce que CE monstre rapporte à CE joueur
//   [next_base_exp:4][next_job_exp:4]   les deux paliers du niveau suivant
//   [exp_flags:1]                       bit0 = niveau de base MAX, bit1 = job MAX
//
// ⚠ Le bloc d'EXP estimée est le SEUL contenu de ce paquet qui dépende du joueur
// qui regarde : mob_estimate_exp_gain y applique le malus de haut niveau et les
// bonus personnels (bExpAddRace / bExpAddClass, Battle Manual, VIP, event EXP),
// exactement comme le fait la warp agent en script via getmonsterexprate. C'est
// un INSTANTANÉ — il vieillit dès que le joueur change de niveau ou de buff, et
// c'est au client de redemander la fiche à ce moment-là.
// Les paliers partent avec pour que le client exprime le gain en pour cent de la
// barre sans dépendre de globales qu'il devrait tenir à jour lui-même. Au niveau
// maximum, `next_*` vaut le plafond de STOCKAGE de l'EXP et non un palier à
// franchir : d'où `exp_flags`, qui le dit plutôt que de laisser deviner.
//
// Les trois listes sont bornées (uint8 de comptage) ; le serveur tronque et le
// client le signale. Tout est envoyé à la volée en WFIFO, packetLength en dernier.
struct PACKET_ZC_BOURGEON_MOBINFO {
	int16  packetType;
	int16  packetLength;
	uint32 mob_id;
	uint8  status;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_MOBINFO, 0x0f20);

// ZC (server -> client): canaux de chat visibles par CE personnage.
// VARIABLE : [packetType:2][packetLength:2][count:1]
//            puis count fois [flags:1][color:4][name:CHAN_NAME_LENGTH][alias:CHAN_NAME_LENGTH]
//   flags : bit0 = le canal n'existe que si le joueur est en guilde (#ally),
//           bit1 = le joueur a le droit d'y écrire (CHAN_OPT_CAN_CHAT).
//   color : couleur du canal telle que le serveur la STOCKE, c'est-à-dire en
//           BGR (channel.cpp, read_config : « RGB to BGR » à la lecture de
//           channels.conf). Le client refait la conversion, il ne devine rien.
//   name  : SANS le '#' — c'est ainsi que `struct Channel` le range, et
//           `channel_name2channel` compare toujours sur `chname + 1`.
//   alias : libellé d'affichage (« [Global] »), tel qu'écrit dans channels.conf.
//
// Pourquoi ce paquet. Le client Bourgeon offre les canaux du serveur dans la
// combo de la barre de chat ; il n'a aucun moyen de les CONNAÎTRE — ils vivent
// dans conf/channels.conf, filtrés par groupid, et la seule façon de les lire
// en jeu est de taper « @channel list » et d'en analyser le texte, qui est
// localisé (msg_txt 1409/1410) donc instable. Même raison, même patron que
// ZC_BOURGEON_STORAGE_LIST : le serveur possède la liste et ses droits, le
// client dessine ce qu'il reçoit.
//
// La liste part au login. Elle n'a pas à être renvoyée ensuite : le nom des
// canaux de MAP et d'ALLIANCE est celui du gabarit (`#map`, `#ally`), le même
// sur toutes les cartes et pour toutes les guildes — seule leur EXISTENCE varie,
// et pour l'alliance c'est le bit0 qui le dit, le client sachant déjà s'il est en
// guilde.
struct PACKET_ZC_BOURGEON_CHANNEL_LIST {
	int16 packetType;
	int16 packetLength;
	uint8 count;
	// suivi de count * [flags:1][color:4][name:CHAN_NAME_LENGTH][alias:CHAN_NAME_LENGTH]
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_CHANNEL_LIST, 0x0f21);

// CZ (client -> server): propriétés SERVEUR d'une entité du monde. Fixe 8.
// Layout: [packetType:2][packetLength:2][gid:4]
//
// `gid` est l'identifiant que le CLIENT voit — celui de son quad de picking.
// Pour un joueur c'est l'account_id, pour tout le reste l'id d'objet : dans les
// deux cas exactement la clé de map_id2bl(), donc rien à convertir.
//
// 🔴 RÉSERVÉ AU STAFF. La réponse décrit des choses qu'un joueur n'a pas à
// connaître (fichier de script d'un NPC, id de spawn d'un mob, char_id d'un
// autre joueur). Le gate est SERVEUR — niveau de groupe >= 80 — et pas
// seulement l'absence de bouton côté client.
struct PACKET_CZ_BOURGEON_REQ_ENTITY_PROPS {
	int16  packetType;
	int16  packetLength;
	uint32 gid;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BOURGEON_REQ_ENTITY_PROPS, 0x0f22);

// ZC (server -> client): propriétés SERVEUR d'une entité. VARIABLE.
//
// Header fixe: [packetType:2][packetLength:2][gid:4][status:1][count:1]
//   status : 0 = ok, 1 = aucune entité pour ce gid, 2 = refusé (pas staff).
// Puis count fois : [key_len:1][key:N][val_len:1][val:M]
//
// ⚠ Le type d'entité ne voyage PAS en champ binaire : `bl_type` est un MASQUE
// (BL_CHAT = 0x100, BL_ELEM = 0x200) qui ne tient pas dans un octet, et un
// second champ que personne ne lit ne vaut pas une largeur de plus. Le type part
// en clair, comme première propriété de la section « Serveur » — c'est de toute
// façon sous cette forme qu'il se lit.
//
// 🔴 Le corps est une LISTE CLÉ/VALEUR, pas une structure. Ce n'est pas de la
// paresse : un inspecteur n'a pas de schéma, il décrit ce qu'il trouve, et ce
// qu'il trouve dépend du type de l'entité — un NPC de warp, un mob invoqué et
// une unité de compétence n'ont pas trois champs en commun. Une structure rigide
// aurait imposé un champ « libre » de toute façon, et surtout : ajouter une
// propriété deviendrait une rupture de protocole à déployer des deux côtés,
// alors qu'ici le serveur seul suffit.
//
// Convention : une paire dont la VALEUR est vide est un TITRE DE SECTION. Le
// client la dessine en séparateur. C'est ce qui remplace un découpage en groupes
// dans le paquet lui-même.
//
// Les clés et les valeurs sont écrites par le SERVEUR et affichées telles
// quelles : elles ne passent par aucun catalogue de traduction, comme les
// messages serveur relayés par le chat.
struct PACKET_ZC_BOURGEON_ENTITY_PROPS {
	int16 packetType;
	int16 packetLength;
	uint32 gid;
	uint8 status;
	uint8 count;
	// suivi de count * [key_len:1][key][val_len:1][val]
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_ENTITY_PROPS, 0x0f23);

// CZ (client -> server): ce que l'interface moderne de ce client SAIT AFFICHER.
// Layout: [packetType:2][packetLength:2][caps:4], masque `e_bourgeon_ui_cap`.
//
// 🔴 À quoi ça sert, et pourquoi `has_bourgeon` ne suffit pas. Le handshake
// d'intégrité dit « c'est un client Bourgeon » ; il ne dit pas laquelle de ses
// interfaces est ALLUMÉE — elles sont toutes opt-in, et le joueur peut en
// éteindre une au milieu d'une conversation. Or les balises maison (`<MOBL>`,
// `<CRAF>`, `<IMG>`…) ne sont rendues que par ces interfaces-là : envoyées à un
// joueur resté sur le dialogue natif, elles s'afficheraient en toutes lettres.
// C'est ce masque qui permet à `clif_scriptmes` de dégrader plutôt que de parier.
//
// Déclaré VARIABLE dans clif_packetdb.hpp (longueur lue du flux, handler qui
// vérifie ce qu'il a reçu) : un champ pourra s'ajouter sans que les deux côtés
// aient à être déployés dans la même seconde.
struct PACKET_CZ_BOURGEON_UI_CAPS {
	int16  packetType;
	int16  packetLength;
	uint32 caps;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BOURGEON_UI_CAPS, 0x0f24);

// CZ (client -> server): outillage NPC du menu contextuel (Bourgeon :
// EntityContextMenu, section staff). Layout: [packetType:2][packetLength:2]
// [gid:4][action:1], `action` = `e_bourgeon_npc_admin_action`.
//
// 🔴 On désigne le NPC par son GID, PAS par son nom. `@unloadnpc`, `@npcmove` et
// consorts passent par `npc_name2id`, dont la clé est `exname` — le nom UNIQUE.
// Le client, lui, ne connaît que le nom AFFICHÉ (celui de la plaque, `nd->name`),
// et les deux diffèrent dès qu'il y a un duplicate ou un `#suffixe`. Rejouer une
// commande @ depuis le client aurait donc raté précisément les NPC dupliqués, en
// silence, avec « This NPC doesn't exist ». Le GID, lui, est ce que le client a
// sous le curseur et ce que `map_id2nd` prend directement.
//
// Corollaire : « recharger » n'était même pas exprimable côté client. Ce qui se
// recharge, c'est le FICHIER (`@reloadnpcfile <path>`), et le chemin ne vit que
// dans `nd->path`, côté serveur.
//
// 🔴 Gate SERVEUR : niveau de groupe >= 99. Plus haut que l'inspecteur (>= 80)
// parce qu'ici on n'affiche pas, on MODIFIE l'état du serveur pour tout le monde
// — décharger un NPC le retire à tous les joueurs connectés.
//
// Pas de ZC en retour : le compte rendu part en `clif_displaymessage`, comme
// celui d'un atcommand, et s'affiche dans le chat sans rien de plus à écrire des
// deux côtés.
struct PACKET_CZ_BOURGEON_NPC_ADMIN {
	int16  packetType;
	int16  packetLength;
	uint32 gid;
	uint8  action;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BOURGEON_NPC_ADMIN, 0x0f25);

// Miroir exact des `kNpcAdmin*` côté Bourgeon
// (src/features/windows/entity_context_menu.cc) : les deux listes bougent
// ensemble, et les valeurs ne se réordonnent pas.
enum e_bourgeon_npc_admin_action : uint8 {
	BOURGEON_NPC_ADMIN_RELOAD_FILE = 0,  // recharger le fichier de script d'où il vient
	BOURGEON_NPC_ADMIN_UNLOAD      = 1,  // décharger ce NPC et ses duplicates
	BOURGEON_NPC_ADMIN_MOVE_TO_ME  = 2,  // le poser sur la case du demandeur
};

// ── Outillage JOUEUR du menu contextuel (staff) ──────────────────────────────
//
// CZ (client -> server) : ce que faisait le NPC `#gmclicdroit`, porté dans le
// sous-menu « Outils du staff » du menu contextuel (Bourgeon :
// EntityContextMenu). Layout : [packetType:2][packetLength:2][aid:4][action:1]
// [param:4], `action` = `e_bourgeon_player_admin_action`.
//
// 🔴 Le serveur RÉSOUT le nom lui-même puis rejoue l'atcommand correspondante
// par `is_atcommand(..., type 1)`, exactement comme si le staff l'avait tapée.
// C'est ce qui fait que la permission de groupe reste la SEULE autorité : un
// « Event Manager » sans `block:` dans conf/groups.yml se voit refuser « Bannir »
// même si le bouton s'affiche, et le compte rendu (réussite comme refus) part
// dans le chat par le canal habituel. Refaire les contrôles ici aurait créé une
// seconde table de droits qui aurait dérivé de la première.
//
// 🔴 Le client envoie l'AID, pas le nom. C'est ce qu'il a sous le curseur, et
// c'est ce que `map_id2sd` prend directement — un nom recopié depuis la plaque
// aurait pu être tronqué, ou désigner un homonyme d'une autre carte.
//
// Gate SERVEUR : niveau de groupe >= 80, le même que l'inspecteur — le seuil fin
// est celui de chaque atcommand. Pas de ZC en retour : tout revient par
// `clif_displaymessage`.
struct PACKET_CZ_BOURGEON_PLAYER_ADMIN {
	int16  packetType;
	int16  packetLength;
	uint32 aid;
	uint8  action;
	int32  param;    // BOURGEON_PLAYER_ADMIN_EVENT_POINTS : le delta, signé
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BOURGEON_PLAYER_ADMIN, 0x0f2b);

// Miroir exact des `kPlayerAdmin*` côté Bourgeon
// (src/features/windows/entity_context_menu.cc) : les deux listes bougent
// ensemble, et les valeurs ne se réordonnent pas.
//
// ⚠ « Muet » et « rendre la parole » sont DEUX actions, pas une bascule. Le
// client ne sait pas si la cible porte SC_NOCHAT, et un joueur muet reste dans le
// monde, à côté de celui qui clique : une bascule aurait rendu la parole à qui
// venait d'être puni par un autre membre du staff. `@mute 60` n'est d'ailleurs
// pas l'inverse d'`@unmute` — elle AJOUTE 60 minutes au compteur de manières.
//
// 🔴 La PRISON, elle, est bien UNE seule action. Deux raisons, et la seconde est
// la vraie : `pc_jail` téléporte sur `MAP_JAIL`, donc un joueur emprisonné n'est
// visible QUE depuis la prison — or on ne peut cliquer droit que sur ce qu'on a à
// l'écran. Dans le monde, la cible n'est jamais emprisonnée ; dans `sec_pri`,
// elle l'est toujours. Les deux sens ne sont jamais disponibles en même temps, et
// une entrée sur deux aurait toujours été morte. (`@jail` et `@unjail` refusent
// de toute façon poliment quand l'état est déjà celui qu'elles produiraient.)
enum e_bourgeon_player_admin_action : uint8 {
	BOURGEON_PLAYER_ADMIN_COME_HERE    = 0,  // le faire marcher jusqu'à nous
	BOURGEON_PLAYER_ADMIN_SIT_STAND    = 1,  // @sitstand (bascule)
	BOURGEON_PLAYER_ADMIN_EVENT_POINTS = 2,  // param = delta de points d'event
	BOURGEON_PLAYER_ADMIN_MUTE         = 3,  // @mute 60
	BOURGEON_PLAYER_ADMIN_UNMUTE       = 4,  // @unmute
	BOURGEON_PLAYER_ADMIN_JAIL_TOGGLE  = 5,  // @jail / @unjail selon SC_JAILED
	BOURGEON_PLAYER_ADMIN_NUKE         = 6,  // @nuke
	BOURGEON_PLAYER_ADMIN_BLOCK        = 7,  // @block : bannit le COMPTE
};

// ⚠ Pas d'action « expulser » ici, et ce n'est pas un oubli : le menu natif
// du client en a déjà une (code 28 -> CZ_GM_KICK 0x00cc), et `clif_parse_GMKick`
// rejoue exactement le même `@kick <nom>` avec les mêmes droits. Bourgeon la
// rejoue donc plutôt que d'ouvrir un second chemin vers la même commande.

// Bornes du don de points d'event, reprises telles quelles du NPC `#gmclicdroit`
// qu'il remplace : au plus 50 dans un sens ou dans l'autre par geste. Le client
// borne sa saisie, le serveur reborne — c'est lui qui décide.
#define BOURGEON_PLAYER_ADMIN_POINTS_STEP 50

// ── Couleurs de corps choisies par le joueur ────────────────────────────────
//
// 🔴 Ce qui circule n'est PAS une palette : c'est une RECETTE, huit réglages HSV
// appliqués aux « rampes » (les dégradés) de la palette interne du sprite. Le
// serveur ne l'interprète jamais — il la stocke telle quelle et la rediffuse.
// C'est ce qui la fait tenir en 40 octets au lieu du kilo-octet d'une palette,
// et c'est aussi ce qui interdit d'y toucher : seuls les clients savent la
// traduire en couleurs, et ils doivent tous la traduire IDENTIQUEMENT.
//
// Un RÉGLAGE de rampe fait 5 octets :
//     [teinte:int16 LE][saturation:int8][luminosité:int8][absolu:uint8]
// Il y en a exactement BOURGEON_STYLE_RAMPS, et ce nombre fait partie du
// format : le changer OBLIGE à incrémenter BOURGEON_STYLE_WIRE_VERSION.
//
// Miroir exact de src/features/fx/style_sync.h côté Bourgeon.
#define BOURGEON_STYLE_RAMPS        8
#define BOURGEON_STYLE_ADJUST_BYTES (BOURGEON_STYLE_RAMPS * 5)  // 40
// 🔴 Miroir de `fx::style_sync::kWireVersion`. Les deux bougent ensemble.
//
// Une SEULE version est acceptée : les autres se jettent, et le joueur retrouve
// son apparence native. Cet octet n'est pas là pour migrer, mais parce que le
// client et le serveur ne sont jamais déployés à la même seconde — pendant un
// patch, des clients d'hier et d'aujourd'hui se croisent sur la même carte.
//
// ⚠ Le serveur ne LIT pas ces octets, il les relaie ; c'est justement pour ça
// qu'il doit filtrer ICI. Le client destinataire n'a aucun moyen de savoir de
// quelle époque vient la recette qu'on lui envoie.
//
// v6 (2026-08-12) : classement des rampes pondéré par la saturation côté client.
// Pas un octet ne bouge dans la trame — c'est le SENS des rangs qui change, une
// recette ne désignant ses pièces que par un rang. Les cinq versions
// précédentes (2026-08-11/12) ajoutaient des champs et se migraient ; celle-ci
// se jette, comme la v2 qui avait déjà déplacé les frontières de rampes.
//
// 🔴 Elle est la SEULE entrée que le serveur interprète. Tout le reste, il le
// range sans le comprendre ; celle-ci, il l'APPLIQUE par `pc_changelook`, ce qui
// l'écrit dans `sd->status.hair`, la sauvegarde avec le personnage et l'annonce
// à la zone par le ZC_SPRITE_CHANGE natif — clients vanilla compris. Elle est
// dans la recette parce que POUR LE JOUEUR la coiffure fait partie du style.
// v7 (2026-08-15) : une recette PAR CORPS. La trame gagne quatre octets de clé,
// et un joueur peut avoir plusieurs entrées dans le même lot.
//
// 🔴 Le fait marquant, pour ce fichier : le serveur ne SAIT PAS ce que cette clé
// désigne, et n'a pas à le savoir. C'est le condensé du chemin de sprite du
// corps, calculé par le client — la seule machine qui résolve ce chemin, avec
// tous ses cas particuliers (montures, styles de corps, costumes). Le serveur
// range N recettes sous leurs clés, les rediffuse toutes, et laisse chaque
// client destinataire choisir celle qui correspond au corps qu'il voit.
//
// Les recettes v6 sont donc jetées : ne portant aucune clé, rien ne dirait à
// quel corps les rattacher.
#define BOURGEON_STYLE_WIRE_VERSION 7

// Nombre de corps qu'un personnage peut habiller séparément.
//
// 🔴 Ce plafond dimensionne le STOCKAGE : une variable de personnage par
// variante (cf. BOURGEON_STYLE_VARS dans clif.cpp), chacune tenant très
// largement sous les 254 caractères d'une `char_reg_str`. Le monter demande donc
// d'ajouter des variables, pas seulement de changer ce nombre. Miroir de
// `fx::style_sync::kMaxVariants`.
#define BOURGEON_STYLE_MAX_VARIANTS 4

// Drapeaux d'une entrée de recette.
enum e_bourgeon_style_flag : uint8 {
	// CZ : efface la variante de CE corps. ZC : ce joueur n'a plus rien du tout.
	BOURGEON_STYLE_CLEAR = 0x01,
	// CZ seulement : efface TOUTES les variantes du personnage.
	BOURGEON_STYLE_CLEAR_ALL = 0x02,
	// ZC seulement : cette variante est celle du REPLI, appliquée par le client
	// aux corps qui n'ont pas la leur. 🔴 C'est le serveur qui la désigne — s'il
	// laissait chaque client la deviner, deux clients pourraient en choisir des
	// différentes selon l'ordre d'arrivée des paquets.
	BOURGEON_STYLE_DEFAULT = 0x04,
};

// CZ (client -> server): le joueur partage le style d'UN de ses corps. Fixe 56.
// Layout: [packetType:2][packetLength:2][version:1][flags:1][body_key:4]
//         [palette_id:2][hair_palette_id:2][hair_style:2][adjusts:40]
//   body_key        : condensé du sprite de corps. OPAQUE ici. 0 = refusé.
//   palette_id      : palette de vêtement officielle 1..553, -1 = d'origine.
//   hair_palette_id : palette de cheveux officielle 1..251, -1 = d'origine.
//   hair_style      : coiffure 1..80, -1 = celle du personnage. 🔴 APPLIQUÉE.
struct PACKET_CZ_BOURGEON_STYLE {
	int16 packetType;
	int16 packetLength;
	uint8 version;
	uint8 flags;
	uint32 body_key;
	int16 palette_id;
	int16 hair_palette_id;
	int16 hair_style;
	uint8 adjusts[BOURGEON_STYLE_ADJUST_BYTES];
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BOURGEON_STYLE, 0x0f26);

// Une entrée du lot ZC : 56 octets. UNE PAR VARIANTE — un joueur qui a habillé
// son corps et sa monture en occupe deux, avec le même `gid`.
//
// ⚠ `hair_style` y figure mais n'est PAS à poser sur l'acteur : le serveur l'a
// déjà appliquée et son ZC_SPRITE_CHANGE natif est parti avant. Elle sert à ce
// que l'écho renvoyé au propriétaire porte une allure complète.
struct PACKET_BOURGEON_STYLE_ENTRY {
	uint32 gid;
	uint8  version;
	uint8  flags;
	uint32 body_key;
	int16  palette_id;
	int16  hair_palette_id;
	int16  hair_style;
	uint8  adjusts[BOURGEON_STYLE_ADJUST_BYTES];
} __attribute__((packed));

// ZC (server -> client): les recettes des joueurs en vue. VARIABLE.
// Layout: [packetType:2][packetLength:2][count:2] puis count × 56 octets.
//
// 🔴 Un lot REDÉFINIT INTÉGRALEMENT les variantes des joueurs qu'il mentionne :
// le client vide ce qu'il sait d'un GID à la première entrée qui le concerne.
// Le serveur doit donc TOUJOURS envoyer l'ensemble complet d'un personnage, même
// quand une seule de ses variantes vient de changer.
//
// Le LOT existe parce qu'arriver sur une carte peuplée fait entrer des dizaines
// de joueurs d'un coup. En pratique la diffusion au spawn n'en envoie qu'un à la
// fois (un joueur entre dans la vue d'un autre) ; le format n'impose donc rien,
// mais il n'interdit pas non plus de grouper plus tard.
struct PACKET_ZC_BOURGEON_STYLES {
	int16 packetType;
	int16 packetLength;
	int16 count;
	// suivi de count × PACKET_BOURGEON_STYLE_ENTRY.
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_STYLES, 0x0f27);

// ZC (server -> client): un NPC pilote la fenêtre de couleurs du joueur. Fixe 5.
// Layout: [packetType:2][packetLength:2][mode:1]
//   mode : 0 = fermer, 1 = ouvrir, 2 = basculer.
//
// 🔴 « Ouvrir » et « basculer » sont DEUX commandes distinctes, et c'est
// délibéré : un styliste qui dirait « bascule » refermerait la fenêtre que le
// joueur venait d'ouvrir lui-même par son raccourci.
struct PACKET_ZC_BOURGEON_STYLE_OPEN {
	int16 packetType;
	int16 packetLength;
	uint8 mode;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_STYLE_OPEN, 0x0f28);

// ── [Stingor] Fenêtre de CIBLE (CZ 0x0F29 -> ZC 0x0F2A) ──────────────────────
//
// Le client connaît déjà le nom, la race et l'élément d'un monstre : le serveur
// les lui écrit dans la plaque de nom (guild_name = race, position_name =
// élément, party_name = « Lv. X | HP: Y% », cf. clif_name). Ce qu'il ne connaît
// d'AUCUNE entité tierce, c'est le **SP** : aucun paquet du protocole ne le
// transporte, et la barre du bas de UIMonsterGage reste donc à zéro côté client.
// C'est ce trou-là que ce couple comble, avec les HP exacts en prime.
//
// Le client DEMANDE (il n'y a pas d'abonnement côté serveur) : tant que sa
// fenêtre de cible est ouverte il réémet la requête à cadence lente. Aucun état
// n'est gardé ici : pas de timer à nettoyer, pas de fuite à la déconnexion, et
// une fenêtre fermée ne coûte rien.
//
// 🔴 GATE PVP. Sur un autre JOUEUR, HP et SP ne partent QUE s'il est du même
// groupe ou de la même guilde (ou soi-même). Un adversaire renvoie son type et
// rien d'autre : c'est une information de jeu, elle ne se donne pas.
struct PACKET_CZ_BOURGEON_TARGET_INFO {
	int16  packetType;
	int16  packetLength;
	uint32 gid;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BOURGEON_TARGET_INFO, 0x0f29);

// ZC (server -> client): l'état de l'entité ciblée. Bloc FIXE.
//
//   status : 0 = ok, 1 = hors de portée ou inexistante (la fenêtre se ferme).
//   known  : masque de ce qui est RENSEIGNÉ — 1 = HP, 2 = SP, 4 = niveau,
//            8 = race/élément/taille. Un champ dont le bit est à 0 est à ignorer :
//            « 0 PV » et « PV inconnus » ne se ressemblent pas à l'écran.
//   type   : e_bourgeon_target_type (PC/MOB/NPC/HOM/MER/PET/ELEM), pas le masque
//            bl_type qui ne tient pas dans un octet.
struct PACKET_ZC_BOURGEON_TARGET_INFO {
	int16  packetType;
	int16  packetLength;
	uint32 gid;
	uint8  status;
	uint8  known;
	uint8  type;
	int16  level;
	uint32 hp;
	uint32 maxhp;
	uint32 sp;
	uint32 maxsp;
	uint8  race;
	uint8  element;
	uint8  element_lv;
	uint8  size;
	uint8  boss;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_TARGET_INFO, 0x0f2a);

// ── [Stingor] Buffs et debuffs d'une entite (CZ 0x0F2C -> ZC 0x0F2D) ────────
//
// Le client RECOIT les changements d'etat des entites en vue (ZC 0x0983, en
// AREA), mais uniquement au moment ou ils COMMENCENT ou FINISSENT. Il n'existe
// aucun paquet vanilla qui donne l'etat COMPLET d'une entite.
//
// 🔴 Le chemin qui semblait le faire n'en fait presque rien. clif_insight ->
// clif_getareachar_unit -> clif_efst_status_change_sub ne lit pas les status
// changes : il lit `sc_display`, que status_change_start ne remplit que pour
// les statuts portant DisplayPc / DisplayNpc. MESURE sur db/pre-re/status.yml :
// 57 sur 599, et pas les bons — ni Blessing, ni Agi Up, ni Endure, ni Kyrie,
// ni meme Poison, Stone ou Freeze.
//
// Consequence : un joueur deja buffe qui entre a l'ecran arrive VIERGE cote
// client. Ce couple comble ce trou-la, et le meme coup celui des membres du
// groupe qu'on ne voit pas du tout (AREA ne sort pas de la vue).
//
// Pas d'abonnement : le client REDEMANDE, comme pour la fenetre de cible. Le
// serveur ne garde aucun etat, donc rien a nettoyer a la deconnexion.
struct PACKET_CZ_BOURGEON_REQ_STATUS_LIST {
	int16  packetType;
	int16  packetLength;
	uint32 gid;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BOURGEON_REQ_STATUS_LIST, 0x0f2c);

// Une entree de la reponse. `remain` vaut 0 pour un etat SANS echeance (duree
// infinie) : ce n'est pas « expire », c'est « ne compte pas a rebours ».
struct BOURGEON_STATUS_ENTRY {
	uint16 efst;
	uint32 remain;
	uint32 total;
} __attribute__((packed));

// ZC : l'etat COMPLET de l'entite. Longueur variable.
//
//   status : 0 = ok · 1 = introuvable, autre carte ou hors de vue · 2 = refuse
//            (ce joueur n'est pas de mon groupe : voir la gate du handler).
//   count  : nombre d'entrees qui suivent.
//
// 🔴 La reponse REMPLACE ce que le client savait de ce GID. C'est un etat, pas
// une difference : une liste vide veut dire « aucun buff », et c'est une
// information — a ne pas confondre avec le silence d'une entite hors de vue,
// que `status` distingue.
struct PACKET_ZC_BOURGEON_STATUS_LIST {
	int16  packetType;
	int16  packetLength;
	uint32 gid;
	uint8  status;
	uint8  count;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_STATUS_LIST, 0x0f2d);

// Plafond d'entrees par reponse. Un joueur tres charge depasse rarement la
// vingtaine ; au-dela on tronque plutot que de faire enfler un paquet qui part
// en rafale sur un groupe de 24.
#define BOURGEON_STATUS_LIST_MAX 40


// ── L'APPARENCE des membres du groupe et des amis (CZ 0x0f2e / ZC 0x0f2f) ────
//
// Pourquoi ce couple existe : la fenetre des membres de GUILDE affiche la tete
// de chacun parce que ZC_MEMBERMGR_INFO porte hair / hair_color / gender. Les
// paquets de groupe et d'amis ne portent RIEN de tel : cote client, l'apparence
// ne peut alors venir que de l'acteur, donc de la portee — un membre sur une
// autre carte n'a pas de tete, et un ami n'en a presque jamais.
//
// Ce couple comble ce trou, et rien d'autre : il ne dit pas qui est en ligne
// (les listes natives le disent deja), seulement a quoi ressemble qui l'est.
//
// 🔴 Le SERVEUR choisit qui repondre, pas le client : il ne renseigne que les
// membres du groupe du demandeur et ses amis. Le champ `what` ne fait que
// RESTREINDRE ce qu'on demande — cocher les deux bits ne donne acces a rien de
// plus. Sans cette regle, ce paquet dirait l'apparence de n'importe qui.
//
// Pas d'abonnement : le client REDEMANDE, comme pour les etats.
struct PACKET_CZ_BOURGEON_REQ_LOOKS {
	int16 packetType;
	int16 packetLength;
	uint8 what;   // bit 0 = groupe · bit 1 = amis
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BOURGEON_REQ_LOOKS, 0x0f2e);

// Une entree : de quoi composer une tete, et rien de plus.
//
// ⚠ `job` sert a choisir la RACE (donc le dossier de sprites et la table de
// coiffures) : sans lui un Doram irait chercher sa tete dans l'arborescence
// humaine. Ce n'est pas un doublon de ce que porte la liste native.
struct BOURGEON_LOOK_ENTRY {
	uint32 aid;
	uint16 job;
	uint16 hair;
	uint16 hair_color;
	uint8  sex;   // 0 = femme
} __attribute__((packed));

struct PACKET_ZC_BOURGEON_LOOKS {
	int16  packetType;
	int16  packetLength;
	uint16 count;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_LOOKS, 0x0f2f);

// Plafond : MAX_PARTY (12) + MAX_FRIENDS (40) tient largement dessous.
#define BOURGEON_LOOKS_MAX 64



// Type d'entité tel qu'il voyage dans ZC_BOURGEON_TARGET_INFO.
enum e_bourgeon_target_type : uint8 {
	BOURGEON_TARGET_UNKNOWN = 0,
	BOURGEON_TARGET_PC      = 1,
	BOURGEON_TARGET_MOB     = 2,
	BOURGEON_TARGET_NPC     = 3,
	BOURGEON_TARGET_HOM     = 4,
	BOURGEON_TARGET_MER     = 5,
	BOURGEON_TARGET_PET     = 6,
	BOURGEON_TARGET_ELEM    = 7,
};

// Bits de `known`.
enum e_bourgeon_target_known : uint8 {
	BOURGEON_TARGET_KNOWN_HP    = 1,
	BOURGEON_TARGET_KNOWN_SP    = 2,
	BOURGEON_TARGET_KNOWN_LEVEL = 4,
	BOURGEON_TARGET_KNOWN_KIND  = 8,   // race + élément + taille + boss
};


// ZC (server -> client): apport des ÉQUIPEMENTS et des CARTES aux stats, compilé
// par status_calc_pc. sd->indexed_bonus.param_equip = apport équipement (copié en
// status.cpp), param_bonus = apport cartes (cf. le split memcpy/memset). Push à
// chaque recalc (équip, level up, login). Le header longueur est CONSERVÉ pour une
// extension future (bonus conditionnels vs race/élément, autocast…) sans nouvel
// opcode. Phase 1 : split des 6 stats primaires (STR..LUK) + ATK/MATK issus de l'équip.
// Bloc FIXE. Suivi en queue de : int16 cond_count, puis cond_count entrées
// PACKET_BOURGEON_STAT_COND (bonus conditionnels vs race/élément/taille).
struct PACKET_ZC_BOURGEON_STAT_BONUS {
	int16 packetType;
	int16 packetLength;
	int16 param_equip[6];  // STR,AGI,VIT,INT,DEX,LUK — apport ÉQUIPEMENT
	int16 param_bonus[6];  // STR,AGI,VIT,INT,DEX,LUK — apport CARTES
	int32 eatk;            // ATK issu de l'équip (sd->bonus.eatk)
	int32 ematk;           // MATK issu de l'équip (sd->bonus.ematk)
	int32 melee_pct;       // % dégât mêlée non-armé (bonus.short_attack_atk_rate)
	int32 ranged_pct;      // % dégât à distance (bonus.long_attack_atk_rate)
	int32 crit_dmg_pct;    // % dégât critique (bonus.crit_atk_rate)
	int32 hp_add;          // PV max ajoutés par l'équip (bonus.hp)
	int32 sp_add;          // SP max ajoutés par l'équip (bonus.sp)
	int32 aspd_add;        // ASPD plate (bonus.aspd_add)
	int32 vcast_pct;       // temps de cast variable, n/100 (bonus.varcastrate ; <0 = réduction)
	int32 fcast_pct;       // temps de cast fixe (bonus.fixcastrate ; <0 = réduction)
	// --- Lot A : offensif ---
	int32 atk_pct;         // % ATK global (bonus.atk_rate)
	int32 matk_pct;        // % MATK global (sd->matk_rate)
	int32 dmg_ret_melee;   // renvoi de dégâts mêlée % (bonus.short_weapon_damage_return)
	int32 dmg_ret_ranged;  // renvoi de dégâts distance % (bonus.long_weapon_damage_return)
	int32 dmg_ret_magic;   // renvoi de dégâts magique % (bonus.magic_damage_return)
	int32 double_pct;      // chance de double attaque % (bonus.double_rate)
	int32 perfect_hit;     // coup parfait % (bonus.perfect_hit)
	// --- Lot B : survie ---
	int32 hp_pct;          // % PV max (sd->hprate)
	int32 sp_pct;          // % SP max (sd->sprate)
	int32 hp_regen_pct;    // % récup PV naturelle (sd->hprecov_rate)
	int32 sp_regen_pct;    // % récup SP naturelle (sd->sprecov_rate)
	int32 crit_def_pct;    // réduction des critiques reçus % (bonus.crit_def_rate)
	int32 hp_on_kill;      // PV gagnés en tuant (bonus.hp_gain_value)
	int32 sp_on_kill;      // SP gagnés en tuant (bonus.sp_gain_value)
	int32 unbreak_pct;     // chance d'éviter la casse d'équip % (bonus.unbreakable)
	// --- Lot C : utilitaire ---
	int32 pot_hp_pct;      // efficacité potions PV % (bonus.itemhealrate2)
	int32 pot_sp_pct;      // efficacité potions SP % (bonus.itemsphealrate2)
	int32 heal_up_pct;     // puissance de soin donné % (bonus.add_heal_rate)
	int32 delay_pct;       // after-cast delay % (bonus.delayrate ; <0 = réduction)
	int32 add_vcast_ms;    // cast variable en ms (bonus.add_varcast)
	int32 add_fcast_ms;    // cast fixe en ms (bonus.add_fixcast)
	int32 steal_pct;       // taux de vol % (bonus.add_steal_rate)
	// --- Lot E : réduction de dégâts par type d'attaque + splash ---
	int32 def_melee_pct;   // réduc. dégâts mêlée reçus % (bonus.near_attack_def_rate)
	int32 def_ranged_pct;  // réduc. dégâts distance reçus % (bonus.long_attack_def_rate)
	int32 def_magic_pct;   // réduc. dégâts magiques reçus % (bonus.magic_def_rate)
	int32 def_misc_pct;    // réduc. dégâts divers reçus % (bonus.misc_def_rate)
	int32 splash;          // portée de splash, en cases (bonus.splash_range)
	int32 splash_add;      // portée de splash additionnelle (bonus.splash_add_range)
	// --- Lot F : vol de vie à l'attaque ---
	int32 hp_drain_pct;    // % de PV volés à l'attaque (right_weapon.hp_drain_rate.per)
	int32 sp_drain_pct;    // % de SP volés à l'attaque (right_weapon.sp_drain_rate.per)
	// --- Lot G : très niche ---
	int32 break_weapon_pct; // chance de casser l'arme de la cible % (bonus.break_weapon_rate)
	int32 break_armor_pct;  // chance de casser l'armure de la cible % (bonus.break_armor_rate)
	int32 zeny_bonus_pct;   // bonus de Zeny sur les monstres % (bonus.get_zeny_rate)
	int32 classchange_pct;  // chance de transformer la cible % (bonus.classchange)
	int32 dmg_ret_reduce;   // réduction des dégâts renvoyés subis % (bonus.reduce_damage_return)
	int32 magic_hp_gain;    // PV gagnés en lançant un sort (bonus.magic_hp_gain_value)
	int32 magic_sp_gain;    // SP gagnés en lançant un sort (bonus.magic_sp_gain_value)
	// --- part du RAFFINAGE dans l'ATK/DEF (pour l'affichage « dont X du refine ») ---
	int32 refine_atk;       // ATK issu du refine+grade de l'arme (base_status.rhw.atk2 + lhw.atk2)
	int32 refine_def;       // DEF issue du refine des armures (bonus.refine_def)
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_STAT_BONUS, 0x0f10);

// Une entrée conditionnelle : (code de catégorie, index élément/race/taille, valeur %).
// Le client mappe (code, idx) -> libellé (cf. character_sheet.cc, tables de noms).
struct PACKET_BOURGEON_STAT_COND {
	uint16 code;   // e_bourgeon_stat_cond
	int16  idx;    // ELE_* / RC_* / SZ_* selon le code
	int32  value;  // magnitude en %
} __attribute__((packed));

// Codes de catégorie des bonus conditionnels. MIROIR côté client (character_sheet.cc).
enum e_bourgeon_stat_cond : uint16 {
	BSC_SUB_ELE  = 1,  // résistance % vs élément reçu   (indexed_bonus.subele[ELE])
	BSC_SUB_RACE = 2,  // résistance % vs race           (indexed_bonus.subrace[RC])
	BSC_SUB_SIZE = 3,  // résistance % vs taille          (indexed_bonus.subsize[SZ])
	BSC_ADD_ELE  = 4,  // +% dégâts vs élément cible      (right_weapon.addele[ELE])
	BSC_ADD_RACE = 5,  // +% dégâts vs race               (right_weapon.addrace[RC])
	BSC_ADD_SIZE = 6,  // +% dégâts vs taille             (right_weapon.addsize[SZ])
	BSC_MADD_ELE  = 7,  // +% dégâts MAGIQUES vs élément  (indexed_bonus.magic_addele[ELE])
	BSC_MADD_RACE = 8,  // +% dégâts MAGIQUES vs race     (indexed_bonus.magic_addrace[RC])
	BSC_MADD_SIZE = 9,  // +% dégâts MAGIQUES vs taille   (indexed_bonus.magic_addsize[SZ])
	BSC_CRIT_RACE = 10, // +crit vs race                  (indexed_bonus.critaddrace[RC])
	BSC_IGN_DEF_RACE  = 11, // ignore DEF vs race %       (indexed_bonus.ignore_def_by_race[RC])
	BSC_IGN_MDEF_RACE = 12, // ignore MDEF vs race %      (indexed_bonus.ignore_mdef_by_race[RC])
	BSC_SUBDEF_ELE = 13, // résist. selon l'élément d'arme ennemie (indexed_bonus.subdefele[ELE])
	BSC_SUB_CLASS  = 14, // réduc. dégâts vs classe (Normal/Boss/…)  (indexed_bonus.subclass[CLASS])
	BSC_SUB_RACE2  = 15, // réduc. dégâts vs groupe de monstres RC2  (indexed_bonus.subrace2[RC2])
	BSC_EXP_RACE   = 16, // +% EXP vs race   (indexed_bonus.expaddrace[RC])
	BSC_EXP_CLASS  = 17, // +% EXP vs classe (indexed_bonus.expaddclass[CLASS])
	BSC_DROP_RACE  = 18, // +% drop vs race   (indexed_bonus.dropaddrace[RC])
	BSC_DROP_CLASS = 19, // +% drop vs classe (indexed_bonus.dropaddclass[CLASS])
	// --- très niche (value = taux %) ---
	BSC_DEFSET_RACE  = 20, // chance de fixer la DEF de la cible (def_set_race[RC].rate)
	BSC_MDEFSET_RACE = 21, // chance de fixer la MDEF de la cible (mdef_set_race[RC].rate)
	BSC_HPVANISH_RACE = 22, // % de PV retirés à la cible (hp_vanish_race[RC].per)
	BSC_SPVANISH_RACE = 23, // % de SP retirés à la cible (sp_vanish_race[RC].per)
	BSC_COMA_RACE    = 24, // chance de coma vs race   (indexed_bonus.coma_race[RC])
	BSC_COMA_CLASS   = 25, // chance de coma vs classe (indexed_bonus.coma_class[CLASS])
	BSC_IGN_RES_RACE  = 26, // ignore RES vs race  (indexed_bonus.ignore_res_by_race[RC])
	BSC_IGN_MRES_RACE = 27, // ignore MRES vs race (indexed_bonus.ignore_mres_by_race[RC])
	BSC_MADD_RACE2    = 28, // +% dégâts magiques vs groupe RC2 (indexed_bonus.magic_addrace2[RC2])
	BSC_IGN_MDEF_RACE2 = 29, // ignore MDEF vs groupe RC2 (indexed_bonus.ignore_mdef_by_race2[RC2])
	BSC_SP_GAIN_RACE  = 30, // SP gagnés en tuant une race (indexed_bonus.sp_gain_race[RC])
	BSC_IGN_DEF_CLASS  = 31, // ignore DEF vs classe %  (ignore_def_by_class[CLASS] ; bitmask bonus1 → 100)
	BSC_IGN_MDEF_CLASS = 32, // ignore MDEF vs classe % (ignore_mdef_by_class[CLASS] ; bitmask bonus1 → 100)
};

// Une entrée conditionnelle liée à un SKILL (tuple enrichi : id + niveau). Le client
// résout le nom via GetSkillName(id). Liste SÉPARÉE des conditionnels indexés.
struct PACKET_BOURGEON_STAT_SKILL {
	uint16 code;      // e_bourgeon_stat_skill
	uint16 skill_id;  // id du skill OU EFST (addeff/reseff), résolu en nom côté client
	int16  lv;        // niveau du skill casté (autospell) ; 0 sinon
	int32  value;     // taux / +% / réduc. % selon le code
	uint16 aux;       // skill déclencheur (autospell3 on-skill) ; 0 sinon
} __attribute__((packed));

// Codes des bonus liés à un skill. MIROIR côté client (character_sheet.cc).
enum e_bourgeon_stat_skill : uint16 {
	BSK_AUTOSPELL     = 1,  // autocast à l'attaque  (sd->autospell)
	BSK_AUTOSPELL_HIT = 2,  // autocast quand touché (sd->autospell2)
	BSK_SKILLATK      = 3,  // +% dégâts sur un skill (sd->skillatk : s_item_bonus{id,val})
	// Pour ADDEFF/RESEFF, skill_id porte un EFST (status_db.getIcon(sc)) ; le client
	// résout le nom via GetStateIconDescript. rate en 1/100 % (10000 = 100%).
	BSK_ADDEFF     = 4,  // inflige un statut à la cible en attaquant (sd->addeff)
	BSK_ADDEFF_HIT = 5,  // inflige un statut à l'attaquant quand touché (sd->addeff_atked)
	BSK_RESEFF     = 6,  // résistance à un statut (sd->reseff : s_item_bonus{sc,val})
	BSK_SUBSKILL   = 7,  // réduction de dégâts d'un skill (sd->subskill : s_item_bonus{id,val})
	BSK_AUTOSPELL_SKILL = 8, // autocast en lançant un skill (sd->autospell3 ; aux=déclencheur)
	// --- modificateurs par-skill (s_item_bonus{id=skill,val}) ---
	BSK_SKILL_SPRATE    = 9,  // coût SP du skill %  (sd->skillusesprate)
	BSK_SKILL_SPCOST    = 10, // coût SP du skill plat (sd->skillusesp)
	BSK_SKILL_VCASTRATE = 11, // cast variable du skill % (sd->skillcastrate)
	BSK_SKILL_FCASTRATE = 12, // cast fixe du skill %     (sd->skillfixcastrate)
	BSK_SKILL_VCAST     = 13, // cast variable du skill ms (sd->skillvarcast)
	BSK_SKILL_FCAST     = 14, // cast fixe du skill ms     (sd->skillfixcast)
	BSK_SKILL_COOLDOWN  = 15, // cooldown du skill ms      (sd->skillcooldown)
	BSK_SKILL_DELAY     = 16, // after-cast delay du skill % (sd->skilldelay)
	BSK_SKILL_HEAL      = 17, // soin donné par le skill % (sd->skillheal)
	BSK_SKILL_HEAL2     = 18, // soin reçu du skill %      (sd->skillheal2)
	BSK_SKILL_BLOWN     = 19, // knockback du skill, cases (sd->skillblown)
};

// Une entrée liée à un ITEM (nameid uint32, trop large pour le tuple skill). Le client
// résout le nom via le DB item. Liste SÉPARÉE.
struct PACKET_BOURGEON_STAT_ITEM {
	uint16 code;    // e_bourgeon_stat_item
	uint32 nameid;  // id d'item (résolu en nom côté client)
	int32  rate;    // taux (add_drop : 1~10000 => /100 = %)
} __attribute__((packed));

// Codes des bonus liés à un item. MIROIR côté client (character_sheet.cc).
enum e_bourgeon_stat_item : uint16 {
	BSI_ADD_DROP       = 1,  // bonus de drop d'un item précis (sd->add_drop, nameid)
	BSI_ADD_DROP_GROUP = 2,  // bonus de drop d'un GROUPE d'items (sd->add_drop, group ; nameid porte le group id)
};

// CZ (client -> server): demande le SCRIPT BRUT + les COMBOS d'un item. Fixe 8.
// Layout: [packetType:2][packetLength:2][id:4].
struct PACKET_CZ_BOURGEON_REQ_ITEMSCRIPT {
	int16  packetType;
	int16  packetLength;
	uint32 id;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BOURGEON_REQ_ITEMSCRIPT, 0x0f11);

// ZC (server -> client): scripts source + combos d'un item, pour les onglets
// « Script » et « Combos » de la description enrichie. VARIABLE.
// Header: [packetType:2][packetLength:2][id:4][status:1]
//   status : 0 = ok, 1 = item introuvable (payload vide au-delà du header).
// Puis, si status==0 :
//   -- scripts (chaînes préfixées longueur 16 bits, sans NUL) --
//     [script_len:2][script:script_len]
//     [equip_len:2][equip:equip_len]
//     [unequip_len:2][unequip:unequip_len]
//   -- combos --
//     [combo_count:1] puis combo_count fois :
//       [member_count:1] puis member_count fois : [member_id:4][namelen:1][name]
//       [script_len:2][script:script_len]
// Les textes sont émis manuellement au WFIFO ; packetLength est écrit en dernier.
struct PACKET_ZC_BOURGEON_ITEMSCRIPT {
	int16  packetType;
	int16  packetLength;
	uint32 id;
	uint8  status;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_ITEMSCRIPT, 0x0f12);

// CZ (client -> server): rapport de bug joueur, CONTEXTUEL. VARIABLE.
// Layout: [packetType:2][packetLength:2][category:1][ctx_len:2][ctx:ctx_len][message: reste]
//   category : 0=générique 1=item 2=skill 3=npc 4=quête 5=style
//              (e_bug_report_category)
//   ctx      : JSON de contexte machine, ex. {"item_id":501,"refine":7} (stocké verbatim)
//   message  : texte libre UTF-8 du joueur (borné VARCHAR(512) côté DB)
// L'IDENTITÉ (compte/perso), la MAP et la POSITION sont ajoutées côté serveur
// depuis la session — jamais lues depuis le paquet.
struct PACKET_CZ_BOURGEON_BUG_REPORT {
	int16  packetType;
	int16  packetLength;
	uint8  category;
	uint16 ctx_len;
	// suivi de ctx[ctx_len] puis message[] jusqu'à packetLength.
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BOURGEON_BUG_REPORT, 0x0f13);

// ZC (server -> client): accusé de réception du rapport de bug. Fixe 5.
// Layout: [packetType:2][packetLength:2][status:1]
//   status : 0 = enregistré, 1 = rate-limité (trop rapide), 2 = erreur/vide.
struct PACKET_ZC_BOURGEON_BUG_REPORT_ACK {
	int16 packetType;
	int16 packetLength;
	uint8 status;
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_BUG_REPORT_ACK, 0x0f14);

// CZ (client -> server): invoquer/basculer un compagnon (chariot / peco / faucon)
// DEPUIS la feuille de perso, sans passer par la NPC Kafra. Fixe 7.
// Layout: [packetType:2][packetLength:2][kind:1][action:1][arg:1]
//   kind   : e_bourgeon_companion_kind (0=cart 1=peco/riding 2=falcon)
//   action : e_bourgeon_companion_action (0=off 1=on 2=deco-cart)
//   arg    : type de décoration/cart pour action=deco (cart), sinon type "on" souhaité (cart) / ignoré
// Le serveur re-valide TOUJOURS le skill requis (pc_setcart/riding/falcon le font ;
// la déco vérifie MC_CHANGECART + le palier de niveau). On ne fait jamais confiance au client.
enum e_bourgeon_companion_kind : uint8 {
	BGCOMP_CART   = 0,
	BGCOMP_PECO   = 1,  // KN_RIDING
	BGCOMP_FALCON = 2,  // HT_FALCON
};
enum e_bourgeon_companion_action : uint8 {
	BGCOMP_OFF  = 0,
	BGCOMP_ON   = 1,
	BGCOMP_DECO = 2,  // cart : changer la décoration (arg = type)
};
struct PACKET_CZ_BOURGEON_COMPANION {
	int16 packetType;
	int16 packetLength;
	uint8 kind;
	uint8 action;
	uint8 arg;
} __attribute__((packed));
DEFINE_PACKET_HEADER(CZ_BOURGEON_COMPANION, 0x0f15);

// ZC (server -> client) SELF: état des compagnons (niveaux de skills + états actifs),
// poussé au login vérifié et à chaque changement (pc_setcart/riding/falcon). Fixe.
// La feuille de perso l'utilise pour AFFICHER/gater les cases sans lire côté client
// des IDs de skills ni le bitmask option (cassé pour le cart sous NEW_CARTS).
struct PACKET_ZC_BOURGEON_COMPANION_STATE {
	int16 packetType;
	int16 packetLength;
	uint8 pushcart_lv;    // MC_PUSHCART   (0 = non appris -> pas de case cart)
	uint8 changecart_lv;  // MC_CHANGECART (0 = non appris -> pas de déco)
	uint8 riding_lv;      // KN_RIDING     (0 = non appris -> pas de case peco)
	uint8 falcon_lv;      // HT_FALCON     (0 = non appris -> pas de case faucon)
	uint8 cart_active;    // type de cart courant (0 = aucun, 1..MAX_CARTS)
	uint8 riding_active;  // 1 = sur peco/monture
	uint8 falcon_active;  // 1 = faucon présent
	uint8 cart_deco_max;  // type de déco max autorisé par le niveau de base (cycle client)
	// Ids AEGIS des skills (= enum e_skill, identiques côté client) pour charger l'ICÔNE
	// de la case sans hardcoder d'id côté client (cf. principe « ne jamais hardcoder »).
	uint16 pushcart_id;   // MC_PUSHCART
	uint16 riding_id;     // KN_RIDING
	uint16 falcon_id;     // HT_FALCON
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_COMPANION_STATE, 0x0f16);

// ZC (server -> client) SELF: table itemId(client) -> ordinal de hat effect
// (e_hat_effects), poussée au login vérifié. Permet au client de PRÉVISUALISER les
// costumes SANS viewid (cashshop / description) sans les équiper : le hat effect est
// une donnée de SCRIPT d'item (hateffect HAT_EF_x), invisible côté client autrement.
// Statique (dérivée des scripts item_db au 1er envoi, mise en cache) donc identique
// pour tous les joueurs. VARIABLE.
// Layout: [packetType:2][packetLength:2][count:2] puis count × { itemId:4, ordinal:2 }.
struct PACKET_ZC_BOURGEON_HATEFFECT_MAP {
	int16  packetType;
	int16  packetLength;
	int16  count;
	// suivi de count × { uint32 itemId; int16 ordinal; }
} __attribute__((packed));
DEFINE_PACKET_HEADER(ZC_BOURGEON_HATEFFECT_MAP, 0x0f17);

// NOTE: there is no ZC_BOURGEON_MAP packet. The Bourgeon client reads the
// current map name from the standard 0x0091 ZC_NPCACK_MAPMOVE packet instead.
// Historique : les anciens opcodes 0x0BFx/0x0C2x partageaient des entrées du
// client Ragexe (longueurs fixes) et pouvaient désync le flux. Tous migrés dans
// la zone sûre 0x0F00+ (2026-07-03) ; prochain libre = 0x0F0F.

#if !defined(sun) && (!defined(__NETBSD__) || __NetBSD_Version__ >= 600000000) // NetBSD 5 and Solaris don't like pragma pack but accept the packed attribute
#pragma pack(pop)
#endif // not NetBSD < 6 / Solaris

#endif /* MAP_PACKETS_STRUCT_HPP */
