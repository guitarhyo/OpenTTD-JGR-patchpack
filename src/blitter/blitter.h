/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file blitter.h Blitter code interface. */

#ifndef BLITTER_H
#define BLITTER_H

#include "../core/pointer.h"
#include "../string.h"
#include "../spritecache.h"
#include "../spriteloader.h"

/** The modes of blitting we can do. */
enum BlitterMode {
	BM_NORMAL,       ///< Perform the simple blitting.
	BM_COLOUR_REMAP, ///< Perform a colour remapping.
	BM_TRANSPARENT,  ///< Perform transparency colour remapping.
	BM_CRASH_REMAP,  ///< Perform a crash remapping.
	BM_BLACK_REMAP,  ///< Perform remapping to a completely blackened sprite
};

/**
 * How all blitters should look like. Extend this class to make your own.
 */
class Blitter {
public:
	/** Parameters related to blitting. */
	struct BlitterParams {
		const Sprite *sprite; ///< Pointer to the sprite how ever the encoder stored it
		const byte *remap;  ///< XXX -- Temporary storage for remap array

		int skip_left;      ///< How much pixels of the source to skip on the left (based on zoom of dst)
		int skip_top;       ///< How much pixels of the source to skip on the top (based on zoom of dst)
		int width;          ///< The width in pixels that needs to be drawn to dst
		int height;         ///< The height in pixels that needs to be drawn to dst
		int left;           ///< The left offset in the 'dst' in pixels to start drawing
		int top;            ///< The top offset in the 'dst' in pixels to start drawing

		void *dst;          ///< Destination buffer
		int pitch;          ///< The pitch of the destination buffer
	};

	/** Types of palette animation. */
	enum PaletteAnimation {
		PALETTE_ANIMATION_NONE,           ///< No palette animation
		PALETTE_ANIMATION_VIDEO_BACKEND,  ///< Palette animation should be done by video backend (8bpp only!)
		PALETTE_ANIMATION_BLITTER,        ///< The blitter takes care of the palette animation
	};

	/** Check if this blitter is usable. */
	static bool usable (void)
	{
		return true;
	}

	/**
	 * Get the screen depth this blitter works for.
	 *  This is either: 8, 16, 24 or 32.
	 */
	virtual uint8 GetScreenDepth() = 0;

	/**
	 * Draw an image to the screen, given an amount of params defined above.
	 */
	virtual void Draw(Blitter::BlitterParams *bp, BlitterMode mode, ZoomLevel zoom) = 0;

	/**
	 * Draw a colourtable to the screen. This is: the colour of the screen is read
	 *  and is looked-up in the palette to match a new colour, which then is put
	 *  on the screen again.
	 * @param dst the destination pointer (video-buffer).
	 * @param width the width of the buffer.
	 * @param height the height of the buffer.
	 * @param pal the palette to use.
	 */
	virtual void DrawColourMappingRect(void *dst, int width, int height, PaletteID pal) = 0;

	/**
	 * Convert a sprite from the loader to our own format.
	 */
	virtual Sprite *Encode (const SpriteLoader::Sprite *sprite, bool is_font, AllocatorProc *allocator) = 0;

	/**
	 * Move the destination pointer the requested amount x and y, keeping in mind
	 *  any pitch and bpp of the renderer.
	 * @param video The destination pointer (video-buffer) to scroll.
	 * @param x How much you want to scroll to the right.
	 * @param y How much you want to scroll to the bottom.
	 * @return A new destination pointer moved the the requested place.
	 */
	virtual void *MoveTo(void *video, int x, int y) = 0;

	/**
	 * Draw a pixel with a given colour on the video-buffer.
	 * @param video The destination pointer (video-buffer).
	 * @param x The x position within video-buffer.
	 * @param y The y position within video-buffer.
	 * @param colour A 8bpp mapping colour.
	 */
	virtual void SetPixel(void *video, int x, int y, uint8 colour) = 0;

	/**
	 * Make a single horizontal line in a single colour on the video-buffer.
	 * @param video The destination pointer (video-buffer).
	 * @param width The length of the line.
	 * @param height The height of the line.
	 * @param colour A 8bpp mapping colour.
	 */
	virtual void DrawRect(void *video, int width, int height, uint8 colour) = 0;

