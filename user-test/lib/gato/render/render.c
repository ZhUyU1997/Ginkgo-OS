#include <math.h>	// fmaxf(), sinf(), cosf()
#include <stdlib.h> // abs()
#include <string.h> // memset()
#include <time.h>
#include <assert.h>
#include "gato/color.h"
#include "gato/svg.h"
#include "gato/render.h"

#include "gato/surface.h"
#include "gato/image.h"
#include "gato/macro.h"

typedef struct active_edge_t
{
	float x;
	int dir;
} active_edge_t;

static float capsule_sdf(point_t p, point_t a, point_t b, float r)
{
	point_t pa = point_sub_point(p, a);
	point_t ba = point_sub_point(b, a);
	float h = fmaxf(fminf((pa.x * ba.x + pa.y * ba.y) / (ba.x * ba.x + ba.y * ba.y), 1.0f), 0.0f);
	float dx = pa.x - ba.x * h, dy = pa.y - ba.y * h;
	return sqrtf(dx * dx + dy * dy) - r;
}

static int draw_sdf_line(surface_t *s, point_t p1, point_t p2, color_t color, float thickness)
{
	int r = thickness / 2;
	int x0 = (int)floorf(fminf(p1.x, p2.x) - r);
	int x1 = (int)ceilf(fmaxf(p1.x, p2.x) + r);
	int y0 = (int)floorf(fminf(p1.y, p2.y) - r);
	int y1 = (int)ceilf(fmaxf(p1.y, p2.y) + r);
	for (int y = y0; y <= y1; y++)
		for (int x = x0; x <= x1; x++)
		{
			float a = fmaxf(fminf(0.5f - capsule_sdf((point_t){x, y}, (point_t){p1.x, p1.y}, (point_t){p2.x, p2.y}, r), 1.0f), 0.0f);
			surface_pixel_set(s, (color_t){color.b, color.g, color.r, color.a * a}, x, y);
		}
	
	return 0;
}

static int cmp_edge(const void *restrict p, const void *restrict q)
{
	const edge_t *restrict a = (const edge_t *)p;
	const edge_t *restrict b = (const edge_t *)q;
	if (a->start.y < b->start.y)
		return -1;
	if (a->start.y > b->start.y)
		return 1;
	return 0;
}

static int cmp_info(const void *restrict p, const void *restrict q)
{
	active_edge_t *restrict a = (active_edge_t *)p;
	active_edge_t *restrict b = (active_edge_t *)q;

	if (a->x != b->x)
		return a->x > b->x ? 1 : -1;

	return 0;
}

static void fill_scanline(context_t *ctx, int *scanline, float x0, float x1, int y)
{
	int i0, i1;
	i0 = fclampf(roundf(x0 - 0.5), 0, ctx->s->width - 1);
	i1 = fclampf(roundf(x1 + 0.5), 0, ctx->s->width - 1);
	float center = (x0 + x1) / 2.0;
	float r = 0.5 + fabsf(x0 - x1) / 2.0;

	while (i0 <= i1)
	{
		float d = r - fabsf(i0 - center);
		float w = d > 1.0f ? 1.0f : (d < 0.0f ? 0.0f : d);
		if (w == 0.0f)
		{
			i0++;
			continue;
		}

		scanline[i0++] += w * 51; // 51 == 255/5

		if (w == 1.0f)
			break;
	}

	while (i0 <= i1)
	{
		float d = r - fabsf(i1 - center);
		float w = d > 1.0f ? 1.0f : (d < 0.0f ? 0.0f : d);
		if (w == 0.0f)
		{
			i1--;
			continue;
		}

		scanline[i1--] += w * 51; // 51 == 255/5

		if (w == 1.0f)
			break;
	}

	for (int i = i0; i <= i1; i++)
		scanline[i] += 51; // 51 == 255/5
}

