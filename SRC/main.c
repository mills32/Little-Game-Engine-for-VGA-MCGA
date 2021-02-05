/***********************
*  LITTLE GAME ENGINE  *
************************/

/*##########################################################################
	A lot of code from David Brackeen                                   
	http://www.brackeen.com/home/vga/                                     

	Sprite loader and bliter from xlib
	
	This is a 16-bit program, Remember to compile in the LARGE memory model!                        
	
	All code is 8086 / 8088 compatible
	
	Please feel free to copy this source code.                            
	
 
	LT_MODE = player movement modes

	//MODE TOP = 0;
	//MODE PLATFORM = 1;
	//MODE PUZZLE = 2;
	//MODE SIDESCROLL = 3;
	
	//33 fixed sprites:
	//	8x8   (16 sprites: 0 - 15)	
	//	16x16 (12 sprites: 16 - 27)
	//	32x32 ( 4 sprites: 28 - 31)
	//	64x64 ( 1 sprite: 32)
	
##########################################################################*/

#include "lt__eng.h"

byte PICO8_palette[] = {//generated with gimp, then divided by 4 (max = 64).
//I don't want to know why the EGA colors have to be arranged like this
//In the VGA registers
	0x00,0x00,0x00,	//0
	0x06,0x0A,0x15,	//1
	0x02,0x21,0x14,	//2
	0x00,0x14,0x35,	//4
	0x1F,0x09,0x14,	//3
	0x3F,0x32,0x42,	//6
	0x2B,0x14,0x0E, //5	
	0x2F,0x31,0x31,	//7
	0x00,0x00,0x00, //Not used in VGA-EGA mode
	0x00,0x00,0x00,	//
	0x00,0x00,0x00,	//
	0x00,0x00,0x00,	//
	0x00,0x00,0x00,	//
	0x00,0x00,0x00,	//
	0x00,0x00,0x00,	//
	0x00,0x00,0x00,	//
	0x16,0x17,0x18,	//8
    0x20,0x1D,0x26,	//9
	0x00,0x39,0x0D,	//10
	0x08,0x2B,0x3F,	//12
	0x3F,0x00,0x12,	//11
	0x3F,0x1D,0x29,	//14
	0x3F,0x38,0x0D, //13	
	0x3f,0x3f,0x3f, //15	
};

const unsigned char palette_cycle_logo[] = {//generated with gimp, then divided by 4 (max = 64).
	//2 copies of 8 colour cycle
	0xff,0xff,0xff,//
	0xff,0xff,0xff,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0xff,0xff,0xff,//
	0xff,0xff,0xff,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00
};

const unsigned char palette_cycle_water[] = {//generated with gimp, then divided by 4 (max = 64).
	//2 copies of 8 colour cycle
	0x35,0x3b,0x3f,//
	0x26,0x39,0x3f,
	0x18,0x36,0x3f,
	0x09,0x32,0x3e,
	0x0f,0x34,0x3f,
	0x17,0x36,0x3f,
	0x1c,0x3a,0x3e,
	0x24,0x3e,0x3d,
	0x35,0x3b,0x3f,//
	0x26,0x39,0x3f,
	0x18,0x36,0x3f,
	0x09,0x32,0x3e,
	0x0f,0x34,0x3f,
	0x17,0x36,0x3f,
	0x1c,0x3a,0x3e,
	0x24,0x3e,0x3d,
	0x35,0x3b,0x3f
};

const unsigned char palette_cycle_space[] = {//generated with gimp, then divided by 4 (max = 64).
	//2 copies of 8 colour cycle
	0x35,0x35,0x3f,//
	0x00,0x00,0x22,
	0x18,0x29,0x3f,
	0x09,0x1d,0x3e,
	0x12,0x0f,0x3f,
	0x17,0x22,0x3f,
	0x1c,0x2d,0x3e,
	0x24,0x33,0x3e,
	0x35,0x35,0x3f,//
	0x00,0x00,0x22,
	0x18,0x29,0x3f,
	0x09,0x1d,0x3e,
	0x12,0x0f,0x3f,
	0x17,0x22,0x3f,
	0x1c,0x2d,0x3e,
	0x24,0x33,0x3e
};

