// 28 june 2016
#include "uipriv_unix.h"

#define cellRendererButtonType (cellRendererButton_get_type())
#define cellRendererButton(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), cellRendererButtonType, cellRendererButton))
#define isCellRendererButton(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), cellRendererButtonType))
#define cellRendererButtonClass(class) (G_TYPE_CHECK_CLASS_CAST((class), cellRendererButtonType, cellRendererButtonClass))
#define isCellRendererButtonClass(class) (G_TYPE_CHECK_CLASS_TYPE((class), cellRendererButton))
#define getCellRendererButtonClass(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), cellRendererButtonType, cellRendererButtonClass))

typedef struct cellRendererButton cellRendererButton;
typedef struct cellRendererButtonClass cellRendererButtonClass;

struct cellRendererButton {
	GtkCellRenderer parent_instance;
	char *text;
};

struct cellRendererButtonClass {
	GtkCellRendererClass parent_class;
};

G_DEFINE_TYPE(cellRendererButton, cellRendererButton, GTK_TYPE_CELL_RENDERER)

static void cellRendererButton_init(cellRendererButton *m)
{
	// nothing to do
}

static void cellRendererButton_dispose(GObject *obj)
{
	G_OBJECT_CLASS(cellRendererButton_parent_class)->dispose(obj);
}

static void cellRendererButton_finalize(GObject *obj)
{
	cellRendererButton *c = cellRendererButton(obj);

	if (c->text != NULL) {
		g_free(c->text);
		c->text = NULL;
	}
	G_OBJECT_CLASS(cellRendererButton_parent_class)->finalize(obj);
}