static void rasterize_sorted_edges(context_t *ctx, edge_t es[], int elen, color_t color)
{

	//FIXME: dynamic alloc
	struct active_edge_t *a = malloc(sizeof(active_edge_t) * 100);
	int na_edge = 100;

	int width = ctx->s->width;
	int *scanline = malloc(sizeof(int) * width);

	assert(a && scanline);
	qsort(es, elen, sizeof(edge_t), cmp_edge);

	for (int y = 0; y < ctx->s->height; y++)
	{
		memset(scanline, 0, sizeof(int) * width);
		for (int s = 0; s < 5; s++)
		{
			float scan_y = s * 0.2 + y + ctx->origin.y;
			int count = 0;
			//TODO:optimize
			for (int i = 0; i < elen; i++)
			{
				edge_t e = es[i];
				if (scan_y >= e.start.y)
				{
					if (scan_y < e.end.y)
					{
						a[count].x = e.start.x + (scan_y - e.start.y) * (e.end.x - e.start.x) / (e.end.y - e.start.y);
						a[count].dir = e.dir;
						count++;

						if (count >= na_edge)
						{
							na_edge *= 2;
							a = realloc(a, sizeof(active_edge_t) * na_edge);
						}
					}
				}
				else
				{
					break;
				}
			}
			qsort(a, count, sizeof(struct active_edge_t), cmp_info);

			if (ctx->rule == FILLRULE_EVENODD)
			{
				for (int i = 0; i < count - 1; i++)
				{
					if (a[i].x != a[i + 1].x)
					{
						fill_scanline(ctx, scanline, a[i].x - ctx->origin.x, a[i + 1].x - ctx->origin.x, y);
						i++;
					}
				}
			}
			else
			{
				float x0 = 0;
				int w = 0;
				for (int i = 0; i < count; i++)
				{
					if (w == 0)
					{
						x0 = a[i].x;
						w += a[i].dir;
					}
					else
					{
						float x1 = a[i].x;
						w += a[i].dir;

						if (w == 0)
						{
							fill_scanline(ctx, scanline, x0 - ctx->origin.x, x1 - ctx->origin.x, y);
						}
					}
				}
			}
		}
		color_t *dl = ctx->s->pixels + y * width;
		color_t c = color;
		int a = color.a;
		for (int x = 0; x < width; x++)
		{
			dl[x] = c;
			dl[x].a = scanline[x];
		}
	}

	if (a)
		free(a);
	if (scanline)
		free(scanline);
}

static void context_surface_set(context_t *ctx, surface_t *s)
{
	ctx->s = s;
}

static void context_reset(context_t *ctx)
{
	ctx->nes = 0;
	ctx->ne_ps = 0;
	ctx->min.y = ctx->min.x = 1000000;
	ctx->max.y = ctx->max.x = -1000000;
	ctx->origin.y = ctx->origin.x = 0;
}

static void context_clear(context_t *ctx)
{
	ctx->nes = 0;
}

static void context_line_width_set(context_t *ctx, float line_width)
{
	ctx->width = line_width;
}

static void context_cap_style_set(context_t *ctx, enum line_cap_t cap)
{
	ctx->cap = cap;
}

static void context_join_style_set(context_t *ctx, enum line_join_t join)
{
	ctx->join = join;
}

static void context_fill_rule_set(context_t *ctx, enum fill_rule_t rule)
{
	ctx->rule = rule;
}

static void context_surface_alloc(context_t *ctx)
{

	ctx->min.x = floorf(ctx->min.x);
	ctx->min.y = floorf(ctx->min.y);
	ctx->max.x = ceilf(ctx->max.x);
	ctx->max.y = ceilf(ctx->max.y);

	if (ctx->s)
	{
		int x = ctx->origin.x - ctx->min.x;
		int y = ctx->origin.y - ctx->min.y;
		int w = ctx->max.x - ctx->min.x + 1;
		int h = ctx->max.y - ctx->min.y + 1;
		surface_t *s = surface_clone(ctx->s, x, y, w, h);
		surface_free(ctx->s);
		ctx->s = s;
	}
	else
	{
		ctx->s = surface_alloc(ctx->max.x - ctx->min.x + 1, ctx->max.y - ctx->min.y + 1);
		assert(ctx->s != NULL);
	}
	ctx->origin = ctx->min;
}

static void context_surface_free(context_t *ctx)
{
	context_clear(ctx);
	ctx->origin.y = ctx->origin.x = 0;

	if (ctx->s)
	{
		surface_free(ctx->s);
		ctx->s = NULL;
	}
}

static void set_point_path_type(context_t *ctx, enum point_path_type_t pp_type)
{
	if (ctx->ne_ps > 0)
		ctx->e_ps[ctx->ne_ps - 1].pp_type = pp_type;
}

static void close_path(context_t *ctx)
{
	set_point_path_type(ctx, POINT_PATH_CLOSE);
}