LT_Col LT_Player_Col;
COLORCYCLE cycle_water;
COLORCYCLE cycle_logo;

extern byte LT_AI_Stack_Table[];

int running = 1;
int x,y = 0;
int i,j = 0;
int Scene = 0;
int menu_option = 0;
int menu_pos[10] = {7.3*16,4*16,7.3*16,5*16,7.3*16,6*16,7.3*16,7*16,7.3*16,8*16};
int game = 0;
int random;
int logotimer = 0;

void Load_Logo(){
	LT_Load_Map("GFX/Logo.tmx");
	if (LT_VIDEO_MODE) {
		LT_Load_Tiles("GFX/Ltil_VGA.bmp");
		VGA_Fade_out();
	}
	else {
		LT_Load_Tiles("GFX/Ltil_EGA.bmp");
		VGA_EGAMODE_CustomPalette(PICO8_palette);
	}
	LT_Set_Map(0,0);
	cycle_init(&cycle_logo,palette_cycle_logo);
}

void Run_Logo(){
	while (logotimer < 512){
		VGA_Scroll(x,y);
		LT_scroll_map();
		if (y<144)y++;
		cycle_palette(&cycle_logo,2);
		if (LT_Keys[LT_ESC]) {VGA_Scroll(0,0); logotimer = 512;}//if esc released, exit
		LT_WaitVsync();
		logotimer++;
	}
	Clearkb();
}



#ifndef M_PI
#define M_PI 3.14159
#endif

unsigned int SINEX[256];
unsigned int SINEY[256];

void Load_Test(){
	LT_Set_Loading_Interrupt(); 
	for( i = 0; i < 256; ++i ) {
		SINEX[ i ] = 320 * ( (sin( 2.0 * M_PI * i / 256.0 ) + 1.0 ) / 2.0 );
		SINEY[ i ] = 200 * ( (sin( 2.0 * M_PI * i / 128.0 ) + 1.0 ) / 2.0 );
	}
	LT_Load_Map("gfx/lisa.tmx");
	if (LT_VIDEO_MODE) LT_Load_Tiles("gfx/lisa_VGA.bmp");
	else LT_Load_Tiles("GFX/lisa_EGA.bmp");
	LT_Load_Sprite("GFX/ship.bmp",28,32);
	LT_Load_Sprite("GFX/ship.bmp",29,32);
	//LT_LoadMOD("MUSIC/MOD/frog.mod"); 
	LT_Delete_Loading_Interrupt();
	LT_Set_Map(0,0);
	LT_MODE = 0;
	Scene = 0;
	i = 0;
	LT_Set_Sprite_Animation(28,8,1,4);
	LT_Set_Sprite_Animation(29,8,1,4);
	//PlayMOD(8);
}

void Run_Test(){
	int A = 0;
	while(Scene == 0){
		if (SCR_X < SINEX[0])SCR_X++;
		else Scene = 1;
		if (SCR_Y < SINEY[0])SCR_Y++;
		LT_Draw_Sprites();
		
		sprite[28].pos_x = SCR_X + 64;
		sprite[28].pos_y = SCR_Y + 64;
		sprite[29].pos_x = SCR_X + 128;
		sprite[29].pos_y = SCR_Y + 128;
		//scroll_map and draw borders
		LT_scroll_map();
		//do_play_music();
		//mod_player_fast();
		LT_WaitVsync();		
	}
	
	while(Scene == 1){
		LT_Draw_Sprites();
		
		SCR_X = SINEX[i & 255];
		SCR_Y = SINEY[i & 255];
		sprite[28].pos_x = SCR_X + 64;
		sprite[28].pos_y = LT_SIN[A&255] + SCR_Y + 80;
		sprite[29].pos_x = SCR_X + 128;
		sprite[29].pos_y = SCR_Y + 128;
		
		i++;
		A++;
		//scroll_map and draw borders
		LT_scroll_map();
		
		if (LT_Keys[LT_ESC]) {Scene = 2;game = 0;}
		//mod_player_fast();
		LT_WaitVsync();
	}
	Clearkb();
	//StopMOD();
	LT_unload_sprite(28);
	LT_unload_sprite(29);
	Scene = 2;game = 0;
}

