#ifndef _COLORCONVERSION_H
#define _COLORCONVERSION_H

namespace Color
{
	   typedef struct {
		double r;       // percent
		double g;       // percent
		double b;       // percent
	} rgb;
	typedef struct {
		double r;       // percent
		double g;       // percent
		double b;       // percent
		double a;		// percent
	} rgba;

		typedef struct {
		double h;       // angle in degrees
		double s;       // percent
		double v;       // percent
	} hsv;

	inline hsv rgb2hsv(rgb in)
	{
		hsv         out;
		double      min, max, delta;

		min = in.r < in.g ? in.r : in.g;
		min = min  < in.b ? min  : in.b;

		max = in.r > in.g ? in.r : in.g;
		max = max  > in.b ? max  : in.b;

		out.v = max;                                // v
		delta = max - min;
		if( max > 0.0 ) {
			out.s = (delta / max);                  // s
		} else {
			// r = g = b = 0                        // s = 0, v is undefined
			out.s = 0.0;
			out.h = -1.0;                            // its now undefined
			return out;
		}
		if( in.r >= max )                           // > is bogus, just keeps compilor happy
			out.h = ( in.g - in.b ) / delta;        // between yellow & magenta
		else
		if( in.g >= max )
			out.h = 2.0 + ( in.b - in.r ) / delta;  // between cyan & yellow
		else
			out.h = 4.0 + ( in.r - in.g ) / delta;  // between magenta & cyan

		out.h *= 60.0;                              // degrees

		if( out.h < 0.0 )
			out.h += 360.0;

		return out;
	}


	inline rgb hsv2rgb(hsv in)
	{
		double      hh, p, q, t, ff;
		long        i;
		rgb         out;

		if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
			if(in.h < 0.0) {   // in.h == NAN
				out.r = in.v;
				out.g = in.v;
				out.b = in.v;
				return out;
			}
			// error - should never happen
			out.r = 0.0;
			out.g = 0.0;
			out.b = 0.0;
			return out;
		}
		hh = in.h;
		if(hh >= 360.0) hh = 0.0;
		hh /= 60.0;
		i = (long)hh;
		ff = hh - i;
		p = in.v * (1.0 - in.s);
		q = in.v * (1.0 - (in.s * ff));
		t = in.v * (1.0 - (in.s * (1.0 - ff)));

		switch(i) {
		case 0:
			out.r = in.v;
			out.g = t;
			out.b = p;
			break;
		case 1:
			out.r = q;
			out.g = in.v;
			out.b = p;
			break;
		case 2:
			out.r = p;
			out.g = in.v;
			out.b = t;
			break;

		case 3:
			out.r = p;
			out.g = q;
			out.b = in.v;
			break;
		case 4:
			out.r = t;
			out.g = p;
			out.b = in.v;
			break;
		case 5:
		default:
			out.r = in.v;
			out.g = p;
			out.b = q;
			break;
		}
		return out;     
	}
	inline rgba blendrgba(rgba c1, rgba c2)
	{
		rgba result;
		double a1 = c1.a;
		double a2 = c2.a;

		result.r =  (a1 * c1.r + a2 * (1 - a1) * c2.r);
		result.g =  (a1 * c1.g + a2 * (1 - a1) * c2.g);
		result.b =  (a1 * c1.b + a2 * (1 - a1) * c2.b);
		result.a =  (1.0 * (a1 + a2 * (1 - a1)));
		return result;
	}

   inline rgb overblendrgb(rgb c1, rgb c2, double alpha)
	{
		rgb result;
		double a1 = alpha;
      double dr = c2.r - c1.r;
      double db = c2.b - c1.b;
      double dg = c2.g - c1.g;

		result.r =  c1.r + a1*dr;
		result.g =  c1.g + a1*dg;
		result.b =  c1.b + a1*db;
		return result;
	}
}
#endif