static void add_point(context_t *ctx, float x, float y, enum point_path_type_t pp_type)
{
	ctx->e_ps[ctx->ne_ps] = (expand_point_t){0};
	ctx->e_ps[ctx->ne_ps].p = (point_t){x, y};
	ctx->e_ps[ctx->ne_ps].pp_type = pp_type;
	ctx->ne_ps++;

	if (ctx->ne_ps >= ctx->se_ps)
	{
		ctx->se_ps *= 2;
		ctx->e_ps = realloc(ctx->e_ps, sizeof(expand_point_t) * ctx->se_ps);
	}
	assert(x >= -1000000 && x <= 1000000);
	assert(y >= -1000000 && y <= 1000000);

	ctx->min = point_min(ctx->min, (point_t){x, y});
	ctx->max = point_max(ctx->max, (point_t){x, y});
}

static void add_edge(context_t *ctx, point_t a, point_t b)
{
	int dir = 0;

	if (a.y < b.y)
		dir = 1;
	else
		dir = -1;

	ctx->min = point_min(ctx->min, a);
	ctx->min = point_min(ctx->min, b);
	ctx->max = point_max(ctx->max, a);
	ctx->max = point_max(ctx->max, b);

	if (a.y < b.y)
	{
		ctx->es[ctx->nes++] = (edge_t){a, b, dir};
	}
	else
	{
		ctx->es[ctx->nes++] = (edge_t){b, a, dir};
	}

	if (ctx->nes >= ctx->ses)
	{
		ctx->ses *= 2;
		ctx->e_ps = realloc(ctx->e_ps, sizeof(edge_t) * ctx->ses);
	}
}

static void move_to(context_t *ctx, float x, float y)
{
	add_point(ctx, x, y, POINT_PATH_BEGIN);
}

static void line_to(context_t *ctx, float x, float y)
{
	add_point(ctx, x, y, POINT_PATH);
}

static void cubic_bez(context_t *ctx, point_t p1, point_t p2, point_t p3, point_t p4, int level)
{
	if (level > 10)
		return;

	point_t d = point_sub_point(p4, p1);
	point_t d24 = point_sub_point(p2, p4);
	point_t d34 = point_sub_point(p3, p4);

	float d2 = fabsf(d24.x * d.y - d24.y * d.x);
	float d3 = fabsf(d34.x * d.y - d34.y * d.x);
	float S1 = d.x * d.x + d.y * d.y;
	float S2 = (d2 + d3) * (d2 + d3);

	if (S2 <= 0.25 * S1)
	{
		line_to(ctx, p4.x, p4.y);
		return;
	}

	point_t p12 = constant_point_add_point(p1, p2, 0.5);
	point_t p23 = constant_point_add_point(p2, p3, 0.5);
	point_t p34 = constant_point_add_point(p3, p4, 0.5);
	point_t p123 = constant_point_add_point(p12, p23, 0.5);
	point_t p234 = constant_point_add_point(p23, p34, 0.5);
	point_t p1234 = constant_point_add_point(p123, p234, 0.5);

	cubic_bez(ctx, p1, p12, p123, p1234, level + 1);
	cubic_bez(ctx, p1234, p234, p34, p4, level + 1);
}

static void cubic_bezto(context_t *ctx, point_t p1, point_t p2, point_t p3, point_t p4)
{
	cubic_bez(ctx, p1, p2, p3, p4, 0);
}

static void q_bezto(context_t *ctx, point_t p1, point_t p2, point_t p3, int level)
{
	if (level > 10)
		return;

	point_t d = point_sub_point(p3, p1);
	point_t d23 = point_sub_point(p2, p3);

	float d2 = fabsf(d23.x * d.y - d23.y * d.x);
	float S1 = d.x * d.x + d.y * d.y;
	float S2 = 4 * d2 * d2;

	if (S2 <= 0.25 * S1)
	{
		line_to(ctx, p3.x, p3.y);
		return;
	}

	point_t p12 = constant_point_add_point(p1, p2, 0.5);
	point_t p23 = constant_point_add_point(p2, p3, 0.5);
	point_t p123 = constant_point_add_point(p12, p23, 0.5);

	q_bezto(ctx, p1, p12, p123, level + 1);
	q_bezto(ctx, p123, p23, p3, level + 1);
}

static void quad_bezto(context_t *ctx, point_t p1, point_t p2, point_t p3)
{
	q_bezto(ctx, p1, p2, p3, 0);
}

