#include <agge.text/text_engine.h>

#include <agge.text/functional.h>

#include <cctype>
#include <functional>
#include <string>

using namespace std;

namespace agge
{
	namespace
	{
		enum {
			c_rescalable_height = 1000,
		};

		struct nc_compare
		{
			bool operator ()(wchar_t lhs, wchar_t rhs) const
			{	return toupper(lhs) == toupper(rhs);	}
		};
	}

	class text_engine_base::cached_outline_accessor : public font::accessor, noncopyable
	{
	public:
		cached_outline_accessor(const font::accessor_ptr &underlying)
			: _underlying(underlying)
		{	}

	private:
		typedef hash_map< uint16_t, pair<glyph::outline_ptr, glyph::glyph_metrics> > glyphs;

	private:
		virtual font::metrics get_metrics() const
		{	return _underlying->get_metrics();	}

		virtual uint16_t get_glyph_index(wchar_t character) const
		{	return _underlying->get_glyph_index(character);	}

		virtual glyph::outline_ptr load_glyph(uint16_t index, glyph::glyph_metrics &m) const
		{
			glyphs::iterator i = _glyphs.find(index);

			if (_glyphs.end() == i)
			{
				glyph::outline_ptr o = _underlying->load_glyph(index, m);
					
				_glyphs.insert(index, make_pair(o, m), i);
			}
			m = i->second.second;
			return i->second.first;
		}

	private:
		const font::accessor_ptr _underlying;
		mutable glyphs _glyphs;
	};

	struct text_engine_base::font_key
	{
		wstring typeface;
		unsigned height : 20;
		unsigned bold : 1;
		unsigned italic : 1;
		text_engine_base::grid_fit grid_fit : 3;

		bool operator ==(const font_key &rhs) const
		{
			return typeface.size() == rhs.typeface.size()
				&& height == rhs.height
				&& bold == rhs.bold
				&& italic == rhs.italic
				&& grid_fit == rhs.grid_fit
				&& equal(typeface.begin(), typeface.end(), rhs.typeface.begin(), nc_compare());
		}
	};

	struct text_engine_base::font_key_hasher
	{
		size_t operator ()(const text_engine_base::font_key &/*key*/) const
		{
			return 1;
		}
	};

	text_engine_base::text_engine_base(loader &loader_, unsigned /*collection_cycles*/)
		: _loader(loader_), _fonts(new fonts_cache), _scalable_fonts(new scalabale_fonts_cache)
	{
	}

	void text_engine_base::collect()
	{
	}

	font::ptr text_engine_base::create_font(const wchar_t *typeface, int height, bool bold, bool italic, grid_fit gf)
	{
		font_key key = { typeface };
		
		key.height = height;
		key.bold = !!bold;
		key.italic = !!italic;
		key.grid_fit = gf;

		fonts_cache::iterator i = _fonts->find(key);

		if (i != _fonts->end() && !i->second.expired())
			return i->second.lock();
		_fonts->insert(key, weak_ptr<font>(), i);

		pair<font::accessor_ptr, real_t> acc = create_font_accessor(key);
		font::ptr f(new font(acc.first, acc.second), bind(&text_engine_base::on_font_released, this, _1));
		
		i->second = f;
		return f;
	}

	pair<font::accessor_ptr, real_t> text_engine_base::create_font_accessor(font_key fk)
	{
		if (gf_none != fk.grid_fit)
			return make_pair(_loader.load(fk.typeface.c_str(), fk.height, fk.bold, fk.italic, fk.grid_fit), 1.0f);

		scalabale_fonts_cache::iterator i;
		const real_t factor = static_cast<real_t>(fk.height) / c_rescalable_height;
		font::accessor_ptr a;

		fk.height = c_rescalable_height;
		if (_scalable_fonts->insert(fk, weak_ptr<font::accessor>(), i) || i->second.expired())
		{
			a.reset(new cached_outline_accessor(_loader.load(fk.typeface.c_str(), fk.height, fk.bold, fk.italic,
				fk.grid_fit)));
			i->second = a;
		}
		else
		{
			a = i->second.lock();
		}
		return make_pair(a, factor);
	}

	void text_engine_base::on_font_released(font *font_)
	{
		on_before_removed(font_);
		delete font_;
	}


	text_engine_base::offset_conv::offset_conv(const glyph::path_iterator &source, real_t dx, real_t dy)
		: _source(source), _dx(dx), _dy(dy)
	{	}

	void text_engine_base::offset_conv::rewind(unsigned /*id*/)
	{	}

	int text_engine_base::offset_conv::vertex(real_t *x, real_t *y)
	{
		int command = _source.vertex(x, y);

		*x += _dx, *y += _dy;
		return command;
	}
}
