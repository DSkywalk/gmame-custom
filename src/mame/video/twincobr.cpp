// license:BSD-3-Clause
// copyright-holders:Quench
/***************************************************************************
  video.c

  Functions to emulate the video hardware of these machines.
  Video is 40x30 tiles. (320x240 for Twin Cobra/Flying shark)
  Video is 40x30 tiles. (320x240 for Wardner)

  Video has 3 scrolling tile layers (Background, Foreground and Text) and
  Sprites that have 4 priorities. Lowest priority is "Off".
  Wardner has an unusual sprite priority in the shop scenes, whereby a
  middle level priority Sprite appears over a high priority Sprite ?

***************************************************************************/

#include "emu.h"
#include "video/mc6845.h"
#include "includes/twincobr.h"

/***************************************************************************
    Callbacks for the TileMap code
***************************************************************************/

TILE_GET_INFO_MEMBER(twincobr_state::get_bg_tile_info)
{
	const u16 code = m_bgvideoram16[tile_index+m_bg_ram_bank];
	const u32 tile_number = code & 0x0fff;
	const u32 color = (code & 0xf000) >> 12;
	SET_TILE_INFO_MEMBER(2,
			tile_number,
			color,
			0);
}

TILE_GET_INFO_MEMBER(twincobr_state::get_fg_tile_info)
{
	const u16 code = m_fgvideoram16[tile_index];
	const u32 tile_number = (code & 0x0fff) | m_fg_rom_bank;
	const u32 color = (code & 0xf000) >> 12;
	SET_TILE_INFO_MEMBER(1,
			tile_number,
			color,
			0);
}

TILE_GET_INFO_MEMBER(twincobr_state::get_tx_tile_info)
{
	const u16 code = m_txvideoram16[tile_index];
	const u32 tile_number = code & 0x07ff;
	const u32 color = (code & 0xf800) >> 11;
	SET_TILE_INFO_MEMBER(0,
			tile_number,
			color,
			0);
}

/***************************************************************************
    Start the video hardware emulation.
***************************************************************************/

void twincobr_state::twincobr_create_tilemaps()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(twincobr_state::get_bg_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,64);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(twincobr_state::get_fg_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,64);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(twincobr_state::get_tx_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,32);

	m_bg_tilemap->set_scrolldx(-55, -134 );
	m_fg_tilemap->set_scrolldx(-55, -134 );
	m_tx_tilemap->set_scrolldx(-55, -134 );
	m_bg_tilemap->set_scrolldy(-30, -243 );
	m_fg_tilemap->set_scrolldy(-30, -243 );
	m_tx_tilemap->set_scrolldy(-30, -243 );

	m_fg_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);
}

void twincobr_state::video_start()
{
	m_spritegen->alloc_sprite_bitmap(*m_screen);

	/* the video RAM is accessed via ports, it's not memory mapped */
	m_txvideoram_size = 0x0800;
	m_bgvideoram_size = 0x2000; /* banked two times 0x1000 */
	m_fgvideoram_size = 0x1000;

	twincobr_create_tilemaps();

	m_txvideoram16 = make_unique_clear<u16[]>(m_txvideoram_size);
	m_fgvideoram16 = make_unique_clear<u16[]>(m_fgvideoram_size);
	m_bgvideoram16 = make_unique_clear<u16[]>(m_bgvideoram_size);

	m_display_on = 0;

	save_pointer(NAME(m_txvideoram16), m_txvideoram_size);
	save_pointer(NAME(m_fgvideoram16), m_fgvideoram_size);
	save_pointer(NAME(m_bgvideoram16), m_bgvideoram_size);
	save_item(NAME(m_txoffs));
	save_item(NAME(m_fgoffs));
	save_item(NAME(m_bgoffs));
	save_item(NAME(m_txscrollx));
	save_item(NAME(m_fgscrollx));
	save_item(NAME(m_bgscrollx));
	save_item(NAME(m_txscrolly));
	save_item(NAME(m_fgscrolly));
	save_item(NAME(m_bgscrolly));
	save_item(NAME(m_display_on));
	save_item(NAME(m_fg_rom_bank));
	save_item(NAME(m_bg_ram_bank));
}



/***************************************************************************
    Video I/O interface
***************************************************************************/

WRITE_LINE_MEMBER(twincobr_state::display_on_w)
{
	m_display_on = state;
}

WRITE_LINE_MEMBER(twincobr_state::flipscreen_w)
{
	machine().tilemap().set_flip_all((state ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0));
}

WRITE_LINE_MEMBER(twincobr_state::bg_ram_bank_w)
{
	m_bg_ram_bank = state ? 0x1000 : 0x0000;
	m_bg_tilemap->mark_all_dirty();
}