static void arc_tobez(float a1, float da, point_t *v1, point_t *v2, point_t *h2)
{
	point_t *h1 = &(point_t){cosf(a1), sinf(a1)}, t1, t2;
	float h = 0;
	float a = M_PI / 2;

	*h2 = (point_t){cosf(da) * h1->x - sinf(da) * h1->y, sinf(da) * h1->x + cosf(da) * h1->y};
	h = 4.0 * tanf(da / 4) / 3.0;
	t1 = (point_t){-sinf(a) * h1->y, sinf(a) * h1->x};
	t2 = (point_t){sinf(a) * h2->y, -sinf(a) * h2->x};

	*v1 = point_add_point(*h1, point_mul_factor(t1, h));
	*v2 = point_add_point(*h2, point_mul_factor(t2, h));
}

static point_t unit_c2arc(float rx, float ry, float rotation, point_t p)
{
	float cos = cosf(rotation);
	float sin = sinf(rotation);
	p = (point_t){p.x * rx, p.y * ry};
	return (point_t){cos * p.x - sin * p.y, sin * p.x + cos * p.y};
}

static point_t arc2unit_c(float rx, float ry, float rotation, point_t p)
{
	float cos = cosf(-rotation);
	float sin = sinf(-rotation);
	p = (point_t){cos * p.x - sin * p.y, sin * p.x + cos * p.y};
	return (point_t){p.x / rx, p.y / ry};
}

//https://www.w3.org/TR/SVG/implnote.html#ArcImplementationNotes
static void arc_to(context_t *ctx, float rx, float ry, float rotation, int large, int sweep, point_t p0, point_t p1)
{
	point_t dp, cp;
	rotation *= M_PI / 180.0;

	if (rx == 0 || ry == 0)
	{
		line_to(ctx, p1.x, p1.y);
		return;
	}

	rx = fabsf(rx);
	ry = fabsf(ry);

	dp = point_sub_point(p1, p0);
	cp = point_mul_factor(point_add_point(p1, p0), 0.5);

	float v = sqrtf((dp.x / 2.0) * (dp.x / 2.0) / (rx * rx) + (dp.y / 2.0) * (dp.y / 2.0) / (ry * ry));

	if (v > 1)
	{
		rx = v * rx;
		ry = v * ry;
	}

	dp = arc2unit_c(rx, ry, rotation, dp);
	float l = sqrtf(dp.x * dp.x + dp.y * dp.y);
	float sin = fclampf(l / 2, -1.0, 1.0);
	float cos = sqrtf(1 - sin * sin);
	point_t c;
	c = point_mul_factor(unit_point((large == sweep) ? (point_t){dp.y, -dp.x} : (point_t){-dp.y, dp.x}), cos);
	c = point_add_point(cp, unit_c2arc(rx, ry, rotation, c));

	point_t start = arc2unit_c(rx, ry, rotation, point_sub_point(p0, c));
	point_t end = arc2unit_c(rx, ry, rotation, point_sub_point(p1, c));

	float a0 = acosf(fclampf(point_dot_point((point_t){1, 0}, start), -1.0, 1.0));
	a0 = point_cross_point((point_t){1, 0}, start) < 0 ? -a0 : a0;
	float da = acosf(fclampf(point_dot_point(start, end), -1.0, 1.0));
	da = point_cross_point(start, end) < 0 ? -da : da;
	da = fmodf(da, 2 * M_PI);

	if (sweep)
	{
		if (da < 0)
			da = ((da < 0) ? 1 : -1) * 2 * M_PI + da;
	}
	else
	{
		if (da > 0)
			da = -2 * M_PI + da;
	}

	int n = fabsf(da) * 2.0 / M_PI;
	float step = (da > 0) ? (M_PI / 2.0) : (-M_PI / 2.0);

	point_t h1 = start;
	for (int i = 0; i < n; i++, a0 += step, da -= step)
	{
		point_t v1, v2, h2;
		arc_tobez(a0, step, &v1, &v2, &h2);
		cubic_bezto(ctx,
					point_add_point(c, unit_c2arc(rx, ry, rotation, h1)),
					point_add_point(c, unit_c2arc(rx, ry, rotation, v1)),
					point_add_point(c, unit_c2arc(rx, ry, rotation, v2)),
					point_add_point(c, unit_c2arc(rx, ry, rotation, h2)));

		h1 = h2;
	}

	if ((step > 0 && da > 0) || (step < 0 && da < 0))
	{
		point_t v1, v2, h2;
		arc_tobez(a0, da, &v1, &v2, &h2);
		cubic_bezto(ctx,
					point_add_point(c, unit_c2arc(rx, ry, rotation, h1)),
					point_add_point(c, unit_c2arc(rx, ry, rotation, v1)),
					point_add_point(c, unit_c2arc(rx, ry, rotation, v2)),
					point_add_point(c, unit_c2arc(rx, ry, rotation, h2)));
	}
}