void Load_Menu(){
	LT_Set_Loading_Interrupt(); 
	
	LT_Load_Map("GFX/menu.tmx");
	if (LT_VIDEO_MODE) LT_Load_Tiles("GFX/menu_VGA.bmp");
	else LT_Load_Tiles("GFX/menu_EGA.bmp");
	LT_Load_Sprite("GFX/cursorb.bmp",16,16);
	LT_Load_Sprite("GFX/Rocketc.bmp",32,64);
	
	LT_Load_Music("music/ADLIB/yellow.imf");
	
	LT_Clear_Samples();
	sb_load_sample("MUSIC/samples/drum.wav");
	sb_load_sample("MUSIC/samples/snare.wav");
	sb_load_sample("MUSIC/samples/explode.wav");
	sb_load_sample("MUSIC/samples/byebye.wav");
	sb_load_sample("MUSIC/samples/eing.wav");
	sb_load_sample("MUSIC/samples/hit.wav");
	/*sb_load_sample("MUSIC/samples/in_hole.wav");
	sb_load_sample("MUSIC/samples/touch.wav");
	sb_load_sample("MUSIC/samples/pop.wav");*/
	//LT_Clear_Samples();
	/*sb_load_sample("MUSIC/samples/drum.wav");
	sb_load_sample("MUSIC/samples/snare.wav");
	sb_load_sample("MUSIC/samples/explode.wav");
	sb_load_sample("MUSIC/samples/byebye.wav");
	sb_load_sample("MUSIC/samples/eing.wav");
	sb_load_sample("MUSIC/samples/hit.wav");
	sb_load_sample("MUSIC/samples/in_hole.wav");
	sb_load_sample("MUSIC/samples/touch.wav");
	sb_load_sample("MUSIC/samples/pop.wav");*/
	//sb_load_sample("MUSIC/samples/what_the.wav");
	
	LT_Delete_Loading_Interrupt();
	
	//PlayMOD(0);

	VGA_SplitScreen(44);
	
	LT_Set_Map(0,0);
	
	i = 0;
	LT_MODE = 0;
}

void Run_Menu(){
	int change = 0; //wait between key press
	int transition = 0;
	Scene = 1;
	menu_option = 0;
	
	x = 0;
	y = 0;
	
	LT_Set_Sprite_Animation(16,0,8,6);
	while(Scene == 1){
		LT_Draw_Sprites_Fast();
		
		SCR_X = -54-(LT_SIN[i]>>1);
		
		sprite[16].pos_x = menu_pos[menu_option] + (LT_COS[j]>>3);
		sprite[16].pos_y = transition+menu_pos[menu_option+1];
		
		sprite[32].pos_x = 24;
		sprite[32].pos_y = 56+ (LT_COS[j]>>3);
		
		if (i > 360) i = 0;
		i+=2;
		if (j > 356) {j = 0; sprite[16].anim_counter = 0;}
		j+=8;
		
		if (LT_Keys[LT_UP] == 1) change = 1;
		if (LT_Keys[LT_DOWN] == 1) change = 2;
			
		if (change == 1){
			transition--;
			if (transition == -17){
				sb_play_sample(5,11025);
				menu_option -=2; 
				game--;
				change = 0;
				transition = 0;
			}
		}
		if (change == 2) {
			transition++;
			if (transition == 17){
				sb_play_sample(5,11025);
				menu_option +=2; 
				game++;
				change = 0;
				transition = 0;
			}
		}
		
		if (menu_option < 0) menu_option = 0;
		if (menu_option > 8) menu_option = 8;		
		if (game < 1) game = 1;
		if (game > 5) game = 5;	
		
		if (LT_Keys[LT_ENTER]) Scene = 2;
		if (LT_Keys[LT_JUMP]) {Scene = 2;game = 6;}
		if (LT_Keys[LT_ESC]) {Scene = 2;game = 255; running = 0;}
		do_play_music();
		LT_WaitVsync();
		//LT_MoveWindow(y);
		y++;
	}
	StopMOD();
	LT_unload_sprite(16);
	LT_unload_sprite(32);
}

