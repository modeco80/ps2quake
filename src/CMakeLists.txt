add_executable(quake
	chase.c
	cl_demo.c
	cl_input.c
	cl_main.c
	cl_parse.c
	cl_tent.c
	cmd.c
	common.c
	console.c
	crc.c
	cvar.c
	d_edge.c
	d_fill.c
	d_init.c
	d_modech.c
	d_part.c
	d_polyse.c
	draw.c
	d_scan.c
	d_sky.c
	d_sprite.c
	d_surf.c
	d_vars.c
	d_zpoint.c
	host.c
	host_cmd.c
	keys.c
	mathlib.c
	menu.c
	model.c
	net_loop.c
	net_none.c
	net_vcr.c
	net_main.c
	nonintel.c
	pr_cmds.c
	pr_edict.c
	pr_exec.c
	r_aclip.c
	r_alias.c
	r_bsp.c
	r_draw.c
	r_edge.c
	r_efrag.c
	r_light.c
	r_main.c
	r_misc.c
	r_part.c
	r_sky.c
	r_sprite.c
	r_surf.c
	r_vars.c
	sbar.c
	screen.c
	#	snd_dma.c
	#snd_mem.c
	#snd_mix.c
	snd_null.c
	sv_main.c
	sv_move.c
	sv_phys.c
	sv_user.c

	view.c
	wad.c
	world.c
	zone.c

	# System specific code
	ps2/sys_sky.cpp
	ps2/cd_sky.c
	ps2/vid_sky.c
	ps2/in_sky.c
	snd_null.c
	)

target_include_directories(quake PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(quake PRIVATE m)