WRITE_LINE_MEMBER(twincobr_state::fg_rom_bank_w)
{
	m_fg_rom_bank = state ? 0x1000 : 0x0000;
	m_fg_tilemap->mark_all_dirty();
}


void twincobr_state::twincobr_txoffs_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_txoffs);
	m_txoffs %= m_txvideoram_size;
}
u16 twincobr_state::twincobr_txram_r()
{
	return m_txvideoram16[m_txoffs];
}
void twincobr_state::twincobr_txram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_txvideoram16[m_txoffs]);
	m_tx_tilemap->mark_tile_dirty(m_txoffs);
}

void twincobr_state::twincobr_bgoffs_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bgoffs);
	m_bgoffs %= (m_bgvideoram_size >> 1);
}
u16 twincobr_state::twincobr_bgram_r()
{
	return m_bgvideoram16[m_bgoffs+m_bg_ram_bank];
}
void twincobr_state::twincobr_bgram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bgvideoram16[m_bgoffs+m_bg_ram_bank]);
	m_bg_tilemap->mark_tile_dirty((m_bgoffs+m_bg_ram_bank));
}

void twincobr_state::twincobr_fgoffs_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fgoffs);
	m_fgoffs %= m_fgvideoram_size;
}
u16 twincobr_state::twincobr_fgram_r()
{
	return m_fgvideoram16[m_fgoffs];
}
void twincobr_state::twincobr_fgram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fgvideoram16[m_fgoffs]);
	m_fg_tilemap->mark_tile_dirty(m_fgoffs);
}


void twincobr_state::twincobr_txscroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (offset == 0)
	{
		COMBINE_DATA(&m_txscrollx);
		m_tx_tilemap->set_scrollx(0, m_txscrollx);
	}
	else
	{
		COMBINE_DATA(&m_txscrolly);
		m_tx_tilemap->set_scrolly(0, m_txscrolly);
	}
}

void twincobr_state::twincobr_bgscroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (offset == 0)
	{
		COMBINE_DATA(&m_bgscrollx);
		m_bg_tilemap->set_scrollx(0, m_bgscrollx);
	}
	else
	{
		COMBINE_DATA(&m_bgscrolly);
		m_bg_tilemap->set_scrolly(0, m_bgscrolly);
	}
}

void twincobr_state::twincobr_fgscroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (offset == 0)
	{
		COMBINE_DATA(&m_fgscrollx);
		m_fg_tilemap->set_scrollx(0, m_fgscrollx);
	}
	else
	{
		COMBINE_DATA(&m_fgscrolly);
		m_fg_tilemap->set_scrolly(0, m_fgscrolly);
	}
}

void twincobr_state::twincobr_exscroll_w(offs_t offset, u16 data)/* Extra unused video layer */
{
	if (offset == 0) logerror("PC - write %04x to unknown video scroll Y register\n",data);
	else logerror("PC - write %04x to unknown video scroll X register\n",data);
}

/******************** Wardner interface to this hardware ********************/
void twincobr_state::wardner_txlayer_w(offs_t offset, u8 data)
{
	int shift = 8 * (offset & 1);
	twincobr_txoffs_w(offset / 2, data << shift, 0xff << shift);
}

void twincobr_state::wardner_bglayer_w(offs_t offset, u8 data)
{
	int shift = 8 * (offset & 1);
	twincobr_bgoffs_w(offset / 2, data << shift, 0xff << shift);
}

void twincobr_state::wardner_fglayer_w(offs_t offset, u8 data)
{
	int shift = 8 * (offset & 1);
	twincobr_fgoffs_w(offset / 2, data << shift, 0xff << shift);
}

void twincobr_state::wardner_txscroll_w(offs_t offset, u8 data)
{
	int shift = 8 * (offset & 1);
	twincobr_txscroll_w(offset / 2, data << shift, 0xff << shift);
}

void twincobr_state::wardner_bgscroll_w(offs_t offset, u8 data)
{
	int shift = 8 * (offset & 1);
	twincobr_bgscroll_w(offset / 2, data << shift, 0xff << shift);
}

void twincobr_state::wardner_fgscroll_w(offs_t offset, u8 data)
{
	int shift = 8 * (offset & 1);
	twincobr_fgscroll_w(offset / 2, data << shift, 0xff << shift);
}

void twincobr_state::wardner_exscroll_w(offs_t offset, u8 data)/* Extra unused video layer */
{
	switch (offset)
	{
		case 01:    //data <<= 8;
		case 00:    logerror("PC - write %04x to unknown video scroll X register\n",data); break;
		case 03:    //data <<= 8;
		case 02:    logerror("PC - write %04x to unknown video scroll Y register\n",data); break;
	}
}