void Load_TopDown(){
	LT_Clear_Samples();
	LT_Set_Loading_Interrupt(); 
	
	LT_Load_Map("GFX/Topdown.tmx");
	if (LT_VIDEO_MODE) LT_Load_Tiles("gfx/top_VGA.bmp");
	else LT_Load_Tiles("GFX/top_EGA.bmp");
	LT_Reset_Sprite_Stack();
	LT_Load_Sprite("GFX/player.bmp",16,16);
	LT_Load_Sprite("GFX/enemy2.bmp",17,16);
	LT_Reset_AI_Stack();
	LT_AI_Sprite[0] = 17;
	LT_Set_AI_Sprites(17,7);
	
	LT_Load_Music("music/adlib/top_down.imf");
	//LoadMOD("MUSIC/MOD/Beach.mod"); 

	LT_Delete_Loading_Interrupt();
	
	//LT_Start_Music(70);
	//PlayMOD(1);
	
	if(LT_VIDEO_MODE)LT_SetWindow("GFX/win_VGA.bmp");
	else LT_SetWindow("GFX/win_EGA.bmp");
	
	LT_MODE = 0;
	
	//animate colours
	cycle_init(&cycle_water,palette_cycle_water);
	Scene = 2;
}

void Run_TopDown(){
	int n;
	LT_Sprite_Stack = 1;
	LT_Set_Map(0,0);
	Scene = 2;
	x = 0;
	y = 0;
	sprite[16].pos_x = 8*16;
	sprite[16].pos_y = 4*16;
	
	//Place AI manually on map in tile units (x16)
	//They will be drawn only if they are inside viewport.
	
	while(Scene == 2){
		
		//SCR_X and SCR_Y are predefined global variables 
		
		
		//Draw sprites first to avoid garbage
		LT_scroll_follow(16);
		LT_Draw_Sprites();
		
		//scroll_map update off screen tiles
		LT_scroll_map();
		
		LT_Print_Window_Variable(32,LT_Player_Col.tile_number);
		
		cycle_palette(&cycle_water,4);
		
		//In this mode sprite is controlled using U D L R
		LT_Player_Col = LT_move_player(16);
		
		//Player animations
		if (LT_Keys[LT_RIGHT]) LT_Set_Sprite_Animation(16,0,6,4);
		else if (LT_Keys[LT_LEFT]) LT_Set_Sprite_Animation(16,6,6,4);
		else if (LT_Keys[LT_UP]) LT_Set_Sprite_Animation(16,12,6,4);
		else if (LT_Keys[LT_DOWN]) LT_Set_Sprite_Animation(16,18,6,4);
		else sprite[16].animate = 0;
		
		//Move the enemies
		LT_Update_AI_Sprites();
		
		//Flip
		for (n = 1; n != LT_Sprite_Stack; n++){
			if (sprite[LT_Sprite_Stack_Table[n]].mspeed_x) LT_Set_Sprite_Animation(LT_Sprite_Stack_Table[n],2,2,8);
			if (sprite[LT_Sprite_Stack_Table[n]].mspeed_y) LT_Set_Sprite_Animation(LT_Sprite_Stack_Table[n],6,2,8);	
		}
		
		//If collision tile = ?, end level
		if (LT_Player_Col.tilecol_number == 11) Scene = 1;
		if (LT_Keys[LT_ESC]) {Scene = 1; game = 0;}//esc exit
		
		do_play_music();
		
		LT_WaitVsync();
		if (LT_Player_Col_Enemy()) {
			if (LT_VIDEO_MODE == 1)VGA_Fade_out(); 
			//sprite[16].init = 0;
			Scene = 0;
			game = 0;
		}
	}
	//StopMOD(1);
	LT_unload_sprite(16); //manually free sprites
	LT_unload_sprite(17);
}