//FIXME: check h1, h2 not in once line
static point_t calc_intersection(point_t p1, point_t p2, point_t h1, point_t h2)
{
	return point_add_point(p1, point_mul_factor(h1, point_cross_point(point_sub_point(p2, p1), h2) / point_cross_point(h1, h2)));
}

static void add_bezier_edge(context_t *ctx, point_t p1, point_t p2, point_t p3, point_t p4, int level)
{
	if (level > 10)
		return;

	point_t d = point_sub_point(p4, p1);
	point_t d24 = point_sub_point(p2, p4);
	point_t d34 = point_sub_point(p3, p4);

	float d2 = fabsf(d24.x * d.y - d24.y * d.x);
	float d3 = fabsf(d34.x * d.y - d34.y * d.x);
	float S1 = d.x * d.x + d.y * d.y;
	float S2 = (d2 + d3) * (d2 + d3);

	if (S2 < 0.25 * S1)
	{
		add_edge(ctx, p1, p4);
		return;
	}

	point_t p12 = constant_point_add_point(p1, p2, 0.5);
	point_t p23 = constant_point_add_point(p2, p3, 0.5);
	point_t p34 = constant_point_add_point(p3, p4, 0.5);
	point_t p123 = constant_point_add_point(p12, p23, 0.5);
	point_t p234 = constant_point_add_point(p23, p34, 0.5);
	point_t p1234 = constant_point_add_point(p123, p234, 0.5);

	add_bezier_edge(ctx, p1, p12, p123, p1234, level + 1);
	add_bezier_edge(ctx, p1234, p234, p34, p4, level + 1);
}

static void mitter_join(context_t *ctx, expand_point_t *ep0, expand_point_t *ep1)
{
	add_edge(ctx, ep0->p2, ep1->p1);
	add_edge(ctx, ep1->p3, ep0->p4);

	if (ep1->type == POINT_CORNER_LEFT)
	{
		add_edge(ctx, ep0->p1, ep0->cusp);
		add_edge(ctx, ep0->cusp, ep0->p2);
		add_edge(ctx, ep0->p4, ep0->p3);
	}
	else
	{
		add_edge(ctx, ep0->p4, ep0->cusp);
		add_edge(ctx, ep0->cusp, ep0->p3);
		add_edge(ctx, ep0->p1, ep0->p2);
	}
}

static void bevel_join(context_t *ctx, expand_point_t *ep0, expand_point_t *ep1)
{
	add_edge(ctx, ep0->p2, ep1->p1);
	add_edge(ctx, ep1->p3, ep0->p4);

	add_edge(ctx, ep0->p4, ep0->p3);
	add_edge(ctx, ep0->p1, ep0->p2);
}

static void round_join(context_t *ctx, expand_point_t *ep0, expand_point_t *ep1)
{
	float da = acosf(point_dot_point(ep0->h, ep1->h));
	float h = (ctx->width / 2.0) * (4.0 * tanf(da / 4) / 3.0);
	point_t h1, h2;

	add_edge(ctx, ep0->p2, ep1->p1);
	add_edge(ctx, ep1->p3, ep0->p4);

	if (ep0->type == POINT_CORNER_LEFT)
	{
		h1 = point_add_point(ep0->p1, point_mul_factor(ep0->h, h));
		h2 = point_add_point(ep0->p2, point_mul_factor(ep1->h, -h));
		add_bezier_edge(ctx, ep0->p1, h1, h2, ep0->p2, 0);
		add_edge(ctx, ep0->p4, ep0->p3);
	}
	else
	{
		h1 = point_add_point(ep0->p3, point_mul_factor(ep0->h, h));
		h2 = point_add_point(ep0->p4, point_mul_factor(ep1->h, -h));
		add_bezier_edge(ctx, ep0->p4, h2, h1, ep0->p3, 0);
		add_edge(ctx, ep0->p1, ep0->p2);
	}
}

static void butt_cap(context_t *ctx, expand_point_t *ep0, expand_point_t *ep1, int head)
{
	if (head)
	{
		add_edge(ctx, ep0->p2, ep1->p1);
		add_edge(ctx, ep1->p3, ep0->p4);
		add_edge(ctx, ep0->p4, ep0->p2);
	}
	else
	{
		add_edge(ctx, ep0->p1, ep0->p3);
	}
}

