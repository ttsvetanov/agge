#pragma once

#include "scanline.h"

namespace agge
{
	class renderer
	{
	public:
		template <typename BitmapT, typename BlenderT>
		class adapter;

	public:
		template <typename BitmapT, typename MaskT, typename BlenderT, typename AlphaFn>
		void operator ()(BitmapT &bitmap, const rect_i *window, const MaskT &mask, const BlenderT &blender, const AlphaFn &alpha);

	private:
		raw_memory_object _scanline_cache;
	};


	template <typename BitmapT, typename BlenderT>
	class renderer::adapter
	{
	public:
		typedef typename BlenderT::cover_type cover_type;

	public:
		adapter(BitmapT &bitmap, const rect_i *window, const BlenderT &blender);

		bool set_y(int y);
		void operator ()(int x, int length, const cover_type *covers);

	private:
		const adapter &operator =(const adapter &rhs);

	private:
		const BlenderT &_blender;
		const int _offset_x, _limit_x;
		typename BitmapT::pixel *_row;
		int _y;
		BitmapT &_bitmap;
		const int _offset_y, _limit_y;
	};



	template <typename BitmapT, typename BlenderT>
	inline renderer::adapter<BitmapT, BlenderT>::adapter(BitmapT &bitmap, const rect_i *window, const BlenderT &blender)
		: _blender(blender),
			_offset_x(window ? window->x1 : 0),
			_limit_x((!window || static_cast<int>(bitmap.width()) < width(*window) ? bitmap.width() : width(*window)) + _offset_x), 
			_bitmap(bitmap),
			_offset_y(window ? window->y1 : 0),
			_limit_y((!window || static_cast<int>(bitmap.height()) < height(*window) ? bitmap.height() : height(*window)) + _offset_y)
	{	}

	template <typename BitmapT, typename BlenderT>
	inline bool renderer::adapter<BitmapT, BlenderT>::set_y(int y)
	{
		if (y < _offset_y || _limit_y <= y)
			return false;
		_y = y;
		_row = _bitmap.row_ptr(y - _offset_y) - _offset_x;
		return true;
	}

	template <typename BitmapT, typename BlenderT>
	inline void renderer::adapter<BitmapT, BlenderT>::operator ()(int x, int length, const cover_type *covers)
	{
		if (x < _offset_x)
		{
			const int dx = x - _offset_x;

			x = _offset_x;
			length += dx;
			covers -= dx;
		}
		if (x + length > _limit_x)
			length = _limit_x - x;
		if (length > 0)
			_blender(_row + x, x, _y, length, covers);
	}


	template <unsigned _1_shift, typename ScanlineT, typename CellsIteratorT, typename AlphaFn>
	AGGE_INLINE void sweep_scanline(ScanlineT &scanline, CellsIteratorT begin, CellsIteratorT end, const AlphaFn &alpha)
	{
		int cover = 0;

		if (begin == end)
			return;

		for (CellsIteratorT i = begin; ; )
		{
			int x = i->x, area = 0;

			do
			{
				area += i->area;
				cover += i->cover;
				++i;
			} while (i != end && i->x == x);

			int cover_m = cover << (1 + _1_shift);

			if (area)
			{
				scanline.add_cell(x, alpha(cover_m - area));
				++x;
			}

			if (i == end)
				break;

			int len = i->x - x;

			if (len && cover_m)
				scanline.add_span(x, len, alpha(cover_m));
		}
	}


	template <typename ScanlineT, typename MaskT, typename AlphaFn>
	inline void render(ScanlineT &scanline, const MaskT &mask, const AlphaFn &alpha, int offset, int step)
	{
		typename MaskT::range vrange = mask.vrange();

		for (int y = vrange.first + offset; y <= vrange.second; y += step)
		{			
			typename MaskT::scanline_cells cells = mask[y];

			if (scanline.begin(y))
			{
				sweep_scanline<MaskT::_1_shift>(scanline, cells.first, cells.second, alpha);
				scanline.commit();
			}
		}
	}


	template <typename BitmapT, typename BlenderT>
	inline void fill(BitmapT &bitmap, const BlenderT &blender)
	{
		const int width = bitmap.width();
		const int height = bitmap.height();

		for (int y = 0; y != height; ++y)
			blender(bitmap.row_ptr(y), 0, y, width);
	}


	template <typename BitmapT, typename MaskT, typename BlenderT, typename AlphaFn>
	void renderer::operator ()(BitmapT &bitmap, const rect_i *window, const MaskT &mask, const BlenderT &blender,
		const AlphaFn &alpha)
	{
		typedef adapter<BitmapT, BlenderT> rendition_adapter;

		rendition_adapter ra(bitmap, window, blender);
		typename MaskT::range hrange = mask.hrange();
		scanline_adapter<rendition_adapter> scanline(ra, _scanline_cache, hrange.second - hrange.first + 1);

		render(scanline, mask, alpha, 0, 1);
	}
}