void Load_Platform(){
	LT_Set_Loading_Interrupt(); 
	
	Scene = 2;
	LT_Load_Map("GFX/Platform.tmx");
	if (LT_VIDEO_MODE) LT_Load_Tiles("GFX/Pla_VGA.bmp");
	else LT_Load_Tiles("GFX/Pla_EGA.bmp");
	LT_Reset_Sprite_Stack();
	LT_Load_Sprite("GFX/player.bmp",16,16);
	LT_Load_Sprite("GFX/enemy.bmp",17,16);
	LT_AI_Sprite[0] = 17;
	LT_Set_AI_Sprites(17,7);
	
	//LT_Load_Music("music/Adlib/platform.imf");
	//LT_LoadMOD("MUSIC/MOD/beach.mod"); 
	LT_Delete_Loading_Interrupt();
	//PlayMOD(4);
	//LT_Start_Music(70);
	
	if(LT_VIDEO_MODE)LT_SetWindow("GFX/win_VGA.bmp");
	else LT_SetWindow("GFX/win_EGA.bmp");
	
	//animate water
	cycle_init(&cycle_water,palette_cycle_water);
	
	LT_MODE = 1;
}

void Run_Platform(){
	int n;
	int diamonds = 0;
	LT_Sprite_Stack = 1;
	LT_Set_Map(0,0);
	sprite[16].pos_x = 4*16;
	sprite[16].pos_y = 4*16;
	
	Scene = 2;
	
	while (Scene == 2){
		
		LT_scroll_follow(16);
		LT_Draw_Sprites();
		
		//EDIT MAP: GET DIAMONDS
		switch (LT_Player_Col.tile_number){
			case 134: LT_Get_Item(16, 1, 0); diamonds++; break;
			case 135: LT_Get_Item(16, 22, 0); diamonds++; break;
			case 136: LT_Get_Item(16, 0, 0); diamonds++; break;
			case 137: LT_Get_Item(16, 109, 0); diamonds++; break;
		} 
		
		
		//scroll_map and draw borders
		LT_scroll_map();
	
		//In this mode sprite is controlled using L R and Jump
		LT_Player_Col = LT_move_player(16);
		
		//set player animations
		if (LT_Keys[LT_RIGHT]) LT_Set_Sprite_Animation(16,0,6,3);
		else if (LT_Keys[LT_LEFT]) LT_Set_Sprite_Animation(16,6,6,3);
		else if (LT_Keys[LT_UP]) LT_Set_Sprite_Animation(16,12,6,3);
		else if (LT_Keys[LT_DOWN]) LT_Set_Sprite_Animation(16,12,6,3);
		else sprite[16].animate = 0;
	
		//Move the enemies
		LT_Update_AI_Sprites();
		
		//Flip
		for (n = 1; n != LT_Sprite_Stack; n++){
			if (sprite[LT_Sprite_Stack_Table[n]].mspeed_x > 0) LT_Set_Sprite_Animation(LT_Sprite_Stack_Table[n],0,6,6);
			if (sprite[LT_Sprite_Stack_Table[n]].mspeed_x < 0) LT_Set_Sprite_Animation(LT_Sprite_Stack_Table[n],6,6,6);	
		}
		
		//If collision tile = ?, end level
		if (LT_Player_Col.tilecol_number == 11) Scene = 1;

		//water palette animation
		cycle_palette(&cycle_water,2);

		//esc go to menu
		if (LT_Keys[LT_ESC]) {Scene = 1; game = 0;}//esc exit
		
		LT_Print_Window_Variable(32,LT_Player_Col_Enemy());
		
		//do_play_music();
		//mod_player_fast();
		LT_WaitVsync();
		//if water or enemy, reset level
		if ((LT_Player_Col.tile_number == 102) || LT_Player_Col_Enemy()) {
			if (LT_VIDEO_MODE == 1)VGA_Fade_out(); 
			//sprite[16].init = 0;
			Scene = 0;
			game = 0;
		}
	}
	Clearkb();
	LT_unload_sprite(16); //manually free sprites
	LT_unload_sprite(17);
	//StopMOD();
}

