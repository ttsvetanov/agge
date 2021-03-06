#include <agge/clipper.h>
#include <agge/curves.h>
#include <agge/filling_rules.h>
#include <agge/path.h>
#include <agge/renderer.h>
#include <agge/rasterizer.h>
#include <agge/stroke.h>
#include <agge/stroke_features.h>

#include <samples/common/shell.h>

using namespace agge;

namespace
{
	template <typename T>
	rect<T> mkrect(T x1, T y1, T x2, T y2)
	{
		rect<T> r = { x1, y1, x2, y2 };
		return r;
	}

	class Figures : public application
	{
	public:
		Figures()
		{
			line_style.width(4.0f);
			line_style.set_cap(caps::butt());
			line_style.set_join(joins::bevel());
		}

	private:
		virtual void draw(platform_bitmap &surface, timings &/*timings*/)
		{
			ras.reset();

			qbezier::iterator bi(10.0f, 150.0f, 440.0f, 300.0f, 200.0f, 150.0f, 0.02f);
			path_generator_adapter<qbezier::iterator, stroke> bezier_line(bi, line_style);

			add_path(ras, bezier_line);

			ras.sort();

			fill(surface, mkrect<int>(0, 0, surface.width(), surface.height()), platform_blender_solid_color(0, 50, 100));
			ren(surface, 0 /*no windowing*/, ras /*mask*/, platform_blender_solid_color(255, 255, 255), winding<>());
		}

	private:
		rasterizer< clipper<int> > ras;
		renderer ren;
		stroke line_style;
	};
}

application *agge_create_application(services &/*s*/)
{
	return new Figures;
}
