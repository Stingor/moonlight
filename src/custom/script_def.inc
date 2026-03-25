// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Script Commands
 * Place the definition of custom script commands in this file.
 * Format:
 *	BUILDIN_DEF(command_name,"parameters"),
 **/

//BUILDIN_DEF(example,""),
	// [Stingor]
	BUILDIN_DEF(viewequip,"vv"),
	BUILDIN_DEF(getpartnername,""),
	BUILDIN_DEF(getskillname,"i"),
	BUILDIN_DEF(getalliance,"i"),
	BUILDIN_DEF(setwaitingroomtitle,"s?"),
	BUILDIN_DEF(npcsit,""),
	BUILDIN_DEF(npcstand,""),
	//BUILDIN_DEF(addcloneskill,"ii"),
	BUILDIN_DEF(starteventexp,""),
	BUILDIN_DEF(stopeventexp,""),
	BUILDIN_DEF(eventexp,""),
	BUILDIN_DEF(starteventdrop,""),
	BUILDIN_DEF(stopeventdrop,""),
	BUILDIN_DEF(eventdrop,""),
	BUILDIN_DEF(getcartlist, "?"),
	BUILDIN_DEF(getstoragelist, "i"),
	BUILDIN_DEF(getguildstoragelist, ""),
	BUILDIN_DEF(loadstorage, "i"),
	// [Kamitsu]
	BUILDIN_DEF(getusersnbr,""),
	BUILDIN_DEF(storagecountitem,"v??"),
	BUILDIN_DEF2(storagecountitem,"storagecountitem2","viiiiiii?"),
	BUILDIN_DEF(instance_create,"s????"),