static void square_cap(context_t *ctx, expand_point_t *ep0, expand_point_t *ep1, int head)
{
	point_t h1, h2;
	float w = (ctx->width / 2.0);

	if (head)
	{
		h1 = point_add_point(ep0->p2, point_mul_factor(ep1->h, -w));
		h2 = point_add_point(ep0->p4, point_mul_factor(ep1->h, -w));
		add_edge(ctx, ep0->p2, ep1->p1);
		add_edge(ctx, ep1->p3, ep0->p4);

		add_edge(ctx, ep0->p4, h2);
		add_edge(ctx, h1, ep0->p2);
		add_edge(ctx, h2, h1);
	}
	else
	{
		h1 = point_add_point(ep0->p1, point_mul_factor(ep0->h, w));
		h2 = point_add_point(ep0->p3, point_mul_factor(ep0->h, w));

		add_edge(ctx, ep0->p1, h1);
		add_edge(ctx, h1, h2);
		add_edge(ctx, h2, ep0->p3);
	}
}

static void round_cap(context_t *ctx, expand_point_t *ep0, expand_point_t *ep1, int head)
{
	point_t h[5];
	float w = (ctx->width / 2.0);

	if (head)
	{
		add_edge(ctx, ep0->p2, ep1->p1);
		add_edge(ctx, ep1->p3, ep0->p4);

		h[0] = point_add_point(ep0->p4, point_mul_factor(ep1->h, -w * KAPPA90));
		h[2] = point_add_point(ep0->p, point_mul_factor(ep1->h, -w));
		h[4] = point_add_point(ep0->p2, point_mul_factor(ep1->h, -w * KAPPA90));
		h[1] = point_add_point(h[2], point_mul_factor(ep1->v, -w * KAPPA90));
		h[3] = point_add_point(h[2], point_mul_factor(ep1->v, w * KAPPA90));

		add_bezier_edge(ctx, ep0->p4, h[0], h[1], h[2], 0);
		add_bezier_edge(ctx, h[2], h[3], h[4], ep0->p2, 0);
	}
	else
	{
		h[0] = point_add_point(ep0->p1, point_mul_factor(ep0->h, w * KAPPA90));
		h[2] = point_add_point(ep0->p, point_mul_factor(ep0->h, w));
		h[4] = point_add_point(ep0->p3, point_mul_factor(ep0->h, w * KAPPA90));
		h[1] = point_add_point(h[2], point_mul_factor(ep0->v, w * KAPPA90));
		h[3] = point_add_point(h[2], point_mul_factor(ep0->v, -w * KAPPA90));

		add_bezier_edge(ctx, ep0->p1, h[0], h[1], h[2], 0);
		add_bezier_edge(ctx, h[2], h[3], h[4], ep0->p3, 0);
	}
}

static void prepare_stroke(context_t *ctx, int s, int ne_ps)
{
	ctx->nes = 0;
	expand_point_t *e_ps = ctx->e_ps + s;

	for (int i = 0; i < ne_ps; i++)
	{
		point_t p1 = e_ps[i % ne_ps].p;
		point_t p2 = e_ps[(i + 1) % ne_ps].p;
		point_t d = unit_point(point_sub_point(p2, p1)); //FIXME: p2 = p1
		e_ps[(i + 1) % ne_ps].v = (point_t){-d.y, d.x};
		e_ps[(i + 1) % ne_ps].h = d;
	}

	for (int i = 0; i < ne_ps; i++)
	{
		expand_point_t *e_p0 = &e_ps[i % ne_ps];
		expand_point_t *e_p1 = &e_ps[(i + 1) % ne_ps];
		float w = ctx->width / 2.0;

		point_t p = e_p0->p;
		point_t v1 = e_p0->v;
		point_t v2 = e_p1->v;
		point_t h1 = e_p0->h;
		point_t h2 = e_p1->h;

		point_t t1 = point_mul_factor(v1, w);
		point_t t2 = point_mul_factor(v2, w);
		point_t t3 = point_mul_factor(v1, -w);
		point_t t4 = point_mul_factor(v2, -w);

		e_p0->p1 = point_add_point(p, t1);
		e_p0->p2 = point_add_point(p, t2);
		e_p0->p3 = point_add_point(p, t3);
		e_p0->p4 = point_add_point(p, t4);

		float cross = point_cross_point(h1, h2);

		if (cross > 0.0)
		{
			e_p0->type = POINT_CORNER_RIGHT;
			e_p0->cusp = calc_intersection(e_p0->p3, e_p0->p4, h1, h2);
		}
		else if (cross < 0.0)
		{
			e_p0->type = POINT_CORNER_LEFT;
			e_p0->cusp = calc_intersection(e_p0->p1, e_p0->p2, h1, h2);
		}
		else
		{
			e_p0->type = POINT_CORNER_LEFT;
			e_p0->cusp = point_add_point(p, t1);
		}
	}
}