void Load_Puzzle(){
	LT_Set_Loading_Interrupt(); 
	
	Scene = 2;
	LT_Load_Map("GFX/puz.tmx");
	if (LT_VIDEO_MODE) LT_Load_Tiles("gfx/puz_VGA.bmp");
	else LT_Load_Tiles("GFX/puz_EGA.bmp");
	
	LT_Reset_Sprite_Stack();
	LT_Load_Sprite("GFX/ball.bmp",16,16);
	LT_Load_Sprite("GFX/ball1.bmp",17,16);
	//LT_Load_Sprite("GFX/cursor2.bmp",0,8);
	//LT_Set_Sprite_Animation(0,0,8,2);
	LT_AI_Sprite[0] = 17;
	LT_Set_AI_Sprites(17,7);
	
	LT_Load_Music("music/ADLIB/puzzle.imf");
	
	LT_Delete_Loading_Interrupt();
	
	//LT_Start_Music(70);
	
	//animate colours
	cycle_init(&cycle_water,palette_cycle_water);
	
	LT_Set_Map(0,0);
	LT_MODE = 2; //Physics mode
}

void Run_Puzzle(){
	int rotate = 0;
	int cos = 0;
	int sin = 0;
	//int power = 40;
	Scene = 2;
	sprite[16].pos_x = 4*16;
	sprite[16].pos_y = 3*16;
	
	//To simulate floats
	sprite[16].fpos_x = sprite[16].pos_x;
	sprite[16].fpos_y = sprite[16].pos_y;
	sprite[16].speed_x = 0;
	sprite[16].speed_y = 0;
	
	while(Scene == 2){
		LT_scroll_follow(16);
		LT_Draw_Sprites();

		//scroll_map update off screen tiles
		LT_scroll_map();
		
		//In mode 3, sprite is controlled using the speed.
		//Also there are physics using the collision tiles
		LT_Player_Col = LT_move_player(16);
		
		if(rotate == 360) rotate = 0;
		if(rotate < 0) rotate = 360;

		//ROTATE 
		if (LT_Keys[LT_RIGHT]) sprite[16].speed_x+=20;
		if (LT_Keys[LT_LEFT]) sprite[16].speed_x-=20;
		if (LT_Keys[LT_UP]) sprite[16].speed_y-=20;
		if (LT_Keys[LT_DOWN]) sprite[16].speed_y+=20;
		
		//sprite[0].pos_x = sprite[16].pos_x+32;
		//sprite[0].pos_y = sprite[16].pos_y;
		
		//HIT BALL
		sprite[16].state = 1; //Move

		//If collision tile = ?, end level
		if (LT_Player_Col.tilecol_number == 11) Scene = 1;
		
		cycle_palette(&cycle_water,2);

		if (LT_Keys[LT_ESC]) {Scene = 1; game = 0;} //esc exit
		
		do_play_music();
		LT_WaitVsync();
	}
	
	//LT_Unload_Sprite(0);
	LT_unload_sprite(16); //manually free sprites
}

void Load_Puzzle2(){
	LT_Set_Loading_Interrupt(); 
	
	Scene = 2;
	LT_Load_Map("GFX/puz1.tmx");
	if (LT_VIDEO_MODE) LT_Load_Tiles("gfx/puz1_VGA.bmp");
	else LT_Load_Tiles("GFX/puz1_EGA.bmp");
	
	LT_Load_Sprite("GFX/bar.bmp",16,16);
	LT_Load_Sprite("GFX/aball.bmp",0,8);
	LT_Load_Music("music/ADLIB/top_down.imf");
	
	LT_Delete_Loading_Interrupt();
	//LT_Start_Music(70);
	
	if(LT_VIDEO_MODE)LT_SetWindow("GFX/win_VGA.bmp");
	else LT_SetWindow("GFX/win_EGA.bmp");
	
	LT_Set_Map(0,0);
	LT_MODE = 0; 
}