static GtkSizeRequestMode cellRendererButton_get_request_mode(GtkCellRenderer *r)
{
	return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

// this is basically what GtkCellRendererToggle did in 3.10
static GtkStyleContext *setButtonStyle(GtkWidget *widget)
{
	GtkStyleContext *context;

	context = gtk_widget_get_style_context(widget);
	gtk_style_context_save(context);
	gtk_style_context_add_class(context, GTK_STYLE_CLASS_BUTTON);
	return context;
}

// this is based on what GtkCellRendererText does
static void cellRendererButton_get_preferred_width(GtkCellRenderer *r, GtkWidget *widget, gint *minimum, gint *natural)
{
	cellRendererButton *c = cellRendererButton(r);
	gint xpad;
	GtkStyleContext *context;
	PangoLayout *layout;
	PangoRectangle rect;
	gint out;

	gtk_cell_renderer_get_padding(GTK_CELL_RENDERER(c), &xpad, NULL);

	context = setButtonStyle(widget);
	layout = gtk_widget_create_pango_layout(widget, c->text);
	pango_layout_set_width(layout, -1);
	pango_layout_get_extents(layout, NULL, &rect);
	g_object_unref(layout);
	gtk_style_context_restore(context);

	out = 2 * xpad + PANGO_PIXELS_CEIL(rect.width);
	if (minimum != NULL)
		*minimum = out;
	if (natural != NULL)
		*natural = out;
}

// this is based on what GtkCellRendererText does
static void cellRendererButton_get_preferred_height_for_width(GtkCellRenderer *r, GtkWidget *widget, gint width, gint *minimum, gint *natural)
{
	cellRendererButton *c = cellRendererButton(r);
	gint xpad, ypad;
	GtkStyleContext *context;
	PangoLayout *layout;
	gint height;
	gint out;

	gtk_cell_renderer_get_padding(GTK_CELL_RENDERER(c), &xpad, &ypad);

	context = setButtonStyle(widget);
	layout = gtk_widget_create_pango_layout(widget, c->text);
	pango_layout_set_width(layout, ((2 * xpad + width) * PANGO_SCALE));
	pango_layout_get_pixel_size(layout, NULL, &height);
	g_object_unref(layout);
	gtk_style_context_restore(context);

	out = 2 * ypad + height;
	if (minimum != NULL)
		*minimum = out;
	if (natural != NULL)
		*natural = out;
}

// this is basically what GtkCellRendererText does
static void cellRendererButton_get_preferred_height(GtkCellRenderer *r, GtkWidget *widget, gint *minimum, gint *natural)
{
	gint width;

	gtk_cell_renderer_get_preferred_width(r, widget, &width, NULL);
	gtk_cell_renderer_get_preferred_height_for_width(r, widget, width, minimum, natural);
}

// this is based on what GtkCellRendererText does
static void cellRendererButton_get_aligned_area(GtkCellRenderer *r, GtkWidget *widget, GtkCellRendererState flags, const GdkRectangle *cell_area, GdkRectangle *aligned_area)
{
	cellRendererButton *c = cellRendererButton(r);
	gint xpad, ypad;
	GtkStyleContext *context;
	PangoLayout *layout;
	PangoRectangle rect;
	gfloat xalign, yalign;
	gint xoffset, yoffset;

	gtk_cell_renderer_get_padding(GTK_CELL_RENDERER(c), &xpad, &ypad);

	context = setButtonStyle(widget);
	layout = gtk_widget_create_pango_layout(widget, c->text);
	pango_layout_set_width(layout, -1);
	pango_layout_get_pixel_extents(layout, NULL, &rect);

	xoffset = 0;
	yoffset = 0;
	if (cell_area != NULL) {
		gtk_cell_renderer_get_alignment(GTK_CELL_RENDERER(c), &xalign, &yalign);
		xoffset = cell_area->width - (2 * xpad + rect.width);
		// use explicit casts just to be safe
		if (gtk_widget_get_direction(widget) == GTK_TEXT_DIR_RTL)
			xoffset = ((gdouble) xoffset) * (1.0 - xalign);
		else
			xoffset *= ((gdouble) xoffset) * xalign;
		yoffset = yalign * (cell_area->height - (2 * ypad + rect.height));
		yoffset = MAX(yoffset, 0);
	}

	aligned_area->x = cell_area->x + xoffset;
	aligned_area->y = cell_area->y + yoffset;
	aligned_area->width = 2 * xpad + rect.width;
	aligned_area->height = 2 * ypad + rect.height;

	g_object_unref(layout);
	gtk_style_context_restore(context);
}

// this is based on both what GtkCellRendererText does and what GtkCellRendererToggle does
static void cellRendererButton_render(GtkCellRenderer *r, cairo_t *cr, GtkWidget *widget, const GdkRectangle *background_area, const GdkRectangle *cell_area, GtkCellRendererState flags)
{
	cellRendererButton *c = cellRendererButton(r);
	gint xpad, ypad;
	GdkRectangle alignedArea;
	gint xoffset, yoffset;
	GtkStyleContext *context;
	PangoLayout *layout;
	PangoRectangle rect;

	gtk_cell_renderer_get_padding(GTK_CELL_RENDERER(c), &xpad, &ypad);
	gtk_cell_renderer_get_aligned_area(GTK_CELL_RENDERER(c), widget, flags, cell_area, &alignedArea);
	xoffset = alignedArea.x - cell_area->x;
	yoffset = alignedArea.y - cell_area->y;

	context = setButtonStyle(widget);
	layout = gtk_widget_create_pango_layout(widget, c->text);

	gtk_render_background(context, cr,
		background_area->x + xoffset + xpad,
		background_area->y + yoffset + ypad,
		background_area->width - 2 * xpad,
		background_area->height - 2 * ypad);
	gtk_render_frame(context, cr,
		background_area->x + xoffset + xpad,
		background_area->y + yoffset + ypad,
		background_area->width - 2 * xpad,
		background_area->height - 2 * ypad);

	pango_layout_set_width(layout, -1);
	pango_layout_get_pixel_extents(layout, NULL, &rect);
	xoffset -= rect.x;
	gtk_render_layout(context, cr,
		cell_area->x + xoffset + xpad,
		cell_area->y + yoffset + ypad,
		layout);

	g_object_unref(layout);
	gtk_style_context_restore(context);
}

static gboolean cellRendererButton_activate(GtkCellRenderer *r, GdkEvent *e, GtkWidget *widget, const gchar *path, const GdkRectangle *background_area, const GdkRectangle *cell_area, GtkCellRendererState flags)
{
	// TODO
	return FALSE;
}

static void cellRendererButton_class_init(cellRendererButtonClass *class)
{
	G_OBJECT_CLASS(class)->dispose = cellRendererButton_dispose;
	G_OBJECT_CLASS(class)->finalize = cellRendererButton_finalize;
	GTK_CELL_RENDERER_CLASS(class)->get_request_mode = cellRendererButton_get_request_mode;
	GTK_CELL_RENDERER_CLASS(class)->get_preferred_width = cellRendererButton_get_preferred_width;
	GTK_CELL_RENDERER_CLASS(class)->get_preferred_height_for_width = cellRendererButton_get_preferred_height_for_width;
	GTK_CELL_RENDERER_CLASS(class)->get_preferred_height = cellRendererButton_get_preferred_height;
	// don't provide a get_preferred_width_for_height()
	GTK_CELL_RENDERER_CLASS(class)->get_aligned_area = cellRendererButton_get_aligned_area;
	// don't provide a get_size()
	GTK_CELL_RENDERER_CLASS(class)->render = cellRendererButton_render;
	GTK_CELL_RENDERER_CLASS(class)->activate = cellRendererButton_activate;
	// don't provide a start_editing()
}

GtkCellRenderer *newCellRendererButton(void)
{
	return GTK_CELL_RENDERER(g_object_new(cellRendererButtonType, NULL));
}