static void expand_stroke(context_t *ctx, int s, int ne_ps)
{
	expand_point_t *e_ps = ctx->e_ps + s;
	int close = e_ps[ne_ps - 1].pp_type == POINT_PATH_CLOSE;

	for (int i = 0; i < ne_ps; i++)
	{
		if ((!close) && (i == 0 || i == ne_ps - 1))
		{
			if (ctx->cap == CAP_ROUND)
				round_cap(ctx, &e_ps[i % ne_ps], &e_ps[(i + 1) % ne_ps], i == 0);
			else if (ctx->cap == CAP_SQUARE)
				square_cap(ctx, &e_ps[i % ne_ps], &e_ps[(i + 1) % ne_ps], i == 0);
			else if (ctx->cap == CAP_BUTT)
				butt_cap(ctx, &e_ps[i % ne_ps], &e_ps[(i + 1) % ne_ps], i == 0);
		}
		else
		{
			if (ctx->join == JOIN_BEVEL)
				bevel_join(ctx, &e_ps[i % ne_ps], &e_ps[(i + 1) % ne_ps]);
			else if (ctx->join == JOIN_MITER)
				mitter_join(ctx, &e_ps[i % ne_ps], &e_ps[(i + 1) % ne_ps]);
			else if (ctx->join == JOIN_ROUND)
				round_join(ctx, &e_ps[i % ne_ps], &e_ps[(i + 1) % ne_ps]);
		}
	}
}

static void add_path(context_t *ctx, int s, int ne_ps)
{
	expand_point_t *e_ps = ctx->e_ps + s;

	if (ne_ps < 2)
		return;

	int close = e_ps[ne_ps - 1].pp_type == POINT_PATH_CLOSE;

	for (int i = 0; i < ne_ps - (!close); i++)
	{
		add_edge(ctx, e_ps[i % ne_ps].p, e_ps[(i + 1) % ne_ps].p);
	}
}

static int context_fill(context_t *ctx, color_t color)
{
	for (int i = 1, begin = 0; i < ctx->ne_ps; i++)
	{
		if (ctx->e_ps[i % ctx->ne_ps].pp_type == POINT_PATH_BEGIN)
		{
			add_path(ctx, begin, i - begin);
			begin = i;
		}
		else if (i == ctx->ne_ps - 1)
		{
			add_path(ctx, begin, i - begin + 1);
		}
	}

	context_surface_alloc(ctx);
	rasterize_sorted_edges(ctx, ctx->es, ctx->nes, color);
	context_clear(ctx);
	return 1;
}

static int context_stroke(context_t *ctx, color_t color, float width)
{
	if (width == 0.0)
		return 0;

	context_line_width_set(ctx, width);

	for (int i = 1, begin = 0; i < ctx->ne_ps; i++)
	{
		if (ctx->e_ps[i % ctx->ne_ps].pp_type == POINT_PATH_BEGIN)
		{
			prepare_stroke(ctx, begin, i - begin);
			expand_stroke(ctx, begin, i - begin);
			begin = i;
		}
		else if (i == ctx->ne_ps - 1)
		{
			prepare_stroke(ctx, begin, i - begin + 1);
			expand_stroke(ctx, begin, i - begin + 1);
		}
	}

	context_surface_alloc(ctx);
	rasterize_sorted_edges(ctx, ctx->es, ctx->nes, color);
	context_clear(ctx);
	return 1;
}