u8 twincobr_state::wardner_videoram_r(offs_t offset)
{
	int shift = 8 * (offset & 1);
	switch (offset / 2)
	{
		case 0: return twincobr_txram_r() >> shift;
		case 1: return twincobr_bgram_r() >> shift;
		case 2: return twincobr_fgram_r() >> shift;
	}
	return 0;
}

void twincobr_state::wardner_videoram_w(offs_t offset, u8 data)
{
	int shift = 8 * (offset & 1);
	switch (offset / 2)
	{
		case 0: twincobr_txram_w(0,data << shift, 0xff << shift); break;
		case 1: twincobr_bgram_w(0,data << shift, 0xff << shift); break;
		case 2: twincobr_fgram_w(0,data << shift, 0xff << shift); break;
	}
}

u8 twincobr_state::wardner_sprite_r(offs_t offset)
{
	u16 *spriteram16 = reinterpret_cast<u16 *>(m_spriteram8->live());
	int shift = (offset & 1) * 8;
	return spriteram16[offset/2] >> shift;
}

void twincobr_state::wardner_sprite_w(offs_t offset, u8 data)
{
	u16 *spriteram16 = reinterpret_cast<u16 *>(m_spriteram8->live());
	if (offset & 1)
		spriteram16[offset/2] = (spriteram16[offset/2] & 0x00ff) | (data << 8);
	else
		spriteram16[offset/2] = (spriteram16[offset/2] & 0xff00) | data;
}





void twincobr_state::log_vram()
{
#ifdef MAME_DEBUG

	if ( machine().input().code_pressed(KEYCODE_M) )
	{
		offs_t tile_voffs;
		int tcode[4];
		while (machine().input().code_pressed(KEYCODE_M)) ;
		logerror("Scrolls             BG-X BG-Y  FG-X FG-Y  TX-X  TX-Y\n");
		logerror("------>             %04x %04x  %04x %04x  %04x  %04x\n",m_bgscrollx,m_bgscrolly,m_fgscrollx,m_fgscrolly,m_txscrollx,m_txscrolly);
		for ( tile_voffs = 0; tile_voffs < (m_txvideoram_size/2); tile_voffs++ )
		{
			tcode[1] = m_bgvideoram16[tile_voffs];
			tcode[2] = m_fgvideoram16[tile_voffs];
			tcode[3] = m_txvideoram16[tile_voffs];
			logerror("$(%04x)  (Col-Tile) BG1:%01x-%03x  FG1:%01x-%03x  TX1:%02x-%03x\n", tile_voffs,
							tcode[1] & 0xf000 >> 12, tcode[1] & 0x0fff,
							tcode[2] & 0xf000 >> 12, tcode[2] & 0x0fff,
							tcode[3] & 0xf800 >> 11, tcode[3] & 0x07ff);
		}
		for ( tile_voffs = (m_txvideoram_size/2); tile_voffs < (m_fgvideoram_size/2); tile_voffs++ )
		{
			tcode[1] = m_bgvideoram16[tile_voffs];
			tcode[2] = m_fgvideoram16[tile_voffs];
			logerror("$(%04x)  (Col-Tile) BG1:%01x-%03x  FG1:%01x-%03x\n", tile_voffs,
							tcode[1] & 0xf000 >> 12, tcode[1] & 0x0fff,
							tcode[2] & 0xf000 >> 12, tcode[2] & 0x0fff);
		}
		for ( tile_voffs = (m_fgvideoram_size/2); tile_voffs < (m_bgvideoram_size/2); tile_voffs++ )
		{
			tcode[1] = m_bgvideoram16[tile_voffs];
			logerror("$(%04x)  (Col-Tile) BG1:%01x-%03x\n", tile_voffs,
							tcode[1] & 0xf000 >> 12, tcode[1] & 0x0fff);
		}
	}
#endif
}


u32 twincobr_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	log_vram();

	u16 *buffered_spriteram16;
	u32 bytes;
	if (m_spriteram16 != nullptr)
	{
		buffered_spriteram16 = m_spriteram16->buffer();
		bytes = m_spriteram16->bytes();
	}
	else
	{
		buffered_spriteram16 = reinterpret_cast<u16 *>(m_spriteram8->buffer());
		bytes = m_spriteram8->bytes();
	}

	if (!m_display_on)
	{
		bitmap.fill(rgb_t::black(), cliprect);
	}
	else
	{
		m_spritegen->draw_sprites_to_tempbitmap(cliprect, buffered_spriteram16, bytes);

		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
		m_spritegen->copy_sprites_from_tempbitmap(bitmap,cliprect,1);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
		m_spritegen->copy_sprites_from_tempbitmap(bitmap,cliprect,2);
		m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
		m_spritegen->copy_sprites_from_tempbitmap(bitmap,cliprect,3);
	}

	return 0;
}
