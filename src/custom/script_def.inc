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
	BUILDIN_DEF(mobismvp,"i"),
	BUILDIN_DEF(mobisboss,"i"),
	BUILDIN_DEF(getmonsterexprate,"ii?"),
	BUILDIN_DEF(getmapspawns,"s"),
	BUILDIN_DEF(viewequip,"vv"),
	BUILDIN_DEF(getpartnername,""),
	BUILDIN_DEF(getskillname,"i"),
	BUILDIN_DEF(getalliance,"i"),
	BUILDIN_DEF(setwaitingroomtitle,"s?"),
	BUILDIN_DEF(sethomskinn,"i"),
	BUILDIN_DEF(npcsit,""),
	BUILDIN_DEF(npcstand,""),
	BUILDIN_DEF(npcwalktothensit,"ii"),
	BUILDIN_DEF(logchat,"s"),
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
	BUILDIN_DEF(getmapdisplayname,"s"),
	BUILDIN_DEF(isinitemgroup,"ii"),
	BUILDIN_DEF(itemlinkuid,"s?"),
	BUILDIN_DEF(arraypush,"rv*"),
	BUILDIN_DEF(arraypop,"r"),
	BUILDIN_DEF(getuidlist,"?"),
	BUILDIN_DEF(alteritem,"sii?"),
	BUILDIN_DEF(equipuid,"s?"),
	BUILDIN_DEF(delitembyuid,"s?"),
	BUILDIN_DEF(getiteminfobyuid,"si?"),
	BUILDIN_DEF(getallrefine,"?"),
	BUILDIN_DEF(getmvprespawn,"i???????"),  // mob_id + 7 optional output vars (spawnx,spawny,map$,killer$,killtime,tombx,tomby)
	BUILDIN_DEF(getallmvprespawn,"rrrrrrr"), // 7 required output array vars