void Run_Puzzle2(){
	int score = 0;
	Scene = 2;
	
	sprite[16].pos_x = 10*16;
	sprite[16].pos_y = 12*16;
	
	sprite[0].pos_x = 10*16;
	sprite[0].pos_y = 10*16;
	
	//To bounce ball
	sprite[0].speed_x = 1;
	sprite[0].speed_y = -1;
	
	while(Scene == 2){
		//SCR_X and SCR_Y are predefined global variables 
		VGA_Scroll(SCR_X,SCR_Y);
		
		//simple physics to bounce a ball
		LT_Player_Col = LT_Bounce_Ball(0);
		
		LT_Draw_Sprites();

		if (LT_Player_Col.col_x == 5) score++;
		if (LT_Player_Col.col_y == 5) score++;
		
		//ROTATE 
		if (LT_Keys[LT_RIGHT]) sprite[16].pos_x++;
		if (LT_Keys[LT_LEFT]) sprite[16].pos_x--;
		
		//HIT BALL
		if ((LT_Keys[LT_ACTION]) && (sprite[16].state == 0)){
			sprite[0].state = 1; //Move
			sprite[0].speed_x = 1;
			sprite[0].speed_y = -1;
		}

		if (LT_Keys[LT_ESC]) {Scene = 1; game = 0;} //esc exit
		
		do_play_music();
		LT_WaitVsync();
	}
	LT_Unload_Sprite(0);
	LT_unload_sprite(16); //manually free sprites
}

void Load_Shooter(){
	LT_Set_Loading_Interrupt(); 
	
	Scene = 2;
	LT_Load_Map("GFX/Shooter.tmx");
	if (LT_VIDEO_MODE) LT_Load_Tiles("gfx/spa_VGA.bmp");
	else LT_Load_Tiles("GFX/spa_EGA.bmp");
	LT_Reset_Sprite_Stack();
	LT_Load_Sprite("GFX/player.bmp",16,16);
	LT_Load_Sprite("GFX/ship.bmp",28,32);
	LT_Load_Sprite("GFX/rocketb.bmp",29,32);
	LT_AI_Sprite[0] = 28;
	LT_Set_AI_Sprites(29,3);
	LT_Load_Music("music/ADLIB/shooter.imf");
	
	LT_Delete_Loading_Interrupt();

	if(LT_VIDEO_MODE)LT_SetWindow("GFX/win_VGA.bmp");
	else LT_SetWindow("GFX/win_EGA.bmp");
}