static void render(context_t *ctx, float x, float y, style_t style)
{
	color_t fill_color = style.fill_color;
	color_t stroke_color = style.stroke_color;
	surface_t *shadow = NULL;
	surface_t *bg = NULL;
	surface_t *copy_fill = NULL;
	surface_t *copy_stroke = NULL;
	surface_t *shape = NULL;
	point_t origin_shadow = {0};
	point_t origin_stroke = {0};
	point_t origin_fill = {0};
	point_t origin_shape = {0};

	if (context_stroke(ctx, stroke_color, style.stroke_width))
	{
		copy_stroke = surface_copy(ctx->s);
		origin_stroke = ctx->origin;
		surface_clear(ctx->s, ARGB(0), 0, 0, ctx->s->width, ctx->s->height);
	}

	if (context_fill(ctx, fill_color))
	{
		assert(ctx->s);
		copy_fill = surface_copy(ctx->s);
		origin_fill = ctx->origin;
	}

	if (copy_stroke)
	{
		shape = surface_copy(copy_stroke);
		surface_mono(shape, fill_color);
		surface_blit(copy_fill, shape, 0, 0);
		surface_cover(shape, copy_fill, 0, 0);
		origin_shape = origin_fill;
	}
	else if (copy_fill)
	{
		shape = surface_copy(copy_fill);
		origin_shape = origin_fill;
	}

	if (copy_stroke || copy_fill)
	{
		surface_t *base = NULL;
		point_t origin = {0};
		//TODO: No set fill color, still show image
		if (copy_fill && style.clip_image != 0)
		{
			surface_mask(copy_fill, style.clip_image, ceilf(x - ctx->origin.x), ceilf(y - ctx->origin.y));
		}

		if (style.background_blur != 0.0f) //TODO: conflict with clip image, shadow
		{
			bg = surface_copy(shape);
			surface_t *cp = surface_copy(shape);
			surface_cover(cp, ctx->base, ceilf(-origin_shape.x), ceilf(-origin_shape.y));
			surface_filter_blur(cp, style.background_blur);
			surface_mask(bg, cp, 0, 0);
			surface_free(cp);
		}

		if (style.n_shadow > 0)
		{
			float max_range = 0;
			for (int i = 0; i < style.n_shadow; i++)
			{
				max_range = fmaxf(max_range, style.shadow[i].blur * 2);
			}
			origin_shadow = point_add_point(origin_shape, (point_t){-max_range, -max_range});

			shadow = surface_alloc(ctx->s->width + 2 * max_range, ctx->s->height + 2 * max_range);

			for (int i = 0; i < style.n_shadow; i++)
			{
				//TODO: How to calc shadow's range
				float range = style.shadow[i].blur * 2;
				surface_t *s = surface_clone(shape, range, range, shape->width + 2 * range, shape->height + 2 * range);
				surface_mono(s, style.shadow[i].color);
				surface_filter_blur(s, style.shadow[i].blur);
				surface_blit(shadow, s, max_range - range + style.shadow[i].shadow_h, max_range - range + style.shadow[i].shadow_v);
				surface_free(s);
			}
		}

		if (shadow)
			base = shadow;
		else if (bg)
			base = bg;
		else if (copy_fill)
			base = copy_fill;
		else if (copy_stroke)
			base = copy_fill;

		if (shadow)
			origin = origin_shadow;
		else
			origin = origin_shape;

		if (bg && (bg != base))
			surface_blit(base, bg, origin_shape.x - origin.x, origin_shape.y - origin.y);

		if (shadow && (!bg))
			surface_composite_out(base, copy_fill, origin_shape.x - origin.x, origin_shape.y - origin.y);

		if (copy_fill != base)
			surface_blit_with_opacity(base, copy_fill, origin_shape.x - origin.x, origin_shape.y - origin.y, style.fill_color.a);
		else
			surface_filter_opacity(copy_fill, style.fill_color.a);

		if (copy_stroke)
			surface_blit_with_opacity(base, copy_stroke, origin_shape.x - origin.x, origin_shape.y - origin.y, style.stroke_color.a);
		surface_blit(ctx->base, base, origin.x, origin.y);
	}
	context_surface_free(ctx);

	if (copy_stroke)
		surface_free(copy_stroke);
	if (copy_fill)
		surface_free(copy_fill);
	if (shape)
		surface_free(shape);
	if (bg)
		surface_free(bg);
	if (shadow)
		surface_free(shadow);
}

void context_init(context_t *ctx, surface_t *base)
{
	*ctx = (context_t){0};
	ctx->base = base;
	ctx->min.y = ctx->min.x = 1000000;
	ctx->max.y = ctx->max.x = -1000000;
	ctx->se_ps = 1024;
	ctx->ses = 1024;
	ctx->e_ps = malloc(sizeof(expand_point_t) * 1024);
	ctx->es = malloc(sizeof(edge_t) * 1024);
	ctx->move_to = move_to;
	ctx->line_to = line_to;
	ctx->cubic_bezto = cubic_bezto;
	ctx->quad_bezto = quad_bezto;
	ctx->arc_to = arc_to;
	ctx->close_path = close_path;
	ctx->render = render;
}

void context_exit(context_t *ctx)
{
	if (ctx->s)
		surface_free(ctx->s);
	if (ctx->e_ps)
		free(ctx->e_ps);
	if (ctx->es)
		free(ctx->es);
	*ctx = (context_t){0};
}