	/**
	 * Draw a line with a given colour.
	 * @param video The destination pointer (video-buffer).
	 * @param x The x coordinate from where the line starts.
	 * @param y The y coordinate from where the line starts.
	 * @param x2 The x coordinate to where the line goes.
	 * @param y2 The y coordinate to where the lines goes.
	 * @param screen_width The width of the screen you are drawing in (to avoid buffer-overflows).
	 * @param screen_height The height of the screen you are drawing in (to avoid buffer-overflows).
	 * @param colour A 8bpp mapping colour.
	 * @param width Line width.
	 * @param dash Length of dashes for dashed lines. 0 means solid line.
	 */
	virtual void DrawLine(void *video, int x, int y, int x2, int y2, int screen_width, int screen_height, uint8 colour, int width, int dash = 0);

	/**
	 * Copy from a buffer to the screen.
	 * @param video The destination pointer (video-buffer).
	 * @param src The buffer from which the data will be read.
	 * @param width The width of the buffer.
	 * @param height The height of the buffer.
	 * @note You can not do anything with the content of the buffer, as the blitter can store non-pixel data in it too!
	 */
	virtual void CopyFromBuffer(void *video, const void *src, int width, int height) = 0;

	/**
	 * Copy from the screen to a buffer.
	 * @param video The destination pointer (video-buffer).
	 * @param dst The buffer in which the data will be stored.
	 * @param width The width of the buffer.
	 * @param height The height of the buffer.
	 * @note You can not do anything with the content of the buffer, as the blitter can store non-pixel data in it too!
	 */
	virtual void CopyToBuffer(const void *video, void *dst, int width, int height) = 0;

	/**
	 * Copy from the screen to a buffer in a palette format for 8bpp and RGBA format for 32bpp.
	 * @param video The destination pointer (video-buffer).
	 * @param dst The buffer in which the data will be stored.
	 * @param width The width of the buffer.
	 * @param height The height of the buffer.
	 * @param dst_pitch The pitch (byte per line) of the destination buffer.
	 */
	virtual void CopyImageToBuffer(const void *video, void *dst, int width, int height, int dst_pitch) = 0;

	/**
	 * Scroll the videobuffer some 'x' and 'y' value.
	 * @param video The buffer to scroll into.
	 * @param left The left value of the screen to scroll.
	 * @param top The top value of the screen to scroll.
	 * @param width The width of the screen to scroll.
	 * @param height The height of the screen to scroll.
	 * @param scroll_x How much to scroll in X.
	 * @param scroll_y How much to scroll in Y.
	 */
	virtual void ScrollBuffer(void *video, int &left, int &top, int &width, int &height, int scroll_x, int scroll_y) = 0;

	/**
	 * Calculate how much memory there is needed for an image of this size in the video-buffer.
	 * @param width The width of the buffer-to-be.
	 * @param height The height of the buffer-to-be.
	 * @return The size needed for the buffer.
	 */
	virtual int BufferSize(int width, int height) = 0;

	/**
	 * Called when the 8bpp palette is changed; you should redraw all pixels on the screen that
	 *  are equal to the 8bpp palette indexes 'first_dirty' to 'first_dirty + count_dirty'.
	 * @param palette The new palette.
	 */
	virtual void PaletteAnimate(const Palette &palette) = 0;

	/**
	 * Check if the blitter uses palette animation at all.
	 * @return True if it uses palette animation.
	 */
	virtual Blitter::PaletteAnimation UsePaletteAnimation() = 0;

	/**
	 * Get how many bytes are needed to store a pixel.
	 */
	virtual int GetBytesPerPixel() = 0;

	/**
	 * Post resize event
	 */
	virtual void PostResize() { };

	virtual ~Blitter() { }

	/** Helper function to allocate a sprite in Encode. */
	template <typename T>
	static T *AllocateSprite (const SpriteLoader::Sprite *sprite,
		AllocatorProc *allocator, size_t extra = 0)
	{
		T *s = (T *) allocator (sizeof(T) + extra);

		s->height = sprite->height;
		s->width  = sprite->width;
		s->x_offs = sprite->x_offs;
		s->y_offs = sprite->y_offs;

		return s;
	}

	/* Static stuff (active blitter). */

	/* Select a blitter. */
	static Blitter *select (const char *name);
};

/** Get the current active blitter (always set by calling Blitter::select). */
static inline Blitter *GetCurrentBlitter()
{
	extern ttd_unique_ptr<Blitter> current_blitter;
	return current_blitter.get();
}

const char *GetCurrentBlitterName (void);

void GetBlittersInfo (stringb *buf);

extern char *_ini_blitter;
extern bool _blitter_autodetected;

#endif /* BLITTER_H */