void Run_Shooter(){
	int timer;
	start:
	timer = 0;
	Scene = 0;
	sprite[28].pos_x = 8*16;
	sprite[28].pos_y = 16*16;
	
	LT_Set_Sprite_Animation(28,0,1,4);
	sprite[16].pos_x = 0;
	sprite[16].pos_y = 17*16;
	LT_Set_Sprite_Animation(16,0,6,4);
	
	LT_Set_Map(0,5);
	
	cycle_init(&cycle_water,palette_cycle_space);
	
	//WALK TO SHIP
	while (Scene == 0){
		LT_Draw_Sprites();
		

		sprite[16].pos_x++;
		
		if (sprite[16].pos_x == sprite[28].pos_x+8){
			sprite[16].pos_x = -16; //hide
			Scene = 1; 
		} 
		cycle_palette(&cycle_water,2);
		LT_WaitVsync();
	}
	
	//CLOSE SHIP
	LT_Set_Sprite_Animation(28,1,4,12);
	while (Scene == 1){
		LT_Draw_Sprites();
		if (sprite[28].frame == 4) Scene = 2; 
		cycle_palette(&cycle_water,2);
		LT_WaitVsync();
	}
	//FLY UP
	sprite[28].animate = 0;
	while (Scene == 2){
		LT_scroll_follow(28);
		LT_Draw_Sprites();
		
		sprite[28].pos_y--;
		if (SCR_Y == 0) Scene = 3; 
		
		LT_scroll_map();
		cycle_palette(&cycle_water,2);
		LT_WaitVsync();
	}
	
	//START ENGINE
	LT_Set_Sprite_Animation(28,4,4,5);
	while (Scene == 3){
		LT_Draw_Sprites();

		timer++;
		if (timer == 100) Scene = 4; 
		cycle_palette(&cycle_water,2);
		LT_WaitVsync();
	}

	//SIDE SCROLL
	//LT_Start_Music(70);
	LT_MODE = 3;
	LT_Set_Sprite_Animation(28,8,4,4);
	while(Scene == 4){
		SCR_X++;
		LT_Draw_Sprites();
		LT_Endless_SideScroll_Map(0);
		
		LT_Player_Col = LT_move_player(28);
		if (LT_Player_Col.col_x == 1) {
			LT_Set_Sprite_Animation(28,12,4,4);
		}
		if (sprite[28].frame == 15) {
			sprite[28].init = 0;
			sleep(2); 
			Scene = -1;
		}
		
		if (LT_Keys[LT_ESC]) {Scene = 255; game = 0;} //esc exit
		
		cycle_palette(&cycle_water,2);
		LT_Print_Window_Variable(32,SCR_X>>4);
		
		do_play_music();
		
		
		LT_WaitVsync();
	}
	
	LT_unload_sprite(28);
	LT_unload_sprite(29);
	LT_unload_sprite(16);
}


void main(){
	
	//Dissable cursor
	outportb(0x3D4, 0x0A);
	outportb(0x3D5, 0x20);
	
	//Select video menu 80x25 text mode
	system("cls");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                                                                ");
	printf("                                  SELECT VIDEO                                  ");
	printf("                                                                                ");
	printf("                                0 => EGA 16 colours                             ");
	printf("                                2 => VGA 16 colours                             ");
	printf("                                1 => VGA 256 colours                            ");
	printf("                                                                                ");
	printf(" - EGA mode runs faster, so use it if the sprites flicker.                      ");
	printf(" - Mode 2 is as fast as mode 0. It uses VGA in EGA mode, with a custom palette  ");
	printf(" - VGA mode looks prettier, but you'll need a faster machine.                   ");

	while (kbhit() == 0);
	LT_VIDEO_MODE = getch() -48;
	if ((LT_VIDEO_MODE != 0) && (LT_VIDEO_MODE != 1)) LT_VIDEO_MODE = 1;
	printf("video mode = %i",LT_VIDEO_MODE);
	system("cls");
	
	if (LT_VIDEO_MODE == 0) VGA_EGAMODE_CustomPalette(PICO8_palette);
	//LT_Check_CPU(); //still causing trouble
	//LT_Adlib_Detect();
	LT_Init_GUS(4);
    LT_init();
	sb_init();//After LT_Init
	
	//You can use a custom loading animation for the Loading_Interrupt
	LT_Load_Animation("GFX/loading.bmp",32);
	LT_Set_Animation(0,8,2);
	
	if (LT_VIDEO_MODE)LT_Load_Font("GFX/font.bmp");
	else LT_Load_Font("GFX/font_EGA.bmp");
	
	Load_Logo();
	Run_Logo();
	
	LT_MODE = 0;
	game = 0;
	//MENU
	while (running){
		if (game == 0){Load_Menu();Run_Menu();}
		if (game == 1){Load_TopDown();Run_TopDown();}
		if (game == 2){Load_Platform();Run_Platform();}
		if (game == 3){Load_Puzzle();Run_Puzzle();}
		if (game == 4){Load_Puzzle2();Run_Puzzle2();}   
		if (game == 5){Load_Shooter();Run_Shooter();}
		if (game == 6){Load_Test();Run_Test();}
	}
	
	LT_ExitDOS();
	
}

